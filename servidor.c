#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>


struct sockaddr_in getipa(const char*, int);
void printerror(const char*);

int main()
{
	struct protoent* tcp;
	tcp = getprotobyname("tcp");

	int sfd = socket(PF_INET, SOCK_STREAM, tcp->p_proto);
	if(sfd == -1){
		printerror("createing a tcp socket");

		return -1;
	}
	printf("[SRV] Socket criado.\n");

	struct sockaddr_in isa = getipa("localhost", 1025);

	if(bind(sfd, (struct sockaddr*)&isa, sizeof isa) == -1){
		printerror("binding socket to IP address");

		return -1;
	}

	printf("[SRV] Bind efetuado.\n");

	if(listen(sfd, 1) == -1){
		printerror("setting socket to listen");

		return -1;
	}

	printf("[SRV] Aguardando conexão...\n");

	int cfd = accept(sfd, NULL, NULL);


	if(cfd == -1){
		if(errno == EAGAIN || errno == EWOULDBLOCK){
			puts("SIGIO recieved for listen socket, but don't know why.");
		}else{
			printerror("accepting a connection");

			return -1;
		}
	}

	printf("[SRV] Conexão recebida.\n");


    	char buff[255];
	ssize_t size = recv(cfd, (void*)buff, sizeof buff, 0);

	printf("[SRV] Mensagem recebida, %lu bytes lidos.\n", size);

	if(size == -1){
		printerror("receiving data from client");
	}else{
		buff[size] = '\0';
	}

    	struct stat info;
    	int in_file  = open(buff, O_RDONLY); // read only  
    	fstat(in_file, &info);

    	printf("tamanho %lu\n", info.st_size);
	char tam[255];
	sprintf(tam, "%lu", info.st_size);

	//if(send(cfd, (void*)tam, sizeof tam, 0) == -1){
	if(send(cfd, (void*)tam, strlen(tam), 0) == -1){
		printerror("sending message to client");
	}

	int lidos;
	int seg=0;
	while((lidos = read(in_file, buff, sizeof buff)) > 0) {
		send(cfd, (void*)buff, lidos, 0);
		printf("[SRV] Segumento %d - Enviados %d bytes.\n", seg, lidos);
		seg++;
	}

	close(in_file);



	//shutdown(sfd, SHUT_RD);
	//shutdown(cfd, SHUT_RD);
	//close(cfd);
	//close(sfd);
	getchar();

	return 0;
}

struct sockaddr_in getipa(const char* hostname, int port){
	struct sockaddr_in ipa;
	ipa.sin_family = AF_INET;
	ipa.sin_port = htons(port);

	struct hostent* localhost = gethostbyname(hostname);
	if(!localhost){
		printerror("resolveing localhost");

		return ipa;
	}

	char* addr = localhost->h_addr_list[0];
	memcpy(&ipa.sin_addr.s_addr, addr, sizeof addr);

	return ipa;
}

void printerror(const char* action){
	int errnum = errno;
	errno = 0;

	if(errnum > 0){
		printf("An error occured while %s.\nError code: %i\nError description: %s\n",
				action, errnum, strerror(errnum));
	}else if(h_errno > 0){
		printf("An error occured while %s.\nError code: %i\nError description: %s\n",
				action, h_errno, hstrerror(h_errno));

		h_errno = 0;
	}else{
		printf("An error occured while %s.\n There is no error data.\n", action);
	}
}
