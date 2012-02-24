#include <XBee.h>
#include <Tlc5940.h>

#define LIGHTS_MAX 16

XBee xbee = XBee();
ZBRxResponse zbRes = ZBRxResponse();

bool flag = true;
int powerLed = 4;

void setup()
{
  delay(1000);
  pinMode(powerLed, OUTPUT);
  digitalWrite(powerLed, HIGH);

  xbee.begin(115200);

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

      for (int i = 0; i < LIGHTS_MAX; i++) {
        if ((targets & (1 << i)) != 0) Tlc.set(i, volume);
      }

      Tlc.update();
      delay(time);
    }
  }

  Tlc.update();
  delay(10);
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
