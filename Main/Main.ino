#include "var.h"
#include "HC_OS.h"
#include "BLE_OS.h"
#include "A9G_OS.h"
#include <portmacro.h>

void setup() {
  Serial.begin(250000);
  Serial1.begin(115200);
  Serial2.begin(9600);
  Wire.begin();
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(10, HIGH);
  digitalWrite(11, LOW);
  delay(2000);
  digitalWrite(11, HIGH);
//  HC01.Setting_Defauts();
  HC01.ReadDataBoard();
  HC01.ConfigGPIO();
  xEvent_Receive_Group =  xEventGroupCreate();
  xTimer2  = xTimerCreate("Auto-Reload",AUTO_RELOAD_TIMER_PERIOD,pdTRUE,0,prvAutoReloadCallback);
  // Tạo một timer bằng software timer tự động reload lại giá trị.
  // Với giá trị định thời là AUTO_RELOAD_TIMER_PERIOD
  // prvAutoReloadCallback là trình phục vụ ngắt tràn của xTimer2.
  if(xTimer2 !=NULL){
    //Kiểm tra xem timer xTimer2 đã được tạo chưa. Nếu  rồi thì khởi động timer.
   xTimer2Started =  xTimerStart(xTimer2,0);
  }
  while( xTimer2Started !=pdPASS){} 
  xTimer3  = xTimerCreate("Auto-Reload-T3",AUTO_RELOAD_TIMER_PERIOD3,pdTRUE,0,prvAutoReloadCallback3);
  // Tạo một timer bằng software timer tự động reload lại giá trị.
  // Với giá trị định thời là AUTO_RELOAD_TIMER_PERIOD
  // prvAutoReloadCallback là trình phục vụ ngắt tràn của xTimer2.
  if(xTimer3 !=NULL){
    //Kiểm tra xem timer xTimer2 đã được tạo chưa. Nếu  rồi thì khởi động timer.
   xTimer3Started =  xTimerStart(xTimer3,0);
  }
  while( xTimer3Started !=pdPASS){} 
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
  BLEQueue  = xQueueCreate(10,12);//xQueueCreate(Số từ của hàng đợi là 10, Mỗi phần tử chứa được 12 byte bộ nhớ)
  A9GQueue  = xQueueCreate(10,70);//xQueueCreate(Số từ của hàng đợi là 10, Mỗi phần tử chứa được 70 byte bộ nhớ)
  xTaskCreate(Task_Ble, "Task Serial BLE", 200, NULL, 1, &TaskHandle_Ble);
  xTaskCreate(Task_Check_Sys, "Task Check Systems", 200, NULL, 1,&TaskHandle_HC);
  xTaskCreate(Task_Warning, "Task Warning", 100, NULL, 1,  &TaskHandle_Warn);
  xTaskCreate(Task_SetupA9G, "Task Setup A9G", 200, NULL, 2,  &TaskHandle_A9G);
  xTaskCreate(Task_Handle_Local, "Task 5", 500, NULL, 1,  &TaskHandle_Local);
  xTaskCreate(Task_Serial_Data, "Task3", 100, NULL, 1,  &TaskHandle_3);
  xTaskCreate(Task_Caculator_Time, "Check time", 100, NULL, 1,  &TaskHandle_CTime);
  xTaskCreate(Task_CLD, "Check CLK", 100, NULL, 1,  &TaskHandle_Door);
}
void loop(){
  //Serial.println(F("Loop Function"));
}





