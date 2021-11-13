#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <time.h>
#include <string.h>

int main()
{
    //database host ip
    const char *host = "127.0.0.1";
    //database username
    const char *user = "root";
    //database user password
    const char *password = "admin";
    //database table to in
    const char *database = "cepper";
    //database export port
    const int port = 3306;
    //LINUX socket to database
    static char *unix_socket = NULL;
    //database flag
    const int flag = 0;

    //user name for interactive in program

    char *userName[100];

    //database connection stabilized
    MYSQL *conn;
    //init mysql connection
    conn = mysql_init(NULL);

    //verify if connection its nul for exit program
    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    //connect to database specified
    if (!(mysql_real_connect(conn, host, user, password, database, port, unix_socket, flag)))
    {
        //show database error.
        fprintf(stderr, "Error %s [%d]\n", mysql_error(conn), mysql_errno(conn));
    }
    //If we arrived here, the connection is ok and we can make mysql querys!

    //!Below we have some methods that the program is using

    //timeout for C wait a little (special for request)
    void setTimeout(int milliSec)
    {
        //verifying if millisec is negative to stop it
        if (milliSec <= 0)
        {
            return;
        }
        //create a clock in 1000 miliSec and /to clock_per_sec time method
        int sinceMilli = clock() * 1000 / CLOCKS_PER_SEC;
        //create clockeEnd miliSec to wait
        int clockEnd = sinceMilli + milliSec;
        do
        {
            //wait when clock have the time
            sinceMilli = clock() * 1000 / CLOCKS_PER_SEC;
        } while (sinceMilli <= clockEnd);
    }

    //Verify if exist some error in tberror, if exists then CEP Query failed.
    bool verifyError()
    {
        //Making query..
        if (mysql_query(conn, "select error from tberror"))
        {
            //mysql_query returns 1 if returns an error in mysql query.
            return true;
        }
        MYSQL_RES *result = mysql_store_result(conn);
        //freedom for result!
        int num_fields = mysql_num_fields(result);

        MYSQL_ROW row;
        char *errors[1];
        errors[0] = false;
        while ((row = mysql_fetch_row(result)))
        {
            for (int i = 0; i < num_fields; i++)
            {
                errors[i] = row[i] ? row[i] : "-";
            }
        }
        if (errors[0])
        {
            mysql_query(conn, "truncate table tberror");
            return true;
        }
        else
        {
            return false;
        }
    }

    //Searching in database for see if we have the CEP data
    bool searchCEP(char *searchQuery, char *cep)
    {
        //create mysqlQuery
        if (mysql_query(conn, searchQuery))
        {
            //if we got an error, return nothing.
            return false;
        }
        //storing result
        MYSQL_RES *result = mysql_store_result(conn);
        //getting num of fields like matriz [10]
        int num_fields = mysql_num_fields(result);
        //dispatch the rows
        MYSQL_ROW row;
        //create an char array to store the result
        char *cepInfos[10];
        //NULL the result for right validation
        cepInfos[1] = NULL;
        cepInfos[3] = NULL;
        //while we have rows to be read, write it in the char array
        while ((row = mysql_fetch_row(result)))
        {
            for (int i = 0; i < num_fields; i++)
            {
                cepInfos[i] = row[i] ? row[i] : "-";
            }
        }
        //if we really found the CEP in database
        if (cepInfos[1] && cepInfos[3])
        {
            //CEP FOUNDED!
            printf("\n\nInformações do CEP : %s\n", cep);
            printf("\nCEP:  %s", cepInfos[1]);
            printf("\nLOGRADOURO:  %s ", cepInfos[2]);
            printf("\nBAIRRO:  %s", cepInfos[3]);
            printf("\nLOCALIDADE:  %s", cepInfos[4]);
            printf("\nUF:  %s", cepInfos[5]);
            printf("\nIBGE:  %s", cepInfos[6]);
            printf("\nGIA:  %s", cepInfos[7]);
            printf("\nDDD:  %s", cepInfos[8]);
            printf("\nSIAFI:  %s", cepInfos[9]);
            return true;
        }
        else
        {
            //return false if we dont found nothing in database about that CEP
            return false;
        }
    }

    //get the cep from database or search CEP in viaCEP external service
    bool getCEP(char *cep)
    {
        printf("\n\nAguarde um pouco.. estamos procurando seu CEP");
        //getting cep from table ceps
        const char *query = "select * from ceps where cep = '";
        char selectQueryAux[100];
        char selectQuery[100];
        //concact string to a right mysql query
        strcat(strcpy(selectQuery, cep), "'");
        strcat(strcpy(selectQueryAux, query), selectQuery);

        // if search CEP found our CEP, it will return the CEP in a log
        //so just return true.
        if (searchCEP(selectQueryAux, cep))
        {
            return true;
        }
        //if the CEP didnt found, we need to Sercht it in the
        //external Services for found CEP
        else
        {
            //create concact cep query to our mysql
            const char *insertQueryString = "insert into tbquery(query) values('";
            char insertQueryAux[100];
            char insertQuery[100];
            //concact with strcat
            strcat(strcpy(insertQueryAux, insertQueryString), cep);
            strcat(strcpy(insertQuery, insertQueryAux), "')");
            //craate mysql query
            if (mysql_query(conn, insertQuery))
            {
                printf("\n\nHouve um erro com o banco.");
                return false;
            }
            //most important part of our program
            //FAIL OVER to get our CEP data
            //how the request is Async, we need to wait a little
            // so we have 14 tryes and some seconds to wait.
            for (int i = 0; i < 14; i++)
            {
                //verify if we got errors in our JS server
                if (verifyError())
                {
                    printf("\n\nPor favor, insira um CEP válido.");
                    return false;
                }
                setTimeout(3000);
                //Searching CEP in our MySQL table
                if (searchCEP(selectQueryAux, cep))
                {
                    return true;
                }
                else
                {
                    //Failver reached the limit.. sorry.
                    if (i + 1 >= 3)
                    {
                        printf("\n\nInfelizmente não conseguimos localizar seu CEP.");
                        return false;
                    }
                }
            }
            //some error ocurred.
            return false;
        }
    }

    bool searchLoginInDatabase(char *login, char*password){
        //getting user from table users
        char *query = "select name from users where email = '";
        char selectQuery[100];
        char selectQueryAux[100];
        char selectQueryAuxForCPF[200];
        char selectQueryAuxForCPF2[200];
        char selectQueryAuxAux[200];
        char selectQueryAuxAuxAux[200];
        char selectQueryAuxAuxAuxAux[200];
        char selectQueryAuxAuxAuxAuxAux[200];
        //concact string to a right mysql query
        strcat(strcpy(selectQuery, login), "' or cpf = '");
        strcat(strcpy(selectQueryAux, query), selectQuery);
        strcat(strcpy(selectQueryAuxForCPF, selectQueryAux), login);
        strcat(strcpy(selectQueryAuxForCPF2, selectQueryAuxForCPF), "' ");
        strcat(strcpy(selectQueryAuxAux,selectQueryAuxForCPF2), "and pass = '");
        strcat(strcpy(selectQueryAuxAuxAux,selectQueryAuxAux), password);
        strcat(strcpy(selectQueryAuxAuxAuxAux,selectQueryAuxAuxAux),"'");

        printf("%s", selectQueryAuxAuxAuxAux);
        return true;

        
    }

    bool makeLogin(bool pass){
        if(!pass){
        printf("\nBem vindo ao Sistema de Login! por favor preencha os campos abaixo...\n");
        }
        char *login[50];
        char *password[50];
        printf("\nInsira seu Email ou CPF: ");
        scanf("%s", &login);
        if (strlen(login) < 10)
        {
            printf("Insira seu CPF ou Email corretamente!");
            makeLogin(true);
            return false;
        }
        printf("\nInsira sua senha:");
        scanf(" %s", &password);
        if(strlen(password)<2){
            printf("Erro na autenticação, preencha os campos novamente!");
            makeLogin(true);
            return false;
        }
        if(searchLoginInDatabase(login,password)){
            return true;
        }else{
            return false;
        }
    }

    //Interface for user navigate in login and register.
    void initInterface(bool pass)
    {
        //login interface method
        void loginInterface()
        {
            int accept = NULL;
            printf("\nDeseja realmente prosseguir para a tela de login?\n-Pressione 0 para prosseguir\n-Pressione 1 para voltar a tela de início\n");
            scanf("%i", &accept);
            if (accept == 0)
            {
                if(makeLogin(false)){

                }
            }
            else if (accept == 1)
            {
                initInterface(true);
            }
            else
            {
                printf("\nPor favor, insira um item válido.. :");
                loginInterface();
            }
        }

        void registerInterface()
        {
            int accept = NULL;
            printf("\nDeseja realmente prosseguir para a tela de cadastro?\n-Pressione 0 para prosseguir\n-Pressione 1 para voltar a tela de início\n");
            scanf("%i", &accept);
            if (accept == 0)
            {



            }
            else if (accept == 1)
            {
                initInterface(true);
            }
            else
            {
                printf("\nPor favor, insira um item válido.. :");
                registerInterface();
            }
        }

        if (!pass)
        {
            printf("Seja bem-vindo ao CEPPER! seu pesquisador de CEPS particular.");
            printf("\npor-favor, faça seu login ou se cadastre para continuar.");
        }
        printf("\n- Pressione 0 para realizar seu Login\n- Pressione 1 para realizar um Cadastro\n\n");
        int pass1_num_pressed = 2;
        scanf("%i", &pass1_num_pressed);
        if (pass1_num_pressed == 0)
        {
            loginInterface();
        }
        else if (pass1_num_pressed == 1)
        {
            registerInterface();
        }
        else
        {
            printf("Por favor, selecione um número válido!");
            initInterface(true);
        }
    }
    //App program to use the function created above

    bool app()
    {
        initInterface(false);
        // char *userCEP[20];
        // printf("\n\nInsira o CEP que deseja buscar\n");
        // printf("CEP:");
        // scanf(" %s",&userCEP);
        // getCEP(userCEP);
    }
    app();
    // while(true){
    // app();
    // }
    return 0;
}
