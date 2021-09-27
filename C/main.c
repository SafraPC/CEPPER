#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
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
    if (conn == NULL){
    fprintf(stderr, "%s\n", mysql_error(conn));
    exit(1);
    }
    //connect to database specified
    if(!(mysql_real_connect(conn,host,user,password,database,port,unix_socket,flag))){
        fprintf(stderr,"Error %s [%d]\n",mysql_error(conn),mysql_errno(conn));
        exit(1);
    }
    //If we arrived here, the connection is ok and we can make mysql querys!
    printf("Database connection stabilized\n\n");


    return 0;
}
