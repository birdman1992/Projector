#include<stdio.h>   
#include<sys/socket.h>   
#include<sys/types.h>  
#include<sys/stat.h>
#include<sys/time.h>
#include<unistd.h>  
#include<stdlib.h>
#include<fcntl.h>
#include<netinet/in.h>  
#include<netdb.h>  
#include<arpa/inet.h>  
#include<pthread.h>
#include<malloc.h>
#include<string.h>
#include"list.h"
  
#define MAX_LISTEN 1024  
#define MAX_LINE 1024  
#define P_FIFO   "/tmp/pro_fifo"

struct socketList{
	int socketFd;
	struct list_head list;
};

struct socketList sktList;
int servfd, clientfd, port, clientlen ,servLocalFd;
fd_set fds; 
struct sockaddr_in servaddr,cliaddr,localAddr;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void initSocketList(){
	INIT_LIST_HEAD(&sktList.list);
}

void socketAppend(struct socketList* node)
{
	list_add_tail(&node->list, &sktList.list);
}

void socketRemove(struct socketList* node)
{
	list_del(&node->list);
	free(node);
}
  
int Socket(int domain, int type, int protocol){  
    int sockfd = socket(domain, type, protocol);  
    if ( sockfd < 0 ){  
        perror("init socket:  ");  
        exit(0);  
    }  
    return sockfd;  
}  
  
void Bind(int sockfd, struct sockaddr *myaddr, int addrlen){  
	int optval = -1;
	socklen_t optlen = -1;

	// 设置地址和端口号可以重复使用
	optval = 1;
	optlen = sizeof(optval);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);

    if ( bind(sockfd, myaddr, addrlen) < 0 ){  
        perror("bind");  
        exit(0);  
    } 
}  
  
void Listen(int sockfd, int backlog){  
    if ( listen(sockfd, backlog) < 0){  
        perror("listen");  
        exit(0);  
    }  
}  
  
int Accept(int listenfd, struct sockaddr *addr, int *addrlen){  
    int clientfd = accept(listenfd, addr, addrlen);  
    if ( clientfd < 0){  
        perror("accept");  
        exit(0);  
    }  
    return clientfd;  
}  
  
void Close(int clientfd){  
    if ( close(clientfd) < 0){  
        perror("close");  
        exit(0);  
    }  
}  
struct hostent* Gethostbyaddr(const char *addr, int len, int domain){  
    struct hostent* host = gethostbyaddr(addr, len, domain);  
    if ( NULL == host ){  
        perror("host_by_addr");  
        exit(0);  
    }  
    return host;  
}  
  
ssize_t Read(int fd, void* buf, size_t n){  
    ssize_t num= read(fd, buf, n);  
    if ( n < 0){  
        perror("read");  
        exit(0);  
    }  
    return num;  
}  
  
ssize_t Write(int fd, const void* buf, size_t n){  
    ssize_t num = write(fd, buf, n);  
    if ( n < 0){  
        perror("write");  
        exit(0);  
    }  
    return num;  
}  
  
void echo(int listenfd){  
    ssize_t n;  
    char write_buff[MAX_LINE];  
    char read_buff[MAX_LINE];  
      
    memset(write_buff, 0, MAX_LINE);  
    memset(read_buff, 0, MAX_LINE);  
  
    n = read(listenfd, read_buff, MAX_LINE);  
    read_buff[n] = '\0';  
  
    strcpy(write_buff, "from server echo: ");  
    strcpy(write_buff+strlen("from server echo: "), read_buff);  
  
    n = write(listenfd, write_buff, MAX_LINE);  
  
      
}  

//接收线程
void* threadAccept(){
	printf("[wait for connect]...\n");
	while(1){

		struct sockaddr cliAddr;
		clientlen = sizeof(cliAddr);
		memset(&cliAddr, 0, clientlen);
		struct socketList* node = (struct socketList*)malloc(sizeof(struct socketList));
		node->socketFd = Accept(servfd, (struct sockaddr*)&cliAddr, &clientlen);
		printf("[Accept]:%d\n", node->socketFd);

		pthread_mutex_lock(&mutex);
		socketAppend(node);
		pthread_mutex_unlock(&mutex);
	}
}

//广播任务
void taskBroadcast(char* msg, int msgSize)
{
	struct list_head *pos;
	struct socketList *p;

	list_for_each(pos,&sktList.list){
        p=list_entry(pos,struct socketList,list);
        Write(p->socketFd, msg, msgSize);
    }
}

//本地通信线程
void* threadLocalMsg(){
	printf("[wait for msg]...\n");
	fd_set localFds;
	struct timeval tv;
	char buffer[1024];

	while(1){
		tv.tv_sec=1;
		FD_ZERO(&localFds);
		struct sockaddr cliAddr;
		clientlen = sizeof(cliAddr);
		memset(&cliAddr, 0, clientlen);
		memset(buffer, 0, sizeof(buffer));
		struct socketList* node = (struct socketList*)malloc(sizeof(struct socketList));
		int fd = Accept(servLocalFd, (struct sockaddr*)&cliAddr, &clientlen);
		printf("[Accept local]:%d\n", fd);
		FD_SET(fd, &localFds);
		if(select(fd+1, &localFds, NULL, NULL, &tv) > 0){
			if(FD_ISSET(fd, &localFds)){
				if(read(fd, buffer, sizeof(buffer)) > 0){
					printf("[msg] %s\n",buffer);
					taskBroadcast(buffer, strlen(buffer));
				}
			}
		}
		FD_CLR(fd, &localFds);
		printf("[close] %d\n",fd);
		Close(fd);
	}
}

void waitPipeMsg()
{
	char cache[1024];
	char cmd[20];
	int fd;
	printf("[init pipe]...\n");
	memset(cache,0, sizeof(cache));   
	unlink(P_FIFO);
	//if(access(P_FIFO,F_OK)==0){                                   
	//	printf("access.\n");
	//	execlp("rm","-f", P_FIFO, NULL);                 
	//}

	if(mkfifo(P_FIFO, 0666) < 0){            
		printf("create named pipe failed.\n");
	}
	fd= open(P_FIFO,O_RDWR);        //     非阻塞方式打开，只读
	
//	sprintf(cmd, "chmod 777 %s\n", P_FIFO);
//	system(cmd);
	printf("[wait pipe msg]...\n");

	while(1){                                                                          //     一直去读
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		memset(cache,0, sizeof(cache));
		if(select(fd+1, &fds, NULL, NULL, NULL)){
			if(FD_ISSET(fd, &fds)){
				printf("select\n");
				if((read(fd,cache, 1024)) == 0 ){                           //     没有读到数据
					printf("no data:\n");
				}
				else{
					printf("get data:%s\n", cache);                //     读到数据，将其打印
					taskBroadcast(cache, strlen(cache));
				}
			}
		}
	}
	close(fd);
}

/*-----------------------------------------------------------
struct socketList sktList;
int servfd, clientfd, port, clientlen ,servLocalFd;
fd_set fds; 
struct sockaddr_in servaddr,cliaddr,localAddr;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

-----------------------------------------------------------*/
  
int main(int argc, char **argv){  
    
	pthread_t id;
	pthread_t id2;
    struct hostent *host;  
    char* hostaddr;  

	initSocketList();

    if ( argc != 2){  
        fprintf(stderr,"usage:%s<port>\n", argv[0]);  
        exit(0);  
    }  
    port = atoi(argv[1]);  // get port  

//初始化远程server  
	printf("[init server]\n");
    servfd = Socket(AF_INET, SOCK_STREAM, 0);  
      
    // init servaddr  
    memset(&servaddr, 0, sizeof(servaddr));  
    memset(&cliaddr, 0, sizeof(cliaddr));  
    servaddr.sin_family = AF_INET;  
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    servaddr.sin_port = htons((unsigned short)port);  
      
    Bind(servfd, (struct sockaddr*)&servaddr, sizeof(servaddr));  
    Listen(servfd, MAX_LISTEN);  
	printf("[listen]...\n");

	pthread_create(&id, NULL, (void*)threadAccept, NULL);

//初始化本地server
	printf("[init local server]\n");
	servLocalFd = Socket(AF_INET, SOCK_STREAM, 0);

	memset(&localAddr, 0, sizeof(localAddr));  
    localAddr.sin_family = AF_INET;  
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    localAddr.sin_port = htons((unsigned short)(8887));  
      
    Bind(servLocalFd, (struct sockaddr*)&localAddr, sizeof(localAddr));  
    Listen(servLocalFd, MAX_LISTEN);  
	printf("[listen]...\n");

	pthread_create(&id2, NULL, (void*)threadLocalMsg, NULL);
	while(1);

//	waitPipeMsg();

//    while(1){   // init server  
//        memset(&cliaddr, 0, sizeof(cliaddr));  
//        clientfd = Accept(servfd, (struct sockaddr*)&cliaddr, &clientlen);  
//        host = Gethostbyaddr((const char*)&cliaddr.sin_addr.s_addr, sizeof(cliaddr.sin_addr.s_addr), AF_INET);  
//        printf("server connect to host: %s %s\n",host->h_name, inet_ntoa(cliaddr.sin_addr));  
//        echo(clientfd);  
//        Close(clientfd);  
//    }  
}  
