#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int spawn (char* program, char** arg_list)
{
  pid_t child_pid;
  /* Duplicate this process. */
  child_pid = fork ();
  if (child_pid != 0) /* This is the parent process. */
    return child_pid;
  else {
    /* Now execute PROGRAM, searching for it in the path. */
    //printf("soy el hijo, lanzo el ls y termino\n");
    execvp (program, arg_list);
    /* The execvp function returns only if an error occurs. */
    //printf ("un error ocurrio en execvp\n");
    return 1;
  }
}

int main ()
{
  int child_status=1;

  char comando[50];  //maxima logitud de comando
  char *divComando;
  int len=0;
  char *ptr;
  //*char argumentos[6][30];  // 6 argumentos max de 30 caracteres max
  int i=0;
  int lonComando=0;

  char *arg_list[6];
  char argumentos[6][30];
  memset(argumentos,'\0',180);

  

  do{

    /*Limpia variables*/
    fflush(stdin);
    memset(comando,'\0',50);
    memset(argumentos,'\0',180);
    memset(arg_list,'\0',6);
    i=0;
    len=0;
    lonComando=0;

    /*Leer el comando*/
    printf("lineadecomandos# ");
    fgets(comando, 50, stdin);

    /*Dividir el comando*/
    divComando = strtok(comando, " ");

    /*Obtiene argumentos*/
    while( divComando != NULL ){
      ptr = divComando; 
      strcpy(argumentos[i], ptr);
      divComando = strtok(NULL, " ");
      i++;
    }

    for (int i = 0; *argumentos[i] != '\0'; i++) {
      len=len+1;
    }

    char aux[30];
    memset(aux,'\0',30);

    char auxArgumentos[30];
    memset(auxArgumentos,'\0',30);

    if(len==1){  //No tiene argumentos 
      //limpiamos el primer comando
      lonComando = strlen(argumentos[0]);
      strncat(aux, argumentos[0], lonComando-1);
    
      arg_list[0]=aux;
      arg_list[1]=NULL;
    

  }else{
    //limpiamos el primer comando
    lonComando = strlen(argumentos[0]);
    strncat(aux, argumentos[0], lonComando);

    arg_list[0]=argumentos[0];

    int a=1;
    for (int i = 1; i<len-1; i++) {
        arg_list[i]=argumentos[i];
        a=a+1;
    }

    
    lonComando = strlen(argumentos[a]);
    strncat(auxArgumentos, argumentos[a], lonComando-1); //quitamos elemento nulo
    arg_list[a]=auxArgumentos;
    
    arg_list[a+1]=NULL;
  }
  

  wait(&child_status);
  /* Spawn a child process running the "ls" command. Ignore the
   * 	returned child process ID. */
  spawn (aux, arg_list);
  /* Wait for the child process to complete. */
  wait (&child_status);
  //if (WIFEXITED (child_status)){
  //  printf ("El proceso hijo termio normalmente, con codigo de salida %d\n", WEXITSTATUS (child_status));
  //}
  //else
    //printf ("El proceso hijo termino anormalmente\n");
  }while(strcmp(comando, "salir\n"));

  return 0;
}
