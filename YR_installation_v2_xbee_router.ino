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
#include "Tlc5940fader.h"

#define LIGHTS_MAX 16

XBee xbee = XBee();
ZBRxResponse zbRes = ZBRxResponse();

TLC_CHANNEL_TYPE channel;

bool flag = true;
int powerLed = 4;
int napion = 0;
int val = 0;
int prevVal = 0;

bool fading = false;
uint32_t start = 0;
uint32_t xbeeKick = 0;
uint32_t napIgnore = 0;

void setup()
{
  delay(1000);
  pinMode(powerLed, OUTPUT);
  digitalWrite(powerLed, HIGH);

  xbee.begin(115200);
  //Serial.begin(9600);

  Tlc.init();
}

void loop()
{
  Tlc.clear();

  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
    {
      xbee.getResponse().getZBRxResponse(zbRes);
      uint8_t devices = zbRes.getData(3);
      uint16_t targets = (zbRes.getData(4) << 8) + zbRes.getData(5);
      uint16_t volume = (zbRes.getData(6) << 8) + zbRes.getData(7);
      uint16_t time = (zbRes.getData(8) << 8) + zbRes.getData(9);

      // Napion の信号を無視する時間をセット
      //TODO : napion と 音楽の優先順位をコマンド切り替え可能にする
      xbeeKick = millis();
      napIgnore = time;

      fadeAll(targets, 1 + volume / 4, time);

      // TODO: send signal to cordinator
    }
  }

  // TODO: val が上下動するのでまるめる(中央値もしくは平均値)
  val = analogRead(napion);

  if ((abs(prevVal - val) > 400) && (millis() - xbeeKick > napIgnore))  {
    if (val > 400) {
      fadeAll(65535, 1024, 1000);
    } else {
      fadeAll(65535, 1, 1000);
    }
  }

  prevVal = val;

  tlc_updateFades(millis());
}

void fade(TLC_CHANNEL_TYPE channel, uint16_t current, uint16_t value, uint16_t time){
    uint32_t startMillis = millis() + 50;
    uint32_t endMillis = startMillis + time;

    tlc_removeFades(channel);
    if (current == value) {
      Tlc.set(channel, value);
    } else {
      tlc_addFade(channel, current, value, startMillis, endMillis);
    }
}

void fadeAll(uint16_t targets, uint16_t value, uint16_t time) {
  for (int i = 0; i < LIGHTS_MAX; i++) {
    if ((targets & (1 << i)) != 0) fade(i, tlc_getCurrentValue(i), value, time);
  }
}
