// ---------------------------------- make2explore.com----------------------------------------------------//
// Project           - Reyax RFID Module RYRR10S Testing - DIY Project - Two Layer Lock System
// Created By        - blog@make2explore.com
// Version - 1.0     - RFID as a First layer
// Last Modified     - 18/11/2022 14:00:00 @admin
// Software          - C/C++, PlatformIO IDE, Visual Studio Code Editor, Libraries
// Hardware          - Arduino Uno, Reyax RFID RYRR10S Module,16x2 I2C LCD Display, 4x4 Matrix Keypad
//                   - 12 Volt solenoid Lock, Relay and LED's             
// Sensors Used      - RFID RYRR10S
// Source Repo       - github.com/make2explore
// -------------------------------------------------------------------------------------------------------//

// Include Libraries
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Keypad.h>

// LCD setup : Set the LCD address to 0x3F for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Software Serial
SoftwareSerial RFserial(2, 3); //RX, TX

// Variables for Serial data
uint8_t received_buf_pos=0;
uint8_t response_byte;
uint8_t data_len;
bool received_complete=false;
String tag_id = "";
char Byte_In_Hex[3];

// GPIO Pin definitions / allocations
uint8_t lockPin = A0;
uint8_t redLED = A1;
uint8_t greenLED = A2;
uint8_t blueLED = A3;
uint8_t buzzer = 13;

// KEYPAD settings and Mapping
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad

char keymap[numRows][numCols]=
{
	{'1', '2', '3', 'A'},

	{'4', '5', '6', 'B'},

	{'7', '8', '9', 'C'},

	{'*', '0', '#', 'D'}
};

// Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {4,5,6,7}; //Rows 0 to 3
byte colPins[numCols]= {8,9,10,11};  //Columns 0 to 3

// Initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);


// Serial Commands - Refer datasheets of ST CR95HF or RYRR10S for more details
uint8_t echo_command[1] = {0x55};
uint8_t initialise_cmd_iso14443_1[6] =  {0x09, 0x04, 0x68, 0x01, 0x07, 0x10};
uint8_t initialise_cmd_iso14443_2[6] =  {0x09, 0x04, 0x68, 0x01, 0x07, 0x00};
uint8_t initialise_cmd_iso14443_3[6] =  {0x02, 0x04, 0x02, 0x00, 0x02, 0x80};
uint8_t initialise_cmd_iso14443_4[6] =  {0x09, 0x04, 0x3A, 0x00, 0x58, 0x04};
uint8_t initialise_cmd_iso14443_5[6] =  {0x09, 0x04, 0x68, 0x01, 0x01, 0xD3};

uint8_t detect_cmd_iso14443_1[4] =  {0x04, 0x02, 0x26, 0x07};
uint8_t detect_cmd_iso14443_2[5] =  {0x04, 0x03, 0x93, 0x20, 0x08};

// Variable for storing Valid Users data
struct user{
  String name;
  String card;
  String code;
  int access;
};

// Manually add Valid Users data - Their names, RFID's, Pin/passcodes and Access Tokens
user validUsers[6] = {{"User1", "c92c4f8c", "*123456#", 1},
                      {"User2", "e92d4f8c", "*654321#", 1},
                      {"User3", "9944f8c", "*321654#", 0},
                      {"User4", "31203164", "*123654#", 1},
                      {"User5", "fecb375b", "*123654#", 1},
                      {"User6", "593a308c", "*654123#", 1}};

uint8_t Tag_Status;
String userName = ""; // saving users name in the String for displaying on LCD 

// Serial verbose for debugging
void show_serial_data(){
  while(RFserial.available()!=0)
  Serial.print(RFserial.read(), HEX);
  Serial.println("");
}

// Buzzer notifications functions
void shortBeep(){
	digitalWrite(buzzer, LOW);
	delay(200);
	digitalWrite(buzzer, HIGH);
}

void longBeep(){
	digitalWrite(buzzer, LOW);
	delay(500);
	digitalWrite(buzzer, HIGH);
}

void doubleBeep(){
	digitalWrite(buzzer, LOW);
	delay(100);
	digitalWrite(buzzer, HIGH);
	delay(100);
	digitalWrite(buzzer, LOW);
	delay(100);
	digitalWrite(buzzer, HIGH);
}

// Initialize the 16x2 LCD Display
void initLCD(){
	lcd.init();
	// Turn on the blacklight and print a message.
	lcd.backlight();
	lcd.print("Starting...");
	delay(1000);
  lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Initializing LCD");
	delay(2000);
	lcd.setCursor(0,1);
	lcd.print("Init LCD : OK");
	delay(1000);
}

// Initialize the RYRR10S RFID module with sequence of commands given in datasheet 
// to configure the RFID module as UID Reader
void initRFID(){
  lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Init RFID ...");

  Serial.println("Echo command: "); 
  RFserial.write(echo_command, 1);  
  delay(1000); 
  show_serial_data();
  delay(1000);
  Serial.println("Initialise commands: ");  
  RFserial.write(initialise_cmd_iso14443_1, 6);
  delay(1000); show_serial_data(); 
  RFserial.write(initialise_cmd_iso14443_2, 6);
  delay(1000); show_serial_data();
  RFserial.write(initialise_cmd_iso14443_3, 6);
  delay(1000); show_serial_data();
  RFserial.write(initialise_cmd_iso14443_4, 6);
  delay(1000); show_serial_data();
  RFserial.write(initialise_cmd_iso14443_5, 6); 
  delay(1000); show_serial_data();
  lcd.setCursor(0,1);
	lcd.print("RFID : OK");
  delay(2000);
}

// Setup GPIO Port and Pins
void initGPIO(){
	pinMode(lockPin, OUTPUT);
	digitalWrite(lockPin, HIGH);

	pinMode(redLED, OUTPUT);
	digitalWrite(redLED, LOW);
	pinMode(greenLED, OUTPUT);
	digitalWrite(greenLED, LOW);
  pinMode(blueLED, OUTPUT);
	digitalWrite(blueLED, LOW);

	pinMode(buzzer, OUTPUT);
	digitalWrite(buzzer, HIGH);
}


// Function which reads UID serially
void Read_Tag(){
  uint8_t received_char;
  while(RFserial.available()!=0){
    received_char = char (RFserial.read());
    if(received_buf_pos==0)response_byte = received_char; 
    else if (received_buf_pos==1)data_len = received_char; 
    else if (received_buf_pos>=2 and received_buf_pos<6) {
      sprintf(Byte_In_Hex,"%x", received_char);
      tag_id += Byte_In_Hex; //adding to a string
      }
    received_buf_pos++;
    if(received_buf_pos >= data_len){
      received_complete = true;
      }
    }
}

// Function which compares the code We type on keypad with predefined codes
int compareCODE(String a){
	//with the accessCode;
  for (int i=0;i<sizeof(validUsers)/sizeof(user); i++){
    if((validUsers[i].card==tag_id)){
      if(validUsers[i].code == a){
        return 1;
      }
      else return 0;
      }
  }
}


// Function for displaying Character * on the LCD to hide the code typed on KeyPad for security purpose
String takeCode(char x){
	char vec[10];
	vec[0]=x;
	lcd.setCursor(0,1);
	lcd.print('*');
	for(int i=1;i<8;i++){
		vec[i]=myKeypad.waitForKey(); 	//Waits for 8 keys to be pressed and after that
		lcd.print('*');					        //is taking the decision
	} 				
	vec[8]=NULL;
	String str(vec);
	return str;
}

// Setup
void setup() {  
  Serial.begin(9600);
  RFserial.begin(57600);
  initGPIO();
  doubleBeep();
  initLCD();
  doubleBeep();
  initRFID();
  longBeep();
}

// main
void loop() {  
  received_buf_pos = 0;
  received_complete = false;
  tag_id="";
  response_byte=0;
  
  Serial.println("Searching new card...");
  RFserial.write(detect_cmd_iso14443_1, 4);
  delay(800);
  Serial.println("Response:"); show_serial_data();
  
  RFserial.write(detect_cmd_iso14443_2, 5);
  delay(300);

  if(RFserial.available()) {
    Read_Tag();
  }

  if(response_byte == 0x80){
    Tag_Status=2;
    for (int i=0;i<sizeof(validUsers)/sizeof(user); i++){
      if(validUsers[i].card==tag_id) {
        Tag_Status=validUsers[i].access;
        userName = validUsers[i].name;
      }
    }
    shortBeep();
    digitalWrite(blueLED, HIGH);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tag Detected !!");
    Serial.print("Tag detected. Tag id: ");
    lcd.setCursor(0,1);
    lcd.print("ID : ");
    lcd.setCursor(5,1);
    lcd.print(tag_id);
    Serial.println(tag_id);    
    Serial.println("");
    delay(2000);
    digitalWrite(blueLED, LOW);
    if(Tag_Status == 1){
      digitalWrite(greenLED, HIGH);
      doubleBeep();
      digitalWrite(greenLED, LOW);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Valid Tag");
      lcd.setCursor(0,1);
      lcd.print("Next Step : PIN");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter Key : ");
      boolean key = 1;
      while(key){
        char c=myKeypad.getKey();
        if(c != NO_KEY){
          String codecurent=takeCode(c);
          int A=compareCODE(codecurent);
            if(A==0){
            //A is a variable that stores the current code
            longBeep();
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("INVALID CODE");
            lcd.setCursor(0,1);
            lcd.print( "Access Denied" );
            delay(2000);
            key = 0;
            return;
          }
          if(A==1){
            doubleBeep();
            digitalWrite(redLED, LOW);
            digitalWrite(greenLED, HIGH);
            digitalWrite(lockPin, LOW);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print( "VALID CODE " );
            lcd.setCursor(0,1);
            lcd.print( "Access Granted" ); //The door will be opened
            delay(5000);
            key = 0;
            return;
          }
        }
      }
    }
    if (Tag_Status == 0){
      longBeep();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("InValid Tag");
      lcd.setCursor(0,1);
      lcd.print("Access Denied");
      delay(1000);
    }
    if (Tag_Status == 2){
      longBeep();
      digitalWrite(blueLED, HIGH);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Unknown Tag");
      lcd.setCursor(0,1);
      lcd.print("Access Denied");
      delay(1000);
    }
  }    
  else{
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, LOW);
    digitalWrite(lockPin, HIGH);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("No RFID : Locked");
    lcd.setCursor(0,1);
    lcd.print("Place RFID Card");
    Serial.println("No tag detected.");
    Serial.println("");
    delay(2000);
  }
}

// ---------------------------------- make2explore.com----------------------------------------------------//