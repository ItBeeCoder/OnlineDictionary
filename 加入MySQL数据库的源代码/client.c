#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define N 256
#define NAME_S 20
#define TEXT_S 256

typedef struct{
	char type;
	char name[NAME_S];
	char text[TEXT_S];
}MSG;

MSG buf;

void p_tips(int fd);
void p_regist(int fd);
void p_login(int fd);
void p_quit(int fd);
void p_history(int fd, char name[NAME_S]);
void p_query(int fd, char name[NAME_S]);

int main(int argc, char *argv[])//./client
{
    int sockfd , a;
    //
    struct sockaddr_in servaddr, myaddr;
   	char clean[N];

   
/*创建客户端套接字，面向连接通信，IPV4协议*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(-1);
    }
//数据初始化，全部清零
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;//
    servaddr.sin_addr.s_addr=inet_addr("10.13.33.108");//该ip地址为运行服务器端程序的主机IP地址，可在终端通过ifconfig命令查询
    servaddr.sin_port=htons(8000); //客户端与服务端通过8000端口通信，也可以改为其他端口
    /*struct   sockaddr_in   {
                 short   int   sin_family;     //2
                 unsigned   short   int   sin_port;     //2
                 struct   in_addr   sin_addr;     //4
                 unsigned   char   sin_zero[8];     //8
         };
    */

  /*將套接字绑定到服务器的网络地址上*/
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect");
        exit(-1);
    }

	while(1){
		printf("\n############ 欢迎使用 #################\n\n");
		printf("#######################################\n");
		printf("# 1.注册  2.登录  3.退出   4.系统功能 #\n");
		printf("#######################################\n");
		printf("请输入你的选择:");//int a;
		if(scanf("%d", &a) == 0)
		{//	char clean[N];
			scanf("%s", clean);
			printf("input error!!\n\n");
			continue;
		}

		getchar();

		switch(a){
			case 1:
				p_regist(sockfd);//注册函数
				break;
			case 2:
				p_login(sockfd);//登录函数
				break;
			case 3:
				p_quit(sockfd);//退出函数
				break;
			case 4:
				p_tips(sockfd);
				break;
			default:
				break;
		}
	}

    return 0;
}

void p_tips(int fd){
printf("\n欢迎使用基于Linux的网络词典，本系统提供的功能有：\n\n");
printf("1.新用户注册，若系统数据库中已经存在同名的用户，则注册失败;\n");
printf("2.用户登录，若用户输入的用户名或密码错误，则登录失败，系统显示登录失败原因;\n");
printf("3.单词查询功能，用户输入要查询的单词，系统根据用户输入查询词库，完成查询单词含义的功能;\n");
printf("4.查询历史显示，每个用户的查询历史存放在数据库中，用户查阅时，系统会显示用户查阅单词的具体时间，精确到分钟;\n");
printf("5.退出功能，用户在查询完成后，可以选择退出系统;\n");
printf("6.并发服务器可实现多个远程合法用户同时查询单词。\n");
}
//
void p_login(int fd){//登录函数
	int  a;
	char name[NAME_S];
	char clean[N];


		printf("请输入用户名:");
		fgets(buf.name, NAME_S, stdin);
		buf.name[strlen(buf.name)-1] = '\0';
		strcpy(name, buf.name);
		printf("请输入密码:");
		fgets(buf.text, TEXT_S, stdin);
		buf.text[strlen(buf.text)-1] = '\0';
		buf.type = 'l';

		send(fd, &buf, sizeof(buf), 0);
		memset(&buf, 0, sizeof(buf));
		recv(fd, &buf, sizeof(buf), 0);

		if(buf.type == 'y')//sucess
		{
			while(1)
			{
		printf("\n**************欢迎使用网络词典***********\n");
		printf("\n****************************************\n");
		printf("**** 1.查询单词  2.查询历史  3.退出 ****\n");
		printf("****************************************\n");
		printf("请输入你的选择:");
		if(scanf("%d",&a) == 0)
		{
		scanf("%s", clean);
		printf("输入错误，请核对你的输入!\n\n");
					continue;
				}

				getchar();

				switch(a){
				case 1:
					p_query(fd,name);//
					break;
				case 2:
					p_history(fd,name);//
					break;
				case 3:
					buf.type = 'x';
					send(fd, &buf, sizeof(buf), 0);
					printf("\n");
					return ;
				}
			}
		}
		else{
			printf("%s!!!\n", buf.text);
		}

}
//注册函数，完成用户的注册功能
void p_regist(int fd)
{
	printf("请输入用户名:");
	fgets(buf.name, NAME_S, stdin);
	buf.name[strlen(buf.name)-1] = '\0';
	printf("请输入密码:");
	fgets(buf.text, TEXT_S, stdin);
	buf.text[strlen(buf.text)-1] = '\0';
	buf.type = 'r';

	send(fd, &buf, sizeof(buf), 0);
	memset(&buf, 0, sizeof(buf));
	recv(fd, &buf, sizeof(buf), 0);
 /*recv函数返回接收到的字节数*/
	printf("注册 %s\n", buf.text);
}
//退出系统
void p_quit(int fd)
{
	memset(&buf, 0, sizeof(buf));
	buf.type = 't';
	send(fd, &buf, sizeof(buf), 0);
	close(fd);
	exit(0);
}
//查询函数
void p_query(int fd, char name[NAME_S]){
	while(1){
		memset(&buf, 0, sizeof(buf));
		printf("\n请输入单词(输入 '#' 时退出查询):");
		fgets(buf.text, TEXT_S, stdin);
		buf.text[strlen(buf.text) -1] = '\0';
		if(buf.text[0] == '#' )
			break;
		strcpy(buf.name, name);
		buf.type = 'q';

		send(fd, &buf, sizeof(buf), 0);
		memset(&buf, 0, sizeof(buf));
		recv(fd, &buf, sizeof(buf), 0);
//y为服务端返回给客户端的
		if(buf.type == 'y'){
			printf("%s\n", buf.text);
		}
		else{
			printf("sorry! 词库中不存在该单词，请核对你的输入后重新输入!\n");
		}
	}
}

//处理查询历史的函数
void p_history(int fd, char name[NAME_S])
{
	memset(&buf, 0, sizeof(buf));
	strcpy(buf.name, name);
	buf.type = 'h';

	send(fd, &buf, sizeof(buf), 0);
	while(1)
	{
		memset(&buf, 0, sizeof(buf));
		recv(fd, &buf, sizeof(buf), 0);
		//buf.type = 'e'
		if(buf.type == 'e')
			break;
		printf("%s\n",buf.text);
return;
	}
}

