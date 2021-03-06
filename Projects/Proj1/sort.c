// Z. Weeden Feb. 13, 2017
#include "stm32l476xx.h"
#include "SysClock.h"    //Sysclock
#include "UART.h"        //UART
#include "sort.h"
#include <string.h>
#include <stdio.h>
#define SAMPLES 1001

/* This function traverses a passed integer type array and sorts elements in
ascending order. There is no return value. The array passed is directly maniuplated. */
void sort_array( int array[]) {
    int c;
    int d;
    int swap;
        
    for (c = 0 ; c < ( SAMPLES - 1 ); c++)
    {
        for (d = 0 ; d < SAMPLES - c - 1; d++)
        {
          if (array[d] > array[d+1])
          {
            swap       = array[d];
            array[d]   = array[d+1];
            array[d+1] = swap;
          }
        }
    }
}
