/*
 * main2.c
 *
 *  Created on: Mar 2, 2023
 *      Author: daniel
 */

#include <stdint.h>


uint8_t var3 =0;


int main(void){
	uint16_t testShift = 0b0000001010011000;

	while(1){
		testShift = testShift<<2;
		testShift = testShift>>1;

	}

	return 0;
}
