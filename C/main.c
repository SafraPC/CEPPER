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
    printf("Database contectado..\n\n");

    //!Below we have some methods who the program is using

    //method for make sql query
    bool cepperQuery(char *query)
    {
        //Making query..
        if (mysql_query(conn, query))
        {
            //mysql_query returns 1 if returns an error in mysql query.
            return false;
        }
        //Store query result
        MYSQL_RES *result = mysql_store_result(conn);
        int num_fields = mysql_num_fields(result);
        printf("Num fields :  %i\n", num_fields);
        //Get row for show data.
        MYSQL_ROW row;
        //feching rows in while, show data
        while ((row = mysql_fetch_row(result)))
        {
            //while i its less than num_fields, show for us.
            for (int i = 0; i < num_fields; i++)
            {
                printf("%s  line : %i\n", row[i] ? row[i] : "NULL", i);
            }

            printf("\n");
        }
        //freedom for result!
        mysql_free_result(result);
        //Closing connection...
        mysql_close(conn);
        return true;
    }

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

    //failover if database it's over
    bool failOverQuery(char *query, int thetries)
    {
        int tries = 0;
        if (thetries > 0)
        {
            tries = thetries;
        }
        if (tries <= 7)
        {
            if (!cepperQuery(query))
            {
                tries = tries + 1;
                setTimeout(1000);
                failOverQuery(query, tries);
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
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
            printf("\n\nerror : %s", errors[0]);
            mysql_query(conn, "truncate table tberror");
            mysql_close(conn);
            mysql_free_result(result);
            return true;           
        }
        else
        {
            printf("\n\n erro: %s",errors[0]);
            mysql_close(conn);
            mysql_free_result(result);
            return false;
        }
    }

    bool getCEP(char *cep)
    {
        //getting cep from table ceps
        const char *query = "select * from ceps where cep = '";
        char selectQueryAux[100];
        char selectQuery[100];
        strcat(strcpy(selectQuery, cep), "'");
        strcat(strcpy(selectQueryAux, query), selectQuery);
        if (mysql_query(conn, selectQueryAux))
        {
            return false;
        }
        MYSQL_RES *result = mysql_store_result(conn);
        int num_fields = mysql_num_fields(result);

        MYSQL_ROW row;
        char *cepInfos[10];
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
            printf("Informações do CEP : %s\n", cep);
            printf("\nCEP:  %s", cepInfos[1]);
            printf("\nLOGRADOURO:  %s ", cepInfos[2]);
            printf("\nBAIRRO:  %s", cepInfos[3]);
            printf("\nLOCALIDADE:  %s", cepInfos[4]);
            printf("\nUF:  %s", cepInfos[5]);
            printf("\nIBGE:  %s", cepInfos[6]);
            printf("\nGIA:  %s", cepInfos[7]);
            printf("\nDDD:  %s", cepInfos[8]);
            printf("\nSIAFI:  %s", cepInfos[9]);

            mysql_free_result(result);
            mysql_close(conn);
            return true;
        }
        else
        {
            //Probably CEP isnt registred in table, so, we need to get it.
            //contact query
            const char *insertQueryString = "insert into tbquery(query) values('";
            char insertQueryAux[100];
            char insertQuery[100];
            strcat(strcpy(insertQueryAux, insertQueryString), cep);
            strcat(strcpy(insertQuery, insertQueryAux), "')");
            printf("%s", insertQuery);
            if (mysql_query(conn, insertQuery))
            {
                printf("\n\nHouve um erro com o banco.");
                return false;
            }
            //WE NEED TO CREATE A LISTENING IN TWO TABLES, TO SEE ERRORS AND RESULTS.
            setTimeout(1500);
            //verify errors

            return false;
        }
    }
    
    getCEP("13057083");
    
    return 0;
}
