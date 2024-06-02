/*
Integrantes:  
- Andres Urbano Andrea
- Gomez Bolanios Luis Aldo
Programa: Servidor que simula una conexion SSH por parte del cliente.
Informacion: Cuando se conecta un cliente puede ejecutar los comandos del servidor.
Licencia: GNU General Public License v3.0
Fecha: 27 mayo 2024 (Ultimo cambio)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define LENGTH 200000 // Define el tamanio del buffer
#define END_SIGNAL "Comando sin salida estandar" //Bandera de comando sin salida estandár

/*La funcion ejecuta el comando recibido y se envia al cliente*/
void execute_command(int client_socket, char* command) {
  int fd[2]; // Array de dos enteros (file descriptor)
  pipe(fd); // Se crea una tuberia pipe para redirigir la salida estandar y la salida de error al proceso hijo

  pid_t pid = fork(); // Se crea un nuevo proceso, si es 0 es el proceso hijo, positivo el proceso padre y -1 es un error

  switch(pid){ // Se usa un switch para manejar los distintos casos del fork
    case 0: // Proceso hijo
      dup2(fd[1], STDOUT_FILENO); // Redirige la salida estandar (stdout)
      dup2(fd[1], STDERR_FILENO); // Redirige la salida de error (stderr)
      close(fd[0]); // Se cierra ambos pipe's ya que el proceso hijo solo 
      close(fd[1]); // necesita la redireccion

      // Prepara la lista de argumentos para ejecutar el comando en la shell del servidor
      char *arg_list[4];
      arg_list[0] = "/bin/sh"; // Es el programa a ejecutar ("/bin/sh")
      arg_list[1] = "-c"; // Indica que lo siguiente es un comando
      arg_list[2] = command; // Comando a ejecutar
      arg_list[3] = NULL; // Indica que acabo la lista de argumentos

      execvp(arg_list[0], arg_list); // Reemplaza el proceso hijo con el comando a ejecutar
      perror("execvp"); // Si falla entonces imprime el error y termina el proceso hijo
      exit(1);
      break;
    case -1:
      perror("fork"); // Se maneja el error en caso de que falle fork
      break;
    default: // Proceso padre
      close(fd[1]); // Se cierra el file descriptor 1
      char buffer[LENGTH];
      int bytes_read = read(fd[0], buffer, sizeof(buffer) - 1); // Lee los datos de lectura del pipe
      
      if(bytes_read == 0){
        // Enviar senial de que el comando ejecutado no tiene salida estandar
        send(client_socket, END_SIGNAL, strlen(END_SIGNAL), 0);
      }else{
        buffer[bytes_read] = '\0'; // Si se lee algo entonces termina con la cadena ('\0')
          if (send(client_socket, buffer, bytes_read, 0) == -1) { // Envia los datos al cliente
            perror("send"); // Si hubo un error entonces se imprime
            close(fd[0]);
            waitpid(pid, NULL, 0); // Se espera que el proceso hijo termine
            return;
          }
      }
    close(fd[0]); //Cierra el pipe
    waitpid(pid, NULL, 0); // Se cierra el pipe y se espera a que el hijo termine
    break;
  }
}

/*Funcion principal que ejecuta el programa*/
int main(int argc, char *argv[]){
  int numbytes;
  char buf_peticion[100]; // Buffer para recv
  char buf_respuesta[LENGTH]; // Buffer para send

  // Estas son las 2 estructuras, la primera  llamada servidor,
  // que se asociara a server_fd y la segunda estructura llamada cliente que se asociara a cliente_fd
  struct sockaddr_in servidor;    // Informacion sobre mi direccion (servidor)
  struct sockaddr_in cliente; // Informacion sobre la direccion del cliente

  // del lado server tenemos 2 estructuras sockaddr_in una para el propio server 
  // y otra para la conexion cliente por lo que necesitamos 2 file descriptor
  int server_fd, cliente_fd;

  // La longitud de tamanio de servidor y de cliente
  int sin_size_servidor;
  int sin_size_cliente;

  // Define variables para el tamanio de la estructura de direcciones
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
   perror("socket");
   exit(1);
  }

  // Se crea el socket
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1){
    perror("Server-setsockopt() error!");
    exit(1);
  }
  else{
    printf("Server-setsockopt is OK...\n");
  }
  
  servidor.sin_family = AF_INET;         // Ordenacion de bytes de la maquina
  servidor.sin_port = htons( atoi(argv[1]) ); // Ordenacion de bytes de la red
  servidor.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi direccion IP
  memset(&(servidor.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

  // Configura la estructura de la direccion del servidor
  sin_size_servidor = sizeof( servidor );
  if (bind(server_fd, (struct sockaddr *)&servidor, sin_size_servidor) == -1){
   perror("bind");
   exit(1);
  }

  // Asocia el socket del servidor a la direccion y puerto especificados
  if (listen(server_fd, 1) == -1){
   perror("listen");
   exit(1);
  }

  // Se configura el socket para escuchar las conexiones entrantes
  sin_size_cliente = sizeof( cliente );
  if ((cliente_fd = accept(server_fd, (struct sockaddr *)&cliente, &sin_size_cliente)) == -1){
   perror("accept");
   exit(1);
  }
  printf("Server: Conexion cliente desde la direccion: %s\n", inet_ntoa(cliente.sin_addr));

  // Con el bucle logramos seguir recibiendo comandos del cliente
  while (1) {
    //Recibimos los comandos del cliente
    if ((numbytes = recv(cliente_fd, buf_peticion, sizeof(buf_peticion) - 1, 0)) == -1) {
      perror("recv");
      exit(1);
    }
    
    buf_peticion[numbytes] = '\0';
    printf("El comando recibido es: %s\n", buf_peticion);

    // SI el comando es salir o exit entonces finalizamos la sesion del servidor
    if (strcmp(buf_peticion, "salir") == 0 || strcmp(buf_peticion, "exit") == 0) {
      printf("El cliente cerro la conexion\n");
      break;
    }    
    execute_command(cliente_fd, buf_peticion); // Se llama a la funcion, recibe el comando y el cliente
  }

  // Si salimos del ciclo while entonces se cierran los descriptores y se finaliza el programa
  close(cliente_fd);
  close(server_fd);
  shutdown(server_fd, SHUT_RDWR);
  return 0;
}