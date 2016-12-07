#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include "parser.h"



struct rwPipe{
	int pi[2]; 
};
typedef struct rwPipe rwPipe;


//Variables globales
rwPipe * pipesArray;
int * pids;


//Metodo para manejar las señales
void signal_callback_handler(int signum)
{
   printf("Caught signal: %d\n",signum);
   printf("msh> ");
   fflush(stdout);
}

//Metodo para comprobar si un comando valido
int isValidCommand(char * file){
	if(file == NULL){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//Metodo para cambiar de directorio
int changeDirectory(int argc,char** argv){
	if(argc == 1){
		//Solo recibe cd, accede a HOME
		chdir(getenv("HOME"));
	}else{
		//Recibe una ruta absoluta
		if(chdir(argv[1])!=0){
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

//Metodo para procesar los comandos
int processCommands(int nCommands,tcommand * commandsArray, char * input, char * output, char * error, int bg){
	
	int fd,i,status;
	pid_t pid;

	//Redirección de entrada.
	if(input != NULL){
		fd = open(input,O_RDONLY);
		if(fd == -1){
			printf("%s, Error: fallo en apertura de fichero.\n",input);
			exit(1);
		}else{
			dup2(fd,0);
			close(fd);
		}
	}
	//Redireccion de salida
	if(output != NULL){
		fd = creat(output,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if(fd == -1){
			printf("%s, Error: fallo en apertura o creacion de fichero.\n",output);
			exit(1);
		}else{
			dup2(fd,1);
			close(fd);
		}
	}
	//Redireccion de error
	if(error != NULL){
		fd = creat(error,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if(fd == -1){
			printf("%s, Error: fallo en apertura de fichero.\n",error);
			exit(1);
		}else{

			dup2(fd,2);
			close(fd);
		}
	}
	if(nCommands == 1){
		//Llega un solo comando
		pid = fork();
		
		if(pid == 0){
		//hijo
			if(isValidCommand(commandsArray[0].filename)==0){
				execvp(commandsArray[0].argv[0],commandsArray[0].argv);
				exit(0);
			}else{
				printf("%s, Error: no es un comando valido.\n",commandsArray[0].argv[0]);
				return EXIT_FAILURE;
			}
		}else{
			//padre
			wait (&status);
			if (WIFEXITED(status) != 0)
				if (WEXITSTATUS(status) != 0)
					printf("El comando no se ejecutó correctamente\n");
			return EXIT_SUCCESS;
		}
	}else if(nCommands > 1){ 
		//Llega mas de un comando
		/*
			//Reserva de memoria para pipes y pids
			pipesArray = (rwPipe *) malloc (sizeof(rwPipe) * (nCommands-1));
			pids = (int *) malloc (sizeof(int) * nCommands -1);
			
			if(nCommands >1){
				for(i=0;i<=nCommands;i++){
					pipe(pipesArray[i].pi);
				}
			}

			for(i=0;i<nCommands;i++){
				pid = fork();
				if(pid < 0){
					fprintf(stderr,"Fallo el fork.\n%s\n",strerror(errno));
					//devolver fallo
					return EXIT_FAILURE;
				}else{
					pids[i]=pid;
				}
			}
		
		//Proceso padre
		if(pid != 0){
			//wait(NULL);
		}else{
		//Proceso hijo
		
		}
		*/
	}	
return EXIT_SUCCESS;
}

int main(void){
	char cwd[1024];
	char buf[1024];
	tline * line;
	signal(SIGINT, signal_callback_handler);
	signal(SIGQUIT, signal_callback_handler);

	printf("msh> ");	
	while (fgets(buf, 1024, stdin)) {
		//Tokenizacion de la linea leida que contiene comandos
		line = tokenize(buf);
		
		//En caso de ser nula continuamos
		if (line==NULL) {
			continue;
		}else if(line->ncommands == 1 && (strcmp(line->commands[0].argv[0],"cd")==0)){
			//Si solo se introduce un comando es cd llamamos al metodo changeDirectory
			if(changeDirectory(line->commands[0].argc,line->commands[0].argv)!=0){
				fprintf(stderr,"Error: Directorio no encontrado\n");
			}else{
				getcwd(cwd, sizeof(cwd));
				fprintf(stdout, "Directorio de trabajo actual: %s\n", cwd);
				}
		}else if(line->ncommands >= 1 && (strcmp(line->commands[0].argv[0],"cd")!=0)){
			//Si llega un comando o mas, se llama al metodo processCommands
			/*
			if(processCommands(line->ncommands,line->commands,line->redirect_input,line->redirect_output,line->redirect_error,line->background) != 0){
				fprintf(stderr,"Error");
			}
			*/
			processCommands(line->ncommands,line->commands,line->redirect_input,line->redirect_output,line->redirect_error,line->background);
		}
		//free(pipesArray);
		//free(pids);
	printf("msh> ");
}
return 0;
}
