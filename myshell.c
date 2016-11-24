#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "parser.h"

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
	//procesador de comandos
	int i,j;
	printf("numero de comandos: %d\n",nComands);
	printf("input: %s\n",input);
	printf("output: %s\n",output);
	printf("error: %s\n",error);
	printf("background: %d\n",bg);
	for(i=0;i<nComands;i++){
		printf("filename: %s\n",commandsArray[i].filename);
		printf("argc: %d\n",commandsArray[i].argc);
		for(j=0;j<commandsArray[i].argc;j++){
			printf("%s \n",commandsArray[i].argv[j]);
		}
	}
	
return EXIT_SUCCESS;
}

//Metodo para procesar un solo comando
int processOneCommand(){
	return 0;
}

//Metodo para procesar dos comandos
int processTwoCommands(){
	return 0;
}

//Metodo para procesar mas de dos comandos
int processMoreCommands(){
	return 0;
}


int main(void){
	char cwd[1024];
	char buf[1024];
	tline * line;

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
