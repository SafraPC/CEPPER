#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <time.h>

//try catch buffer
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
                printf("%s  line : %i\n", row[i] ? row[i] : "NULL",i);
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
        if(thetries > 0){
            tries = thetries;
        }
        if (tries <= 7)
        {
            if (!cepperQuery(query))
            {
                tries = tries + 1;
                setTimeout(1000);
                failOverQuery(query,tries);
            }
            else
            {
                return true;
            }
        }else{
            return false;
        }
    }
    
    if(failOverQuery("select * from usersssss",0)){
        printf("Query executada com sucesso!");
    }else{
        printf("Não foi possível executar a query...");
    }
    return 0;
}