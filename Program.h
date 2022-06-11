//-----------------------------FINAL----------------------------
//LIBRARY
#include <SPI.h>
#include <Wire.h>
#include <Keypad.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include <NewPing.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial14.h>
#include <fonts/Droid_Sans_12.h>



//------------------------------COUNTER------------------------
int Pin_ir = 2;  //interrupt
int Pin_ir1 = 3; //interrupt
int counter;
int maxcounter;
int hitObject = false; //ir aktif low
bool isMaxCounter;
bool enableUp;
bool enableDown;

void down()
{
  if (enableDown)
  {
    if (counter > 0) counter--;
    enableDown = false;
    EEPROM.write(1, counter);
  }
}
void up()
{
  if (enableUp)
  {
    if (counter < maxcounter)
    {
      counter++;
      EEPROM.write(1, counter);
      isMaxCounter = false;
    }
    else if (counter == maxcounter)
    {
      isMaxCounter = true;    
    }
    enableUp = false;
  }
}
void detectUp()
{
  if (enableDown == true)down();
  else enableUp = true;
  Serial.print("UP => "); Serial.println(enableUp);
}
void detectDown()
{
  if (enableUp == true)up();
  else enableDown = true;
  Serial.print("DOWN => "); Serial.println(enableDown);
}


//--------------------- SET N RESET COUNTER ---------------------
//KEYPAD
bool isMax_set;
String data;
const byte numRows = 4; //Jumlah baris pada keypad
const byte numCols = 4; //Jumlah Kolom pada keypad

/*keymap mendefinisikan tombol ditekan sesuai
  dengan baris dan kolom seperti muncul pada keypad*/
char keymap[numRows][numCols] =
    {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}};

byte rowPins[numRows] = {A15, A14, A13, A12}; //pin keypad
byte colPins[numCols] = {A11, A10, A9, A8};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

void keyPad()
{
  char keypressed = myKeypad.getKey();
  if (keypressed != NO_KEY)
//........ A for enter........
  {
    data += keypressed;
    if (keypressed == 'A')
    {
      maxcounter = data.toInt();
      EEPROM.write(0, maxcounter);
      Serial.println("Setting max counter : " + (String)maxcounter);
      data = "";
      isMax_set = true;
      //      Serial.println(keypressed);
      //      counter = keypressed - '0';
    }
//.....B for setting counter......
    if (keypressed == 'B')
    {
      counter = data.toInt();
      EEPROM.write(1, counter);
      counter = EEPROM.read(1);
      isMax_set = true;
      Serial.println("Counter : " + (String)counter);
      data = "";
    }
//......C for reset.........
    if (keypressed == 'C')
    {
      EEPROM.write(0, 0);
      EEPROM.write(1, 0);
      counter = 0;
      maxcounter = 0;
      isMax_set = false;
      isMaxCounter = false;
      data = "";
    }
  }
}

//-------------------------PEN - TRIGGER MOTOR ---------------------------
// S.SUHU
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
unsigned long millissuhu;
void suhu()
{
  // Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC());
 // Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  if (millis() - millissuhu > 500)
  {
    if (mlx.readObjectTempC() > 35)
    {
        // delay(800);
      lcd1();
    }
    if (mlx.readObjectTempC() < 35)
    {
      //  delay(800);
      lcd2();
    }
    millissuhu = millis();
  }
}

// ULTRASONIK
#define TRIGGER1 15
#define ECHO1 14
#define MAX_DISTANCE1 3
#define TRIGGER2 17
#define ECHO2 16
#define MAX_DISTANCE2 3
NewPing sonar1(TRIGGER1, ECHO1, MAX_DISTANCE1);
NewPing sonar2(TRIGGER2, ECHO2, MAX_DISTANCE2);

void Ultrasonik1()
{
  sonar1.ping_cm();
}
void Ultrasonik2()
{
  sonar2.ping_cm();
}

//---------------------------------DISPLAY LCD----------------------------------
LiquidCrystal_I2C lcd(0x27, 20, 4);
void lcd1()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("This Is Smart Door !");
  lcd.setCursor(0, 1);
  lcd.print("Kapasitas Maks : ");
  lcd.print(maxcounter);
  lcd.setCursor(2, 2);
  lcd.print("Suhu Anda : ");
  lcd.print(mlx.readObjectTempC());
  lcd.setCursor(2, 3);
  lcd.print ("Have a good day !");
}

void lcd2()
{
  // delay (1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("This Is Smart Door !");
  lcd.setCursor(0, 1);
  lcd.print("Kapasitas Maks : ");
  lcd.print(maxcounter);
  lcd.setCursor(2, 3);
  lcd.print ("Have a good day !");
}

//------------------------------------- LS && DRIVER MOTOR --------------------------------------------
//LS
#define LSkanan 11
#define LSkiri 12

// DRIVER
#define RPWM 8
#define LPWM 7
#define PWM 6

void setMotor_CCW()
{ //LPWM
  digitalWrite(RPWM, LOW);
  digitalWrite(LPWM, HIGH);
  analogWrite(PWM, 110);
  Serial.println("LPWM ON. putar kiri, PINTU TUTUP");
}

void setMotor_CW()
{ //RPWM
  digitalWrite(RPWM, HIGH);
  digitalWrite(LPWM, LOW);
  analogWrite(PWM, 110);
  Serial.println("RPWM ON. putar kanan, PINTU BUKA");
}

void setMotor_OFF()
{ //PWM off
  digitalWrite(RPWM, LOW);
  digitalWrite(LPWM, LOW);
  analogWrite(PWM, 0);
  Serial.println("MOTOR OFF");
}

//----------------------------------------- SET MOTOR ------------------------------------------
unsigned long timeout;

void orang_Masuk()
{
  //Ultrasonik1();
  suhu();
//  float suhuuu = mlx.readObjectTempC();
//  if ((((suhuuu > 35) && (suhuuu < 37.5)) || (sonar1.ping_cm() > 0)) && (isMaxCounter == false)) (YANG BENER)
if ((((mlx.readObjectTempC() > 35) && (mlx.readObjectTempC() < 38)) || (sonar1.ping_cm() > 0)) && (isMaxCounter == false))
  {
  //  dotWelcome();
   lcd1();
    while (true)
    {
     
      setMotor_CW();
      if(digitalRead(LSkanan) == HIGH) break;
      if(digitalRead(Pin_ir1) == LOW) break;
 //     delay(100);
    }
    setMotor_OFF();
    timeout = millis();
    while (true)
    {
      if(millis() - timeout > 5000) break;
      if(digitalRead(Pin_ir1) == LOW) break;
      if(digitalRead(Pin_ir) == LOW) timeout = millis();
     //Serial.println("IR => "+(String)digitalRead(Pin_ir1));
     // delay(100);
    }
    
    while (true)
    {
      setMotor_CCW();
      if(digitalRead(LSkiri) == HIGH) break;
    }
    setMotor_OFF();    
    lcd2();
  }
}

void orang_Keluar()
{
  if (sonar2.ping_cm() > 0)
  {
    while (true)
    {
      setMotor_CW();
      if(digitalRead(LSkanan) == HIGH) break;
      if(digitalRead(Pin_ir) == LOW) break;
    }
    setMotor_OFF();
    timeout = millis();
    while (true)
    {
      if(millis() - timeout > 5000) break;
      if(digitalRead(Pin_ir)==LOW) break;
      if(digitalRead(Pin_ir1) == LOW) timeout = millis();
    //  Serial.println("IR => "+(String)digitalRead(Pin_ir1));
     // delay(100);
    }
    
    while (true)
    {
      setMotor_CCW();
      if(digitalRead(LSkiri) == HIGH) break;
    }
    setMotor_OFF();    
  }
}

//-------------------------------------- DOT MATRIX DISPLAY ---------------------------------------
//const uint8_t *FONT = SystemFont5x7;
const uint8_t *FONT = Droid_Sans_12;
SoftDMD dmd(3, 2, 49, 46, 47, 48, 51, 52);

void dotWelcome(){
    dmd.drawString (20, 18, "WELCOME");
}

void klir (){
  dmd.drawString (20,18, "          ");
}

void dotWait(){
  dmd.drawString (23, 18, "WAIT !!!");
}

void dotRuangkaprodi(){
    dmd.drawString(2, 4, "RUANG KAPRODI");
}

//----------------------------------- INTERRUPT -------------------------------------------------
volatile int timertick;
void Interrupt()
{
  if (timertick>20){
    keyPad();
    timertick=0;
  }
  timertick++;
  dmd.scanDisplay();
}

void setup()
{
  maxcounter = EEPROM.read(0);
  counter = EEPROM.read(1);
  pinMode(Pin_ir, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Pin_ir), detectUp, RISING);
  pinMode(Pin_ir1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Pin_ir1), detectDown, RISING);
  Serial.begin(115200);
 // Timer1.initialize(100000);
  Timer1.initialize(5000);
  Timer1.attachInterrupt(Interrupt);

  lcd.init();
  lcd.backlight();
  if (!mlx.begin())
  {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1)
      ;
  }

  pinMode(PWM, OUTPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(LSkanan, INPUT_PULLUP);
  pinMode(LSkiri, INPUT_PULLUP);
  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  dmd.begin();
}

unsigned long loopmillis;
void loop()
{
 // suhu();
 dotRuangkaprodi();
  if (millis() - loopmillis > 1000)
  {
    Serial.println("Counter : " + (String)counter);
    loopmillis = millis();
  }
  //  if ((isMaxCounter == true) && (isMax_set == true))Serial.println("DAH PENUH");
  if ((counter == maxcounter) && (isMax_set == true))
  {
    Serial.println("DAH PENUH");
 //   lcd2();
    klir();
    dotWait();
  }

  else
  {
    Serial.println("MONGGO");
    orang_Masuk();
    dotWelcome();
  }
  Ultrasonik1();
  Ultrasonik2();
 // delay (200);
  orang_Keluar();

}
