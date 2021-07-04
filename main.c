#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.c"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.c"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/uart.c"
#include "driverlib/pin_map.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


#define LED_PERIPH SYSCTL_PERIPH_GPIOF
#define LED_BASE GPIO_PORTF_BASE
#define RED_LED GPIO_PIN_1
#define BLUE_LED GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3
#define SWITCH GPIO_PIN_4
/**
 * main.c
 */

/*void UARTPrintString(const char* str){
    uint16_t i = 0;
    while(str[i] !='\0'){
        UARTCharPut(UART0_BASE, str[i++]);
    }
}*/

SemaphoreHandle_t serialHandle;
void Task_A(void *pvParameters){    //high priority
    while(1){
        xSemaphoreTake(serialHandle,(TickType_t)portMAX_DELAY); //first block
        //UARTPrintString("The Button has been pushed \n");
        GPIOPinWrite(LED_BASE, RED_LED, RED_LED);
        vTaskDelay(pdMS_TO_TICKS(500));
        GPIOPinWrite(LED_BASE, RED_LED, 0);
    }

}


void Task_B(void *pvParameters){
    char data;
    while(1){
        data = GPIOPinRead(LED_BASE, SWITCH);
        if(data == 0 ){
            xSemaphoreGive(serialHandle);
        }else{
            UARTCharPut(UART0_BASE, 'N');
            vTaskDelay(pdMS_TO_TICKS(5000));
        }

    }

}


int main(void)
{
    SysCtlPeripheralEnable(LED_PERIPH);
     SysCtlDelay(3);
     GPIOPinTypeGPIOOutput(LED_BASE, RED_LED);
     GPIOPinTypeGPIOOutput(LED_BASE, BLUE_LED);
     GPIOPinTypeGPIOOutput(LED_BASE, GREEN_LED);
     GPIOPinTypeGPIOInput(LED_BASE, SWITCH);
     GPIOPadConfigSet(LED_BASE, SWITCH, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

     /*external code*/
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
         while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
         GPIOUnlockPin(LED_BASE, RED_LED|BLUE_LED|GREEN_LED);
         GPIOPinTypeGPIOOutput(LED_BASE, RED_LED|BLUE_LED|GREEN_LED);

         GPIOPinConfigure(GPIO_PA0_U0RX);
         GPIOPinConfigure(GPIO_PA1_U0TX);
         GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);

         SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
         while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

         uint32_t freq = SysCtlClockGet();

         UARTConfigSetExpClk(UART0_BASE, freq, 9600, UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE);
         char str[] = "Press the Button: ";
                uint32_t i = 0;
                while(str[i] != '\0'){
                    UARTCharPut(UART0_BASE, str[i]);
                    i++;
                }
       /*end of external code*/

         TaskHandle_t A,B;
         if( serialHandle = xSemaphoreCreateBinary()){
             xTaskCreate(Task_A, "A", 128, NULL, 2, &A);
             xTaskCreate(Task_B, "B", 128, NULL, 1, &B);
             vTaskStartScheduler();
         }
         while(1);
	return 0;
}
