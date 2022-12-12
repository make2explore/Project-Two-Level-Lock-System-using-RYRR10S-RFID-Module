// ---------------------------------- make2explore.com ----------------------------------------------------//
// Project           - Reyax RFID Module RYRR10S Testing - DIY Project - Two Layer Lock System
// Created By        - blog@make2explore.com
// Version - 2.0     - KeyPad as a First layer, and HMI
// Last Modified     - 18/11/2022 14:00:00 @admin
// Software          - C/C++, PlatformIO IDE, Visual Studio Code Editor, Libraries
// Hardware          - Arduino Uno, Reyax RFID RYRR10S Module,16x2 I2C LCD Display, 4x4 Matrix Keypad
//                   - 12 Volt solenoid Lock, Relay and LED's             
// Sensors Used      - RFID RYRR10S
// Source Repo       - github.com/make2explore
// --------------------------------------------------------------------------------------------------------//

// Include Libraries
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <EEPROM.h>

// GPIO Pin definitions / allocations
int lockPin = A0;
int resetButton = 12;
bool button_state = 1;

int redLED = A1;
int greenLED = A2;
int blueLED = A3;
int buzzer = 13;

// Variables for different parameters 
uint8_t state = 0;

// EEPROM Access Array variables
byte  COD[10];
byte  AUX[10];

// KeyPad HMI codes to Enter into access, Enroll and Reset Modes
// You can change these codes, but keep same number of characters
String accessCode = "*123456#";   // KeyPad Access code
String enrolCode = "*654321#";    // RFID Enroll Code
String resetCode = "*789789#";    // RESET RFID Code

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

// LCD setup : Set the LCD address to 0x3F for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Software Serial
SoftwareSerial RFserial(2, 3); //RX, TX

// Variables for Serial data 
uint8_t received_buf_pos=0;
uint8_t response_byte;
uint8_t data_len;
boolean received_complete=false;
String tag_id = "";
char Byte_In_Hex[3];

// Serial Commands - Refer datasheets of ST CR95HF or RYRR10S for more details
uint8_t echo_command[1] = {0x55};
uint8_t initialise_cmd_iso14443_1[6] =  {0x09, 0x04, 0x68, 0x01, 0x07, 0x10};
uint8_t initialise_cmd_iso14443_2[6] =  {0x09, 0x04, 0x68, 0x01, 0x07, 0x00};
uint8_t initialise_cmd_iso14443_3[6] =  {0x02, 0x04, 0x02, 0x00, 0x02, 0x80};
uint8_t initialise_cmd_iso14443_4[6] =  {0x09, 0x04, 0x3A, 0x00, 0x58, 0x04};
uint8_t initialise_cmd_iso14443_5[6] =  {0x09, 0x04, 0x68, 0x01, 0x01, 0xD3};

uint8_t detect_cmd_iso14443_1[4] =  {0x04, 0x02, 0x26, 0x07};
uint8_t detect_cmd_iso14443_2[5] =  {0x04, 0x03, 0x93, 0x20, 0x08};

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
	lcd.print("Hello There !!!");
	shortBeep();
	delay(2000);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Greetings From");
	lcd.setCursor(0,1);
	lcd.print("make2explore !!");
	doubleBeep();
	delay(3000);

	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Project : Reyax");
	lcd.setCursor(0,1);
	lcd.print("RYRR10S RFID");
	delay(3000);

	shortBeep();
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Starting...");
	delay(1000);
	lcd.setCursor(0,1);
	lcd.print("Initialize LCD");
	delay(1500);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Initializing LCD");
	lcd.setCursor(0,1);
	delay(1500);
	lcd.print("Init LCD : OK");
	delay(2000);
	shortBeep();
}

// Initialize the RYRR10S RFID module with sequence of commands given in datasheet 
// to configure the RFID module as UID Reader
void initRFID(){
	lcd.clear();
	lcd.print("Initialize RFID");
	RFserial.begin(57600);

	RFserial.write(echo_command, 1);  
	delay(1000);
	RFserial.write(initialise_cmd_iso14443_1, 6);
	delay(1000); 
	RFserial.write(initialise_cmd_iso14443_2, 6);
	delay(1000);
	RFserial.write(initialise_cmd_iso14443_3, 6);
	delay(1000);
	RFserial.write(initialise_cmd_iso14443_4, 6);
	delay(1000);
	RFserial.write(initialise_cmd_iso14443_5, 6); 
	delay(1000);

	lcd.setCursor(0,1);
	lcd.print("Init RFID : OK");
	delay(1000);
}

// Function which reads UID serially
void Read_Tag(){
	uint8_t received_char;
	tag_id = "";
	uint8_t i = 0;
	while(RFserial.available()!=0){
		received_char = char (RFserial.read());
		if(received_buf_pos==0){
			response_byte = received_char;
		} 
		else if (received_buf_pos==1){
			data_len = received_char;
		} 
		else if (received_buf_pos>=2 and received_buf_pos<6) {
			COD[i] = received_char;
			sprintf(Byte_In_Hex,"%x", received_char);
			tag_id += Byte_In_Hex; //adding to a string
			i++;
		}
		received_buf_pos++;
		if(received_buf_pos >= data_len){
		received_complete = true;
		}
	}
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

	pinMode(resetButton, INPUT_PULLUP);		
}

// (Optional) Function for displaying Tag value on LCD 
void Display_Tag(String Tag){
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print( "Tag Detected !!" );
	lcd.setCursor(0,1);
	lcd.print( "ID : " );
	lcd.setCursor(5,1);
	lcd.print(Tag);
	delay(2000);
}

// Function for Enrolling or registering new RFID Tag into EEPROM of Arduino
void enrolTag() {
	int c = 0;
	for(int i=1;i<=EEPROM.read(0);i++){
		//The UID cannot be stored on one variable, it was needed to be split
		switch(i%4){
			case 1 :{AUX[0]=EEPROM.read(i); break;}
			case 2 :{AUX[1]=EEPROM.read(i); break;}
			case 3 :{AUX[2]=EEPROM.read(i); break;}
			case 0 :{AUX[3]=EEPROM.read(i); break;}
		}

		if((i)%4==0){
			if( AUX[0]== COD[0] && AUX[1]==COD[1] && AUX[2]==COD[2] && AUX[3]==COD[3] ){
				//Verify if the code is in EEPROM
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print("CARD ALREADY IN");
				lcd.setCursor(0,1);
				lcd.print("SYSTEM");
				digitalWrite(blueLED,LOW); //The red blue will be on
				delay(2000);
				c=1;
				break;}
		}
	}

	if(c==0){
		int aux2=EEPROM.read(0);
		EEPROM.write(aux2+1,COD[0]);  //Writing code in EEPROM
		EEPROM.write(aux2+2,COD[1]);
		EEPROM.write(aux2+3,COD[2]);
		EEPROM.write(aux2+4,COD[3]);
		aux2=aux2+4; // Position for a new code
		EEPROM.write(0,0);
		EEPROM.write(0,aux2);
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("CARD ENROLLED");
		lcd.setCursor(0,1);
		lcd.print("New User Added");
		longBeep();
		digitalWrite(blueLED,LOW); //The red blue will be off
		delay(2000);
	}
}

// Function for ValidateRFID saved code in EEPROM
boolean validateTag(){
	boolean c=false;
	for(int i=1;i<=EEPROM.read(0);i++){
		//Read the EEPROM
		switch(i%4){
			case 1 :{AUX[0]=EEPROM.read(i); break;}
			case 2 :{AUX[1]=EEPROM.read(i); break;}
			case 3 :{AUX[2]=EEPROM.read(i); break;}
			case 0 :{AUX[3]=EEPROM.read(i); break;}
		}

		if((i)%4==0){
			if( (AUX[0]==COD[0]) && (AUX[1]==COD[1]) && (AUX[2]==COD[2]) &&  (AUX[3]==COD[3]))
			c=true; //Verify if the code is in EEPROM and make flag=true;
		}
	}

	return c;  
}

// Function which compares the code We type on keypad with predefined codes
int compareCODE(String a){
	//with the accessCode;
	if(a.equals(accessCode)){
		return 1;
    }
    else if(a.equals(enrolCode)){
		return 2;
    }
    else if(a.equals(resetCode)){
		return 3;
    }
    else return 0;  
}

// Function for displaying Character * on the LCD to hide the code typed on KeyPad for security purpose
String takeCode(char x){
	char vec[10];
	vec[0]=x;
	lcd.setCursor(0,0);
	lcd.clear();
	lcd.print('*');
	for(int i=1;i<8;i++){
		vec[i]=myKeypad.waitForKey(); 	//Waits for 8 keys to be pressed and after that
		lcd.print('*');					//is taking the decision
	} 				
	vec[8]=NULL;
	String str(vec);
	return str;
}

// Function which checks for Reset Button is pressed or Not
bool monitorWipeButton(uint32_t interval){
	uint32_t now = (uint32_t)millis();
	while ((uint32_t)millis() - now < interval){
		// check on every half a second
		if (((uint32_t)millis() % 500) == 0){
			if (digitalRead(resetButton) != LOW)
			return false;
		}
	}

	return true;
}

// Function which process RESET RFID Record (DELETE all RFID records from EEPROM)
void clearRecord(){
    digitalWrite(blueLED, HIGH); // Red Led stays on to inform user we are going to wipe
	lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press RED Button");
	lcd.setCursor(0, 1);
	lcd.print("to Wipe old Data");
	delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("This will remove");
    lcd.setCursor(0, 1);
    lcd.print("all records");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You have 3 ");
    lcd.setCursor(0, 1);
    lcd.print("secs to Cancel");
    delay(2000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unpres to cancel");
    lcd.setCursor(0, 1);
    lcd.print("Counting: ");
    lcd.setCursor(10, 1);
    lcd.print("1");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unpres to cancel");
    lcd.setCursor(0, 1);
    lcd.print("Counting: ");
    lcd.setCursor(10, 1);
    lcd.print("2");
    delay(1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unpres to cancel");
    lcd.setCursor(0, 1);
    lcd.print("Counting: ");
    lcd.setCursor(10, 1);
    lcd.print("3");
    delay(1000);
    
    bool buttonState = monitorWipeButton(3000); // Give user enough time to cancel operation
    if (buttonState == true && digitalRead(resetButton) == LOW){
		lcd.print("Wiping EEPROM...");
		for (uint16_t x = 0; x < EEPROM.length(); x = x + 1){
			//Loop end of EEPROM address
			if (EEPROM.read(x) == 0){
				//If EEPROM address 0
				// do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
			}
			else{
				EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
			}
		}

		longBeep();
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Wiping Done !!");
		lcd.setCursor(0,1);
		lcd.print("User Data Reset");
		// visualize a successful wipe
		digitalWrite(blueLED, LOW);
		delay(200);
		digitalWrite(blueLED, HIGH);
		delay(200);
		digitalWrite(blueLED, LOW);
		delay(200);
		digitalWrite(blueLED, HIGH);
		delay(200);
		digitalWrite(blueLED, LOW);

    } else{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Wiping Cancelled"); // Show some feedback that the wipe button did not pressed for 10 seconds
		delay(2000);
		digitalWrite(blueLED, LOW);
      	// If button still be pressed, wipe EEPROM
	}
}

// Setup
void setup()
{
	Serial.begin(9600);
	initGPIO();
	initLCD();
	initRFID();
	

	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print( "LOCKED !!!" );
	lcd.setCursor(0,1);
	lcd.print( "Enter Pass Code" );
	digitalWrite(redLED, HIGH);
	longBeep();
}

// main
void loop(){
	switch(state){
		case 0: {
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
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.print("LOCKED !!!");
					lcd.setCursor(0,1);
					lcd.print( "Enter Pass Code" );
					return;
				}

				if(A==1){
					doubleBeep();
					digitalWrite(greenLED,HIGH); //The blue led will be on
					delay(500);
					digitalWrite(greenLED,LOW); //The blue led will be on
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.print( "VALID CODE " );
					lcd.setCursor(0,1);
					lcd.print( "Next Step : RFID" ); //The door will be opened
					delay(2000);
					state = 1;
					return;
				}

				if(A==2){
					//Serial.println("I am in 2");
					doubleBeep();
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.print( "Process of New" );
					lcd.setCursor(0,1);
					lcd.print( "RFID Enrolling" );
					digitalWrite(blueLED,HIGH); //The blue led will be on
					delay(2000);
					state=2;
					return;
				}


				if(A==3){
					//Serial.println("I am in 3");
					doubleBeep();
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.print( "Process of RESET" );
					lcd.setCursor(0,1);
					lcd.print( "RFID Records" );
					digitalWrite(A0,HIGH); //The blue led will be on
					delay(2000);
					state=3;
					return;
				}
			}
	}break;


	case 1:{
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print( "LOCKED" );
		lcd.setCursor(0,1);
		lcd.print( "Place RFID Card" );
		received_buf_pos = 0;
		received_complete = false;
		response_byte=0;

		RFserial.write(detect_cmd_iso14443_1, 4);
		delay(800);
		Serial.println("Response:"); show_serial_data();
		RFserial.write(detect_cmd_iso14443_2, 5);
		delay(300);
		if(RFserial.available()) {
			Read_Tag();
		}
		if(response_byte == 0x80){
			if(validateTag()){
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print( "VALID RFID CARD" ); //The door will be opened
				lcd.setCursor(0,1);
				lcd.print( "Access Granted" ); //The door will be opened
				digitalWrite(redLED, LOW);
				digitalWrite(greenLED, HIGH);
				digitalWrite(lockPin, LOW);
				longBeep();
				delay(5000);
				digitalWrite(redLED, HIGH);
				digitalWrite(greenLED, LOW);
				digitalWrite(lockPin, HIGH);
				state = 0;
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print( "LOCKED !!!" );
				lcd.setCursor(0,1);
				lcd.print( "Enter Pass Code" );
			}else{
				longBeep();
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print( "INVALID RFID Tag" ); //If the code was wrongblocked
				lcd.setCursor(0,1);
				lcd.print( "Access Denied" ); //The door will be opened
				delay(2000);
				state = 0;
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print( "LOCKED !!!" );
				lcd.setCursor(0,1);
				lcd.print( "Enter Pass Code" );
			}
		}
		else{
			doubleBeep();
			digitalWrite(redLED, HIGH);
			digitalWrite(greenLED, LOW);
			digitalWrite(blueLED, LOW);
			digitalWrite(lockPin, HIGH);
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("No Tag Detected");
			lcd.setCursor(0,1);
			lcd.print("Place RFID Card");
			delay(2000);
		}
		return;
	}

	case 2:{
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print( "Enrol New Card" );
		lcd.setCursor(0,1);
		lcd.print( "Place RFID Card" );
		received_buf_pos = 0;
		received_complete = false;
		response_byte=0;
		RFserial.write(detect_cmd_iso14443_1, 4);
		delay(800);
		Serial.println("Response:"); show_serial_data();
		RFserial.write(detect_cmd_iso14443_2, 5);
		delay(300);
		if(RFserial.available()) {
			Read_Tag();
		}
		if(response_byte == 0x80){
			enrolTag();
			state=0;
			delay(2000);
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print( "LOCKED !!!" );
			lcd.setCursor(0,1);
			lcd.print( "Enter Pass Code" );  
		}
		else{
			doubleBeep();
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("No Tag Detected");
			lcd.setCursor(0,1);
			lcd.print("Place RFID Card");
			delay(2000);
		}
		return;
	}

	case 3:{
		clearRecord();
		state=0;
		delay(2000);
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print( "LOCKED !!!" );
		lcd.setCursor(0,1);
		lcd.print( "Enter Pass Code" );   
	}
	break;

	}
}

// ---------------------------------- make2explore.com----------------------------------------------------//