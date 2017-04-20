#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include "socketlib.h"

socketStr sock;

int openSOCKET(char * host_ip, int port){

	struct in_addr ipv4addr;
	struct hostent *host = NULL;
	int on = 1;

    inet_pton(AF_INET, host_ip, &ipv4addr); // host_ip -> Direcci�n ip del servidor.
    host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);

    if (host == NULL) {
    	errorSocket("No se ha encontrado el host especificado.");
    	return -1;
    }
    printf("Host name: %s\n", host->h_name);

    bzero((char *) &sock.serv_addr, sizeof(sock.serv_addr));

    bcopy((char *)host->h_addr, (char *)&sock.serv_addr.sin_addr.s_addr,host->h_length);
    sock.serv_addr.sin_family = AF_INET;
    sock.serv_addr.sin_port = htons(port);

	//sock.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sock.sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(sock.sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));
    if (sock.sockfd < 0) {
        errorSocket("Error Abriendo Socket.");
        return -1;
    }

    if (connect(sock.sockfd,(struct sockaddr *) &sock.serv_addr,sizeof(sock.serv_addr)) < 0) {
    	errorSocket("Error conectando socket.");
        return -1;
    }

    printf("Socket conectado.\n");
    return 1;
}

int readSOCKET(char * buffer){
	int res = 0;
	res = read(sock.sockfd , buffer , 255);
	if (res < 0){
        errorSocket("Error leyendo el Socket.");
        return -1;
	}
	buffer[res] = 0;
	return 1;
}

int writeSOCKET(const char * buffer){
	int res = 0;
	//int len = strlen(buffer);
	//buffer[len] = 0x0A; /// \n pone el ultimo en 0x0A para indicar una nueva linea;
	res = send(sock.sockfd , buffer , strlen(buffer) , 0);

	if (res < 0){
        errorSocket("Error escribiendo en el Socket.");
        return -1;
	}
	return 1;
}

int closeSOCKET(){
	int fd = close(sock.sockfd);
	if(fd == -1 ){
		errorSocket("Error cerrando Socket.");
		return -1;
	}
	sock.sockfd = fd;
	return 1;
}

void errorSocket(char *msgError){
	printf("\nError SOCKETLIB: %s\n",msgError);
}
