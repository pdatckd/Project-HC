//--------------------------------------------------------------
#include <ArduinoJson.h>
#include <A9G.h>
int getMemoryFree() {
  extern int __heap_start;
  extern int *__brkval; 
  return (int) SP - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

static void prvAutoReloadCallback3(TimerHandle_t xTimer)
{
  BaseType_t qStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  //Để đảm bảo ổn định app phải gửi dữ liệu với khoảng cách thời gian 2 lần liên tiếp là tối thiểu 150ms.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ){
        Serial.println(F("Timer 3 is running"));
        xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
  memset(RxDataA9G, '\0', 70);
  while (Serial1.available()) {
    char c = (char)Serial1.read();
    RxDataA9G[index9++] = c;
    if (Serial1.available() == 0) {
      RxDataA9G[index9] = '\0';
      qStatus = xQueueSend(A9GQueue, &RxDataA9G[0], 10);
      if (qStatus !=  pdPASS) {
      };
      index9 = 0;
      xEventGroupSetBits(xEvent_Receive_Group, RECEIVE_COMPLETE_BIT);
    };
  }
  free(RxDataA9G);
}
//---------------------------------------------------------------
static void Task_SendTCP(void* pvParameters)
{
  vTaskSuspend(TaskHandle_Local);
  state = Idle;
  while (1)
  {
    portDISABLE_INTERRUPTS();
    switch (state)
    {
      case Idle :
        Serial1.flush();
        Serial1.println(F("AT+CIPSTART=\"TCP\",\"138.68.83.109\",1334"));
        state = SendedTCPStart;
        break;
      case SendedTCPStart :
        Serial1.flush();
        Serial1.println(F("AT+CIPSEND"));
        state = SendedTCPsend; 
        break;
      case SendedTCPsend :
        Serial1.flush();
        Serial1.println(Jsonstring);
        state = SendedData;
        break;
      case SendedData :
        Serial1.flush();
        Serial1.write(0x1A);
        state = DoNothing;
        break;
    }
      vTaskDelay(pdMS_TO_TICKS(500));
      if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 1000 ) == pdTRUE ) {
            Serial.print(F("Send TCP is running!:"));
            Serial.println(state);
            xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
      }
      if(state == DoNothing){
        vTaskResume(TaskHandle_Local);
        vTaskDelete(TaskHandle_SendWeb); 
      }
  }
}
//-------------------------------------------------------
static void MyTask6(void* pvParameters)
{
  char   DataGPSLat[11];
  char   DataGPSLon[11];
  while(1)
  {
  memset(Jsonstring, '\0', 200);
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& data = root.createNestedArray("GPS");

  dtostrf((double)XMM,3,6,DataGPSLat);
  dtostrf((double)YMM,3,6,DataGPSLon);
    
  data.add(DataGPSLat);  // thêm 2 tọa độ vào Data
  data.add(DataGPSLon);
  root["a1"]      = State_Car.OilPres;       // Throttleposition       // các du lieu dc truyen vao
  root["a2"]      = State_Car.Handbrake;       // Intemperature
  root["a3"]      = State_Car.EngineCoolantTemp;       // Temp
  root["io12"]    = State_Car.rpm;     // RPM
  root["io13"]    = State_Car.BatteryVoltage;     // Voltage
  root["io14"]    = State_Car.spd;     // MAF
  root["io15"]    = Time_Start_Engine;     // Timingadvance
  root["io16"]    = random(20);     // Fuelinjectiontiming
  root["d1"]      = random(100);       // Engineoiltemperature
  root["d2"]      = random(5);      // Number of DTC
  int sum_DTC     = 0;  
  
  JsonArray& dtc = root.createNestedArray("dtc");
  for (int i = 0; i < sum_DTC; i ++) {  // đưa mã lỗi lên web
  }
  //root["d3"] = *(pOBD + 11);       // DTCs
  root["d3"] = 0;
  root["d4"] = 0;
  root["d5"] = 0;
  root.printTo(Jsonstring); // lưu chuối Json vừa tạo vào chuỗi Jsonstring
  if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 1000 ) == pdTRUE ) {
    Serial.flush();
    Serial.print(F("Jsonstring: "));
    for (int i = 0; i <= 200; i++) {
      Serial.print(Jsonstring[i]);
    }
    Serial.println();
    xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
  }
  vTaskDelete(TaskHandle_JS);    
  }
}
//----------------------------------------------------------
static void Task_Handle_Local(void* pvParameters)
{
  BaseType_t qStatus;
  char * Data =(char *) malloc(70); 
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  const EventBits_t xBitsToWaitFor  = RECEIVE_COMPLETE_BIT;
  EventBits_t xEventGroupValue;
  while (1)
  {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 1000 ) == pdTRUE ) {
    Serial.println(F("Task 5 is Running"));
    xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    };
   WAITING:
    xEventGroupValue  = xEventGroupWaitBits(xEvent_Receive_Group,
                                            xBitsToWaitFor,
                                            pdTRUE,
                                            pdFALSE,
                                            pdMS_TO_TICKS(5000)
                                           );
    if ((xEventGroupValue & RECEIVE_COMPLETE_BIT) != 0) {
      qStatus =  xQueueReceive(A9GQueue, &Data[0], pdMS_TO_TICKS(100));
      if (qStatus  == pdPASS) {
        // Chuỗi GPS chuẩn thì ký tự 35 Là N, 48 là E
        if ((strstr(Data, "+GPSRD") != NULL)&&((Data[35] == 'N')&&(Data[48] == 'E'))){
          if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 1000 ) == pdTRUE ) {
            for (int i = 0; i < strlen(Data); i++) {
              Serial.print(Data[i]);
            }
            Serial.println(F("Read Succesful!"));
            xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
          };
          LaDD = strtol(&Data[25],Data[29],10)/100;
          LaMM = strtol(&Data[25],Data[29],10)%100;
          LaMMMM = strtol(&Data[30],Data[34],10);
          LaM = (float)LaMM*10000 + (float)LaMMMM;
          LaM  /= 600000.0;
          XMM   = (float)LaDD+LaM;
          LaDD = strtol(&Data[37],Data[42],10)/100;
          LaMM = strtol(&Data[37],Data[42],10)%100;
          LaMMMM = strtol(&Data[43],Data[47],10);
          LaM = (float)LaMM*10000 + (float)LaMMMM;
          LaM  /= 600000.0;
          YMM   = (float)LaDD+LaM;          
        }
      }
    }
    free(Data);
    vTaskDelay(pdMS_TO_TICKS(500));
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 700 ) == pdTRUE ) {
      Serial.print(F("Lat: "));
      Serial.println(XMM,6);
      Serial.print(F("Lon: "));
      Serial.println(YMM,6);      
      Serial.println(getMemoryFree()); 
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
    if ((YMM == 0)&&(XMM == 0)){
      goto WAITING;
    }
    xTaskCreate(MyTask6, "Task WrapJS", 800, NULL, 2,  &TaskHandle_JS);
    vTaskDelay(pdMS_TO_TICKS(500));
    xTaskCreate(Task_SendTCP, "Tasksendweb", 500, NULL, 2,  &TaskHandle_SendWeb);
  };
}
//---------------------------------------------------------------------
static void Task_SetupA9G(void* pvParameters)
{
  BaseType_t qStatus;
  const EventBits_t xBitsToWaitFor  = RECEIVE_COMPLETE_BIT;

  const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
  EventBits_t xEventGroupValue;

  char Rx[70];
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  byte count_wait = 0;
  vTaskSuspend(TaskHandle_Local);
  vTaskSuspend(TaskHandle_HC);
  vTaskSuspend(TaskHandle_Warn);
  vTaskSuspend(TaskHandle_Ble);
  vTaskSuspend(TaskHandle_3);
  vTaskSuspend(TaskHandle_CTime);
  vTaskSuspend(TaskHandle_Door);
  Serial.println(F("Initializing..."));
  while (1)
  {
    int count_wait = 0;
Lebal:
    Serial1.flush();
    switch (stringComplete)
    {
      case 0: Serial1.println(F("AT+RST=1"));
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
      case 4: Serial1.println(F("AT+CGDCONT=1,\"IP\",\"V-INTERNET\""));
        count_respone++;
        break;
      case 5: Serial1.println(F("AT+CGACT=1,1"));
        count_respone++;
        break;
      case 6: Serial1.println(F("AT+GPS?"));
        count_respone++;
        break;
      case 7: Serial1.println(F("AT+GPS=1"));
        count_respone++;
        break;
      case 8: Serial1.println(F("AT+GPSRD=1"));
        count_respone++;
        break;
    }
Lebal1:
    count_wait++;// đếm số lần chờ.
    xEventGroupValue  = xEventGroupWaitBits(xEvent_Receive_Group,
                                            xBitsToWaitFor,
                                            pdTRUE,
                                            pdFALSE,
                                            xTicksToWait
                                           );
    if ((stringComplete  >= 9) && (count_respone <= 20)) {
      if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) {
        Serial.flush();
        Serial.println(F("Successed !"));
        xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
      }
      vTaskResume(TaskHandle_Local);
      vTaskResume(TaskHandle_HC);
      vTaskResume(TaskHandle_Warn);
      vTaskResume(TaskHandle_Ble);
      vTaskResume(TaskHandle_3);
      vTaskResume(TaskHandle_CTime);
      vTaskResume(TaskHandle_Door);
      vTaskDelete(TaskHandle_A9G);
    }
    else if ((stringComplete  < 9) && (count_respone > 50)) {
      // Reset and Waiting a respone of A9G.
      digitalWrite(10, LOW);
      vTaskDelay(pdMS_TO_TICKS(200));
      digitalWrite(10, HIGH);
      vTaskDelay(pdMS_TO_TICKS(1000));
      //Reset variable count;
      stringComplete = 0;
      count_respone  = 0;
    }
    else {
      if ((xEventGroupValue & RECEIVE_COMPLETE_BIT) != 0) {
        qStatus =  xQueueReceive(A9GQueue, &Rx[0], pdMS_TO_TICKS(1000));
        if (qStatus  == pdPASS) {
            switch (stringComplete)
            {
              case 0:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
              break;
              case 1:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+GPS=0") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
              case 2:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+CREG=1") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
              case 3:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+CGATT=1") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
              case 4:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+CGDCONT=1,\"IP\",\"V-INTERNET\"") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
              case 5:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+CGACT=1,1") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
              case 6:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+GPS?") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
              case 7:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+GPS=1") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
              case 8:if ((strstr(Rx, "OK") != NULL)&&(strstr(Rx,"AT+GPSRD=1") != NULL)) {
                        stringComplete++;
                        count_respone = 0;
              }
                break;
            }
          if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) {
            for (int i = 0; i < strlen(Rx); i++) {
              Serial.print(Rx[i]);
            }
            Serial.println(F("Read Succesful!"));
            xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
          };
        }
      }
      else {
        if (count_wait > 5) {
          goto Lebal;
        }
        else {
          goto Lebal1;
        }
      }
    }
  }
}


