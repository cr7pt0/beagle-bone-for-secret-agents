/**
 * Keypad entry w/ecc 
 * Tim Steiner
 * 
 * This is a modified Keypad entry sketch for Arduino based on the original keypad.ino by Joshua Datko <jbdatko@gmail.com>
 * "https://www.packtpub.com/hardware-and-creative/beaglebone-secret-agents".
 *
 */
#include <Keypad.h>
#include <Wire.h>
#include <EEPROM.h>
#include <uECC.h>

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] =
  {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
  };

/* Connect as follows:
   This is the following keypad:
   https://www.sparkfun.com/products/8653
   Keypad Pin -> Arduino Digital
   3 -> 2
   1 -> 3
   5 -> 4
   2 -> 5
   7 -> 6
   6 -> 7
   4 -> 8
*/

/* connect to the row pinouts of the keypad */
byte rowPins[ROWS] = {5, 6, 7, 8};
/* connect to the column pinouts of the keypad */
byte colPins[COLS] = {2, 3, 4};

/* Declare program constants */
const int LED = 13;
Keypad keypad = Keypad( makeKeymap (keys), rowPins, colPins, ROWS, COLS );
const int I2C_ADDR = 0x42;
/* This is a stupid combination; one an idiot would have on his
   luggage - Spaceballs, 1987*/
char passcode[] = {'1', '2', '3', '4', '5'};
/* flag to trigger when to go collect the code or not */
bool collect_code = false;



/**
 * Obligatory Arduino setup function
 *
 */
void setup ()
{
  pinMode (LED, OUTPUT);
  digitalWrite (LED, LOW);
  Wire.begin (I2C_ADDR);
  Wire.onRequest (provide_code);
  Wire.onReceive (receiveEvent3);
  Serial.begin(19200);
}

/**
 * Briefly flash the LED
 *
 */
void flash()
{
  digitalWrite (LED, LOW);
  delay (100);
  digitalWrite (LED, HIGH);
}

/**
 * Collect the keypad code into the buffer. This function blocks on
 * each key.
 *
 * @param buf The buffer to fill
 * @param len The length of the buffer to fill
 *
 * @return true if filled, otherwise false
 */
bool get_code (char * buf, const int len)
{

  bool result = false;

  if (NULL != buf)
    {
      for (int x = 0; x < len; x++)
        {
          buf[x] = keypad.waitForKey();
            Serial.println("PIN Entered");
            Serial.println(buf[x]);
          keygen ();
          flash ();
        }
      result = true;
    }

  return result;
}


void loop()
{
  delay (100);

  if (collect_code)
    {
      digitalWrite (LED, HIGH);
      get_code (passcode, sizeof(passcode));
      digitalWrite (LED, LOW);
      collect_code = false;
    }


}

void provide_code ()
{
  Wire.write ((uint8_t *)passcode, sizeof(passcode));
}

void receiveEvent3(int bytes)
{

  /* Pull the data off, but we don't care what it is */
  while(0 < Wire.available())
    {
      char c = Wire.read();
    }

  collect_code = true;

}

void keygen() {
  uint8_t private1[uECC_BYTES];
  uint8_t public1[uECC_BYTES * 2];

  uECC_make_key(public1, private1);
  
  Serial.println("Public Key");
  Serial.println(public1);
  Serial.println("Private Key");
  Serial.println(private1);

}
