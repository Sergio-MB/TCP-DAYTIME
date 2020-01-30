// Practica tema 6, Muñumer Blazquez Sergio
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

#define QUEUE	 5
#define SIZEBUFF 50

int sockfd, clientSocket;

//Manejador de interrupciones, caso: ^C
//Finalizamos programa cerrando los sockets previamente
void signal_handler(int signal){
	
	if( signal == SIGINT ){
	
		printf("\n...Apagando servicio...\n");
		shutdown(sockfd, SHUT_RDWR);
		shutdown(clientSocket, SHUT_RDWR);
		exit(EXIT_SUCCESS);
	}
    
}

//Manejador de interrupciones, caso: SIGCHLD 
//La función es eliminar todos los procesos zombies
void signal_handler_SIGCHLD_(int signal) {
	
	if( signal == SIGCHLD ){
//Salvamos el valor de "errno" para no interferir en en la ejecucion normal del código
//mientras se activa el manejador 	
		int saved_errno = errno;
//Ejecutamos un loop para eliminar todos los posibles procesos zombies
//Args: 
//(pid_t)(-1) --> Esperamos por cualquier proceso hijo
//(int) 0     --> Status
// WNOHANG	  --> Opcion: retorna inmediatamente si ningún hijo ha terminado.
		pid_t pid;
		int stat;
		while ((pid=waitpid((pid_t)(-1), &stat, WNOHANG)) > 0);
//Comprobamos que el error no sea que no hay mas hijos para matar (ECHILD)
		if ((pid < 0) && (errno != ECHILD)){
			perror("\nwaitpid()");
		}	
//Devolvemos el valor de errno a la ejecucion normal
		errno = saved_errno;
	}
}

//Main
int main(int argc, char *argv[]) {
	
//Registramos las funciones manejadora de señales
//Args: tipoSeñal, funcion.		Comprobamos error
	if (signal(SIGINT, signal_handler) == SIG_ERR){
		perror("\nsignalINT()");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGCHLD, signal_handler_SIGCHLD_) == SIG_ERR ){
		perror("\nsignalCHLD()");
		exit(EXIT_FAILURE);
	}
	
	struct sockaddr_in server;  	//Estructura del servidor
	struct sockaddr_in client;		//Estructura del cliente
	socklen_t len;					
	char hostNameBuffer[SIZEBUFF];	//Buffer cadena enviada
	char buff[SIZEBUFF];			//Buff para leer del fichero
	FILE *fich;						//Fichero para extraer la fecha

//Comprobamos si el numero de argumentos es inferior a 2 (sin recibir puerto)
	if (argc < 2 || (argc == 2 && (strcmp("-p",argv[1]) == 0 ))){
//Creamos la entidad servidor para localizar su puerto 	
		struct servent *sve;
//Asignamos la entidad servidor al servicio daytime
		sve = getservbyname("daytime","tcp");
//Comprobamos error en la obtencion del puerto
		if(sve == NULL){
			printf("\nError al obtener el puerto por defecto");
			perror("\ngetservbyname");
			exit(EXIT_FAILURE);
		}
//Asignamos el puerto obtenido a la estructura del cliente
		server.sin_port = sve->s_port;
	}
//Comprobamos si nos ofrecen un puerto especifico
	else if( argc == 3 && (strcmp("-p",argv[1]) == 0 )){	
		int puerto;
		sscanf(argv[2],"%d", &puerto);
//Asignamos el puerto obtenido a la estructura del cliente
		server.sin_port = htons(puerto);
	}else{
//Sino estamos en los casos comprobados anteriormente lanzamos error
		printf("\nError de sintaxis\n");
		printf("--> ./programa [-p Puerto]\n");
		exit(EXIT_FAILURE);
	}
//Creamos descriptor socket y definimos su tipo, comprobando el exito de la operacion
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("\nsocket()");
		exit(EXIT_FAILURE);
	}
//Completamos la estructura del cliente, dando familia y una direccion bien conocida cualquiera
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htons(INADDR_ANY);
//Enlazamos socket con la estructura del cliente, comprobando error en la operacion
	if(bind(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0){
		perror("\nbind()");
		exit(EXIT_FAILURE);
	}
	
	if(listen(sockfd, QUEUE) < 0){
		perror("\nlisten()");
		exit(EXIT_FAILURE);
	}

//Creamos la variable "len" con el valor del tamaÃ±o de la estructura del servidor

//Comenzamos el bucle "while" que soportarÃ¡ el servicio
//Estaremos "a la escucha" esperando el mensaje del cliente
	int pid;
	while(1){
		
		len = sizeof(client);
//Aceptamos conexion entrantes, creando socket para dicha conexion
//Comprobamos fallo
		if((clientSocket = accept(sockfd, (struct sockaddr*) &client, &len)) < 0){
			perror("\naccept()");
			exit(EXIT_FAILURE);
		}
	
//Ejecutamos fork
		pid = fork();
//Comprobamos cada caso
		switch(pid)
		{
		//EN caso de error
			case -1:
				perror("\nfork()");
				exit(EXIT_FAILURE);
				break;
				
		//EN caso de ser el hijo				
			case 0:  
			//Cerramos copia del socket	de escucha
			//El proceso hijo no la utiliza, comprobamos fallo
				if(close(sockfd) < 0){
					perror("\nclose()");
					exit(EXIT_FAILURE);
				}
				
			//Almacenamos el valor "Nombre del host" en el buffer correspondiente
			//Comprobando exito en la operacion
				if(gethostname(hostNameBuffer,SIZEBUFF) < 0){
					perror("\ngethostname");
					exit(EXIT_FAILURE);
				}
					
			//Capturamos la hora utilizando una llamada a system, con el comando date
			//Lo almacenamos en un fichero, para ahora extraer la cadena
				system("date > /tmp/tt.txt");
			//Abrimos fichero modo lectura
				fich = fopen("/tmp/tt.txt","r");
			//Capturamos la cadena del fichero en el buffer correspondiente
			//Comprobando error en la operacion
				if (fgets(buff,SIZEBUFF,fich)==NULL) {
					printf("Error en system(), en fopen(), o en fgets()\n");
					exit(EXIT_FAILURE);
				}	
			//Concatenamos HostName y la fecha con el formato apropiado		
				strcat(hostNameBuffer,": ");
				strcat(hostNameBuffer,buff);
			//Enviamos al cliente la cadena comprobando error en el envio
				if(send(clientSocket, hostNameBuffer,SIZEBUFF, 0) < 0){
					
					perror("\nsend()");
					exit(EXIT_FAILURE);
				}
			//Cerramos conexion
				if(shutdown(clientSocket, SHUT_RDWR) < 0){
					perror("\nshutdown()");
					exit(EXIT_FAILURE);
				}	
			//Cerramos socket de conexion al cliente
				if(close(clientSocket) < 0){
					perror("\nclose()");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_SUCCESS);
				break;
				
			//EN caso de ser el hilo padre				
			default:		
			//Cerramos socket de conexion al cliente
			//Volvemos a la escucha
				if(close(clientSocket) < 0){
					perror("\nclose()");
					exit(EXIT_FAILURE);
				}
				break;
		}//fin switch

		
	
	}//fin while
	
//FIN PROGRAMA
	exit(EXIT_SUCCESS);
}
