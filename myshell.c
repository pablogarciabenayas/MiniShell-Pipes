#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
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
		printf("no es un comando valido.\n");
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
int processCommands(int nComands,tcommand * commandsArray, char * input, char * output, char * error, int bg){
	
	int file,i;
	/*
	int i,j;
	printf("numero de comandos: %d\n",nComands);
	printf("input: %s\n",input);
	printf("output: %s\n",output);
	printf("error: %s\n",error);
	printf("background: %d\n",bg);
	for(i=0;i<nComands;i++){
		if(isValidCommand(commandsArray[i].filename)==0){
			printf("filename: %s\n",commandsArray[i].filename);
			printf("argc: %d\n",commandsArray[i].argc);
			for(j=0;j<commandsArray[i].argc;j++){
				printf("%s \n",commandsArray[i].argv[j]);
			}
		}
	}
	*/
	/*
	if(nComands == 1){
		if(isValidCommand(commandsArray[0].filename)==0){
			execvp(commandsArray[0].argv[0],commandsArray[0].argv);
		}
	}
	*/
	
		//Redirección de entrada.
	if(input != NULL){
		file = open(input,O_RDONLY);
		if(file == -1){
			fprintf(stderr,"%s,Error: ",input);
			exit(1);
		}else{
			dup2(file,0);
			close(file);
		}
	}
	//Redireccion de salida
	if(output != NULL){
		file = creat(output,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if(file == -1){
			fprintf(stderr,"%s,Error: ",output);
			exit(1);
		}else{
			dup2(file,1);
			close(file);
		}
	}
	
	//Redireccion de error
	if(output != NULL){
		file = creat(error,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if(file == -1){
			fprintf(stderr,"%s,Error: ",error);
			exit(1);
		}else{

			dup2(file,2);
			close(file);
		}
	}
	
	//Reserva de memoria para pipes y pids
	pipesArray = (rwPipe *) malloc (sizeof(rwPipe) * (nComands-1));
	pids = (int *) malloc (sizeof(int) * nComands -1);
	
	if(nComands <1){
		for(i=0;i<=nComands;i++){
			pipe(pipesArray[i].pi);
		}
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
			if(processCommands(line->ncommands,line->commands,line->redirect_input,line->redirect_output,line->redirect_error,line->background) != 0){
				fprintf(stderr,"Error");
			}
		}
	printf("msh> ");
}
return 0;
}
