#include <Arduino_FreeRTOS.h>

TaskHandle_t LPT_TaskHandle;
TaskHandle_t HPT_TaskHandle;

SemaphoreHandle_t binSemaphore_A = NULL;

 
void printMsg(TaskHandle_t taskhandle,char str[])  {
        Serial.print(F("Priority "));  // Print task priority 
        Serial.print(uxTaskPriorityGet(taskhandle));
        Serial.print(F(" : "));
        Serial.println(str);        // Print user string
}

void setup()
{  
    Serial.begin(250000);
    Serial.println(F("In Setup function, Creating Binary Semaphore"));

    vSemaphoreCreateBinary(binSemaphore_A);  /* Create binary semaphore */

    if(binSemaphore_A != NULL)
    {
        Serial.println(F("Creating low priority task"));
        xTaskCreate(LPT_Task, "LPT_Task", 100, NULL, 1, &LPT_TaskHandle);
    }
    else
    {
        Serial.println(F("Failed to create Semaphore"));
    }
}


void loop()
{ // Hooked to Idle Task, will run when CPU is Idle
    Serial.println(F("Loop function"));
    delay(50);
}


/*LPT: Low priority task*/
void LPT_Task(void* pvParameters)
{
    printMsg(LPT_TaskHandle,"LPT_Task Acquiring semaphore"); 
    xSemaphoreTake(binSemaphore_A,portMAX_DELAY);

    printMsg(LPT_TaskHandle,"LPT_Task Creating HPT"); 
    xTaskCreate(HPT_Task, "HPT_Task", 100, NULL, 3, &HPT_TaskHandle); 
    
    printMsg(LPT_TaskHandle,"LPT_Task Releasing the semaphore");
    xSemaphoreGive(binSemaphore_A);

    printMsg(LPT_TaskHandle,"LPT_Task Finally Exiting");
    vTaskDelete(LPT_TaskHandle);
}



/*HPT: High priority task*/
void HPT_Task(void* pvParameters)
{
    printMsg(HPT_TaskHandle,"HPT_Task Trying to Acquire the semaphore");
    xSemaphoreTake(binSemaphore_A,portMAX_DELAY);

    printMsg(HPT_TaskHandle,"HPT_Task Acquired the semaphore");    

    printMsg(HPT_TaskHandle,"HPT_Task Releasing the semaphore");
    xSemaphoreGive(binSemaphore_A);    

    printMsg(HPT_TaskHandle,"HPT_Task About to Exit");
    vTaskDelete(HPT_TaskHandle);
}
