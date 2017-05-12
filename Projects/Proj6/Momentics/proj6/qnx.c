// Z. Weeden 2017

#include <stdio.h>
#include <stdlib.h>       /* for EXIT_* */
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/neutrino.h> /* for ThreadCtl() */
#include <sys/syspage.h>  /* for for cycles_per_second */
#include <sys/mman.h>     /* for mmap_device_io() */
#include <unistd.h>       /* for sleep() */

#define IO_PORT_SIZE                (1)
#define BASE_ADDRESS                (0x280) //QNX base address - a/d lsb
#define A_D_MSB                     (BASE_ADDRESS+1)
#define A_D_CHANNEL                 (BASE_ADDRESS+2)
#define A_D_GAIN_STATUS             (BASE_ADDRESS+3)
#define INTRPT_DMA_CONTROL_COUNTER  (BASE_ADDRESS+4)
#define FIFO_THRESHOLD              (BASE_ADDRESS+5)
#define D_A_LSB                     (BASE_ADDRESS+6)
#define D_A_MSB                     (BASE_ADDRESS+7)
#define PORT_A_OUT                  (BASE_ADDRESS+8)
#define PORT_B_OUT                  (BASE_ADDRESS+9)
#define PORT_C_OUT                  (BASE_ADDRESS+0xA)
#define DIRECTION_CONTROL           (BASE_ADDRESS+0xB)
#define COUNTER_TIMER_B0            (BASE_ADDRESS+0xC)
#define COUNTER_TIMER_B1            (BASE_ADDRESS+0xD)
#define COUNTER_TIMER_B2            (BASE_ADDRESS+0xE)
#define COUNTER_TIMER_CONTROL       (BASE_ADDRESS+0xF)

int LSB, MSB = 0;
long a_d_val = 0;
double volts;
uintptr_t baseHandle, adMSBHandle, adChannelHandle, adGainStatusHandle, portAHandle, portBHandle, dataDirectionHandle;

/* Request I/O access permission - needed for accessing registers. */
void request_access_permission(void){
    if ( ThreadCtl(_NTO_TCTL_IO, NULL) == -1){
        perror("Failed to get I/O access permission");
        return 1;
    }
}

/* Map ports into address space so we can observer/write to them using handler pointers. */
void map_ports(void){
    baseHandle = mmap_device_io(IO_PORT_SIZE, BASE_ADDRESS);
    if(baseHandle == MAP_DEVICE_FAILED){
        perror("Failed to map base addr");
        return 2;
    }

    adGainStatusHandle = mmap_device_io(IO_PORT_SIZE, A_D_GAIN_STATUS);
    if(adGainStatusHandle == MAP_DEVICE_FAILED){
        perror("Failed to map A/D gain status register");
        return 2;
    }

    adChannelHandle = mmap_device_io(IO_PORT_SIZE, A_D_CHANNEL);
    if(adChannelHandle == MAP_DEVICE_FAILED){
        perror("Failed to map A/D channel register");
        return 2;
    }

    adMSBHandle = mmap_device_io(IO_PORT_SIZE, A_D_MSB);
    if(adMSBHandle == MAP_DEVICE_FAILED){
        perror("Failed to map A/D MSB register");
        return 2;
    }

    portAHandle = mmap_device_io(IO_PORT_SIZE,PORT_A_OUT);
    if(portAHandle == MAP_DEVICE_FAILED){
        perror("Failed to map port a register");
        return 2;
    }

    portBHandle = mmap_device_io(IO_PORT_SIZE,PORT_B_OUT);
    if(portBHandle == MAP_DEVICE_FAILED){
        perror("Failed to map port b register");
        return 2;
    }

    dataDirectionHandle = mmap_device_io(IO_PORT_SIZE,DIRECTION_CONTROL);
    if(dataDirectionHandle == MAP_DEVICE_FAILED){
        perror("Failed to map data direction register");
        return 2;
    }
}


/* Preliminary setup for a/d conversion. Set pins to search for signal and choosing input range for bipolar from -5V to 5V */
void analog_to_digital_setup(void){
    //Select input channel
    out8(adChannelHandle,0xF0); //1111 0000 Read channels 0 through 15

    //Select input range
    out8(adGainStatusHandle, 0x01); //0000 0001 bipolar +-5V gain of 2
}

/* Convert signal from analog to digital by writing proper values to registers. Scales A/D code back to voltage. Prints out
   bar graph that represents input signal. Returns double type voltage. */
double analog_to_digital(void){
    int i;
    //Wait for analog circuit to settle
    while( (in8(adGainStatusHandle) & 0x20) ){ //base+3 bit 5 is not less than 32 0010 0000 - subject to 'hardware fault'
        ;                                   //A/D is setting new value
    }
    //bit 5 went low - ok to start conversion

    //Initiate conversion
    out8(baseHandle,0x80);                //1000 0000 STRTAD start A/D

    //Wait for conversion to finish
    while( (in8(adGainStatusHandle) & 0x80) ){ //base+3 bit 7 is not less than 128 1000 0000 - subject to 'hardware fault'
        ;                                   //converstion still in progress
    }
    //bit 7 went low conversion complete

    //Resolving adc value
    LSB = in8(baseHandle);
    MSB = in8(adMSBHandle);
    a_d_val = MSB * 256 + LSB; //essentially shifts MSB over 8bits and appends the lsb
    volts = (a_d_val/32768.0)*5.0;
    if (volts > 5.0 || volts < -5.0){
        ;
    }
    else{
        for (i=0; i<10; i++){
            if (i-5 > (int)volts){
                printf(' ');
            }
            else{
                printf('|');
            }
        }
        printf('\n');
    }
    return volts;
}

/* Function to output the parameter value to portA. This parameter is the return value of analog_to_digital(). */
void output_to_stm(double convertedAD){
    out8(portAHandle, convertedAD);
}

/* This function serves to scale the voltage so that no negative numbers are seen by the STM. Voltage now ranges from 0V to 20V. */
double scale(double volts){
    return (volts+5.0)*2.0;
}