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

int startBytesForModeA[] = { 0x22, 0x75 };
int startBytesForModeB[] = { 0x22, 0x76 };
int startBytesForModeC[] = { 0x22, 0x77 };
int startBytesForModeD[] = { 0x22, 0x78 };
int startBytesForModeE[] = { 0x22, 0x79 };
char currentCommand = 'c';

bool isAutoFadeInOutEnabled = true;
bool isRandomFadeInOutEnabled = false;
bool fadeInOutAuto = true;
bool isHigh = false;
bool isFadeInOutAuto[LIGHTS_MAX * NUM_TLCS];
uint16_t autoFadeInOutValueMax = 64;
uint16_t autoFadeInOutTime = 3000;

int valuesForModeD[] = { 0, 2048 };

uint32_t _target1 = 0;
uint32_t _target2 = 0;


void setup()
{
  delay(1000);

  xbee.begin(115200);
  
  randomSeed(analogRead(0));
  
  Tlc.init(0);
  
  for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
          isFadeInOutAuto[i] = true;
        }
}

void loop()
{
  Tlc.clear();

  xbee.readPacket();
  uint32_t loopStart = millis();

  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      xbee.getResponse().getZBRxResponse(zbRes);
      
      int startBytesInReceivedData[] = { zbRes.getData(0), zbRes.getData(1) };
      uint8_t devices = zbRes.getData(5);
      _target1 = (zbRes.getData(6) << 24) + (zbRes.getData(7) << 16) + (zbRes.getData(8) << 8) + zbRes.getData(9);
      _target2 = (zbRes.getData(10) << 24) + (zbRes.getData(11) << 16) + (zbRes.getData(12) << 8) + zbRes.getData(13);
      uint16_t volume = (zbRes.getData(14) << 8) + zbRes.getData(15);
      uint16_t time = (zbRes.getData(16) << 8) + zbRes.getData(17);
      
      if (startBytesForModeA[0] == startBytesInReceivedData[0] && startBytesForModeA[1] == startBytesInReceivedData[1]) {
        isAutoFadeInOutEnabled = false;
        fadeInOutAuto = true;
        currentCommand = 'a';
        fadeAll(_target1, _target2, volume, time);
        for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
          isFadeInOutAuto[i] = true;
        }
      } else if (startBytesForModeB[0] == startBytesInReceivedData[0] && startBytesForModeB[1] == startBytesInReceivedData[1]) {
        // TODO:ピンを指定できるようにする
        isAutoFadeInOutEnabled = false;
        currentCommand = 'b';
        flickAndFadeOutAll(_target1, _target2, volume, time);
      } else if (startBytesForModeC[0] == startBytesInReceivedData[0] && startBytesForModeC[1] == startBytesInReceivedData[1]) {
        isAutoFadeInOutEnabled = true;
        fadeInOutAuto = true;
        currentCommand = 'c';
        autoFadeInOutValueMax = volume;
        autoFadeInOutTime = time;
        for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
          isFadeInOutAuto[i] = true;
        }
      } else if (startBytesForModeD[0] == startBytesInReceivedData[0] && startBytesForModeD[1] == startBytesInReceivedData[1]) {
        isAutoFadeInOutEnabled = false;
        currentCommand = 'd';
        fadeInOutAuto = false;
        for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
          isFadeInOutAuto[i] = false;
        }
        //randomFlickAndFadeOutAll(target1, target2, volume, time);
      } else if (startBytesForModeE[0] == startBytesInReceivedData[0] && startBytesForModeE[1] == startBytesInReceivedData[1]) {
        isAutoFadeInOutEnabled = false;
        currentCommand = 'e';
        fadeInOutAuto = false;
        for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
          isFadeInOutAuto[i] = false;
        }
      }
    }
  } else {
    if ('c' == currentCommand && isAutoFadeInOutEnabled) {
      for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
        if (isFadeInOutAuto[i]) {
          if (0 == tlc_getCurrentValue(i)) {
            fade(i, tlc_getCurrentValue(i), autoFadeInOutValueMax, 1000 + random(autoFadeInOutTime));
            isFadeInOutAuto[i] = false;
          }
        } else {
          if (autoFadeInOutValueMax <= tlc_getCurrentValue(i)) {
            fade(i, tlc_getCurrentValue(i), 0, 1000 + random(autoFadeInOutTime));
            isFadeInOutAuto[i] = true;
          }
        }
      }
    } else if ('d' == currentCommand && !isAutoFadeInOutEnabled) {
      setRandomAll(_target1, _target2);
    } else if ('e' == currentCommand && !isAutoFadeInOutEnabled) {
      strobo();
    }
  }

  zbRes.init();
  Serial.flush();
  
  if ('d' != currentCommand) {
    tlc_updateFades(loopStart);
    delay(20);
  } else if ('d' == currentCommand) {
    delay(56);
  } else if ('e' == currentCommand) {
    delay(10);
  }
}

void fade(TLC_CHANNEL_TYPE channel, uint16_t current, uint16_t value, uint16_t time){
    uint32_t startMillis = millis() + 50;
    uint32_t endMillis = startMillis + time;

    tlc_removeFades(channel);
    tlc_addFade(channel, current, value, startMillis, endMillis);
}

void fade(TLC_CHANNEL_TYPE channel, uint16_t current, uint16_t value, uint16_t time, uint16_t delay){
    uint32_t startMillis = millis() + delay;
    uint32_t endMillis = startMillis + time;

    tlc_removeFades(channel);
    tlc_addFade(channel, current, value, startMillis, endMillis);
}

void fadeAll(uint32_t target1, uint32_t target2, uint16_t value, uint16_t time) {
  int offset = 16;
  for (int i = 0; i < LIGHTS_MAX; ++i) {
    if ((target1 & (1 << i)) != 0) {
      fade(i, tlc_getCurrentValue(i), value, time);
    } else {
      fade(i, tlc_getCurrentValue(i), tlc_getCurrentValue(i), time);
    }
    
    int index = offset + i;
    if ((target2 & (1 << i)) != 0) {
      fade(index, tlc_getCurrentValue(index), value, time);
    } else {
      fade(index, tlc_getCurrentValue(index), tlc_getCurrentValue(index), time);
    }
  }
}

void flickAndFadeOutAll(uint32_t target1, uint32_t target2, uint32_t value, uint16_t time)
{
  for (int i = 0; i < LIGHTS_MAX; ++i) {
    if ((target1 & (1 << i)) != 0) {
      fade(i, value, 0, time);
    }
    if ((target2 & (1 << i)) != 0) {
      fade(LIGHTS_MAX + i, value, 0, time);
    }
  }
}

void randomFlickAndFadeOutAll(uint32_t target1, uint32_t target2, uint32_t value, uint16_t time)
{
  for (int i = 0; i < LIGHTS_MAX; ++i) {
    if (target1 & (1 << i) != 0) {
      fade(i, random(value), 0, time);
    } else {
      fade(i, tlc_getCurrentValue(i), tlc_getCurrentValue(i), time);
    }
    
    int index = LIGHTS_MAX + i;
    if (target2 & (1 << i) != 0) {
      fade(index, random(value), 0, time);
    } else {
      fade(i, tlc_getCurrentValue(index), tlc_getCurrentValue(index), time);
    }
  }
}

void fadeInToMaxAll(uint16_t value, uint16_t time)
{

  for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
    fade(i, tlc_getCurrentValue(i), value, time);
  }
}

void fadeOutToMinAll(uint16_t time)
{
  for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
    fade(i, tlc_getCurrentValue(i), 0, time);
  }
}

void fadeInToMaxAllByRandomDuration(uint16_t value, uint16_t time)
{
  for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
    fade(i, tlc_getCurrentValue(i), value, 50 + random(time));
  }
}

void fadeOutToMinAllByRandomDuration(uint16_t time)
{
  for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
    fade(i, tlc_getCurrentValue(i), 0, 50 + random(time));
  }
}

void setRandomAll(uint32_t target1, uint32_t target2)
{
  for (int i = 0; i < LIGHTS_MAX; ++i) {
    if (target1 & (1 << i) != 0) {
      tlc_removeFades(i);
      Tlc.set(i, valuesForModeD[random(2)]);
    }
    
    int index = LIGHTS_MAX + i;
    if (target2 & (1 << 0) != 0) {
      tlc_removeFades(index);
      Tlc.set(index, valuesForModeD[random(2)]);
    }
  }
  Tlc.update();
}

void strobo()
{
  for (int i = 0; i < LIGHTS_MAX * NUM_TLCS; ++i) {
    tlc_removeFades(i);
    if (!isHigh) {
      Tlc.set(i, 4095);
      isHigh = true;
    } else {
      Tlc.set(i, 0);
    }
  }
  Tlc.update();
}
