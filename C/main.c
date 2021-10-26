#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <time.h>
#include <string.h>

int main()
{
    const char *host = "127.0.0.1";
    const char *user = "root";
    const char *password = "admin";
    const char *database = "cepper";
    const int port = 3306;
    static char *unix_socket = NULL;
    const int flag = 0;

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
        fprintf(stderr, "Error %s [%d]\n", mysql_error(conn), mysql_errno(conn));
    }
    //If we arrived here, the connection is ok and we can make mysql querys!
    printf("Banco de dados contectado..");

    //!Below we have some methods who the program is using

    //timeout for C wait a little (special for request)
    void setTimeout(int milliSec)
    {
        if (milliSec <= 0)
        {
            return;
        }
        int sinceMilli = clock() * 1000 / CLOCKS_PER_SEC;
        int clockEnd = sinceMilli + milliSec;
        do
        {
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

    bool searchCEP(char *searchQuery, char *cep)
    {
        if (mysql_query(conn, searchQuery))
        {
            return false;
        }
        MYSQL_RES *result = mysql_store_result(conn);
        int num_fields = mysql_num_fields(result);
        MYSQL_ROW row;
        char *cepInfos[10];
        cepInfos[1] = NULL;
        cepInfos[3] = NULL;
        while ((row = mysql_fetch_row(result)))
        {
            for (int i = 0; i < num_fields; i++)
            {
                cepInfos[i] = row[i] ? row[i] : "-";
            }
        }
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
            return false;
        }
    }

    bool getCEP(char *cep)
    {
        printf("\n\nAguarde um pouco.. estamos procurando seu CEP");
        //getting cep from table ceps
        const char *query = "select * from ceps where cep = '";
        char selectQueryAux[100];
        char selectQuery[100];
        strcat(strcpy(selectQuery, cep), "'");
        strcat(strcpy(selectQueryAux, query), selectQuery);

        if (searchCEP(selectQueryAux, cep))
        {
            return true;
        }
        else
        {
            const char *insertQueryString = "insert into tbquery(query) values('";
            char insertQueryAux[100];
            char insertQuery[100];
            strcat(strcpy(insertQueryAux, insertQueryString), cep);
            strcat(strcpy(insertQuery, insertQueryAux), "')");
            if (mysql_query(conn, insertQuery))
            {
                printf("\n\nHouve um erro com o banco.");
                return false;
            }

            for (int i = 0; i < 14; i++)
            {
                if (verifyError())
                {
                    printf("\n\nPor favor, insira um CEP válido.");
                    return false;
                }
                setTimeout(3000);
                if (searchCEP(selectQueryAux, cep))
                {
                    return true;
                }
                else
                {
                    if(i+1 >=3){
                        printf("\n\nInfelizmente não conseguimos localizar seu CEP.");
                        return false;
                    }
                }
            }
            return false;
        }
    }
    char *userCEP[20];

    bool app(){
    printf("\n\nInsira o CEP que deseja buscar\n");
    printf("CEP:");
    scanf(" %s",&userCEP);
    getCEP(userCEP);
    }
    while(true){
    app();
    }
    return 0;
}
