/*
 *
    Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|-> PowerLED                  .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    Analog Pin 0 -> NaPiOn

*/

#include <XBee.h>
#include <Tlc5940.h>
#include <tlc_fades.h>

#define LIGHTS_MAX 16

XBee xbee = XBee();
ZBRxResponse zbRes = ZBRxResponse();

TLC_CHANNEL_TYPE channel;

bool flag = true;
int powerLed = 4;
int napion = 0;
int val = 0;

int debugSwitch = 2;
bool fading = false;
uint32_t start = 0;

void setup()
{
  delay(1000);
  pinMode(powerLed, OUTPUT);
  pinMode(debugSwitch, OUTPUT);
  digitalWrite(powerLed, HIGH);

  //xbee.begin(115200);
  Serial.begin(9600);

  Tlc.init();
}

void loop()
{
  Tlc.clear();
  /*
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
    {
      xbee.getResponse().getZBRxResponse(zbRes);
      uint8_t devices = zbRes.getData(3);
      uint16_t targets = (zbRes.getData(4) << 8) + zbRes.getData(5);
      uint16_t volume = (zbRes.getData(6) << 8) + zbRes.getData(7);
      uint16_t time = (zbRes.getData(8) << 8) + zbRes.getData(9);

      for (int i = 0; i < LIGHTS_MAX; i++) {
        if ((targets & (1 << i)) != 0) Tlc.set(i, volume);
      }

      Tlc.update();
      delay(time);
    }
  }
  */

  val = analogRead(napion);
  if ((val > 400) && !fading) {
    fading = true;
    start = millis();
  } else {
  }

  if (fading) {
    fadeIn(1, 1024, 3000);
  }

  if (millis() - start > 3000) {
    fading = false;
  }

  Serial.println(fading);
  Serial.println(start);


  digitalWrite(debugSwitch, LOW);
  Tlc.update();
  delay(10);
}

void fadeIn(uint16_t targets, uint16_t volume, uint16_t time)
{
  for (int i = 0; i < LIGHTS_MAX; i++) {
    //if ((targets & (1 << i)) != 0) Tlc.set(i, volume);
    if ((targets & (1 << i)) != 0) {
      if (tlc_fadeBufferSize < TLC_FADE_BUFFER_LENGTH - 2) {
        if (!tlc_isFading(channel)) {
          uint32_t startMillis = millis() + 50;
          uint32_t endMillis = startMillis + time / 2;
          tlc_addFade(i, 0, volume, startMillis, endMillis);
        }
      }
    }
  }

  tlc_updateFades();
}

void fadeInOut(uint16_t targets, uint16_t volume, uint16_t time)
{
  for (int i = 0; i < LIGHTS_MAX; i++) {
    //if ((targets & (1 << i)) != 0) Tlc.set(i, volume);
    if ((targets & (1 << i)) != 0) {
      if (tlc_fadeBufferSize < TLC_FADE_BUFFER_LENGTH - 2) {
        if (!tlc_isFading(channel)) {
          uint32_t startMillis = millis() + 50;
          uint32_t endMillis = startMillis + time / 2;
          tlc_addFade(i, 0, volume, startMillis, endMillis);
          tlc_addFade(i, volume, 0, endMillis, endMillis + time / 2);
        }
      }
    }
  }

  tlc_updateFades();
}

void debug(uint8_t tgt)
{
  for (int i = 0; i / 2 < tgt / 16; i++) {
    if (i % 2 == 0) {
      Tlc.set(tgt % 16, 4095);
    } else {
      Tlc.set(tgt % 16, 512);
    }
    Tlc.update();
    delay(500);
  }
}
