#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "parser.h"

//Funcion para cambiar de directorio
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


int main(void){
	
	char cwd[1024];
	char buf[1024];
	tline * line;

	printf("msh> ");	
	while (fgets(buf, 1024, stdin)) {
		
		line = tokenize(buf);
		if (line==NULL) {
			continue;
		}else if(line->ncommands == 1 && (strcmp(line->commands[0].argv[0],"cd")==0)){
			if(changeDirectory(line->commands[0].argc,line->commands[0].argv)!=0){
				fprintf(stderr,"Error: Directorio no encontrado\n");
			}else{
				getcwd(cwd, sizeof(cwd));
				fprintf(stdout, "Directorio de trabajo actual: %s\n", cwd);
				}
	}
	printf("msh> ");
}
return 0;
}
