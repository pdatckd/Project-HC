#ifndef PDLib_cpp
#define PDLib_cpp

#include "PDLib.h"
#include <Arduino.h>
#include <SimpleKalmanFilter.h>

PDLib HC01;
static PDLib *instance;
PDLib::PDLib()
{
#if defined __AVR_ATmega2560__
	instance = this;
	attachInterrupt(0,ConfigRPM,RISING);
	attachInterrupt(1,ConfigSPD,RISING);
#endif
}
float PDLib::ReadTemp()
{
  	float Temp = 0;
  	double a = 0, b = 0, r2 = 0;
  	double c = 0;
  	double h = 0;
  	a   = (double)analogRead(TEMP); //  yêu cầu điện áp cảm biến từ 0 - 3.75V
  	r2  = a*(double)R1ECU;
  	b   = 1023.0 - a;
  	r2  = r2/b;
  	c   = r2/(double)Ro;
  	h   = log(c);
  	c   = h*(double)To;
  	c   = c + (double)Beta;
  	h   = (double)Beta*(double)To;
  	Temp = h/c;
  	Temp = Temp - 273;
  	if((Temp >= 140)||(Temp <= -40))
  		{
      		if(Temp >= 140){Temp = 140;}
      		if(Temp <= -40){Temp = -40;}
  		}
  	Temp = Filter_WaterTemp.updateEstimate(Temp);
  	return Temp;
}
float PDLib::ReadBatVolt()
{
	float Voltage = 0;
   	Voltage = analogRead(BAT);
   	Voltage /= 1023.0;
   	Voltage *= 5.0;
   	Voltage *= (R1+R2);
   	Voltage /=  R2;
   	Voltage = Filter_BatVolt.updateEstimate(Voltage);
	return Voltage;
}
bool PDLib::ReadOilPressure()
{
	return digitalRead(PRE);
};
#if defined __AVR_ATmega2560__
float PDLib::ReadCurrent()
{
	float Cur = 0;
	Cur = analogRead(CUR);
	Cur -= 509.0;
	Cur /= 1023.0;
   	Cur *= 5.01;
   	Cur *= 100;
   	Cur  = Filter_Current.updateEstimate(Cur);
	return Cur;
}	
float PDLib::ReadFuelTank()
{
	float FuelTank = 0;
	return FuelTank;
}
bool PDLib::ReadHandBrake()
{
	return digitalRead(BRAKE);
}
unsigned int PDLib::ReadRPM()
{
	float rpm = 0;
	noInterrupts();
	rpm = Rev*(float)time_period; 
    rpm = 6*(float)pow(10,7)/rpm;
    interrupts();
    return (unsigned int)rpm;

}
unsigned int PDLib::ReadSPD()
{
	float spd = 0;
	noInterrupts();
	spd = _Rev*(float)_time_period; 
    if( spd <= 0){
    spd = 0;
    }
    else{
    spd   =  (float)pow(10,6)/spd;
    spd  *=  R; // lúc này vận tốc có đơn vị m/s.
    spd  *=  3.6; // km/h
    };
    interrupts();
    return (unsigned int)spd;
}

static void PDLib::ISR_RPM()
{
	Cur_timepoint  = micros();
  	time_period    = Cur_timepoint - Re_timepoint;
  	Re_timepoint   = Cur_timepoint;
}
static void PDLib::ISR_SPD()
{
	_Cur_timepoint  = micros();
  	_time_period    = _Cur_timepoint - _Re_timepoint;
  	_Re_timepoint   = _Cur_timepoint;
}
static void  PDLib::ConfigRPM()
{
	instance -> ISR_RPM();

}
static void  PDLib::ConfigSPD()
{
	instance -> ISR_SPD();

}
#endif
void PDLib::ConfigGPIO(void)
{
	#if defined (__AVR__)
		#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__)
			pinMode(BUZZER,OUTPUT);
			pinMode(IFUEL,OUTPUT);
			pinMode(PRE,INPUT);
			pinMode(BUTTON,INPUT);

			pinMode(BAT,INPUT_PULLUP);
			//pinMode(TEMP,INPUT_PULLUP);
			digitalWrite(BUZZER,LOW);
			digitalWrite(IFUEL,HIGH);
		#endif
	#if defined __AVR_ATmega2560__
		pinMode(BUZZER,OUTPUT);
		pinMode(IFUEL,OUTPUT);
		pinMode(FAN,OUTPUT);
		pinMode(HAZ,OUTPUT);
		pinMode(PRE,INPUT);
		pinMode(BRAKE,INPUT);
		//pinMode(BUTTON,INPUT);

		pinMode(BAT,INPUT_PULLUP);
		pinMode(TEMP,INPUT_PULLUP);
		pinMode(FUELG,INPUT_PULLUP);
		pinMode(CUR,INPUT_PULLUP);
		#endif
	#endif
}

void PDLib::Check_Eror(Buffer_Eror* Eror)
{

	if ((analogRead(TEMP) >= 1020)||(analogRead(TEMP) <= 10)){
		Eror->NotReadTemp = TRUE;
	}
	else{
		Eror->NotReadTemp = FALSE;
	};
	#if defined __AVR_ATmega2560__
			if ((analogRead(CUR) >= 1020)||(analogRead(CUR) <= 10)){
				Eror->NotReadCurrent = TRUE;
			}
			else{
				Eror->NotReadCurrent = FALSE;
			};
			if ((analogRead(FUELG) >= 1020)||(analogRead(FUELG) <= 10)){
				Eror->NotReadGauge = TRUE;
			}
			else{
				Eror->NotReadGauge = FALSE;
			};
	#endif
}
void PDLib::Check_DangerState(Buffer_DangerState* State, StateSys* StateSy)
{
	static byte FlagEV = 0;
	if ((StateSy->BatteryVoltage >= VBat_HIGH)||(StateSy->BatteryVoltage <= VBat_LOW)){
		FlagEV++;
		if(FlagEV >= 10){State->VoltEror = TRUE;}
	}
	else{
		State->VoltEror = FALSE;
		FlagEV = 0;
	}
	static byte FlagTE = 0;
	if ((StateSy->EngineCoolantTemp >= Temp_WARN)&&(StateSy->StateVehicle < 2)){
		FlagTE++;
		if(FlagTE >= 10){State->TempHot = TRUE;}
	}
	else{
		State->TempHot = FALSE;
		FlagTE = 0;
	}
	//
	if (StateSy->Current >= MaxCurrent){
		State->HighCurrent = TRUE;
	}
	else{
		State->HighCurrent = FALSE;
	};	
	static byte FlagOIL = 0;
	if ((StateSy->StateVehicle < 2)&&(StateSy->OilPres == 0)){
		FlagOIL++;
		if(FlagOIL >= 10){State->OilPressureLow = TRUE;}
	}
	else{
		State->OilPressureLow = FALSE;
		FlagOIL = 0;
	};
	//
	if ((StateSy->StateVehicle == Running)&&(StateSy->Handbrake == 0)){
		State->HandBrakeON = TRUE;
	}
	else{
		State->HandBrakeON = FALSE;
	};
	//
	if (StateSy->TimeRunning >= TimeRunTooLong){
		State->RunTooLong = TRUE;
	}
	else{
		State->RunTooLong = FALSE;
	};	
	//
	if (StateSy->TimeRun >= PointTimeMain){
		State->MaintenanceTime = TRUE;
	}
	else{
		State->MaintenanceTime = FALSE;
	};	

}
State_Vehicle PDLib::GetStateVehicle()
{
	float Volt = 0;
	unsigned int RPM = 0,SPD = 0; 
	Volt = ReadBatVolt();
	#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__)
		if (Volt >= VCharge){
			return Operating;
		}
		else if ((Volt < VCharge)&&(Volt >= VIG_OFF)){
			return IG_ON;
		}
		else{
			return IG_OFF;
		};	
	#elif __AVR_ATmega2560__
		RPM  =  ReadRPM();
		SPD  =  ReadSPD();
		if ((RPM >= 400)&&(SPD > 0)){
			return Running;
		}
		else if ((SPD == 0)&&(RPM >= 400)){
			return Operating;
		}
		else if ((Volt < VCharge)&&(Volt >= VIG_OFF)){
			return IG_ON;
		}
		else {
			return IG_OFF;
		};
	#endif
}
void PDLib::GetStateSys(StateSys* State)
{
	State->BatteryVoltage		= ReadBatVolt();
	State->EngineCoolantTemp	= ReadTemp();
	State->StateVehicle 		= GetStateVehicle();
	State->OilPres 				= ReadOilPressure();
	State->TimeRunning   		= GetTimeRunning();
	State->TimeRun 				= TimeRun;
	#if __AVR_ATmega2560__
		State->Current 		= ReadCurrent();
		State-> Handbrake 	= ReadHandBrake();
		State->spd 			= ReadSPD();
		State->rpm 			= ReadRPM();
	#endif	
}

byte PDLib::BeginC_TimeRun(void)
{
	State_Vehicle state = GetStateVehicle();
	if (FlagCTBegin == 1) return FlagCTBegin;
	#if defined __AVR_ATmega2560__
		if ((state == Operating)||(state == Running)){
			setTime(0,0,0,0,0,0,0);
			FlagCTBegin = 1;
			FlagCTEnd   = 0;
		}
		else{
			FlagCTBegin = 0;
		};
	#elif  (__AVR_ATmega328P__) || defined (__AVR_ATmega168__)
		if ((state == Operating)||(state == IG_ON)){
			setTime(0,0,0,0,0,0,0);
			FlagCTBegin = 1;
			FlagCTEnd   = 0;
		}
		else{
			FlagCTBegin = 0;
		};
	#endif		
	return FlagCTBegin;
}
byte PDLib::EndC_TimeRun(void)
{
	State_Vehicle state = GetStateVehicle();
	if (FlagCTBegin == 0) return FlagCTEnd;
	if ((FlagCTBegin == 1) || (state == IG_OFF)){
		ReadDS1307(&TimeRunning);
		TimeRun 	= TimeRun 		+ (GetTimeRunning() / 60);
		TripTimeRun = TripTimeRun 	+ (GetTimeRunning() / 60);
		AT24CXX.write_4_byte (31,TimeRun); // Ô nhớ chưa thời gian động cơ hoạt động từ trước đến giờ.
      	delay(5); 
      	AT24CXX.write_4_byte (35,TripTimeRun); // Ô nhớ chưa thời gian động cơ hoạt động từ trước đến giờ.
      	delay(5); 
		FlagCTEnd   = 1;
		FlagCTBegin = 0;
		return FlagCTEnd;
	}
}
unsigned long PDLib::GetTimeRunning(void) // trả về số thời gian mà xe đã hoạt động đơn vị giây(s).
{
	if ((FlagCTBegin == 0)&&(FlagCTEnd == 0)) return 0;
	else if ((FlagCTBegin == 1)&&(FlagCTEnd == 0)){
		ReadDS1307(&TimeRunning);
		return TimeRunning.hour * 3600 + TimeRunning.minute * 60 + TimeRunning.second;
	}
	else{
		return TimeRunning.hour * 3600 + TimeRunning.minute * 60 + TimeRunning.second;
	}
}
void PDLib::ReadDS1307(TimeDS1307* Mangtime)
{
  Wire.beginTransmission(DS1307);
  Wire.write((byte)0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307, NumberOfFields);
  Mangtime->second = bcd2dec(Wire.read() & 0x7f);
  Mangtime->minute = bcd2dec(Wire.read() );
  Mangtime->hour   = bcd2dec(Wire.read() & 0x3f); // chế độ 24h.
  Mangtime->wday   = bcd2dec(Wire.read() );
  Mangtime->month  = bcd2dec(Wire.read() );
  Mangtime->year   = bcd2dec(Wire.read() );
  Mangtime->year += 2000;
	if ((Mangtime->second > 60)||(Mangtime->second < 0)){
		Mangtime->second 	= 0;
		Mangtime->minute 	= 0;
		Mangtime->hour 		= 0;
		Mangtime->day 		= 0;
		Mangtime->wday 		= 0;		
		Mangtime->month 	= 0;
		Mangtime->year		= 0;
		setTime(0,0,0,0,0,0,0);
	}
	if ((Mangtime->minute > 60)||(Mangtime->minute < 0)){
		Mangtime->second 	= 0;
		Mangtime->minute 	= 0;
		Mangtime->hour 		= 0;
		Mangtime->day 		= 0;
		Mangtime->wday 		= 0;		
		Mangtime->month 	= 0;
		Mangtime->year		= 0;

		setTime(0,0,0,0,0,0,0);		
	}
	if ((Mangtime->hour > 24)||(Mangtime->hour < 0)){
		Mangtime->second 	= 0;
		Mangtime->minute 	= 0;
		Mangtime->hour 		= 0;
		Mangtime->day 		= 0;
		Mangtime->wday 		= 0;		
		Mangtime->month 	= 0;
		Mangtime->year		= 0;

		setTime(0,0,0,0,0,0,0);		
	}
	if ((Mangtime->month > 12)||(Mangtime->month < 0)){
		Mangtime->second 	= 0;
		Mangtime->minute 	= 0;
		Mangtime->hour 		= 0;
		Mangtime->day 		= 0;
		Mangtime->wday 		= 0;		
		Mangtime->month 	= 0;
		Mangtime->year		= 0;
		setTime(0,0,0,0,0,0,0);		
	}
}
int  PDLib::bcd2dec(byte num){
  return ((num / 16 * 10) + (num % 16));
}
int  PDLib::dec2bcd(byte num){
  return ((num / 10 * 16) + (num % 10));
}
void PDLib::setTime(byte hr, byte min, byte sec, byte wd, byte d, byte mth, byte yr){
        Wire.beginTransmission(DS1307);
        Wire.write(byte(0x00)); // đặt lại pointer
        Wire.write(dec2bcd(sec));
        Wire.write(dec2bcd(min));
        Wire.write(dec2bcd(hr));
        Wire.write(dec2bcd(wd)); // day of week: Sunday = 1, Saturday = 7
        Wire.write(dec2bcd(d)); 
        Wire.write(dec2bcd(mth));
        Wire.write(dec2bcd(yr));
        Wire.endTransmission();
}
void PDLib::ReadDataBoard (void)
{
	  Temp_WARN = AT24CXX.eeprom_read_fl(0);   // Đọc giá trị ban đầu cho nhiệt độ cảnh báo
      delay(5);
      Temp_ON   = AT24CXX.eeprom_read_fl(4);// Đọc giá trị ban đầu cho nhiệt độ bật quạt.
      delay(5);
      Temp_OFF  = AT24CXX.eeprom_read_fl(8);// Đọc giá trị ban đầu cho nhiệt độ tắt quạt.
      delay(5);
      VBat_LOW  = AT24CXX.eeprom_read_fl(12);// Đọc giá trị ban đầu cho điện áp cảnh báo quá thấp.
      delay(5);
      VBat_HIGH = AT24CXX.eeprom_read_fl(16);// Đọc giá trị ban đầu cho điện áp cảnh báo quá cao.
      delay(5);
      R1ECU     = AT24CXX.read_2_byte(21);//    Thông số diện trở treo R1ECU của xe.
      delay(5);
      Ro        = AT24CXX.read_2_byte(23);//    Thông số Điện trở của cảm biến ở 25 độ c.
      delay(5);
      Beta      = AT24CXX.read_2_byte(25);//    Hệ số truyền nhiệt của cảm biến nhiệt độ.
      delay(5);
      VCharge   = AT24CXX.eeprom_read_fl(27);// Ô nhớ chứa giá trị điện áp dùng nhận biết động cơ hoạt động
      delay(5); 
      TimeRun   = AT24CXX.read_4_byte (31); // Ô nhớ chưa thời gian động cơ hoạt động từ trước đến giờ.
      delay(5); 
      TripTimeRun   	= AT24CXX.read_4_byte (35); // Ô nhớ chứa thời gian động cơ hoạt động kể từ lúc người dùng reset.
      delay(5); 
      NumberOfReset   	= AT24CXX.read_1_byte (39); // Ô nhớ chứa số lần reset bảo dưỡng.
      delay(5); 	
      CycleMaintenance  = AT24CXX.read_2_byte (40); // Ô nhớ chứa Chu kỳ bảo dưỡng.
      delay(5); 
      MaxCurrent		= AT24CXX.read_2_byte (42); // Ô nhớ chứa Tải điện tối đa của xe.
      delay(5); 
      TimeRunTooLong	= AT24CXX.read_2_byte (44); // Ô nhớ chứa thời gian chạy quá lâu đv phút.
      delay(5); 
      PointTimeMain  = (unsigned long)CycleMaintenance*NumberOfReset;
      PointTimeMain += (unsigned long)CycleMaintenance;
      PointTimeMain *= 60;
}
void PDLib::Setting_Defauts(void)
{
	  AT24CXX.eeprom_write_fl(0,100);   // Viết giá trị ban đầu cho nhiệt độ cảnh báo
      delay(5);
      AT24CXX.eeprom_write_fl(4,80);// Viết giá trị ban đầu cho nhiệt độ bật quạt.
      delay(5);
      AT24CXX.eeprom_write_fl(8,78);// Viết giá trị ban đầu cho nhiệt độ tắt quạt.
      delay(5);
      AT24CXX.eeprom_write_fl(12, 12.3);// Viết giá trị ban đầu cho điện áp cảnh báo quá thấp.
      delay(5);
      AT24CXX.eeprom_write_fl(16, 17.5);// Viết giá trị ban đầu cho điện áp cảnh báo quá cao.
      delay(5);
      AT24CXX.write_2_byte(21,1000);// Thông số diện trở treo R1ECU của xe.
      delay(5);
      AT24CXX.write_2_byte(23,2500);//   Thông số Điện trở của cảm biến ở 25 độ c.
      delay(5);
      AT24CXX.write_2_byte(25,3800);// Hệ số truyền nhiệt của cảm biến nhiệt độ.
      delay(5);
      AT24CXX.eeprom_write_fl(27,13.2);//   Ô nhớ chứa giá trị điện áp dùng nhận biết động cơ hoạt động
      delay(5);
      AT24CXX.write_4_byte (31,0); // Ô nhớ chưa thời gian động cơ hoạt động từ trước đến giờ.
      delay(5); 
      AT24CXX.write_4_byte (35,0); // Ô nhớ chứa thời gian động cơ hoạt động kể từ lúc người dùng reset.
      delay(5); 
      AT24CXX.write_1_byte (39,0); // Ô nhớ chứa số lần reset bảo dưỡng.
      delay(5); 
      AT24CXX.write_2_byte (40,250); // Ô nhớ chứa chu kỳ bảo dưỡng
      delay(5);
      AT24CXX.write_2_byte (42,80); // Ô nhớ chứa Tải điện tối đa của xe.
      delay(5);
      AT24CXX.write_2_byte (44,360);// Ô nhớ chứa thời gian chạy quá lâu đv phút.
      delay(5);
}
#endif
