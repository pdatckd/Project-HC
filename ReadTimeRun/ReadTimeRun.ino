#include <SoftwareSerial.h>
#include <PDLib.h>
SoftwareSerial     EEBlue(12, 13);

#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "PCD8544_HOANGSA.h"
#include "var.h"

PCD8544 lcd(3,4,5,6);//RST,D/C, Din,CLK
State_Vehicle A;
Buffer_DangerState B;
StateSys StateV;

void setup() {
  //Serial.begin(115200);
  EEBlue.begin(9600);
  Wire.begin();
  lcd.ON();
  lcd.SET(60,0,0,0,4);
//  HC01.Setting_Defauts();
  HC01.ReadDataBoard();
  HC01.ConfigGPIO();
  WDT_Init();
  state_display = DisplayClear;
}
void loop(){
  if ((millis() - count3) > 100){
    Read_button();
    count1 = millis();
  }
wdt_reset();
  if ((millis() - count4) > 200){
    switch (state_display)
        {
          case DisplayClear: lcd.clear();
          lcd.display();
          break;
          case DisplayInforsystem:lcd.clear();
          Display_Ifor(HC01.ReadTemp(),HC01.ReadBatVolt());
          break;
          case DisplayOdotime:lcd.clear();
          lcd.Asc_String(0, 0, Asc("ODO TIME:"),BLACK); 
          Display_Uptime(HC01.TimeRun);
          break;
          case DisplayTriptime: lcd.clear();
          lcd.Asc_String(0, 0, Asc("TRIP TIME:"), BLACK); 
          Display_Uptime(HC01.TripTimeRun);
          break;
          case DisplaySetWarning: SetWarningParameter();
          break;
        }
    count4 = millis();
  }
wdt_reset();
  if ((millis() - count5) > countS){
    A = HC01.GetStateVehicle();
    if ((A == Operating)||(A == IG_ON)){
        HC01.BeginC_TimeRun();
        countS = 1000;
        if (FlagFirstStart == 0){
        state_display = DisplayInforsystem;// Đưa màn hình về hiển thị thông tin.
        FlagFirstStart  = 1; // Set cờ khởi động lên mục đích để tác vụ chỉ làm 1 lần.
        countBD = 0; // Đưa biến đếm số lần cảnh báo bảo dưỡng cho lần khởi động đầu tiên.
        WDT_Init(); 
        };
    }
    if(A == IG_OFF){
        HC01.EndC_TimeRun();
        if(FlagFirstStart == 1){
        lcd.clear();
        lcd.display();
        };
        count3 = millis();
        count4 = millis();
        count6 = millis();
        count7 = millis();
        count8 = millis();
        WDTCSR = (0<<WDCE)|(0<<WDE);
        countS = 50;
        FlagFirstStart  = 0;
    }
    count5 = millis();
  }
wdt_reset();
  if ((millis() - count6) > countW){
    HC01.GetStateSys(&StateV);
    HC01.Check_DangerState(&B,&StateV);
    if(B.TempHot == 1){
      digitalWrite(BUZZER,!digitalRead(BUZZER));
      countW = 100;
    }
    else if(B.OilPressureLow == 1){
      digitalWrite(BUZZER,!digitalRead(BUZZER));
      countW = 100;
    }
    else if(B.VoltEror == 1){
      digitalWrite(BUZZER,!digitalRead(BUZZER));
      countW = 600;
    }
    else if((B.MaintenanceTime == 1)&&(countBD < 50)){
      digitalWrite(BUZZER,!digitalRead(BUZZER));
      countW = 400;
      countBD++;
    }  
    else{
      digitalWrite(BUZZER,HIGH);
      countW = 1000;
    };
    count6 = millis();
  }
wdt_reset();
  if ((millis() - count7) > 2000){
    if ((B.TempHot == 1)||(B.OilPressureLow == 1)){
        if(FlagI == 0){
        TimeI = HC01.GetTimeRunning();
        FlagI = 1;
        }
    }else{
        TimeI = HC01.GetTimeRunning();
        FlagI = 0;
    };
    if ((HC01.GetTimeRunning() - TimeI) >= 20){
      Interrupt:
      digitalWrite(IFUEL,LOW);
      wdt_reset();
      if (HC01.ReadBatVolt() <= 5.0){
        digitalWrite(IFUEL,HIGH);
      }
      else{
        goto Interrupt;
      };
    }
    else{
      digitalWrite(IFUEL,HIGH);
    }
    count7 = millis();
  }
wdt_reset();
  if ((millis() - count8) > 300){
    EEBlueEvent();
    HandingString(Receive);
    Receive = "";
    count8 = millis();
  }
wdt_reset();
}
void Read_button(void){
  unsigned long timetam = 0;
  switch (digitalRead(BUTTON)){
      case 1: state_button = Release;
      break;
      case 0:
         timetam = millis();
         while(((millis() - timetam) <= 2100)&(laststate_button != Push_pres)){
              if(digitalRead(BUTTON)){
               state_button = Click; 
               break;};
              if(((millis() - timetam) >= 2000)&(digitalRead(BUTTON) == 0)){
                state_button = Push_pres;
                break;
              }
         }
      break;
     }
switch (state_button){
  case Click:
  { 
    if(state_display == DisplayInforsystem){
      state_display  = DisplayOdotime;
      break;
    }
    if(state_display == DisplayOdotime){
      state_display      = DisplayTriptime;
      break;
    }
    if(state_display == DisplayTriptime){
      state_display     = DisplaySetWarning;
      break;
    }
    if(state_display == DisplaySetWarning){
      state_display = DisplayInforsystem;
      break;
    };
  }
  case Push_pres:
  {
    if(state_display == DisplayTriptime){
            /// Code reset.
            demPush = 0;
            digitalWrite(BUZZER,LOW);
            HC01.TripTimeRun = 0;
            AT24CXX.write_4_byte (35,0); // Ô nhớ chứa thời gian động cơ hoạt động kể từ lúc người dùng reset.
            delay(10);
            digitalWrite(BUZZER,HIGH);
      /*Reset thời gian về 0*/
      break;
    };
    break;
  }
   break;
  }
  laststate_button = state_button;
}
void Display_Uptime(unsigned long timeoper){
    unsigned long  H,M,S;
    H = timeoper/60;
    M = timeoper - (H*60);
    lcd.Asc_String(0,10,Asc("Hours :"),BLACK);
    lcd.Number_Long(45,10,H,ASCII_NUMBER,BLACK);
    lcd.Asc_String(0,20,Asc("Minute:"),BLACK);
    lcd.Number_Long(45,20,M,ASCII_NUMBER,BLACK);
    lcd.Asc_String(0,30,Asc(" Time Runing "),BLACK);
    lcd.Number_Long(30,40,HC01.GetTimeRunning(),ASCII_NUMBER,BLACK);
    lcd.display();
    }
void Display_Ifor(float Tempwater,float Batvoltage){
  unsigned int i = 0,j = 5;
  static byte cursors = 0; 
  // Hiển thị thông tin nhiệt độ.
  lcd.Asc_String(i,j,Asc("1.Temp: "),BLACK);
  lcd.Number_Float(i+45,j,Tempwater,1,ASCII_NUMBER,BLACK);
  lcd.Asc_Char(i+70,j,247,BLACK);
  lcd.Asc_Char(i+75,j,67,BLACK);
  // Hiển thị thông tin điện áp bình.
  lcd.Asc_String(i,j+10,Asc("2.Volt: "),BLACK);
  lcd.Number_Float(i+45,j+10,Batvoltage,1,ASCII_NUMBER,BLACK);
  lcd.Asc_Char(i+70,j+10,86,BLACK);
  // Hiển thị trạng thái áp lực nhớt.
  lcd.Asc_String(i,j+20,Asc("3.Oil Pres: "),BLACK);
  lcd.Asc_String(i+25,j+30,HC01.ReadOilPressure() ? Asc("Open"): Asc("Close"),BLACK);
  lcd.display();
 }    
void SetWarningParameter(){
  // Khi đặt lên xe các biến trở ok thì cho phép người dùng điều chỉnh các biến trở này để thay đổi các giá trị
  // Nhiệt độ, điện áp cao, điện áp thấp.
  // Các biến này sẽ được lưu lại nếu người dùng nhấn nút và giữ trong 2s. 
  lcd.Clear();
  lcd.Asc_String(0,0,Asc("SetupWarning"), BLACK);
  lcd.Asc_String(0,10,Asc("1.HTemp:"), BLACK);
  lcd.Number_Float(48,10,HC01.Temp_WARN,1,ASCII_NUMBER,BLACK);
  lcd.Asc_String(0,20,Asc("2.HVolt:"), BLACK);
  lcd.Number_Float(50,20,HC01.VBat_HIGH,1,ASCII_NUMBER,BLACK);
  lcd.Asc_String(0,30,Asc("3.LVolt:"),BLACK);
  lcd.Number_Float(50,30,HC01.VBat_LOW,1,ASCII_NUMBER,BLACK);
  lcd.Asc_String(0,40,Asc("4.NTime:"),BLACK);
  lcd.Number_Long(50,40,(HC01.PointTimeMain / 60),ASCII_NUMBER,BLACK);
  lcd.display();
}
void EEBlueEvent() {
  while (EEBlue.available()) {
    delay(30);
    // get the new byte:
    char c = EEBlue.read();             // tiến hành đọc
    Receive += c;                         // data = data + c
  }
}
void WriteString(String stringData) { 

  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);
  }
}
void ReturnString(String Chuoi){
    EEBlue.flush();
    EEBlue.print(Chuoi);
    EEBlue.print(" ");
}
void HandingString(String StringData){
  String StringBuffer = StringData.substring(0,2);
  unsigned int str = 0;
  if (StringBuffer == "01"){
     str = (StringData.substring(2)).toInt();
     switch (str){
      case 1: ReturnString(StringData);
              EEBlue.println(StateV.BatteryVoltage);
              break;
      case 2: ReturnString(StringData); 
              EEBlue.println(StateV.EngineCoolantTemp);
              break;
      case 3: ReturnString(StringData);
              EEBlue.println(StateV.OilPres);
              break;
      case 4: ReturnString(StringData);
              EEBlue.println(1);
              break;
      case 5: ReturnString(StringData);
              EEBlue.println(1);
              break; 
      case 6: ReturnString(StringData);
              EEBlue.println(75);
              break;   
      case 7: ReturnString(StringData);
              EEBlue.println(70);
              break;   
      case 8: ReturnString(StringData);
              EEBlue.println(HC01.Temp_WARN);
              break;     
      case 9: ReturnString(StringData);
              EEBlue.println(HC01.VBat_HIGH);
              break;  
      case 10:ReturnString(StringData);
              EEBlue.println(HC01.VBat_LOW);
              break;   
      case 11:ReturnString(StringData);
              EEBlue.println(HC01.R1ECU);
              break;   
      case 12:ReturnString(StringData);
              EEBlue.println(HC01.Ro);
              break;    
      case 13:ReturnString(StringData);
              EEBlue.println(HC01.Beta);
              break;    
      case 14:ReturnString(StringData);
              for(int i = 3;i >= 0;i--){
                EEBlue.print(0);// FIX CODE
              }
              EEBlue.println();
              break; 
      case 15:ReturnString(StringData);
              EEBlue.println(HC01.VCharge);
              break; 
      case 16:ReturnString(StringData);
              EEBlue.println(HC01.NumberOfReset);  // số lần đã reset bảo dưỡng.
              break; 
      case 17:ReturnString(StringData);
              EEBlue.println(HC01.CycleMaintenance); //đơn vị giờ.
              break;  
      case 18:ReturnString(StringData);
              EEBlue.println(HC01.TimeRunTooLong); // Thời gian cảnh báo chạy đơn vị phút.
              break;  
      case 19:ReturnString(StringData);
              EEBlue.println(60); //  Thời gian yêu cầu nghỉ ngơi trong TimeT đơn vị phút
              break;  
     }
  }
  else if (StringBuffer == "02"){
     str = (StringData.substring(2," ")).toInt();
     switch (str){
      case 1: ReturnString(StringData.substring(0,4));
              break;
      case 2: ReturnString(StringData.substring(0,4));
              break;
      case 3: HC01.Temp_WARN = (StringData.substring(5)).toFloat();
              digitalWrite(BUZZER,LOW);
              AT24CXX.eeprom_write_fl(0,HC01.Temp_WARN);// Viết giá trị ban đầu cho nhiệt độ bật quạt.
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.Temp_WARN);
              digitalWrite(BUZZER,HIGH); 
              break;
      case 4: HC01.VBat_HIGH = (StringData.substring(5)).toFloat();
              digitalWrite(BUZZER,LOW);
              AT24CXX.eeprom_write_fl(16,HC01.VBat_HIGH);// Viết giá trị ban đầu cho nhiệt độ bật quạt.
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.VBat_HIGH);
              digitalWrite(BUZZER,HIGH); 
              break;
      case 5: HC01.VBat_LOW = (StringData.substring(5)).toFloat();
              digitalWrite(BUZZER,LOW);
              AT24CXX.eeprom_write_fl(12,HC01.VBat_LOW);// Viết giá trị ban đầu cho nhiệt độ bật quạt.
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println( HC01.VBat_LOW);
              digitalWrite(BUZZER,HIGH); 
              break; 
      case 6: HC01.R1ECU = (StringData.substring(5)).toInt();
              digitalWrite(BUZZER,LOW);
              AT24CXX.write_2_byte(21,HC01.R1ECU);// Viết giá trị ban đầu cho nhiệt độ bật quạt.
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.R1ECU);
              digitalWrite(BUZZER,HIGH);  
              break;   
      case 7: HC01.Ro = (StringData.substring(5)).toInt();
              digitalWrite(BUZZER,LOW);
              AT24CXX.write_2_byte(23,HC01.Ro);// Viết giá trị ban đầu cho nhiệt độ bật quạt.
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.Ro);
              digitalWrite(BUZZER,HIGH); 
              break;   
      case 8: ReturnString(StringData.substring(0,4));
              break;
      case 9: HC01.VCharge = (StringData.substring(5)).toFloat();
              digitalWrite(BUZZER,LOW);
              AT24CXX.eeprom_write_fl(27,HC01.VCharge);// Viết giá trị ban đầu cho nhiệt độ bật quạt.
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.VCharge);
              digitalWrite(BUZZER,HIGH); 
              break;
      case 10: HC01.TimeRun = (unsigned long)((StringData.substring(5)).toFloat());
              digitalWrite(BUZZER,LOW);
              AT24CXX.write_4_byte(31,HC01.TimeRun);
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.TimeRun);
              digitalWrite(BUZZER,HIGH); 
              break;   
      case 11: HC01.TripTimeRun = (unsigned long)((StringData.substring(5)).toFloat());
              digitalWrite(BUZZER,LOW);
              AT24CXX.write_4_byte(35,HC01.TripTimeRun);
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.TripTimeRun);
              digitalWrite(BUZZER,HIGH); 
              break;   
      case 12: HC01.TimeRunTooLong = (unsigned long)((StringData.substring(5)).toInt());
              digitalWrite(BUZZER,LOW);
              AT24CXX.write_2_byte(44,HC01.TimeRunTooLong);
              delay(50);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.TimeRunTooLong);
              digitalWrite(BUZZER,HIGH); 
              break;   
      case 13: HC01.CycleMaintenance = ((StringData.substring(5)).toInt());
              digitalWrite(BUZZER,LOW);
              AT24CXX.write_2_byte(40,HC01.CycleMaintenance);//HC01.CycleMaintenance đơn vị là giờ.
              delay(5);
              ReturnString(StringData.substring(0,4));
              HC01.PointTimeMain  = HC01.CycleMaintenance*HC01.NumberOfReset;
              HC01.PointTimeMain += HC01.CycleMaintenance;
              HC01.PointTimeMain *= 60;
              EEBlue.println(HC01.CycleMaintenance);
              digitalWrite(BUZZER,HIGH); 
              break;  
      case 14: HC01.NumberOfReset++;
              digitalWrite(BUZZER,LOW);
              AT24CXX.write_1_byte(39,HC01.NumberOfReset);
              delay(5);
              ReturnString(StringData.substring(0,4));
              EEBlue.println(HC01.NumberOfReset);
              HC01.PointTimeMain  = HC01.CycleMaintenance*HC01.NumberOfReset;
              HC01.PointTimeMain += HC01.CycleMaintenance;
              HC01.PointTimeMain *= 60;
              digitalWrite(BUZZER,HIGH); 
              break;   
      case 15:digitalWrite(BUZZER,LOW);
              ReturnString("0215");
              digitalWrite(BUZZER,HIGH); 
              break;                                               
     }
  }
}
void WDT_Init(){
//disable interrupts
cli();
//reset watchdog
wdt_reset();
//set up WDT interrupt
WDTCSR = (1<<WDCE)|(1<<WDE);
//Start watchdog timer with 4s prescaller
WDTCSR = (1<<WDIE)|(1<<WDE)|(1<<WDP3)|(0<<WDP2)|(0<<WDP2)|(0<<WDP0);
//Enable global interrupts
sei();
}
