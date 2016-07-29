#include<stdio.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string.h>

int fds[128];

static void usage(const char *proc)
{
	printf("usage:%s [ip] [port]\n",proc);
}

int startup(char *ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		perror("socket\n");
		return 2;
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);

    if(bind(sock,(struct sockaddr *)&local,sizeof(local)) < 0){
   		perror("bind\n");
   		return 3;
   	}
    
   	if(listen(sock,5) < 0){
		perror("listen");
		return 4;
	}

	return sock;
}

int main(int argc,char *argv[])
{
	if(argc != 3){
		usage(argv[0]);
		return 1;
	}

	int i = 0;
	for(;i < sizeof(fds)/sizeof(fds[0]);++i)
		fds[i] = -1;

	int listen_sock = startup(argv[1],atoi(argv[2]));

	int max_fd = listen_sock;
	fd_set rset;
	
	int done = 0;
	while(!done){
		FD_ZERO(&rset);
		fds[0] = listen_sock;
		for(i = 0;i < sizeof(fds)/sizeof(fds[0]);++i){
			if(fds[i] != -1){
				max_fd = max_fd < fds[i] ? fds[i]:max_fd;
 				FD_SET(fds[i],&rset);
			}
		}

		//struct timeval _timeout = {5,0};
		switch(select(max_fd + 1,&rset,NULL,NULL,NULL)){
			case 0:
				printf("select return,timeout...\n");
				break;
			case -1:
				perror("select");
				break;
			default:
				{
					for(i = 0;i < sizeof(fds)/sizeof(fds[0]);++i){
					
						if(i == 0 && FD_ISSET(fds[i],&rset)){
							//listen_sock ready!!!
							struct sockaddr_in peer;
							socklen_t len = sizeof(peer);
							int fd = accept(listen_sock,(struct sockaddr *)&peer,&len);
							if(fd > 0){
								printf("get a new client\n");
							}
							
							int j = 0;
							for(;j < sizeof(fds)/sizeof(fds[0]);++j){
								if(fds[j] == -1){
									fds[j] = fd;
									break;
								}
							}
							if(j == sizeof(fds)/sizeof(fds[0])){
								printf("server is full\n");
								close(fd);
							}
						}else{
							char buf[1024];
							memset(buf,'\0',sizeof(buf));

							if(fds[i] != -1 && FD_ISSET(fds[i],&rset)){
								ssize_t _s = read(fds[i],buf,sizeof(buf)-1);
								if(_s > 0){
									buf[_s-1] = '\0';
									printf("client # %s\n",buf);
								}else if(_s == 0){
									printf("client close..\n");
									close(fds[i]);
									fds[i] = -1;
								}else{
									perror("read");
								}
							}
						}
					}
				}
				break;
		}
	}
	return 0;
}
