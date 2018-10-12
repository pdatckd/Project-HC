static void Task_Check_Sys(void* pvParameters)
{
  while (1)
  {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) {
    Serial.println(F("Task Check is Running"));
    xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    };
   HC01.GetStateSys(&State_Car);
   HC01.Check_DangerState(&Buffer_Danger,&State_Car);
   vTaskDelay(pdMS_TO_TICKS(1000)); 
  }
 }
//-------------------------------------------------------------------
static void Task_Kill_Machine(void* pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, (20000 / portTICK_PERIOD_MS));
    digitalWrite(IFUEL,LOW);
    vTaskDelay(5000 / (portTICK_PERIOD_MS));
    vTaskDelete(TaskHandle_Kill_Machine);
  };
}
//-------------------------------------------------------------------
static void Task_Serial_Data(void* pvParameters)
{
  while (1)
  {
   if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 100 ) == pdTRUE ){
      Serial.println(F("Task Serial is Running"));
      Serial.print(F("RPM:"));
      Serial.print('\0');
      Serial.print(State_Car.rpm);
      Serial.print('\0');
      Serial.print(F("SPD:"));
      Serial.print('\0');
      Serial.print(State_Car.spd);
      Serial.print('\0');
      Serial.print(F("Bat Voltage:"));
      Serial.print('\0');
      Serial.println(State_Car.BatteryVoltage);
      Serial.print(F("_Rev:"));
      Serial.print('\0');
      Serial.println(_Rev,4);
      Serial.print(F("Radius:"));
      Serial.print('\0');
      Serial.println(Radius,4);
      Serial.print(F("Micros:"));
      Serial.print('\0');
      Serial.println(micros());
      Serial.print(F("Temp warning:"));
      Serial.print('\0');
      Serial.println(HC01.Temp_WARN);
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
   vTaskDelay(pdMS_TO_TICKS(3000)); 
  };
}
//-------------------------------------------------------------------
static void Task_Warning(void* pvParameters)
{
  while (1)
  {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) {
    Serial.println(F("Task Warning is Running"));
    xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
   if((Buffer_Danger.TempHot)||(Buffer_Danger.OilPressureLow)){
    digitalWrite(BUZZER,!digitalRead(BUZZER));
    vTaskDelay(pdMS_TO_TICKS(100));
    //xTaskCreate(Task_Kill_Machine, "Task_kill", 100, NULL, 1,  &TaskHandle_Kill_Machine);
   }
   else if (Buffer_Danger.HighCurrent){
    digitalWrite(BUZZER,!digitalRead(BUZZER));
    vTaskDelay(pdMS_TO_TICKS(100));
   }
   else if (Buffer_Danger.HandBrakeON){
    digitalWrite(BUZZER,!digitalRead(BUZZER));
    vTaskDelay(pdMS_TO_TICKS(200));
   }
   else if (Buffer_Danger.VoltEror){
    digitalWrite(BUZZER,!digitalRead(BUZZER));
    vTaskDelay(pdMS_TO_TICKS(500));
   }      
   else{
    digitalWrite(BUZZER,LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));
   }
  }
}
//------------------------------------------------------------------------
static void Task_Caculator_Time(void* pvParameters)
{
  
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount ();
  while (1)
  {
    xLastWakeTime = xTaskGetTickCount ();
      if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 100 ) == pdTRUE ){
      Serial.println(F("Task Caculator Time Engine Running"));
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
  Time_Start_Engine++;
  vTaskDelayUntil( &xLastWakeTime, (1000 / portTICK_PERIOD_MS) );
  };
}
//------------------------------------------------------------------------------
static void Task_CLD(void* pvParameters)
{
  pinMode(LDO,OUTPUT);
  digitalWrite(LDO,HIGH);
  while (1)
  {
      if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 100 ) == pdTRUE ){
      Serial.println(F("Task Check Lock Door"));
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
     if ((State_Car.spd > 10)&&(FLAGDOR == 0)){
      digitalWrite(LDO,LOW);
      FLAGDOR = 1;
     }
     if (State_Car.spd < 10){
      FLAGDOR = 0;
      digitalWrite(LDO,HIGH);
     }
  vTaskDelay(pdMS_TO_TICKS(2000)); 
  };
}

