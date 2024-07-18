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


int main(){
    
    

	struct protoent* tcp;
	tcp = getprotobyname("tcp");

	int sfd = socket(PF_INET, SOCK_STREAM, tcp->p_proto);
	if(sfd == -1){
		printerror("createing a tcp socket");

		return -1;
	}
	printf("[CLI] Socket criada.\n");

	struct sockaddr_in isa = getipa("localhost", 1025);

	if(connect(sfd, (struct sockaddr*)&isa, sizeof isa) == -1){
		printerror("connecting to server");

		return -1;
	}

	printf("[CLI] Conectado ao servidor.\n");


	char jpg[] = "jpg.png";

    	if(send(sfd, (void*) jpg, sizeof jpg, 0) == -1){
		printerror("sending message to server");
	}

	printf("[CLI] Mensagem enviada.\n");

	char tam[255];
	ssize_t size = recv(sfd, (void*)tam, sizeof tam, 0);
	int tam_bytes = atoi(tam); 

	if(size == -1){
		printerror("recieving data from server");
	}{
		printf("[CLI] %lu bytes lidos.\n", size);
		printf("[CLI] %d\n", tam_bytes);
	}

	printf("[CLI] Mensagem recebida do servidor.\n");

	int out_file  = open("arq.png", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU); 	
	int seg=0;
	while(tam_bytes > 0) {
		int lidos = recv(sfd, (void*)tam, sizeof tam, 0);
		tam_bytes -= lidos;
		printf("[CLI] Segmento %d - Lidos %d bytes do servidor. Restante %d\n", seg, lidos, tam_bytes);
		write(out_file, tam, lidos);
		seg++;
	}
	close(out_file);

	printf("[CLI] Arquivo salvo. Encerrando cliente.\n");	

	shutdown(sfd, SHUT_RDWR);
	close(sfd);

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
