#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
//#include<Windows.h>

#define HTTP_DEF_PORT 80//默认端口
#define BUF_SIZE 1024//缓冲区大小
#define MAX_CONNECT 5	//最大连接数
#define FILE_NAME_LENGTH 256//文件名最大长度
#define PATH_NAME_LENGTH 256//路径名最大长度
#define THREAD_NUM 5//最大线程数
#define SIGNAL_LEN 256//开关信号的最大长度

char *header_pre = (char*)"HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: close\r\n\r\n";//报文头
int thread_in_use = 0;//当前使用的线程数
char path[PATH_NAME_LENGTH] = "D:\\vs_work\\computer_net\\web_server\\web_server" ;//文件路径，默认文件地址
int port = 80;//端口,默认80
char ip[PATH_NAME_LENGTH] = "127.0.0.1" ;
int block_mode;//阻塞模式
char command[SIGNAL_LEN];//开关信号

typedef struct ThreadParameter {
	SOCKET ConnectSocket;//负责通信的套接字
	SOCKADDR_IN addrClient;//地址
	bool state;//该套接字当前状态，占用1，否则0；
	HANDLE thread;
};

ThreadParameter threads[THREAD_NUM];
HANDLE get_command;
void init_config(void)
{
	int choose;
	printf("Please config your web server:\n"
		"	1.config ip\n"
		"	2.config port\n"
		"	3.config file_path\n"
		"	4.config with default\n"
		"	5.help\n"
		"	6.start service\n"
		"	7.quit\n\n");
		while (true)
		{
				printf("Please input your option:");
				scanf("%d", &choose);
				printf("\n");
				switch (choose)
				{
				case 1:
					printf("Please input your ip:");
					scanf("%s", ip);
					printf("ip is set to %s\n\n", ip);
					break;
				case 2:
					printf("Please input the port:");
					scanf("%d", &port);
					printf("port is set to %d\n\n", port);
					break;
				case 3:
					printf("Please input the path:");
					scanf("%d", path);
					printf("path is set to %s\n\n", path);
					break;
				case 4:
					strcpy(path, "D:\\vs_work\\computer_net\\web_server\\web_server");
					strcpy(ip, "127.0.0.1");
					port = 80;
					printf("ip is set to %s\n", ip);
					printf("port is set to %d\n", port);
					printf("path is set to %s\n\n", path);
					printf("\n");
					break;
				case 5:
					printf("You can config the following command while running:\n");
					printf("quit: to shut the service down\n\n");
					break;
				case 6:
					printf("Your ip is %s\n", ip);
					printf("Your port is %d\n", port);
					printf("Your path is %s\n\n", path);
					break;
				case 7:
					strcpy(command, "quit");
					break;
				deafult:
					break;
				}
				if ((choose == 6)||(choose == 7))
					break;
		}
}
void *get_filename(char* buf,int buflen,char* filename)
/*
*	函数功能：截取浏览器响应报文中申请的文件名并返回
*	输入：char* buf	接受响应报文缓冲区
*	输出：char *filename	申请的目标文件
*/
{
	int length = 0,line_length = 0;
	char *begin, *end, *bias,*end_line,*begin_line;
	char command_line[FILE_NAME_LENGTH];

	//printf("ID：%d %s", GetCurrentThreadId(), buf);
	begin = strchr(buf, ' ');
	begin += 1;
	end = strchr(begin, ' ');
	*end = '\0';
	bias = strrchr(begin, '/');
	length = end - bias;
	if ((*bias == '/' || *bias == '\\'))
	{
		bias++;
		length++;
	}
	if (length > 0)
	{
		memcpy(filename, bias, length);
		filename[length] = '\0';
	}


	/*
	end_line = strchr(buf, '\n');
	begin_line = buf;
	line_length = end_line - begin_line;
	memcpy(command_line, begin_line, line_length);
	printf("	ID：%d commandline: %s\n", GetCurrentThreadId(), command_line);
	*/
	return 0;
}


int send_file(SOCKET soc, char *buf, int buflen)
/*
*		函数功能：根据响应报文发送指定文件
*		输入：	SOCKET soc	用于通信的套接字
				char *buf	接收缓冲区
				int buflen	缓冲区大小
*		返回值：	成功1 失败0
*/
{
	char filename[FILE_NAME_LENGTH];
	char header[BUF_SIZE],read_buf[BUF_SIZE];
	char p[FILE_NAME_LENGTH];//用于拼接绝对路径的中间变量
	FILE *file;
	int file_len,header_len,send_len,read_len;
	get_filename(buf, buflen, filename);
	strcpy(p, path);
	strcat(p, "\\");
	strcat(p, filename);
	strcpy(filename, p);
	
	if ((fopen_s(&file, filename, "rb+")) != 0)
	{
		printf("	ID：%d file %s is not find\n", GetCurrentThreadId(),filename);
		return -1;//打开操作不成功
	}
	fseek(file, 0, SEEK_END);
	file_len = ftell(file);
	fseek(file, 0, SEEK_SET);

	/*构造http首部*/
	header_len = sprintf(header, header_pre, file_len);
	if (send_len = send(soc, header, header_len, 0) == SOCKET_ERROR)
	{
		fclose(file);
		printf("	ID：%d file send failed with error: %ld\n", GetCurrentThreadId(), WSAGetLastError());
		return 0;
	}
	do
	{
		read_len = fread(read_buf, sizeof(char), BUF_SIZE, file);
		if (read_len > 0)
		{
			send_len = send(soc, read_buf, read_len, 0);
			file_len -= read_len;
		}
	} while ((read_len > 0) && (file_len > 0));
	fclose(file);
	printf("	ID：%d file %s send succcessfully!\n", GetCurrentThreadId(),filename);
	return 1;
}

DWORD WINAPI control(LPVOID pParam)
//键盘监听线程函数
{
	while (true)
	{
		if(strcmp(command,"quit")!=0)
		printf(">>> ");
		scanf("%s", command);
		//Sleep(100);
	}

}

DWORD WINAPI communicat(LPVOID pParam)
//通信线程函数
{
	int rcv_len, result = 0;
	char rcv_buf[BUF_SIZE];
	ThreadParameter *tp;
	tp = (ThreadParameter *)pParam;

	printf("	ID：%d creat successfully! thread num in use is ：%d\n",
		GetCurrentThreadId(),thread_in_use);
	
	if (tp->ConnectSocket == INVALID_SOCKET)
	{
		InterlockedDecrement((LPLONG)&thread_in_use);//线程使用数减一
		printf("	ID：%d accept failed with error: %ld\n", GetCurrentThreadId(),WSAGetLastError());
		tp->state = 0;
		closesocket(tp->ConnectSocket);
		return -1;
	}
	printf("	ID：%d ip：%s，port：%d has connected\n",
		GetCurrentThreadId(),inet_ntoa(tp->addrClient.sin_addr), ntohs(tp->addrClient.sin_port));


	if (rcv_len = recv(tp->ConnectSocket, rcv_buf, BUF_SIZE, 0) == SOCKET_ERROR)
	{
		InterlockedDecrement((LPLONG)&thread_in_use);//线程使用数减一
		closesocket(tp->ConnectSocket);
		printf("	ID：%d receive failed with error: %ld\n", GetCurrentThreadId(), WSAGetLastError());
		tp->state = 0;
		closesocket(tp->ConnectSocket);
		return -1;
	}
	result = send_file(tp->ConnectSocket, rcv_buf, rcv_len);
	closesocket(tp->ConnectSocket);
	InterlockedDecrement((LPLONG)&thread_in_use);//线程使用数减一
	tp->state = 0;//该线程状态置为可用状态
	return 0;
}
int main()
{
	//初始化
	WSADATA  wsaData;
	SOCKET ListenSocket;//用于负责监听
	SOCKET ConnectSocket;//用于负责连接通信
	sockaddr_in addrServer;
	sockaddr_in addrClient;
	int i;
	int nRc = WSAStartup(0x0101, &wsaData);


	//初始化
	if (nRc)
	{
		//Winsock初始化错误
		return -1;
	}
	if (wsaData.wVersion != 0x0101)
	{
		//版本支持不够
		//报告错误给用户，清除Winsock，返回
		WSACleanup();
		return -1;
	}
	printf("Init successful! Welcome to use!\n\n");
	init_config();//用户设置web服务器
	if (strcmp(command, "quit") != 0)
		printf("Service start successfuly!\n");
	//端口，路径配置：
	//printf("Please input Path config(such as: D:\\vs_work\\computer_net\\web_server\\web_server):\n");
	//scanf("%s", path);
	//printf("Please input Port config(0~65535):\n");
	//scanf("%d",&port);
	//创建监听套接字
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) 
	{
		printf("	socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//监听套接字信息
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
	addrServer.sin_port = htons(port);
	//绑定
	if (bind(ListenSocket, (SOCKADDR *)& addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
		printf("	bind failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//监听
	if (listen(ListenSocket, MAX_CONNECT) == SOCKET_ERROR) {
		printf("	listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//开启屏幕输入监控线程
	get_command = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)control, NULL, 0, NULL);
	int len = sizeof(SOCKADDR);
	//将套接字设置为非阻塞模式
	block_mode = 1;
	ioctlsocket(ListenSocket, FIONBIO, (u_long FAR*) &block_mode);//非阻塞设置
	while (TRUE)
	{
		ConnectSocket = accept(ListenSocket, (SOCKADDR*)&addrClient, &len);	
		//command 处理
		if (strcmp(command, "quit") == 0)
		{
			printf("Services have been shut down");
			break;
		}
		if (ConnectSocket != INVALID_SOCKET)
		{
			for (i = 0; i < THREAD_NUM; i++)
				if (threads[i].state == 0)
				{
					threads[i].addrClient = addrClient;
					threads[i].ConnectSocket = ConnectSocket;
					threads[i].state = 1;//该线程状态置为占用状态
					threads[i].thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)communicat, &threads[i], 0, NULL);
					InterlockedIncrement((LPLONG)&thread_in_use);//线程使用数加一
					break;
				}
			if (i == THREAD_NUM) //线程连接数已满；
			{
				printf("	creat thread failed...");
				break;
			}
		}
	}
	closesocket(ListenSocket);
}

