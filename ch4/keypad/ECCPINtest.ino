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

//The following is used to generate random seed, to be replaced by HRNG
extern "C" {

static int RNG(uint8_t *dest, unsigned size) {
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of 
  // random noise). This can take a long time to generate random data if the result of analogRead(0) 
  // doesn't change very frequently.
  while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
      int init = analogRead(0);
      int count = 0;
      while (analogRead(0) == init) {
        ++count;
      }
      
      if (count == 0) {
         val = (val << 1) | (init & 0x01);
      } else {
         val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

}  // extern "C"


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
  keygen ();
}

void keygen() {
  uint8_t private1[uECC_BYTES];
  uint8_t private2[uECC_BYTES];
  
  uint8_t public1[uECC_BYTES * 2];
  uint8_t public2[uECC_BYTES * 2];
  
  uint8_t secret1[uECC_BYTES];
  uint8_t secret2[uECC_BYTES];
  
  unsigned long a = millis();
  uECC_make_key(public1, private1);
  unsigned long b = millis();
  
  Serial.print("Made key 1 in "); Serial.println(b-a);
  a = millis();
  uECC_make_key(public2, private2);
  b = millis();
  Serial.print("Made key 2 in "); Serial.println(b-a);

  a = millis();
  int r = uECC_shared_secret(public2, private1, secret1);
  b = millis();
  Serial.print("Shared secret 1 in "); Serial.println(b-a);
  if (!r) {
    Serial.print("shared_secret() failed (1)\n");
    return;
  }

  a = millis();
  r = uECC_shared_secret(public1, private2, secret2);
  b = millis();
  Serial.print("Shared secret 2 in "); Serial.println(b-a);
  if (!r) {
    Serial.print("shared_secret() failed (2)\n");
    return;
  }
    
  if (memcmp(secret1, secret2, sizeof(secret1)) != 0) {
    Serial.print("Shared secrets are not identical!\n");
  } else {
    Serial.print("Shared secrets are identical\n");
  }
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
            Serial.println();
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
