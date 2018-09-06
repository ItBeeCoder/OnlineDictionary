#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <mysql/mysql.h>
#include <stdbool.h>


#define NAME_S 20
#define TEXT_S 256
#define N 512
#define HOST "localhost"
#define USERNAME "root"
#define PASSWD "1234"
#define DATABASE "hexj"
#define DATABASESL "my.db"

typedef struct {
	char type;
	char name[NAME_S];
	char text[TEXT_S];
}MSG;



void data(char *);//
char * func(char *s);//从词库中获取单词的解释
void p_login(int fd,MSG buf);//登录处理函数
void p_regist(int fd,MSG buf);//注册函数
void p_query(int fd,MSG buf, sqlite3 *db);//
void p_history(int fd,MSG buf, sqlite3 *db);//

int handler(void *para, int f_num, char ** f_value, char ** f_name );


int main(int argc, char *argv[])//./server
{


 int listenfd, connfd;
    //
    struct sockaddr_in myaddr, peeraddr;
    socklen_t len;
	MSG buf;
	pid_t pid;


	signal(SIGCHLD, SIG_IGN);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(-1);
    }
//  struct sockaddr_in myaddr,
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;

    myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
     myaddr.sin_port=htons(8000); //


    if (bind(listenfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1){
        perror("bind");
        exit(-1);
    }

    if (-1 == listen(listenfd, 5)){
        perror("listen");
        exit(-1);
    }

   	len = sizeof(peeraddr);
    while (1){
	    memset(&peeraddr, 0, sizeof(peeraddr));
   /**/
        if ((connfd = accept(listenfd, (struct sockaddr *)&peeraddr, &len)) == -1)
        {
            perror("accept");
            exit(-1);
        }

		if((pid = fork()) == -1)
		{
			perror("fork");
			exit(-1);
		}
		else if(pid == 0){
			while(1)
			{
				memset(&buf, 0, sizeof(buf));
				recv(connfd, &buf, sizeof(buf), 0);//
				switch(buf.type)
				{//int connfd;
					case 'l': p_login(connfd, buf); break;
					case 'r': p_regist(connfd, buf); break;
					case 't':
							  close(connfd);
							  exit(0);
							  break;
				}
			}
		}

        close(connfd);
    }

    return 0;
}

// connect to mysql database
        
        


//case 'l': p_l(connfd, buf); break;
void p_login(int fd, MSG buf){
printf("p_login start\n");
 MYSQL* connection;
         MYSQL_RES* res_ptr;
         MYSQL_FIELD* field;
        MYSQL_ROW row;
        int res;
	sqlite3 *db;
	char *errmsg, **resultp;
	int nrow, ncolumn;
	char buff[N]={0};
	
	if(sqlite3_open(DATABASESL,&db)!=0){
 	printf("sqlite3_open error:%s\n",sqlite3_errmsg(db));
	exit(-1);
	}

           connection = mysql_init(NULL);   

	

printf("p_login connection start\n");
if(!mysql_real_connect(connection,HOST,USERNAME,PASSWD,DATABASE,0,NULL,0))  {

            perror("mysql connection failed");
            mysql_close(connection);
            //exit(EXIT_FAILURE);
             }

 	// exec sql query
	char  sql1[1024]; 
	sprintf(sql1,"select * from user where name='%s'", buf.name);
	printf("p_login user name= %s\n",buf.name);
   
                    if(mysql_query(connection,sql1) != 0){
                        perror("mysql query failed");
                        //exit(EXIT_FAILURE);
                     }
	bool result=false;
                     // get the query result  MYSQL_RES* res_ptr;
                      if((res_ptr = mysql_store_result(connection)) != NULL) {
                        int col_num = mysql_num_fields(res_ptr);
                      int row_num = mysql_num_rows(res_ptr);
                     
                int i;         
                 for(i = 0;i < row_num;i++){
               row = mysql_fetch_row(res_ptr);
		if(strcmp(row[0],buf.name)==0)
			result=true;
                      }
                   }

	if(!result){
                perror("mysql query failed");
		buf.type = 'n';
		strcpy(buf.text, "用户名错误，请核对你的输入\n");
		send(fd, &buf, sizeof(buf), 0);
                 //exit(EXIT_FAILURE);
		//exit(-1);
                     }else{

	char sql2[1024]; 
	sprintf(sql2,"select name ,password from user where name='%s' and password='%s'", buf.name,buf.text);

                    if(mysql_query(connection,sql2) != 0){
                        perror("mysql query failed");
                       // exit(EXIT_FAILURE);
                     }
	 result=false;
                     // get the query result  MYSQL_RES* res_ptr;
                      if((res_ptr = mysql_store_result(connection)) != NULL) {
                        int col_num = mysql_num_fields(res_ptr);
                      int row_num = mysql_num_rows(res_ptr);
                     
                               int i;
                        printf("rownum=%d\n",row_num);
                          
                 for(i = 0;i < row_num;i++){
               row = mysql_fetch_row(res_ptr);
		if(strcmp(row[0],buf.name)==0&&strcmp(row[1],buf.text)==0)
			result=true;
                      }
                   }

	printf("p_login bool t= %d\n",result);
		  if(!result){
			buf.type = 'n';
			strcpy(buf.text, "密码错误，请核对你的输入");
			send(fd, &buf, sizeof(buf), 0);
                        //exit(-1);
                     }else{
			buf.type = 'y';
			strcpy(buf.text,"ok\n");
			send(fd, &buf, sizeof(buf), 0);
			while(1){
				memset(&buf, 0, sizeof(buf));
				recv(fd, &buf, sizeof(buf), 0);
				switch(buf.type){
				case 'h': p_history(fd, buf, db); break;
				case 'q': p_query(fd, buf, db); break;
				case 'x': sqlite3_close(db); return ;
				}
			}
		}
                    
}
	// exec sql query
	mysql_close(connection); 
}


void p_regist(int fd, MSG buf){
printf("p_regist start\n");
	char buff[N];
	char * errmsg;
	MYSQL* connection;
        MYSQL_RES* res_ptr;
        MYSQL_FIELD* field;
        MYSQL_ROW row;
        int res;
	sqlite3 *db;

	if(sqlite3_open(DATABASESL,&db)!=0){
 	printf("sqlite3_open error:%s\n",sqlite3_errmsg(db));
	exit(-1);
	}

        connection = mysql_init(NULL);  
 
if(!mysql_real_connect(connection,HOST,USERNAME,PASSWD,DATABASE,0,NULL,0))  {

            perror("mysql connection failed");
            mysql_close(connection);
            //exit(EXIT_FAILURE);
             }
printf("regist connection successful\n");
	

char sql3[1024] ;

 sprintf(sql3, "insert into user values('%s' ,'%s')",buf.name, buf.text );
//printf("p_regist user name= %s\n",buf.name);
//printf("p_regist user password= %s\n",buf.text);

 printf("p_regist query start\n");
 res=mysql_query(connection,sql3);
int affactedrows=mysql_affected_rows(connection);
printf("p_regist query affactedrows=%d\n",affactedrows);

//printf("p_regist query res res=%d\n",res);
//printf("p_regist query end \n");
if(affactedrows>0){
	//memset(&buf, 0, sizeof(buf)); 
   	 printf("insert %lu rows\n",(unsigned long)mysql_affected_rows(connection));
	strcpy(buf.text, "成功!\n");
	send(fd, &buf, sizeof(buf), 0);
}else{
mysql_error(connection);
fprintf(stderr,"insert error %d: %s\n",mysql_errno(connection),mysql_error(connection));

	//memset(&buf, 0, sizeof(buf)); 
strcpy(buf.text, "该用户已经存在!!!\n");
send(fd, &buf, sizeof(buf), 0);
}
mysql_close(connection); 
}

void p_history(int fd, MSG buf, sqlite3 *db){
	char *errmsg;
	char buff[N];
	
MYSQL* connection;
        MYSQL_RES* res_ptr;
        MYSQL_FIELD* field;
        MYSQL_ROW row;
        int res;
    connection = mysql_init(NULL);   
	
printf("p_history connection start\n");
if(!mysql_real_connect(connection,HOST,USERNAME,PASSWD,DATABASE,0,NULL,0))  {

            perror("mysql connection failed");
            mysql_close(connection);
            //exit(EXIT_FAILURE);
             }

 	// exec sql query
	char  sql5[1024]; 
	
	

	sprintf(sql5, "select data, word from record where name='%s'", buf.name);
	printf("p_login user name= %s\n",buf.name);
   	printf("p_history sql5= %s\n",sql5);
memset(&buf,0,sizeof(buf));
         if(mysql_query(connection,sql5) != 0){
                        perror("mysql query failed");
                        //exit(EXIT_FAILURE);
memset(&buf,0,sizeof(buf));
buf.type='e';
send(fd, &buf, sizeof(buf), 0);
                     }
	printf("p_history query end\n");
	
                    
                     // get the query result  MYSQL_RES* res_ptr;
                      if((res_ptr = mysql_store_result(connection)) != NULL) {
                        int col_num = mysql_num_fields(res_ptr);
                      int row_num = mysql_num_rows(res_ptr);
                     
                               int i;
                        printf("rownum=%d\n",row_num);
                          // print query result
                 for(i = 0;i < row_num;i++){
               row = mysql_fetch_row(res_ptr);
	int j;
	for(j = 0;j < col_num;j++){
	printf("%s  ",row[j]); 
	strcat(buf.text,row[j]);
	strcat(buf.text,"  ");
	}
	    printf("\n");
strcat(buf.text,"\n");
                      }

                   }
send(fd, &buf, sizeof(buf), 0);
mysql_close(connection); 
return ;
}

void p_query(int fd, MSG buf, sqlite3 *db)
{
	int  n;
	FILE *fp;
	char buff[N] = {};
	MYSQL* connection;
        MYSQL_RES* res_ptr;
        MYSQL_FIELD* field;
        MYSQL_ROW row;
        int res;

 connection = mysql_init(NULL); 
 
if(!mysql_real_connect(connection,HOST,USERNAME,PASSWD,DATABASE,0,NULL,0))  {

            perror("mysql connection failed");
            mysql_close(connection);
            //exit(EXIT_FAILURE);
             }

	if((fp = fopen("dict.txt", "r")) == NULL)//从dixt.txt文件中读取单词，读文件
	{
		perror("open");
		exit(-1);
	}

	while( fgets(buff, N, fp) != NULL){
		char buf_rec[N];
		char *errmsg;
		char data_time[24]={0};

		if((strncmp(buf.text, buff,strlen(buf.text)) == 0) && buff[strlen(buf.text)] == ' '){
			data(data_time);
 
char sql4[1024] ;

 sprintf(sql4, "insert into record values('%s' ,'%s','%s')", data_time, buf.text,buf.name);


 res=mysql_query(connection,sql4);
int affactedrows=mysql_affected_rows(connection);
printf("p_query query affactedrows=%d\n",affactedrows);


	//memset(&buf, 0, sizeof(buf)); 
   	// printf("insert %lu rows\n",(unsigned long)mysql_affected_rows(connection));
	strcpy(buf.text , func(buff));
	buf.type = 'y';
	send(fd,  &buf, sizeof(buf), 0);
	fclose(fp);
			return ;


	}
	}
mysql_close(connection); 
	buf.type = 'n';
	send(fd, &buf, sizeof(buf), 0);
	fclose(fp);
}
int handler(void *para, int f_num, char ** f_value, char ** f_name ){
	int i, fd;
	MSG buf;
	fd = (*(int *)para);
	memset(&buf, 0, sizeof(buf));

	for(i = 0; i < f_num; i ++){
		strcat(buf.text, f_value[i]);
		strcat(buf.text, "  ");
	}
	send(fd, &buf, sizeof(buf), 0);
	return 0;
}
char * func(char *s){
	int i = 0, j = 0;
	char * buff;
	buff = s;

	while(s[i++] != ' ');
	while(s[i++] == ' ');
	i --;
	while(s[i] != '\0')
		buff[j++] = s[i++];

	buff[j] = '\0';
	return buff;
}
void data(char *s)//该函数功能为生成查询时间
{
	time_t now_time;
	struct tm *ts;

	now_time = time(NULL);
	ts = localtime(&now_time);

	sprintf(s, "%4d-%2d-%2d %2d:%2d", ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday, ts->tm_hour, ts->tm_min);
}
