
int dataIn = 6;
int load = 8;
int clock = 7;

int rtcSQW = 10;
int rtcClock = 2;
int rtcMiso = 3;
int rctMosi = 9;
int rtcSS = 4;

int SS_pin = rtcSS;
int SCK_pin = rtcClock;
int MISO_pin = rtcMiso;
int MOSI_pin = rctMosi;

int summerPin = 11;
int summerGndPin = 12;
int brightPin = 3;
int brightPressValue = 200;
int brightPressed = 0;
byte brightValue = 0;
byte brightValueSteps = 4;

int th = 0;
int tm = 0;
int ts = 0;

int maxInUse = 1;    //change this variable to set how many MAX7219's you'll use

int e = 0;           // just a variable

// define max7219 registers
byte max7219_reg_noop        = 0x00;
byte max7219_reg_digit0      = 0x01;
byte max7219_reg_digit1      = 0x02;
byte max7219_reg_digit2      = 0x03;
byte max7219_reg_digit3      = 0x04;
byte max7219_reg_digit4      = 0x05;
byte max7219_reg_digit5      = 0x06;
byte max7219_reg_digit6      = 0x07;
byte max7219_reg_digit7      = 0x08;
byte max7219_reg_decodeMode  = 0x09;
byte max7219_reg_intensity   = 0x0a;
byte max7219_reg_scanLimit   = 0x0b;
byte max7219_reg_shutdown    = 0x0c;
byte max7219_reg_displayTest = 0x0f;

void putByte(byte data) {
  byte i = 8;
  byte mask;
  while(i > 0) {
    mask = 0x01 << (i - 1);      // get bitmask
    digitalWrite( clock, LOW);   // tick
    if (data & mask){            // choose bit
      digitalWrite(dataIn, HIGH);// send 1
    }else{
      digitalWrite(dataIn, LOW); // send 0
    }
    digitalWrite(clock, HIGH);   // tock
    --i;                         // move to lesser bit
  }
}

void maxSingle(byte reg, byte col) {    
  digitalWrite(load, LOW);       // begin     
  putByte(reg);                  // specify register
  putByte(col);//((data & 0x01) * 256) + data >> 1); // put data   
  digitalWrite(load, LOW);       // and load da stuff
  digitalWrite(load,HIGH); 
}

void maxAll (byte reg, byte col) {    // initialize  all  MAX7219's in the system
  int c = 0;
  digitalWrite(load, LOW);  // begin     
  for ( c =1; c<= maxInUse; c++) {
  putByte(reg);  // specify register
  putByte(col);//((data & 0x01) * 256) + data >> 1); // put data
    }
  digitalWrite(load, LOW);
  digitalWrite(load,HIGH);
}

void maxOne(byte maxNr, byte reg, byte col) {    
//maxOne is for addressing different MAX7219's, 
//while having a couple of them cascaded

  int c = 0;
  digitalWrite(load, LOW);  // begin     

  for ( c = maxInUse; c > maxNr; c--) {
    putByte(0);    // means no operation
    putByte(0);    // means no operation
  }

  putByte(reg);  // specify register
  putByte(col);//((data & 0x01) * 256) + data >> 1); // put data 

  for ( c =maxNr-1; c >= 1; c--) {
    putByte(0);    // means no operation
    putByte(0);    // means no operation
  }

  digitalWrite(load, LOW); // and load da stuff
  digitalWrite(load,HIGH); 
}

void showDigit(int place, int digit, bool dp)
{
  maxSingle(place*2+1 ,digit ^ (dp ? 0x80 : 0));
  delay(100);
  maxSingle(place*2+2 ,digit ^ (dp ? 0x80 : 0));
}

void setup ()
{
  //Serial.begin(9600);
  
  pinMode(dataIn, OUTPUT);
  pinMode(clock,  OUTPUT);
  pinMode(load,   OUTPUT);
  pinMode(summerPin,INPUT_PULLUP);
  pinMode(summerGndPin, OUTPUT);
  pinMode(A3, INPUT_PULLUP);
  
//initiation of the max 7219
  maxAll(max7219_reg_scanLimit, 0x07);      
  maxAll(max7219_reg_decodeMode, 0xff);
  maxAll(max7219_reg_shutdown, 0x01);    // not in shutdown mode
  maxAll(max7219_reg_displayTest, 0x00); // no display test
   for (e=1; e<=8; e++) {    // empty registers, turn all LEDs off 
    maxAll(e,0x7f);
  }
  maxAll(max7219_reg_intensity, 0x00);    // the first 0x0f is the value you can set
                                                  // range: 0x00 to 0x0f

  RTC_init();
}  

byte brtc;

void loop ()
{
  brtc = rtc_readFrom(0);
  int a=brtc & B00001111;
  int b=(brtc & B01110000)>>4;
  ts = a+b*10;
  
  brtc = rtc_readFrom(1);
  a=brtc & B00001111;
  b=(brtc & B01110000)>>4;
  tm = a+b*10;
  
  brtc = rtc_readFrom(2);
  a=brtc & B00001111;
  b=(brtc & B01110000)>>4;
  th = a+b*10;

  // Date
  brtc = rtc_readFrom(4);
  a=brtc & B00001111;
  b=(brtc & B00110000)>>4;
  int day = a+b*10;
  
  brtc = rtc_readFrom(5);
  a=brtc & B00001111;
  b=(brtc & B00010000)>>4;
  int month = a+b*10;

  boolean talBD = day == 30 && month == 4;
  
  if (digitalRead(summerPin) == HIGH)
  {
    th += 1;
    if (th==24)
    {
      th = 0;
    }
  }

  if (talBD && ts%30==5)
  {
    showTal();
  }
  else if (ts == 0 && tm == 5)
  {
    showTal();
  }
  
  showDigit(0, th/10, false);
  showDigit(1, th%10, ts%2==0);
  showDigit(2, tm/10, ts%2==0);
  showDigit(3, tm%10, false);

  // Check brightness button.
  //Serial.println(digitalRead(brightPin));
  if (brightPressed == 0 && analogRead(3) < brightPressValue)
  {
    brightPressed = 1;
    for (int iDelay = 0; iDelay < 10; iDelay++)
    {
      delay(10);
      if (analogRead(3) >= brightPressValue)
      {
        brightPressed = 0;
        break;
      }
    }

    if (brightPressed = 1)
    {
      brightValue = (brightValue + brightValueSteps) & 0xf;
      maxAll(max7219_reg_intensity, brightValue);
    }
  }
  else if (brightPressed == 1 && digitalRead(brightPin) == HIGH)
  {
    brightPressed = 0;
  }
  
  delay(10);
}

// RTC methods.
void rtc_sendByte(byte data) {
  byte i = 8;
  byte mask;
  while(i > 0) {
    mask = 0x01 << (i - 1);      // get bitmask
    digitalWrite(SCK_pin, LOW);   // tick
    //delay(1);
    if (data & mask){            // choose bit
      digitalWrite(MOSI_pin, HIGH);// send 1
    }else{
      digitalWrite(MOSI_pin, LOW); // send 0
    }
    //delay(1);
    digitalWrite(SCK_pin, HIGH);   // tock
    //delay(1);
    --i;                         // move to lesser bit
  }
}

void rtc_writeTo(byte address, byte data)
{
  digitalWrite(SS_pin, LOW);
  rtc_sendByte(address);
  rtc_sendByte(data);
  digitalWrite(SS_pin, HIGH);
}

byte rtc_readByte() {
  byte i = 8;
  byte res = 0;
  
  while(i > 0) {
    digitalWrite(SCK_pin, LOW);   // tick
    //delay(1);
    res <<= 1;
    if (digitalRead(MISO_pin) == HIGH)
      res |= 1;
    
    digitalWrite(SCK_pin, HIGH);   // tock
    //delay(1);
    --i;                         // move to lesser bit
  }

  return res;
}

byte rtc_readFrom(byte address)
{
  digitalWrite(SS_pin, LOW);
  //delay(1);
  rtc_sendByte(address);
  byte res = rtc_readByte();
  digitalWrite(SS_pin, HIGH);
  //delay(1);
  return res;
}

void rtc_sramWrite(byte sramAddress, byte data)
{
  rtc_writeTo(0x98, sramAddress);
  rtc_writeTo(0x99, data);
}

byte rtc_sramRead(byte sramAddress)
{
  rtc_writeTo(0x98, sramAddress);
  return rtc_readFrom(0x19);
}

int RTC_init(){ 
  pinMode(SS_pin, OUTPUT);
  pinMode(SCK_pin, OUTPUT);
  pinMode(MISO_pin, INPUT);
  pinMode(MOSI_pin, OUTPUT);
  //pinMode(rtcSQW, OUTPUT);

  //digitalWrite(rtcSQW, LOW);
  //digitalWrite(SCK_pin, LOW);
  digitalWrite(SS_pin, HIGH);  // Start with SS high
}

void showTal()
{
  maxAll(max7219_reg_decodeMode, 0);
  for (e=1; e<=8; e++) {
    maxAll(e,0x0);
  }

  for (int iTal=0;iTal<5;iTal++)
  {
    showTalOn(2);
    delay(400);
    showTalOn(1);
    delay(400);
    showTalOn(0);
    delay(400);
  }

  for (int iTalMiddle=0;iTalMiddle<5;iTalMiddle++)
  {
    for (e=1; e<=8; e++) {
      maxAll(e,0x0);
    }
    delay(200);
    showTalOn(1);
    delay(800);
  }
  delay(2000);
  
  maxAll(max7219_reg_decodeMode, 0xFF);
}
void showTalOn(int iDigit)
{
  for (e=1; e<=8; e++) {
    maxAll(e,0x0);
  }
  // Teit
  maxSingle(iDigit*2+4,B00111100);
  maxSingle(iDigit*2+3,B00011111);

  // lamed
  maxSingle(iDigit*2+2,B00011011);
  maxSingle(iDigit*2+1,B00011011);
}


