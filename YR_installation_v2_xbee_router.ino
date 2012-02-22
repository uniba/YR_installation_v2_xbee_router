#include <XBee.h>
#include <Tlc5940.h>

XBee xbee = XBee();
XBeeResponse xbRes = XBeeResponse();
ZBRxResponse zbRx = ZBRxResponse();

uint16_t tlcValues[NUM_TLCS * 16];

const int START_BYTES[] = { 0x22, 0x75 };

//-----------------------------------------------------------------
void setup()
{
  xbee.begin(115200);
  
  for (int i = 0; i < sizeof(tlcValues); ++i)
  {
    tlcValues[i] = 0;
  }
  
  Tlc.init();
}

//-----------------------------------------------------------------
void loop()
{
  xbee.readPacket();
  if (xbee.getResponse().isAvailable())
  {
    digitalWrite(7, HIGH);
    xbee.getResponse().getZBRxResponse(zbRx);
    
    if (ZB_RX_RESPONSE == xbee.getResponse().getApiId())
    {
      xbee.getResponse().getZBRxResponse(zbRx);
      if (START_BYTES[0] == zbRx.getData(0) && START_BYTES[1] == zbRx.getData(1))
      {
        int index = zbRx.getData(2);
        
        tlcValues[index] = 4095;
      }
      else
      {
        
      }
    }
  }
  
  Tlc.clear();
  for (int i = 0; i < NUM_TLCS * 16; ++i)
  {
    Tlc.set(i, tlcValues[i]);
  }
  Tlc.update();
  delay(75);
}

//-----------------------------------------------------------------
void writeTlcPin()
{
  int pinNum = zbRx.getData(2);
  uint16_t value = decodeBytes((int[]){ zbRx.getData(3), zbRx.getData(4), zbRx.getData(5), zbRx.getData(6) });
  tlcValues[pinNum] = value;
}

//-----------------------------------------------------------------
uint16_t decodeBytes(int bytes[])
{
  return (uint16_t)(bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3]);
}
