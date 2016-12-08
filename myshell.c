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



//Variables globales
int ** lPipes;
int * pids;


//Metodo para manejar las señales
void signal_callback_handler(int signum)
{
   printf("\nmsh> ");
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

//Metodo para redireccionar
int redirect(char * input, char * output, char * error){
	int fd;
	
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
			printf("%s, Error: fallo en apertura o creacion de fichero.\n",error);
			exit(1);
		}else{
			dup2(fd,2);
			close(fd);
		}
	}
	return EXIT_SUCCESS;
}



//Metodo para procesar los comandos
int processCommands(int nCommands,tcommand * commandsArray, char * input, char * output, char * error, int bg){
	
	int i,j,status;
	pid_t pid;

	
	if(nCommands == 1){
		//Llega un solo comando
		pid = fork();
		if(pid == 0){
		//hijo
			redirect(input,output,error);
			if(isValidCommand(commandsArray[0].filename)==0){
				execvp(commandsArray[0].argv[0],commandsArray[0].argv);
				exit(0);
			}else{
				fprintf(stderr,"%s: no se encuentra el mandato.\n",commandsArray[0].argv[0]);
				return EXIT_FAILURE;
			}
		}else{
			//padre
			wait (&status);
			if (WIFEXITED(status) != 0)
				if (WEXITSTATUS(status) != 0)
					printf("El comando no se ejecutó correctamente.\n");
			return EXIT_SUCCESS;
		}
	}else if(nCommands > 1){ 
		//Llega mas de un comando
		
		//Reserva de memoria para pipes y pids
		lPipes = (int **) malloc (sizeof(int) * (nCommands-1));
		pids = (int *) malloc (sizeof(int) * nCommands -1);
		
		//Inicializamos los pipes
		for(i=0;i<nCommands-1;i++){
			lPipes[i]= (int *) malloc (2*sizeof(int));
			pipe(lPipes[i]);
		}
		
		//Proceso de los comandos
		for(i=0;i<nCommands;i++){
			pids[i]=fork();
			if(pids[i]==0){
				//Es hijo
				if(i==0){
					//Es el primer hijo
					redirect(input,NULL,NULL);
					dup2(lPipes[i][1],1);
					for(j=0;j<nCommands-1;j++){
						if(j==0){
							close(lPipes[j][0]);
						}else{
							close(lPipes[j][0]);
							close(lPipes[j][1]);
						}
					}
				}else if(i==nCommands-1){
					//Es el ultimo hijo
					redirect(NULL,output,error);
					dup2(lPipes[i-1][0],0);
					
					for(j=0;j<nCommands-1;j++){
						if(j== i-1){
							close(lPipes[j][1]);
						}else{
							close(lPipes[j][0]);
							close(lPipes[j][1]);
						}
					}
				}else{
					//Es cualquier otro hijo
					dup2(lPipes[i][1],1);
					dup2(lPipes[i-1][0],0);
					for(j=0;j<nCommands-1;j++){
						if(j==i){
							close(lPipes[j][0]);
						}else if(j==i-1){
							close(lPipes[j][1]);
						}else{
							close(lPipes[j][0]);
							close(lPipes[j][1]);
						}
					}
				}
				if(isValidCommand(commandsArray[i].filename)==0){
					execvp(commandsArray[i].argv[0],commandsArray[i].argv);
					exit(0);
				}else{
					fprintf(stderr,"%s: no se encontro el mandato.\n", commandsArray[i].argv[0]);
					exit(1);
				}
			}
		}
		//Cerrar pipes
		for(i=0;i<nCommands-1;i++){
			close(lPipes[i][0]);
			close(lPipes[i][1]);
		}
		
		//Esperar a que terminen los hijos
		for(i=0;i<nCommands;i++){
			waitpid(pids[i],NULL,0);
		}
		
		//Liberar memoria
		for(i=0;i<nCommands-1;i++){
			free(lPipes[i]);
		}
		free(lPipes);
		free(pids);
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
			processCommands(line->ncommands,line->commands,line->redirect_input,line->redirect_output,line->redirect_error,line->background);
		}
		printf("msh> ");
}
return 0;
}
