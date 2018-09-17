// Đây là thư viện riêng cho tất cả phần cứng của Hiệp Cường bao gồm các hàm:
// Đọc nhiệt độ của xe, Điện áp bình, tiêu thụ dòng điện, thời gian động cơ hoạt động,
// Tốc độ xe, tốc độ động cơ, Các hàm điều khiển: quạt, ngắt máy, bật đèn hazard. Ngoài ra còn chứa các hàm đọc 
// các giá trị cảnh báo, điều khiển được cài đặt trong eeprom của thết bị; Ngoài ra có một chương trình chịu trách nhiệm
// phản ứng với module mã lệnh yêu cầu thông qua cổng UASRT.
// Bộ thư viện được tạo ngày 15/06/2018:

#ifndef PDLib_h
#define PDLib_h

#include <Arduino.h>
#include <Eeprom24Cxx.h>
#include <Wire.h>
#include <SimpleKalmanFilter.h>
#include <avr/interrupt.h>


static Eeprom24C AT24CXX(32,0x50); 
static SimpleKalmanFilter Filter_WaterTemp(1,1,0.1);
static SimpleKalmanFilter Filter_BatVolt(1,1,0.1);
static SimpleKalmanFilter Filter_Current(1,1,0.01);

#define 	TRUE 	true
#define 	FALSE	false

#define     R1 		47000
#define     R2 		10000
#if defined (__AVR__)
	#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__)
		#define     BUZZER   9
		#define     IFUEL    10
		#define     PRE      8
		#define 	BUTTON	 2

		#define     BAT      A0
		#define     TEMP     A1
	#elif defined __AVR_ATmega2560__
		#define     BUZZER   37
		#define     IFUEL    32
		#define     HAZ      34
		#define     PRE      31
		#define     FAN      7
		#define     BRAKE    39
		#define 	LDO		 33	

		#define     BAT      A0
		#define     TEMP     A1
		#define 	FUELG    A3
		#define 	CUR		 A4
	#else
		#error "Please ensure chosen Board"
	#endif
#endif
typedef enum{
	  Running,
      Operating,
      IG_ON,
      IG_OFF
}State_Vehicle;

typedef enum {
    Close,
    Open
}ONOF_state;  

typedef enum {
    OFF,
    ON
}ONOF_stateFan;
typedef struct TimeDS1307{
		int second;
		int minute;
		int hour;
		int day;
		int wday;
		int month;
		int year;
}; 
//Struct chứa lỗi hệ thống: cảm biến bị short,mất tín hiệu...
typedef struct Buffer_Eror{
	bool NotReadTemp;
	bool NotReadVolt;
	bool NotReadGauge;
	bool NotReadCurrent;
};
// Struct chứa các trạng thái nguy hiểm của ô tô.
typedef struct Buffer_DangerState{
	bool TempHot;
	bool VoltEror;
	bool HighCurrent;
	bool OilPressureLow;
	bool HandBrakeON;
	bool RunTooLong; //  Chạy quá lâu.
	bool MaintenanceTime;// Tới thời điểm bảo dưỡng.
};
//Struct chứa trạng thái xe hiện tại bao gồm: điện áp bình, nhiệt độ nước, dòng tiêu thụ,...
typedef struct StateSys{
	float BatteryVoltage;
	float EngineCoolantTemp;
	float Current;
	State_Vehicle StateVehicle;
	bool OilPres;
	bool Handbrake;
	unsigned long TimeRunning;
	unsigned long TimeRun;
	unsigned int  rpm;
	unsigned int  spd;
};
// Biến của Bộ RealTime DS1307;
const byte DS1307 			= 0x68;
const byte NumberOfFields 	= 7;
const float VIG_OFF 		= 7.0;

const byte Rev 				= 36;

const float _Rev 			= 0.02; // độ phân giải 36 xung.
const float Radius 			= 0.015; // bán kính bánh xe đơn vị cm.
const float R 				= (Radius / 2.0);

static volatile unsigned long time_period = 0; // 
static volatile unsigned long Cur_timepoint = 0;
static volatile unsigned long Re_timepoint = 0;

static volatile unsigned long _time_period = 0; // 
static volatile unsigned long _Cur_timepoint = 0;
static volatile unsigned long _Re_timepoint = 0;

class PDLib {
public:
	PDLib(); // Create Contructor.
	float ReadTemp(void);
	float ReadBatVolt (void);
	bool  ReadOilPressure(void);
	#if defined __AVR_ATmega2560__
		float ReadCurrent(void);
		float ReadFuelTank(void);
		bool  ReadHandBrake(void);
		unsigned int ReadRPM(void);
		unsigned int ReadSPD(void);
	#endif
	State_Vehicle GetStateVehicle(void);
	unsigned long GetTimeRunning(void);
	void ResetMaintenance(void);
	
	byte BeginC_TimeRun (void);
	byte EndC_TimeRun   (void);
	//bool Check_BLE (void);
	void Check_Eror(Buffer_Eror* Eror);
	void Check_DangerState(Buffer_DangerState* State, StateSys* StateSy);
	void GetStateSys(StateSys* State);
	void Setting_Defauts(void);
	void ReadDataBoard(void);
	void ReadDS1307	(TimeDS1307* Mangtime);
	void setTime	(byte hr, byte min, byte sec, byte wd, byte d, byte mth, byte yr);
	void ConfigGPIO(void);
	#if defined __AVR_ATmega2560__
		static void ISR_RPM();
		static void ISR_SPD();
		static void ConfigRPM();
		static void ConfigSPD();
	#endif
	TimeDS1307 			TimeRunning;
	Buffer_Eror			ErorSys;
	Buffer_DangerState	StateDanger;

 	volatile byte 	FlagCTBegin = 0, FlagCTEnd = 0;
 	volatile float 	Temp_ON ;//  = 85.0;
 	volatile float 	Temp_OFF;//  = 78.0;
 	volatile float 	Temp_WARN;// = 100.0;
 	volatile float 	VBat_HIGH;//  = 17.5;
 	volatile float 	VBat_LOW;//   = 12.3; 
 	volatile float 	VCharge;
	// Thông số nhiệt độ của các loại xe khác nhau.
 	volatile unsigned int 	R1ECU;//  = 1500;
 	volatile unsigned int 	To     = 298;
 	volatile unsigned int	Ro ;//     = 2500;
 	volatile unsigned int  	Beta ;//  = 3800;
 	// Dòng tối đa của phương tiện.
 	volatile unsigned int 	MaxCurrent = 0;
 	// Thời gian xe hoạt động.
 	volatile unsigned long 	TimeRun     	= 0; // Đơn Vị phút.
 	volatile unsigned long 	TripTimeRun 	= 0; // Đơn vị phút
 	volatile unsigned int 	TimeRunTooLong 	= 0;
 	// Số lần Reset bảo dưỡng.
 	volatile byte 	NumberOfReset 			= 0;
 	volatile unsigned int 	CycleMaintenance 		= 0; // Chu kỳ bảo dưỡng đơn vị là giò.
 	volatile unsigned long  PointTimeMain 	= 0; // Thời điểm bảo dưỡng kế tiếp.
	// Thông số kết cấu của bộ phận cảm biến tốc độ động cơ.
private:
	int  bcd2dec(byte num);
	int  dec2bcd(byte num);
};
extern PDLib HC01;
#endif