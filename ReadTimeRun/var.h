typedef enum {
    Release,
    Click,
    Push_pres
  }Button_state;
typedef enum{
  DisplayClear,
  DisplayInforsystem,
  DisplayOdotime,
  DisplayTriptime,
  DisplaySetWarning
  }Display_state;
Display_state state_display = DisplayInforsystem;
Button_state state_button = Release;
Button_state laststate_button = state_button;
unsigned long count1 = 0;
unsigned long count2 = 0;
unsigned long count3 = 0;
unsigned long count4 = 0;
unsigned long count5 = 0;
unsigned long count6 = 0;
unsigned long count7 = 0;
unsigned long count8 = 0;
unsigned long countW = 0;
unsigned long countS = 1000;
byte countBD = 0;
volatile unsigned char demClick, demPush;
unsigned long TimeI;
bool FlagI = 0;
volatile byte FlagFirstStart = 0;
volatile byte FlagFirstOFF = 0;
String Receive ="";


