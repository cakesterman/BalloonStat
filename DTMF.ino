#include <Servo.h>

/* Hamshield
 * Example: DTMF
 * This is a simple example to demonstrate how to use DTMF.
 *
 * Connect the HamShield to your Arduino. Screw the antenna 
 * into the HamShield RF jack. 
 * Connect the Arduino to wall power and then to your computer
 * via USB. After uploading this program to your Arduino, open
 * the Serial Monitor. Press the switch on the HamShield to 
 * begin setup. After setup is complete, type in a DTMF value
 * (0-9, A, B, C, D, *, #) and hit enter. The corresponding
 * DTMF tones will be transmitted. The sketch will also print
 * any received DTMF tones to the screen.
**/

#include </home/pi/src/HamShield/examples/DTMF/HamShield.h>
//#include </home/pi/src/HamShield/examples/DTMF/HamShield.cpp>
#include </home/pi/src/HamShield/examples/DTMF/HamShield_comms.h>
//include </home/pi/src/HamShield/examples/DTMF/HamShield_comms.cpp>
//#include <vector>

// create object for radio
HamShield radio;

#define LED_PIN 13

#define MIC_PIN 3
#define RESET_PIN A3
#define SWITCH_PIN 2

uint32_t freq;

char codeList [3];
int counter = 0;

int servoPin = 7;
int pulse = 500;

int ledPin = 10;

//vector<char> codeList;

void setup() {
  // NOTE: if not using PWM out, it should be held low to avoid tx noise
  pinMode(MIC_PIN, OUTPUT);
  digitalWrite(MIC_PIN, LOW);
  
  // prep the servo
  pinMode(servoPin, OUTPUT);
  
  //prep the LED
  pinMode(ledPin, OUTPUT);
  
  // set up the reset control pin
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  
  // initialize serial communication
  Serial.begin(9600);
  //Serial.println("press the switch to begin...");
  
  //while (digitalRead(SWITCH_PIN));
  
  // now we let the AU ot of reset
  digitalWrite(RESET_PIN, HIGH);
  delay(5); // wait for device to come up
  
  Serial.println("beginning radio setup");

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(radio.testConnection() ? "HamShield connection successful" : "HamShield connection failed");

  // initialize device
  radio.initialize();

  Serial.println("setting default Radio configuration");

  Serial.println("setting squelch");

  radio.setSQHiThresh(-10);
  radio.setSQLoThresh(-30);
  Serial.print("sq hi: ");
  Serial.println(radio.getSQHiThresh());
  Serial.print("sq lo: ");
  Serial.println(radio.getSQLoThresh());
  radio.setSQOn();
  //radio.setSQOff();

  Serial.println("setting frequency to: ");
  freq = 432100; // 70cm calling frequency
  radio.frequency(freq);
  Serial.print(radio.getFrequency());
  Serial.println("kHz");
  
  // set RX volume to minimum to reduce false positives on DTMF rx
  radio.setVolume1(6);
  radio.setVolume2(0);
  
  // set to receive
  radio.setModeReceive();
  
  radio.setRfPower(0);
    
  // configure Arduino LED for
  pinMode(LED_PIN, OUTPUT);

  // set up DTMF
  radio.enableDTMFReceive();
  
  /* DTMF timing settings are optional.
   * These times are set to default values when the device is started.
   * You may want to change them if you're DTMF receiver isn't detecting
   * codes from the HamShield (or vice versa).
   */
  radio.setDTMFDetectTime(24); // time to detect a DTMF code, units are 2.5ms
  radio.setDTMFIdleTime(50); // time between transmitted DTMF codes, units are 2.5ms
  radio.setDTMFTxTime(60); // duration of transmitted DTMF codes, units are 2.5ms
  
  Serial.println("ready");
}

void loop() {
  
  // look for tone
  char m = radio.DTMFRxLoop();
  if (m != 0) {
   
    // if user presses * it will reset the code list
    if(m == '*') {
      
      resetCodeList();
    
    }else {
    
      // if the list size reaches greater than 2, it will reset the list
      if(counter > 2) {
        
        resetCodeList();
      }
      
      codeList[counter] = m; 
      
      // for loop prints out the current code list
      for(int x = 0; x < 3; x++) {
        
        Serial.print(codeList[x]);
        Serial.print(", ");
      }
      
      Serial.println();
      
      counter++;
      
      // This checks to see if the user entered in the pre-determined combo to trigger the servo
      if(codeList[0] == '1' && codeList[1] == '4' && codeList[2] == '7') {
       
       Serial.println("HELL YEAH, combo has been produced");
       
       // This moves the servo in a for loop
       for(int x = 0; x < 100; x++) {
       
         digitalWrite(ledPin, HIGH);
         
         digitalWrite(servoPin, HIGH);
         delayMicroseconds(pulse);
         digitalWrite(servoPin, LOW);
         delay(10);

         digitalWrite(ledPin, LOW);
         
       
       }
       
       
       
      }
      
      Serial.println("----------------------------------------------------");
    
    }
    
    //Uncomment this V if you want to view what is outputted each time
    //Serial.print(m );
  }
  
  // Is it time to send tone?
  if (Serial.available()) {
    // get first code
    uint8_t code = radio.DTMFchar2code(Serial.read());
    
    // start transmitting
    radio.setDTMFCode(code); // set first
    radio.setTxSourceTones();
    radio.setModeTransmit();
    delay(300); // wait for TX to come to full power
    
    bool dtmf_to_tx = true;
    while (dtmf_to_tx) {
      // wait until ready
      while (radio.getDTMFTxActive() != 1) {
        // wait until we're ready for a new code
        delay(10);
      }
      if (Serial.available()) {
        code = radio.DTMFchar2code(Serial.read());
        if (code == 255) code = 0xE; // throw a * in there so we don't break things with an invalid code
        radio.setDTMFCode(code); // set first
      } else {
        dtmf_to_tx = false;
        break;
      }

      while (radio.getDTMFTxActive() != 0) {
        // wait until this code is done
        delay(10);
      }

    }
    // done with tone
    radio.setModeReceive();
    radio.setTxSourceMic();
  }
}

// function resets the code list to white spaces
void resetCodeList() {

  counter = 0;
      
  codeList[0] = ' ';
  codeList[1] = ' ';
  codeList[2] = ' ';
}
