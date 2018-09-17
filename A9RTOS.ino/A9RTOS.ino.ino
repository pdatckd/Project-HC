#include <PDLib.h>
#include <Arduino_FreeRTOS.h>
#include <avr/pgmspace.h>
volatile char   RxDataA9G[100];
volatile char   DataGPS[100];
volatile byte index = 0;
unsigned int LaDD = 0;
unsigned int LaMM = 0;
unsigned int LaMMMM = 0;
float LaM = 0;
float XMM; // Kinh tuyen
float YMM; // Vi tuyen
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;
TaskHandle_t TaskHandle_4;
TaskHandle_t TaskHandle_3;


volatile unsigned int stringComplete = 1;
void setup() {
  Serial.begin(250000);
  Serial1.begin(57600);
  Serial.println(F("In Setup function"));
  HC01.ConfigGPIO();
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  digitalWrite(10, HIGH);
  xTaskCreate(MyTask2, "Task2", 250, NULL, 2, &TaskHandle_2);
}
void loop() {};
static void MyTask2(void* pvParameters)
{
  while (1)
  {
    Serial1.flush();
    switch (stringComplete)
    {
      case 0: Serial1.println(F("AT+LOCATION=2"));
        break;
      case 1: Serial1.println(F("AT"));
        break;
      case 2: Serial1.println(F("AT+GPS=0"));
        break;
      case 3: Serial1.println(F("AT+CREG=1"));
        break;
      case 4: Serial1.println(F("AT+CGATT=1"));
        break;
      case 5: Serial1.println(F("AT+GPS?"));
        break;
      case 6: Serial1.println(F("AT+GPS=1"));
        break;
      case 7: Serial1.println(F("AT+CCLK?"));
        break;
      case 8: Serial1.println(F("AT+LOCATION=2"));
        break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
    if (stringComplete  == 0){
      Serial.println(F("Looking for your location..."));    
      xTaskCreate(MyTask4, "Task4", 250, NULL, 3, &TaskHandle_4);
    }
    else {
      Serial.print(F("Initializing Setting for A9G..."));
      xTaskCreate(MyTask3, "Task3", 250, NULL, 3, &TaskHandle_3);
   }
  }
 }
static void MyTask3(void* pvParameters)
{
  while (1)
  {
    for(int i = 0; i < 100; i++){
      RxDataA9G[i] = '\0';
    };
    index = 0;
    serial1Event();
    Serial.flush();
    for (int i = 0; i < strlen(RxDataA9G); i++) {
      Serial.print(RxDataA9G[i]);
    }
    Serial.println();
    for (int i =  0; i < 100; i++) {
      if (RxDataA9G[i] == 'O'){
        if (RxDataA9G[i + 1] == 'K'){
          stringComplete++;
          break;
        }
      }
    }
    if (stringComplete == 9) {
      stringComplete = 0;
    };
    //Serial.println(F("Task 3 delete it'self"));
    vTaskDelete(TaskHandle_3);
  }
}
static void MyTask4(void* pvParameters)
{ 
  while (1)
  {
    for(int i = 0; i < 100; i++){
      RxDataA9G[i] = '\0';
    };
    index = 0;
    serial1Event();
    Serial.flush();
    for (int i = 0; i < strlen(RxDataA9G); i++) {
      Serial.print(RxDataA9G[i]);
    }
    Serial.println();
    for (int i = 13;i< 33;i++){
      DataGPS[i-13] = RxDataA9G[i];
    };
    //writestring(DataGPS,strlen(DataGPS));
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
    Serial.print(F("Latitude: "));
    Serial.print(XMM,6);
    Serial.print('\t');
    Serial.print(F("Longtitude: "));
    Serial.println(YMM,6);    
    //Serial.println(F("Task 4 delete it'self"));
    vTaskDelete(TaskHandle_4);
    stringComplete = 0;
  }
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
void writestring(char * b,int n){
  Serial.flush();
  for(int i = 0; i<n;i++){
    Serial.print(b[i]); 
  }
  Serial.println();
}

