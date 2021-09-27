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

    MYSQL *conn;

    conn = mysql_init(NULL);
    if(!(mysql_real_connect(conn,host,user,password,database,port,unix_socket,flag))){
        fprintf(stderr,"Error %s [%d]\n",mysql_error(conn),mysql_errno(conn));
        exit(1);
    }
    printf("DATABASE CONNECTED!!");

    return 0;
}
