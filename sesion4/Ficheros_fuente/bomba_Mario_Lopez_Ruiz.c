//gcc -m32  bomba_Mario_Lopez_Ruiz.c -o bomba_Mario_Lopez_Ruiz
#include <stdio.h>	// para printf()
#include <stdlib.h>	// para exit()
#include <string.h>	// para strncmp()/strlen()
#include <sys/time.h>	// para gettimeofday(), struct timeval

char password[]= "adavitcased";
int  passcode  = 7654;

void boom(){
	printf("***************\n");
	printf("*** BOOM!!! ***\n");
	printf("***************\n");
	exit(-1);
}

void defused(){
	printf("*************************\n");
	printf("*** bomba desactivada ***\n");
	printf("*************************\n");
	exit(0);
}

void CmpContrs(char *contrasenia){
	char aux[strlen(password)];
	char temporal;
	char final[2] = "\n";
	int i;
	int j=strlen(password)-1;
	for(int i=0; i<strlen(password); i++)
		aux[i] = password[i];

	for(int i=0; i<strlen(password)/2;i++){//Invierto la contraseña
		temporal = aux[i];
		aux[i] = aux[j];
		aux[j] = temporal;
		j--;
	}

	char palabra = '\n';
	aux[strlen(aux-1)] = palabra;

	if(strncmp(contrasenia, aux, strlen(password)))
		boom();
}

void error(int vcod){
	int cod = passcode+128;

	if(vcod != cod)
		boom();
}

void Bomba(){//No hace nada
	
}

int main(){
#define SIZE 100
	char pass[SIZE];
	int  pasv;
#define TLIM 5
	struct timeval tv1,tv2;	// gettimeofday() secs-usecs

	gettimeofday(&tv1,NULL);

	printf("Introduce la contraseña: ");
	fgets(pass,SIZE,stdin);
	if (strncmp(pass,password,strlen(password)))
	    Bomba();
	CmpContrs(pass);

	gettimeofday(&tv2,NULL);
	if (tv2.tv_sec - tv1.tv_sec > TLIM)
	    boom();

	printf("Introduce el código: ");
	scanf("%i",&pasv);
	error(pasv);
	if (pasv!=passcode) 
		Bomba();

	gettimeofday(&tv1,NULL);
	if (tv1.tv_sec - tv2.tv_sec > TLIM)
	    boom();

	defused();
}
