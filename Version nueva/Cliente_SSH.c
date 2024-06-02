/*
Integrantes:  
- Andres Urbano Andrea
- Gomez Bolanios Luis Aldo
Programa: Cliente que simula una conexion SSH hacia el servidor.
Informacion: Cuando se conecta a un servidor se puede ejecutar comandos.
Licencia: GNU General Public License v3.0
Fecha: 27 mayo 2024 (Ultimo cambio)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define MAXDATASIZE 100 // Tamanio del buffer para los comandos
#define MAXDATASIZE_RESP 200000 // Tamanio del buffer para recibir respuesta del servidor

/*Funcion principal que ejecuta el programa*/
int main(int argc, char *argv[]){
  char comando[MAXDATASIZE]; // Almacena el comando ingresado por el usuario
  int len_comando; // Almacena la longitud del comando
  int numbytes; // Numero de bytes recibidos del servidor
  char buf[MAXDATASIZE_RESP]; //Buffer que recibe la respuesta del servidor

  int sockfd;  // Definicion de variables para el socket
  struct hostent *he;
  struct sockaddr_in cliente; // Informacion de la direccion de destino 

  // Verifica que se haya pasado correctamente el host y el puerto
  if (argc != 3) {
    fprintf(stderr,"usage: client hostname puerto\n");
    exit(1);
  }

  // Obtiene la informacion del host del servidor
  if ((he=gethostbyname(argv[1])) == NULL) { 
   perror("gethostbyname"); // Si hay un error se imprime
   exit(1);
  }

  // Se crea el socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
   perror("socket");
   exit(1);
  }

  // Se configura la estructura cliente con la informacion del servidor
  cliente.sin_family = AF_INET; // Familia de direcciones
  cliente.sin_port = htons( atoi(argv[2]) );  // Puerto (se convierte a formato de red)
  cliente.sin_addr = *((struct in_addr *)he->h_addr); // Direccion IP del servidor
  memset(&(cliente.sin_zero), '\0',8);  // Inicializa en cero el resto de la estructura 

  // Se hace el intento de conexion con el servidor
  if (connect(sockfd, (struct sockaddr *)&cliente, sizeof(struct sockaddr)) == -1) {
   perror("connect"); // Si ocurre una falla se imprime al usuario
   exit(1);
  }
  
 // Se hace un ciclo para pedir los comandos al usuario
  while (1) {
    printf("lineadecomandos# "); // Estetica
    fgets(comando, MAXDATASIZE - 1, stdin); // Se lee el comando ingresado
    len_comando = strlen(comando) - 1; // Se calcula su longitud
    comando[len_comando] = '\0'; // Se quita el caracter salto de linea (fin de cadena)

    // Si el usuario teclea salir o exit, se termina el programa
    if (strcmp(comando, "salir") == 0 || strcmp(comando, "exit") == 0) {
      printf("Saliendo.....");
      break;
    }

    // Se envia el comando al servidor
    if (send(sockfd, comando, len_comando, 0) == -1) {
      perror("send()");
      exit(1);
    }

    // Si el send no devuelve error continua, se lee la respuesta
    if((numbytes = recv(sockfd, buf, MAXDATASIZE_RESP - 1, 0)) == -1) {
       perror("recv");
       exit(1);
    }
    buf[numbytes] = '\0';

    printf("Recibido:\n%s\n", buf);
  }
  // Si se termina el programa se cierra el file descriptor del cliente
  close(sockfd);
  return 0;
} 
