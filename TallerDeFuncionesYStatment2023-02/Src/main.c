#include <stdint.h>


/*
 * Funcion principal del programa
 * Esta funcion es el corazon del programa!!
 *
 * */
void funcion1(void);
void funcion2(void);
void funcionContadora(void);

uint8_t sumar(void);
uint8_t funcionLocal(void);

uint8_t num1 = 0;
uint8_t num2 = 20;

uint16_t contadorGlobal = 0;

uint8_t estado = 0;			// Estado inicial


int main(void){

	/*
	* Es el programa principal, solo se corre o se ejcuta lo que se encuentra
	* aquí o que se configura a traves de este, se acostumbra a retornar cero
	* por buena practica y para verficar errores en casos especiales.
	*/

	/*
	Explicar el for luego de conceutivos
	*/
	for(int i = 0; i < 5; i++){
	    funcion1();
	}

//	printf("%d\n", num1);

	/*
	Expicar el live expression y variablese
	*/
	uint8_t varLocal = 7;
	(void)varLocal;

	//funcion2();

	uint8_t resultado = sumar();
	(void)resultado;
//	printf("%d\n", resultado);
	uint8_t contadorLocal = 0;

	while(1){
	    funcionContadora();
	    contadorLocal = funcionLocal();

	    if(contadorGlobal > contadorLocal){

	    	// Se indica que se entró al estado 2.
	        estado = 2;
	    }

	    else if(contadorGlobal > (contadorLocal/2)){

	    	// Se indica que se entró al estado 1.
	    	estado = 1;
	    }

	    else{
	    	estado = 0;		// No se cambia el estado.
	    }
	}

	return 0;
}


void funcion1(void){
    num1 = num1 + 1;
}

// void funcion2(void){
//     varLocal = varLocal + 1;
// }

uint8_t sumar(void){
    uint8_t res1 = num1 + num2;
    return res1;
}

void funcionContadora(void){
    contadorGlobal = contadorGlobal + 100;
}

uint8_t funcionLocal(void){
    uint8_t contadorLocal = 253;
    return contadorLocal + 1;
}
