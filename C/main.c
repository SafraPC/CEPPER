#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

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

    //database connection stabilized
    MYSQL *conn;
    //init mysql connection
    conn = mysql_init(NULL);

    typedef struct
    {
        char *name; //define name of Logged user
    } LoggedUser;

    LoggedUser loggedUser;

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
    //clean bashs alive
    void clearScreen()
    {
        printf("\e[1;1H\e[2J");
    }

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
            clearScreen();
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
                setTimeout(3500);
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

    //search login inside database
    bool searchLoginInDatabase(char *login, char *password)
    {
        //getting user from table users
        char *query = "select name from users where email = '";
        char selectQuery[100];
        char ConcactQueryAux[100];
        char ConcactQueryAuxForCPF[200];
        char ConcactQueryAuxForCPF2[200];
        char ConcactQueryAuxAux[200];
        char ConcactQueryAuxAuxAux[200];
        char ConcactQueryAuxAuxAuxAux[200];
        char ConcactQueryAuxAuxAuxAuxAux[200];
        //concact string to a right mysql query
        strcat(strcpy(selectQuery, login), "' or cpf = '");
        strcat(strcpy(ConcactQueryAux, query), selectQuery);
        strcat(strcpy(ConcactQueryAuxForCPF, ConcactQueryAux), login);
        strcat(strcpy(ConcactQueryAuxForCPF2, ConcactQueryAuxForCPF), "' ");
        strcat(strcpy(ConcactQueryAuxAux, ConcactQueryAuxForCPF2), "and pass = '");
        strcat(strcpy(ConcactQueryAuxAuxAux, ConcactQueryAuxAux), password);
        strcat(strcpy(ConcactQueryAuxAuxAuxAux, ConcactQueryAuxAuxAux), "'");

        if (mysql_query(conn, ConcactQueryAuxAuxAuxAux))
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
        char *user[0];
        //NULL the result for right validation
        user[0] = NULL;
        //while we have rows to be read, write it in the char array
        while ((row = mysql_fetch_row(result)))
        {
            for (int i = 0; i < num_fields; i++)
            {
                user[i] = row[i] ? row[i] : "-";
            }
        }
        //if we really found the USER in database
        if (user[0])
        {
            //USER FOUNDED!
            clearScreen();
            loggedUser.name = user[0];
            printf("Bem-vindo, %s", loggedUser.name);
            return true;
        }
        else
        {
            //return false if we dont found nothing in database about that USER
            return false;
        }
    }

    //sistem interface for login
    bool makeLogin(bool pass)
    {
        if (!pass)
        {
            clearScreen();
            printf("Bem vindo ao Sistema de Login! por favor preencha os campos abaixo...\n");
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
        if (strlen(password) < 2)
        {
            printf("Senha inválida, tente novamente!");
            makeLogin(true);
            return false;
        }
        if (searchLoginInDatabase(login, password))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    //method for verify if the arrays are equals
    bool isDiff(char *array, char *validationArray)
    {
        const int arrayLength = strlen(array);
        const int arrayValLength = strlen(validationArray);

        bool error = false;
        for (int i = 0; i < arrayLength; i++)
        {
            if (error)
            {
                break;
            }
            for (int j = 0; j < arrayValLength; j++)
            {
                if (array[i] == validationArray[j])
                {
                    break;
                }
                else if (j + 1 >= arrayValLength)
                {
                    error = true;
                    break;
                }
            }
        }
        return error;
    }

    //Interface for user navigate in login and register.
    bool initInterface(bool pass)
    {

        //sistem register in database
        bool makeRegister(bool pass)
        {
            if (!pass)
            {
                clearScreen();
                printf("Bem-vindo ao sistema de Cadastro! preencha os campos abaixo para continuar ...\n");
            }

            //items to validate
            /*
        !name
        !born
        !cpf
        !email
        !pass
        !rePass
        */
            char name[200];
            char born[20];
            char cpf[20];
            char email[200];
            char password[100];
            char rePassword[100];

            //array for validations
            char *validLetters = "abcdefghijklmnopqrstuvwxyz";
            char *validEmail = "abcdefghijklmnopqrstuvwxyz@.123456789";
            char *validNumbers = "0123456789";
            char *validDate = "0123456789/";

            printf("\nInsira seu primeiro nome (sem espaços): ");
            scanf("%s", &name);
            char *auxName = name;
            for (int i = 0; i < strlen(auxName); i++)
            {
                auxName[i] = tolower(auxName[i]);
            }
            bool difName = isDiff(name, validLetters);
            if (difName)
            {
                printf("\n Nomes apenas podem conter letras! insira um nome válido, sem espaço. \n");
                makeRegister(true);
                return false;
            }
            printf("\nPerfeito! agora insira sua data de Nasc. (DD/MM/YYYY) Respectivamente.");
            scanf("%s", born);
            if (born[2] == '/' && born[5] == '/')
            {
                bool difDate = isDiff(born, validDate);
                if (difDate)
                {
                    printf("\nInsira uma data válida! padrão (DD/MM/YYYY) \n");
                    makeRegister(true);
                    return false;
                }
            }
            else
            {
                printf("\nInsira uma data válida com barras! padrão (DD/MM/YYYY) \n");
                makeRegister(true);
                return false;
            }
            printf("\nÓtimo! agora, por favor, insira seu CPF, (apenas números): ");
            scanf("%s", &cpf);
            if (strlen(cpf) != 11)
            {
                printf("\nATENÇÃO! um CPF contém apenas 11 digitos.");
                makeRegister(true);
                return false;
            }
            bool cpfDif = isDiff(cpf, validNumbers);
            if (cpfDif)
            {
                printf("ATENÇÃO! CPF, deve conter apenas numeros");
                makeRegister(true);
                return false;
            }
            printf("\nIncrível! agora, por favor, insira seu Email: ");
            scanf("%s", &email);
            for (int i = 0; i < strlen(email); i++)
            {
                email[i] = tolower(email[i]);
            }
            bool emailDif = isDiff(email, validEmail);
            if (emailDif)
            {
                printf("\nATENÇÃO! Emails não contém carácteres especiais além do @!");
                makeRegister(true);
                return false;
            }
            printf("\nPara finalizar, insira sua senha de login: ");
            scanf("%s", &password);
            printf("\nConfirme sua senha: ");
            scanf("%s", &rePassword);
            for (int i = 0; i < strlen(password); i++)
            {
                if (password[i] != rePassword[i])
                {
                    printf("\nATENÇÃO! As senhas são divergentes!");
                    makeRegister(true);
                    return false;
                }
            }
            for (int i = 0; i < strlen(rePassword); i++)
            {
                if (password[i] != rePassword[i])
                {
                    printf("\nATENÇÃO! As senhas são divergentes!");
                    makeRegister(true);
                    return false;
                }
            }
            int confirm = false;
            clearScreen();
            printf("\n\nSeus dados:\nNome: %s\nData de nascimento: %s\nCPF: %s\nEmail: %s\nSenha: %s\n\nConfirma?\n-Pressione 0 para confirmar\n-Pressione 1 para cancelar\n\n", name, born, cpf, email, password);
            scanf("%i", &confirm);
            if (confirm == 0)
            {
                /*
                !req model :
                ?insert into users(name,born,cpf,email,pass) values('Teste','06/10/2002','42413069800','leandrosafra@gmail.com','123');
                */

                char *query = "insert into users(name,born,cpf,email,pass) values('";
                char selectQuery[100];
                char ConcactQueryAux[300];
                char ConcactQueryAuxAux[300];
                char ConcactQueryAuxAuxAux[300];
                char ConcactQueryAuxAuxAuxAux[300];
                char ConcactQueryAuxAuxAuxAuxAux[400];
                char ConcactQueryAuxAuxAuxAuxAuxAux[500];
                char ConcactQueryAuxAuxAuxAuxAuxAuxAux[500];
                char ConcactQueryAuxAuxAuxAuxAuxAuxAuxAux[600];
                char ConcactQueryAuxAuxAuxAuxAuxAuxAuxAuxAux[600];

                //concact string to a right mysql query
                strcat(strcpy(selectQuery, query), name);
                strcat(strcpy(ConcactQueryAux, selectQuery), "','");
                strcat(strcpy(ConcactQueryAuxAux, ConcactQueryAux), born);
                strcat(strcpy(ConcactQueryAuxAuxAux, ConcactQueryAuxAux), "','");
                strcat(strcpy(ConcactQueryAuxAuxAuxAux, ConcactQueryAuxAuxAux), cpf);
                strcat(strcpy(ConcactQueryAuxAuxAuxAuxAux, ConcactQueryAuxAuxAuxAux), "','");
                strcat(strcpy(ConcactQueryAuxAuxAuxAuxAuxAux, ConcactQueryAuxAuxAuxAuxAux), email);
                strcat(strcpy(ConcactQueryAuxAuxAuxAuxAuxAuxAux, ConcactQueryAuxAuxAuxAuxAuxAux), "','");
                strcat(strcpy(ConcactQueryAuxAuxAuxAuxAuxAuxAuxAux, ConcactQueryAuxAuxAuxAuxAuxAuxAux), password);
                strcat(strcpy(ConcactQueryAuxAuxAuxAuxAuxAuxAuxAuxAux, ConcactQueryAuxAuxAuxAuxAuxAuxAuxAux), "');");
                if (mysql_query(conn, ConcactQueryAuxAuxAuxAuxAuxAuxAuxAuxAux))
                {
                    //if we got an error, return nothing.
                    return false;
                }
                if (searchLoginInDatabase(email, password))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                clearScreen();
                initInterface(true);
                return false;
            }
        }

        //login interface method
        bool loginInterface()
        {
            int accept = NULL;
            printf("\nDeseja realmente prosseguir para a tela de login?\n-Pressione 0 para prosseguir\n-Pressione 1 para voltar a tela de início\n");
            scanf("%i", &accept);
            if (accept == 0)
            {
                if (makeLogin(false))
                {
                    return true;
                }
                else
                {
                    printf("\nCredenciais erradas! por favor, tente novamente.\n");
                    initInterface(true);
                    return false;
                }
            }
            else if (accept == 1)
            {
                initInterface(true);
                return false;
            }
            else
            {
                printf("\nPor favor, insira um item válido.. :");
                loginInterface();
                return false;
            }
        }

        //sistem register interface method
        bool registerInterface()
        {
            clearScreen();
            int accept = NULL;
            printf("\nDeseja realmente prosseguir para a tela de cadastro?\n-Pressione 0 para prosseguir\n-Pressione 1 para voltar a tela de início\n");
            scanf("%i", &accept);
            if (accept == 0)
            {
                if (makeRegister(false))
                {
                    return true;
                }
                else
                {
                    printf("\nAlgo deu errado..\n");
                    initInterface(true);
                    return false;
                }
            }
            else if (accept == 1)
            {
                initInterface(true);
                return false;
            }
            else
            {
                printf("\nPor favor, pressione uma opção válida.. :");
                registerInterface();
                return false;
            }
        }

        if (!pass)
        {
            clearScreen();
            printf("\nSeja bem-vindo ao CEPPER! seu pesquisador de CEPS particular.");
            printf("\npor-favor, faça seu login ou se cadastre para continuar.");
        }
        printf("\n- Pressione 0 para realizar seu Login\n- Pressione 1 para realizar um Cadastro\n\n");
        int pass1_num_pressed = 2;
        scanf("%i", &pass1_num_pressed);
        if (pass1_num_pressed == 0)
        {
            if (loginInterface())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (pass1_num_pressed == 1)
        {
            if (registerInterface())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            printf("Por favor, selecione um número válido!");
            initInterface(true);
            return false;
        }
    }
    //App program to use the function created above

    void loggedApp(bool pass)
    {

        if (!pass)
        {
            clearScreen();
            printf("Olá, %s!\nFicamos muito felizes por acessar os serviços CEPPER.\n", loggedUser.name);
        }
        printf("\n\n-Pressione 0 para pesquisar um CEP\n-Pressione 1 para sair\n\n");
        int pressed = NULL;
        scanf("%i", &pressed);
        if (pressed == 0)
        {
            char *userCEP[20];
            printf("\n\nInsira o CEP que deseja buscar\n");
            printf("CEP:");
            scanf("%s", &userCEP);
            getCEP(userCEP);
            loggedApp(true);
        }
        else if (pressed == 1)
        {
            exit(1);
        }
        else
        {
            loggedApp(true);
        }
    }

    bool app()
    {
        if (initInterface(false))
        {
            loggedApp(false);
        }
    }

    app();
    return 0;
}
