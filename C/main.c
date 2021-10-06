#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
//try catch buffer
static jmp_buf s_jumpBuffer;
int main()
{
    char *host = "127.0.0.1";
    char *user = "root";
    char *password = "admin";
    char *database = "testedb";
    int port = 3306;
    static char *unix_socket = NULL;
    int flag = 0;

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

    int selectQuery(char *query)
    {
        if (!setjmp(s_jumpBuffer))
        {
            //Making query..
            mysql_query(conn, query);
            //Store query result
            MYSQL_RES *result = mysql_store_result(conn);
            int num_fields = mysql_num_fields(result);
            //Get row for show data.
            MYSQL_ROW row;
            //feching rows in while, show data
            while ((row = mysql_fetch_row(result)))
            {
                //while i its less than num_fields, show for us.
                for (int i = 0; i < num_fields; i++)
                {
                    printf("%s ", row[i] ? row[i] : "NULL");
                }

                printf("\n");
            }
            //freedom for result!
            mysql_free_result(result);
            //Closing connection...
            mysql_close(conn);
        }
        else
        {
        }
    }

    selectQuery("select * from test");
    return 0;
}
