//  según la versión de gcc y opciones de optimización usadas, tal vez haga falta
//  usar gcc –fno-omit-frame-pointer si gcc quitara el marco pila (%ebp)

#include <stdio.h>  // para printf()
#include <stdlib.h>	// para exit()
#include <sys/time.h>	// para gettimeofday(), struct timeval


#define NBITS 16
#define SIZE (1<<NBITS)
#define WSIZE 8*sizeof(int)

int resultado1 = 0, resultado2 = 0, resultado3 = 0;

unsigned lista1[SIZE] = {0x80000000, 0x00100000, 0x00000800, 0x00000001};
unsigned lista2[SIZE] = {0x0, 0x10204080, 0x3590ac06, 0x70b0d0e0, 0xffffffff, 0x12345678, 0x9abcdef0, 0xcafebeef};
unsigned lista3[SIZE] = {0x7fffffff, 0xffefffff, 0xfffff7ff, 0xfffffffe, 0x01000024, 0x00356700, 0x8900ac00, 0x00bd00ef};





//********************* PRIMERA VERSIÓN(FOR) ***************************//

unsigned parity1(unsigned *array, int len){
	unsigned result=0, val=0, x, i;
	int j;
	for (j=0; j<len; j++){
		x = array[j];
		for (i=0; i<WSIZE; i++){
			unsigned mask = 0x1<<i;
			val ^= (x & mask) != 0;
		}
		result += val;
		val = 0;
	}
	return result;
}

//********************* SEGUNDA VERSIÓN(WHILE) ***************************//
unsigned parity2(unsigned *array, int len){
	unsigned result=0, val=0, x;
	int j;
	for (j=0; j<len; j++){
		x = array[j];
		do{
			val ^= x & 0x1;
		}while (x >>= 1);
		result += val;
		val = 0;
	}
	return result;
}

//********************* TERCERA VERSIÓN(LIBRO) ***************************//

unsigned parity3(unsigned *array, int len){
	unsigned result=0, val=0, x;
	int j;
	for (j=0; j<len; j++){
		x = array[j];
		while (x){
			val ^= x;
			x >>= 0x1;
		}
	result += (val & 0x1);
	val = 0;
	}
	return result;
}

//********************* CUARTA VERSIÓN(ASM) ***************************//

unsigned parity4(unsigned *array, int len){
	unsigned result=0, val=0, x;
	int i;
	for (i=0; i<len; i++){
		x = array[i];
		val = 0;
		asm("\n"
			"ini3:					\n\t"	//While (x != 0).
			"	xor %[x],%[val]		\n\t"
			"	shr %[x]			\n\t"
			"	test %[x],%[x]		\n\t"
			"	jnz ini3			\n\t"
			"	and $1, %[val]		\n\t"
			: [val]"+r" (val)				//Es entrada y salida. Entrada:0, Salida:paridad.
			: [x] "r" (x)					//Es entrada. Entrada:elemento.
		);
		result += val;
	}
	return result;
}

//********************* QUINTA VERSIÓN(Libro 2) ***************************//

unsigned parity5(unsigned *array, int len){
	unsigned result=0, x;
	int j, k;
	for (j=0; j<len; j++){
		x = array[j];
		for (k=16; k>=1; k=k/2)
			x ^= (x>>k);
		result += (x & 0x1);
	}
	return result;
}

//********************* SEXTA VERSIÓN(Libro pero con asm) ***************************//

unsigned parity6(unsigned *array, int len){
	unsigned result=0, x;
	int j;
	for (j=0; j<len; j++){
		x = array[j];
		asm("\n"
			"mov %[x], %%edx		\n\t"	//Copia de edx
			"shr $16, %[x]			\n\t"
			"xor %[x], %%edx		\n\t"
			"xor %%dh, %%dl			\n\t"
			"setpo %%dl 			\n\t"
			"movzx %%dl, %[x]		\n\t"
			: [x]"+r" (x)			  		//Es entrada y salida. Entrada:valor del elemento, Salida:paridad
			:
			:"edx"						  	//Devuelve la copia
		);
		result += x;
	}
	return result;
}
//********************* TIEMPOS ***************************//

void crono(unsigned (*func)(), char* msg){
	unsigned result;
	struct timeval tv1, tv2;
	long tv_usecs;
	gettimeofday(&tv1, NULL);
	resultado1 = func(lista1, SIZE);
	resultado2 = func(lista2, SIZE);
	resultado3 = func(lista3, SIZE);
	gettimeofday(&tv2, NULL);

	tv_usecs = (tv2.tv_sec-tv1.tv_sec)*1E6 + (tv2.tv_usec-tv1.tv_usec);

	//printf("-Resultado: %d\t", result);
    printf("\nResultado para la primera lista = %d\t\n", resultado1);
	printf("Resultado para la segunda lista = %d\t\n", resultado2);
	printf("Resultado para la tercera lista = %d\t\n", resultado3);
	printf("%s:%5ld us\n", msg, tv_usecs);
}

//********************* main ***************************//

int main(){

	printf("\n");
	crono(parity1,"Parity1 (for C)    ");
	crono(parity2,"Parity2 (while C)  ");
	crono(parity3,"Parity3 (libro)    ");
	crono(parity4,"Parity4 (asm)      ");
	crono(parity5,"Parity5 (libro2º)  ");
	crono(parity6,"Parity6 (lib2º&asm)");
	printf("N*(N+1)/2: %d\n", (SIZE-1)*(SIZE/2));
	printf("\n");
}
