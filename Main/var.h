#include <PDLib.h>
#include <A9G.h>
#include <ArduinoJson.h>
#include <Arduino_FreeRTOS.h>
#include <timers.h>
#include <queue.h>
#include <event_groups.h>
#include <task.h>

#ifdef __AVR__
#include <avr/io.h>
#include <avr/pgmspace.h> 
#endif

#include <stdlib.h>


State_Vehicle State_M;
Buffer_DangerState Buffer_Danger;
StateSys State_Car;

unsigned int CycleW = 1000; //  Biến set tần số cảnh báo.


#define RECEIVE_COMPLETE_BIT (1U << 0U)
EventGroupHandle_t  xEvent_Receive_Group;

#define AUTO_RELOAD_TIMER_PERIOD  (pdMS_TO_TICKS(150))
#define AUTO_RELOAD_TIMER_PERIOD3  (pdMS_TO_TICKS(200))
static char RxData[12];
byte index = 0;


byte FLAGDOR = 0;


char * RxDataA9G =(char *) malloc(70); 
volatile byte index9 = 0;
volatile unsigned int stringComplete = 0;
volatile unsigned int count_respone = 0;

unsigned long Time_Start_Engine = 0;

TaskHandle_t  TaskHandle_Ble;
TaskHandle_t TaskHandle_HC;
TaskHandle_t TaskHandle_Warn;
TaskHandle_t TaskHandle_A9G;
TaskHandle_t TaskHandle_Local;
TaskHandle_t TaskHandle_SendWeb;
TaskHandle_t TaskHandle_JS;
TaskHandle_t TaskHandle_3;
TaskHandle_t TaskHandle_Kill_Machine;
TaskHandle_t TaskHandle_CTime;
TaskHandle_t TaskHandle_Door;



QueueHandle_t BLEQueue;
QueueHandle_t A9GQueue;
TimerHandle_t  xTimer2;
BaseType_t  xTimer2Started;


TimerHandle_t  xTimer3;
BaseType_t  xTimer3Started;
SemaphoreHandle_t xSerialSemaphore;
SemaphoreHandle_t xSerial1Semaphore;

char Jsonstring[200];  // send this String

volatile SendTCP_State state  = DoNothing;  

bool EventSerialA9G = true;

unsigned int LaDD = 0;
unsigned int LaMM = 0;
unsigned int LaMMMM = 0;
float LaM = 0;
float XMM; // Kinh tuyen
float YMM; // Vi tuyen

