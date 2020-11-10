/* Bomberman multitap -> 4 x USB joysticks
 *  Should work on Pro Micro, using a Teensy right now.
 *  
 *  TODO:  
 *  USB Joystick code (Teensy only supports 1 joystick without hacking)
 *  Support detection of the multitap - DONE
 *  Support detection of the mouse

   7 pin SNES proprietary female connector view:
    -----------------\
   | 1 2 3 4 | 5 6 7 |
    -----------------/
   pin 1: +5v
   pin 2: Data Clock
   pin 3: Data Latch (4016)
   pin 4: Controlers 1/2  (D0)
   pin 5: Controllers 3/4 (D1)
   pin 6: Select (PP7)
*/

const int PIN_CLOCK = 0;
const int PIN_LATCH = 1;
const int PIN_DATA = 2;
const int PIN_DATA2 = 3;
const int PIN_SELECT = 4;

//#include <Joystick.h>

// Controller Buttons
#define SNES_B       1      //000000000001
#define SNES_Y       2      //000000000010
#define SNES_SELECT  4      //000000000100
#define SNES_START   8      //000000001000
#define SNES_UP      16     //000000010000
#define SNES_DOWN    32     //000000100000
#define SNES_LEFT    64     //000001000000
#define SNES_RIGHT   128    //000010000000
#define SNES_A       256    //000100000000
#define SNES_X       512    //001000000000
#define SNES_L       1024   //010000000000
#define SNES_R       2048   //100000000000



#define MAX_PADS 4
/*
  Joystick_ Joystick[MAX_PADS] = {
  Joystick_(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false),
  Joystick_(JOYSTICK_DEFAULT_REPORT_ID + 1, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false),
  Joystick_(JOYSTICK_DEFAULT_REPORT_ID + 2, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false),
  Joystick_(JOYSTICK_DEFAULT_REPORT_ID + 3, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false),
  };
*/

uint16_t state[4]; //What data we read out of the SNES pad.
bool  disconnected[4]; //Whether there's a controller connected to that port on the multitap


enum device_type_t {DISCONNECTED, MULTITAP, GAMEPAD, MOUSE};
device_type_t device_type = DISCONNECTED;

void setup()
{
  Serial.begin (115200);
  Serial.println ("SNES2USB - Bomberman edition");
  for (int i = 0; i < MAX_PADS; i++)
  {
    disconnected[i] = true;
    state[i] = 0xFFFF; //Because too lazy to flip the bits
    /*
    
    Joystick[i].begin ();
    Joystick[i].setXAxisRange(-1, 1);
    Joystick[i].setYAxisRange(-1, 1);
*/
  }

  pinMode(PIN_CLOCK, OUTPUT);
  digitalWrite(PIN_CLOCK, HIGH);

  pinMode(PIN_LATCH, OUTPUT);
  digitalWrite(PIN_LATCH, LOW);

  pinMode(PIN_SELECT, OUTPUT);
  digitalWrite (PIN_SELECT, HIGH);

  pinMode(PIN_DATA, INPUT_PULLUP);
  pinMode(PIN_DATA2, INPUT_PULLUP);

}

void joystick_add (int i)
{
  if (i > MAX_PADS || i < 0)
    return;
  Serial.println ("Joystick " + String(i) + " Connected!");
  /*
    Joystick[i] = Joystick_(JOYSTICK_DEFAULT_REPORT_ID + i, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false),
              Joystick[i].begin ();
    Joystick[i].setXAxisRange(-1, 1);
    Joystick[i].setYAxisRange(-1, 1);
  */
}

void joystick_remove (int i)
{
  Serial.println ("Joystick " + String(i) + " disconnected :(");
  //  Joystick[i].end();
  state[i] = 0;
  disconnected[i] = true;
}

void read_pair (int offset)
{
  state[offset] = 0;
  state[offset + 1] = 0;
  
  for (int i = 0; i < 16; i++)
  {
    digitalWrite(PIN_CLOCK, LOW);
    delayMicroseconds(6);
    state[offset] |= digitalRead(PIN_DATA) << i;
    state[offset + 1] |= digitalRead(PIN_DATA2) << i;
    digitalWrite(PIN_CLOCK, HIGH);
    delayMicroseconds(6);
  }
  
  digitalWrite(PIN_CLOCK, LOW);
  delayMicroseconds(6);
  disconnected[offset] = digitalRead(PIN_DATA);
  disconnected[offset + 1] = digitalRead(PIN_DATA2);
  digitalWrite(PIN_CLOCK, HIGH);
}

bool detect_multitap ()
{
  uint8_t result = 0;

  digitalWrite(PIN_LATCH, HIGH);
  delayMicroseconds(6);
  digitalWrite(PIN_CLOCK, LOW);
  delayMicroseconds(6);
  result = digitalRead(PIN_DATA2);
  digitalWrite(PIN_CLOCK, HIGH);
  delayMicroseconds(6);
  digitalWrite(PIN_LATCH, LOW);
  if (result != 0)
    return false;
  else
  {
    device_type = MULTITAP;
    return true;
  }
}

void read_snespads ()
{
  digitalWrite(PIN_LATCH, HIGH);
  digitalWrite (PIN_SELECT, HIGH);
  delayMicroseconds(12);

  digitalWrite(PIN_LATCH, LOW);
  delayMicroseconds(6);

  read_pair (0);
  digitalWrite (PIN_SELECT, LOW);
  delayMicroseconds(12);
  read_pair (2);
  digitalWrite (PIN_SELECT, HIGH);

}

void set_buttons (int p)
{
  /*
    // buttons
    SNES_B & ~state[p] ? Joystick[p].pressButton(0) : Joystick[p].releaseButton(0);
    SNES_Y & ~state[p] ? Joystick[p].pressButton(1) : Joystick[p].releaseButton(1);
    SNES_SELECT & ~state[p] ? Joystick[p].pressButton(2) : Joystick[p].releaseButton(2);
    SNES_START & ~state[p] ? Joystick[p].pressButton(3) : Joystick[p].releaseButton(3);
    SNES_A & ~state[p] ? Joystick[p].pressButton(4) : Joystick[p].releaseButton(4);
    SNES_X & ~state[p] ? Joystick[p].pressButton(5) : Joystick[p].releaseButton(5);
    SNES_L & ~state[p] ? Joystick[p].pressButton(6) : Joystick[p].releaseButton(6);
    SNES_R & ~state[p] ? Joystick[p].pressButton(7) : Joystick[p].releaseButton(7);
  */
}

void set_axis (int p)
{
  /*
    Joystick[p].setXAxis(0);
    Joystick[p].setYAxis(0);
    if (SNES_UP & ~state[p]) Joystick[p].setYAxis(1);
    if (SNES_RIGHT & ~state[p]) Joystick[p].setXAxis(1);
    if (SNES_DOWN & ~state[p]) Joystick[p].setYAxis(-1);
    if (SNES_LEFT & ~state[p]) Joystick[p].setXAxis(-1);
  */
}

void loop() 
{
  uint16_t state_old[4];
  bool disconnected_old[4];
  
  if (!detect_multitap ())
  {
    Serial.println ("Multitap not connected");
    delay(2000);
    return;
  }

  //Backup the current state so we can see if anything changes later;
  for (int i=0;i<MAX_PADS;i++)
  {
    state_old[i] = state[i];
    disconnected_old[i] = disconnected[i];
  }
  
  read_snespads();

  for (int i=0;i<MAX_PADS;i++)
  {
    if (disconnected_old[i] != disconnected[i])
    {
      if (disconnected[i])
        joystick_remove(i);
      else
        joystick_add(i);
        
      disconnected_old[i] = disconnected[i];
    }
    
    
    if (state_old[i] != state[i])
    {
      state_old[i] = state[i];
      Serial.print ("J" + String(i) + ": ");
      Serial.println (state[i], BIN);
    }
  }
  
  /*
  for (int i = 0; i < MAX_PADS; i++)
  {
    set_buttons(i);
    set_axis (i);
    Joystick[i].sendState();
  }
  */
  delay(24);
}
