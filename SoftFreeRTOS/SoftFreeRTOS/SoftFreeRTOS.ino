#include <PDLib.h>
#include <Arduino_FreeRTOS.h>

volatile char   RxDataA9G[100];
volatile char   DataGPS[100];
volatile byte index = 0;
unsigned int LaDD = 0;
unsigned int LaMM = 0;
unsigned int LaMMMM = 0;
float LaM = 0;
float XMM; // Kinh tuyen
float YMM; // Vi tuyen
volatile unsigned int stringComplete = 1;
volatile unsigned int count_respone = 0;
unsigned int CycleW = 1000;
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;
TaskHandle_t TaskHandle_3;
TaskHandle_t TaskHandle_4;
TaskHandle_t TaskHandle_5;
TaskHandle_t TaskHandle_6;

SemaphoreHandle_t xSerialSemaphore;
SemaphoreHandle_t xSerial1Semaphore;
StateSys stateS;
Buffer_DangerState ErorS;

void MyTask1( void *pvParameters );
void MyTask2( void *pvParameters );
void MyTask3( void *pvParameters );
void MyTask4( void *pvParameters );
void MyTask5( void *pvParameters );
void MyTask6( void *pvParameters );

void setup() {
  Serial.begin(250000);
  Serial1.begin(57600);
  Serial.println(F("In Setup function"));
  HC01.ConfigGPIO();
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  digitalWrite(10, HIGH);
  //HC01.Setting_Defauts();
  HC01.ReadDataBoard();
  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  };
  if ( xSerial1Semaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerial1Semaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerial1Semaphore ) != NULL )
      xSemaphoreGive( ( xSerial1Semaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  };  
  xTaskCreate(MyTask1, "Task1", 100, NULL, 1,  &TaskHandle_1);
  xTaskCreate(MyTask2, "Task2", 100, NULL, 1,  &TaskHandle_2);
  xTaskCreate(MyTask3, "Task3", 100, NULL, 1,  &TaskHandle_3);
  xTaskCreate(MyTask4, "Task4", 100, NULL, 2,  &TaskHandle_4);
  xTaskCreate(MyTask6, "Task6", 100, NULL, 1,  &TaskHandle_6);
  vTaskStartScheduler();
}
void loop() {
}
static void MyTask1(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  while (1)
  {
   HC01.GetStateSys(&stateS);
   HC01.Check_DangerState(&ErorS,&stateS);
   if(ErorS.TempHot == 1){
    if(eTaskGetState(TaskHandle_2) == eSuspended){
    vTaskResume(TaskHandle_2);
    CycleW = 200;
    }
   }
   else if (ErorS.OilPressureLow){
    if(eTaskGetState(TaskHandle_2) == eSuspended){
    vTaskResume(TaskHandle_2);
    CycleW = 200;    
    }
   }
   else if (ErorS.HighCurrent){
    if(eTaskGetState(TaskHandle_2) == eSuspended){
    vTaskResume(TaskHandle_2);
    CycleW = 200;      
    }
   }
   else if (ErorS.HandBrakeON){
    if(eTaskGetState(TaskHandle_2) == eSuspended){
    vTaskResume(TaskHandle_2);
    CycleW = 400;        
    }
   }
   else if (ErorS.VoltEror){
    if(eTaskGetState(TaskHandle_2) == eSuspended){
    vTaskResume(TaskHandle_2);
    CycleW = 500;        
    }
   }      
   else{
    if(eTaskGetState(TaskHandle_2) != eSuspended){
    vTaskSuspend(TaskHandle_2);
    }
    digitalWrite(BUZZER,LOW);
   };
   vTaskDelayUntil( &xLastWakeTime, (1000 / portTICK_PERIOD_MS) );
  }
 }
//-------------------------------------------------------------------
static void MyTask2(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  while (1)
  {
  digitalWrite(BUZZER,!digitalRead(BUZZER));
  vTaskDelayUntil( &xLastWakeTime, (CycleW / portTICK_PERIOD_MS) );
  };
}
//---------------------------------------------------------------------
static void MyTask3(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  while (1)
  {
   if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ){
      Serial.print(F("RPM:"));
      Serial.print('\0');
      Serial.print(stateS.rpm);
      Serial.print('\0');
      Serial.print(F("Battery Voltage:"));
      Serial.print('\0');
      Serial.println(stateS.BatteryVoltage);
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
   vTaskDelayUntil( &xLastWakeTime, (1000 / portTICK_PERIOD_MS) );
  };
}
///----- Task setup A9G.
static void MyTask4(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  vTaskSuspend(TaskHandle_1);
  vTaskSuspend(TaskHandle_2);
  vTaskSuspend(TaskHandle_3);
  Serial.println(F("Initializing..."));
  while (1)
  {
    Serial1.flush();
    switch (stringComplete)
    {
      case 0: Serial1.println(F("AT"));
        count_respone++;
        break;
      case 1: Serial1.println(F("AT+GPS=0"));
        count_respone++;
        break;
      case 2: Serial1.println(F("AT+CREG=1"));
        count_respone++;
        break;
      case 3: Serial1.println(F("AT+CGATT=1"));
        count_respone++;
        break;
      case 4: Serial1.println(F("AT+GPS?"));
        count_respone++;
        break;
      case 5: Serial1.println(F("AT+GPS=1"));
        count_respone++;
        break;
      case 6: Serial1.println(F("AT+LOCATION=2"));
        count_respone++;
        break;
    }
    vTaskDelayUntil( &xLastWakeTime, (100 / portTICK_PERIOD_MS) );
    if ((stringComplete  > 6)&&(count_respone <= 20))
    {
        Serial.println(F("Successed !")); 
        xTaskCreate(MyTask5, "Task5", 100, NULL, 1,  &TaskHandle_5);
        vTaskResume(TaskHandle_1);
        vTaskResume(TaskHandle_3);
        vTaskDelete(TaskHandle_4);
    }
    else if((stringComplete  < 6)&&(count_respone > 20)){
        // Reset and Waiting a respone of A9G.
        digitalWrite(10, LOW);
        vTaskDelayUntil( &xLastWakeTime, (200 / portTICK_PERIOD_MS) );
        digitalWrite(10, HIGH);
        vTaskDelayUntil( &xLastWakeTime, (1000 / portTICK_PERIOD_MS) );
        //Reset variable count;
        stringComplete = 0;  
        count_respone  = 0;
    }
    else {
    for(int i = 0; i < 100; i++){
      RxDataA9G[i] = '\0';
    };
    index = 0;
    serial1Event();
    for (int i =  0; i < 100; i++) {
      if (RxDataA9G[i] == 'O'){
        if (RxDataA9G[i + 1] == 'K'){
          stringComplete++;
          count_respone  = 0;
          break;
        }
      }
    }
    }
  }
}
static void MyTask5(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  while (1)
  {
    Serial1.flush();
    Serial1.println(F("AT+LOCATION=2"));
    vTaskDelayUntil( &xLastWakeTime, (100 / portTICK_PERIOD_MS) );
    for(int i = 0; i < 100; i++){
      RxDataA9G[i] = '\0';
    };
    index = 0;
    serial1Event();
    for (int i = 13;i< 33;i++){
      DataGPS[i-13] = RxDataA9G[i];
    };
    LaDD = strtol(&DataGPS[0],DataGPS[4],10)/100;
    LaMM = strtol(&DataGPS[0],DataGPS[4],10)%100;
    LaMMMM = strtol(&DataGPS[5],DataGPS[10],10);
    LaM = (float)LaMM*10000 + (float)LaMMMM;
    LaM  /= 600000.0;
    XMM   = (float)LaDD+LaM;
    LaDD = strtol(&DataGPS[10],DataGPS[14],10)/100;
    LaMM = strtol(&DataGPS[10],DataGPS[14],10)%100;
    LaMMMM = strtol(&DataGPS[16],DataGPS[20],10);
    LaM = (float)LaMM*10000 + (float)LaMMMM;
    LaM  /= 600000.0;
    YMM   = (float)LaDD+LaM;
    
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ){
      Serial.print(F("Latitude: "));
      Serial.print(XMM,6);
      Serial.print('\t');
      Serial.print(F("Longtitude: "));
      Serial.println(YMM,6);
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
    vTaskDelayUntil( &xLastWakeTime, (100 / portTICK_PERIOD_MS) );
  };
}
//-------------------------------------------------------------------
static void MyTask6(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  while (1)
  {
   vTaskDelayUntil( &xLastWakeTime, (200 / portTICK_PERIOD_MS) );
  };
}
void serial1Event(){
  while (Serial1.available()) {
    char c = (char)Serial1.read();
    if ((c == '\n')||(c == '\r')){}
    else{
      RxDataA9G[index++] = c;
    };
  }
}


