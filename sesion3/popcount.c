//  según la versión de gcc y opciones de optimización usadas, tal vez haga falta
//  usar gcc –fno-omit-frame-pointer si gcc quitara el marco pila (%ebp)

#include <stdio.h>	// para printf()
#include <stdlib.h>	// para exit()
#include <sys/time.h>	// para gettimeofday(), struct timeval

#define NBITS 16
#define SIZE (1<<NBITS)	// tamaño para medicion de tiempos (2^20)
#define WSIZE 8*sizeof(int)

int resultado1 = 0, resultado2 = 0, resultado3 = 0;

unsigned lista1[SIZE] = {0x80000000, 0x00100000, 0x00000800, 0x00000001};
unsigned lista2[SIZE] = {0x7fffffff, 0xffefffff, 0xfffff7ff, 0xfffffffe, 0x01000024, 0x00356700, 0x8900ac00, 0x00bd00ef};
unsigned lista3[SIZE] = {0x0	, 0x10204080, 0x3590ac06, 0x70b0d0e0, 0xffffffff, 0x12345678, 0x9abcdef0, 0xcafebeef};
//Los resultados para estas listas deberían ser 4, 156, 116, respectivamente

//********************* PRIMERA VERSIÓN(FOR) ***************************//
int pcount_for(unsigned x){
	int i=0, contador ;

	for(contador = 0; x != 0; contador++)
		x &= x-1;	
	return contador;
}

int suma1(int* array, int len)
{
	int  i,   result=0;
	for (i=0; i<len; i=i+4){//Desenrrollado de bucle
		result += pcount_for(array[i]);
		result += pcount_for(array[i+1]);
		result += pcount_for(array[i+2]);
		result += pcount_for(array[i+3]);
	}
    return result;
}


//********************* SEGUNDA VERSIÓN(WHILE) ***************************//
int pcount_while(unsigned x){
	int contador = 0;
	while(x != 0){
		x &= x-1;//Guarda 1 o 0 dependiendo del ultimo bit de x	
		contador++;	
	}
    return contador;
}

int suma2(int* array, int len)
{
    int  i,   result=0;
    for (i=0; i<len; i=i+4){//Desenrrollado de bucle
		result += pcount_while(array[i]);
		result += pcount_while(array[i+1]);
		result += pcount_while(array[i+2]);
		result += pcount_while(array[i+3]);
	}
    return result;
}




//********************* TERCERA VERSIÓN(ASM) ***************************//

int suma3(int* array, int len)
{
    int  i,   result=0;
	unsigned x;
    for (i=0; i<len; i++){
		x = array[i];
		asm("\n"
			"ini3:						\n\t"	// seguir mientras que x!=0	
				"shr %[x]				\n\t"	// LSB en CF (desplazamiento a la derecha (shift right)
				"adc $0, %[r]			\n\t"
				"test %[x], %[x]		\n\t"
				"jnz	ini3			\n\t"	
				: [r]"+r"	(result)			// e/s: 	añadir a lo acumulado por el momento
				: [x]"r"	(x)					// entrada: valor elemento		
		);	
		

	}
    return result;
}


//********************* CUARTA VERSIÓN(máscara) ***************************//

int suma4(int* array, int len)
{
    int  i, j, x, result=0;

    for (i=0; i<len; i++){//Desenrrollado de bucle

		x = array[i];

		for(j=0; j < 8; j++){
			result += x & 0x01010101;
			x >>= 1;
		}
	}

	result += (result >> 16);
	result += (result >> 8);

    return result & 0xFF;
}

//********************* QUINTA VERSIÓN(SSSE3) ***************************//

int suma5 (int *array, int len){
	int i;
	int val, result=0;
	int SSE_mask[] = {0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f};
	int SSE_LUTb[] = {0x02010100, 0x03020201, 0x03020201, 0x04030302};
	if (len & 0x3)
		printf("Leyendo 128B, pero la longitud no es multiplo de 4\n");
	for (i=0;i<len;i+=4){
		asm("movdqu 	%[x], %%xmm0	\n\t"
			"movdqa		%%xmm0, %%xmm1	\n\t"	// Dos copias de x
			"movdqu 	%[m], %%xmm6	\n\t"	// Máscara
			"psrlw		$4, %%xmm1 		\n\t"
			"pand 		%%xmm6, %%xmm0	\n\t"	// xmm0 = inferiores
			"pand		%%xmm6, %%xmm1	\n\t"	// xmm1 = superiores
			"movdqu		%[l], %%xmm2	\n\t"
			"movdqa		%%xmm2, %%xmm3	\n\t"
			"pshufb		%%xmm0,	%%xmm2	\n\t"	// xmm2 = vector inferiores
			"pshufb		%%xmm1,	%%xmm3	\n\t"	// xmm3 = vector superiores
			"paddb		%%xmm2, %%xmm3	\n\t"
			"pxor		%%xmm0, %%xmm0	\n\t"
			"psadbw		%%xmm0, %%xmm3	\n\t"
			"movhlps	%%xmm3,	%%xmm0	\n\t"
			"paddd		%%xmm3,	%%xmm0	\n\t"
			"movd 		%%xmm0, %[val]	\n\t"
			: [val]"=r" (val)
			: [x]  "m"	(array[i]),
			[m]  "m"	(SSE_mask[0]),
			[l]  "m"	(SSE_LUTb[0])
		);
		result += val;
	}
	return result;
}

//********************* TIEMPOS ***************************//

void crono(int (*func)(), char* msg){
    struct timeval tv1,tv2;	// gettimeofday() secs-usecs
    long           tv_usecs;	// y sus cuentas

    gettimeofday(&tv1,NULL);
    resultado1 = func(lista1, SIZE);
	resultado2 = func(lista2, SIZE);
	resultado3 = func(lista3, SIZE);
    gettimeofday(&tv2,NULL);

    tv_usecs=(tv2.tv_sec -tv1.tv_sec )*1E6+
             (tv2.tv_usec-tv1.tv_usec);
    printf("\nResultado para la primera lista = %d\t\n", resultado1);
	printf("Resultado para la segunda lista = %d\t\n", resultado2);
	printf("Resultado para la tercera lista = %d\t\n", resultado3);
    printf("%s:%9ld us\n", msg, tv_usecs);
}

//********************* main ***************************//

int main()
{

    crono(suma1, "suma1  (en lenguaje C, con bucle for    ) ");
	crono(suma2, "suma2  (en lenguaje C, con bucle while  ) ");
	crono(suma3, "suma3  (en lenguaje C, con ASM  ) ");
	crono(suma4, "suma4  (en lenguaje C, aplicando mascara sucesivamente   ) ");
	crono(suma5, "suma5  (en lenguaje C, usando SSSE3   ) ");
    
    printf("\nN*(N+1)/2 = %d\n", (SIZE-1)*(SIZE/2)); /*OF*/

    exit(0);
}
