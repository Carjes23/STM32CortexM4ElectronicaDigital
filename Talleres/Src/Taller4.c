/*
 * main2.c
 *
 *  Created on: Mar 2, 2023
 *      Author: dacardenasj
 */

#include <stdint.h>




int main(void){
	uint16_t faceSet1 	= 		0b0000011011001110;
	uint16_t faceSet2 	= 		0b1101100001000111;
	uint16_t mascaraBSet1 = 	0;
	uint16_t mascaraBSet2 = 	0;
	uint16_t mascaraCSet1 = 	0;
	uint16_t mascaraCSet2 = 	0;
	uint16_t mascaraDSet1 = 	0;
	uint16_t mascaraDSet2 = 	0;
	uint32_t faceSetFinal = 	0;
	/*
	 * A Caras masculinas: ya que tenemos que
	 * en un primer momento las mujeres son 1 y los
	 * hombres 0 con un bitwise not consegiriamos este
	 * propocito
	 */
	faceSet1 = ~faceSet1;
	faceSet2 = ~faceSet2;



	/*
	 * B Caras de mujeres con lentes y necesitamos
	 * que donde hayan mujeres con gafas este el valor 1
	 * teniendo en cuenta lo que se le aplico antes
	 * utilizaremos un and con una mascara donde se tengan
	 * las mujeres con lentes
	 */

	// Mascara
	mascaraBSet1 += 1;
	mascaraBSet1 = mascaraBSet1 << 10;
	mascaraBSet2 += 0b101;
	mascaraBSet2 = mascaraBSet2 << 12;
	// Aplicacion máscara
	faceSet1 = faceSet1 & mascaraBSet1;
	faceSet2 = faceSet2 & mascaraBSet2;

	/*
	 	*C Las posiciones de las caras masculinas que tengan
		* bigote: como las caras masculinas son 0 y necesitamos
		* que donde halla bigote se encuentre 1 utilizaremos una
		* mascara y aplicaremos primero un not para tener los homrbes
		* con 1 (que es lo mismo que usar faceSetxA y luego la
		*  función a.
		*/


	//Mascara


	mascaraCSet1 += 1;
	mascaraCSet1 = mascaraCSet1 << 4;
	mascaraCSet1 += 1;
	mascaraCSet1 = mascaraCSet1 << 3;

	mascaraCSet2 += 1;
	mascaraCSet2 = mascaraCSet2 << 4;
	mascaraCSet2 += 1;
	mascaraCSet2 = mascaraCSet2 << 4;
	mascaraCSet2 += 1;
	//Apliacion máscara
	faceSet1 = faceSet1 & mascaraCSet1;
	faceSet2 = faceSet2 & mascaraCSet2;

	/*
	* D se crean las máscaras de los que pueden ser
	* ingenieros fisico.
	*/

	//mascaras
	mascaraDSet1 += 1;
	mascaraDSet1 = mascaraDSet1 << 3;
	mascaraDSet1 += 1;
	mascaraDSet1 = mascaraDSet1 << 2;
	mascaraDSet1 += 1;
	mascaraDSet1 = mascaraDSet1 << 3;
	mascaraDSet1 += 1;
	mascaraDSet1 = mascaraDSet1 << 4;
	mascaraDSet1 += 1;
	mascaraDSet1 = mascaraDSet1 << 1;
	mascaraDSet1 += 1;
	mascaraDSet1 = mascaraDSet1 << 1;
	mascaraDSet1 += 1;

	mascaraDSet2 += 1;
	mascaraDSet2 = mascaraDSet2 << 5;
	mascaraDSet2 += 1;
	mascaraDSet2 = mascaraDSet2 << 1;
	mascaraDSet2 += 1;
	mascaraDSet2 = mascaraDSet2 << 7;
	mascaraDSet2 += 1;
	mascaraDSet2 = mascaraDSet2 << 1;



	//aplicacion mascara

	faceSet1 = mascaraDSet1 & faceSet1;
	faceSet2 = mascaraDSet2 & faceSet2;

	faceSetFinal += faceSet1;
	faceSetFinal = faceSetFinal << 16;
	faceSetFinal += faceSet2;






	return 0;
}
