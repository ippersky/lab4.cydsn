/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "params.h"
#include "queue.h"

volatile SemaphoreHandle_t bouton_semph;
volatile uint8_t flag = 0;

task_params_t task_A = {
    .delay = 1000,
    .message = "Tache A en cours\n\r"
};

task_params_t task_B = {
    .delay = 999,
    .message = "Tache B en cours\n\r"
};



void vLedTask(void *arg){
    for(;;){
        Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void isr_bouton(void){
    xSemaphoreGiveFromISR(bouton_semph, NULL);
    Cy_GPIO_ClearInterrupt(Bouton_0_PORT, Bouton_0_NUM);
    NVIC_ClearPendingIRQ(Bouton_ISR_cfg.intrSrc);
}

void bouton_task(void *arg){
    
    for(;;){
        Cy_SCB_UART_PutString(UART_1_HW, "Bouton relache\n\r");
        if(xSemaphoreTake(bouton_semph, portMAX_DELAY) == pdTRUE){
            Cy_SCB_UART_PutString(UART_1_HW, "Bouton appuye\n\r");
            vTaskDelay(pdMS_TO_TICKS(20));
        }

    }
}

void print_loop(void * data) {
    task_params_t * params = (task_params_t *)data;
    for(;;){
        vTaskDelay(params->delay);
        Cy_SCB_UART_PutString(UART_1_HW, params->message);
    }
}

int main(void){
    bouton_semph = xSemaphoreCreateBinary();
    
    __enable_irq(); /* Enable global interrupts. */
    
   
    Cy_SysInt_Init(&Bouton_ISR_cfg, isr_bouton);
    NVIC_ClearPendingIRQ(Bouton_ISR_cfg.intrSrc);
    NVIC_EnableIRQ(Bouton_ISR_cfg.intrSrc);
   
    
    UART_1_Start();
    Cy_SCB_UART_ClearRxFifo(UART_1_HW);
    
    // Partie 1
    xTaskCreate(vLedTask, "Led Task", 400, NULL, 0, NULL);
    
    // Partie 2
    
    
    xTaskCreate(bouton_task, "bouton Task", 1024, NULL, 1, NULL);
    
    
    // Partie 3
    
    xTaskCreate(print_loop, "task A", configMINIMAL_STACK_SIZE, (void *) &task_A, 1, NULL);
    xTaskCreate(print_loop, "task B", configMINIMAL_STACK_SIZE, (void *) &task_B, 1, NULL);
    
    vTaskStartScheduler();
    
    
    for(;;){
    }

}
