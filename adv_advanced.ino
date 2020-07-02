/*********************************************************************
  This is an example for our nRF52 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

/* This sketch demonstrates the Bluefruit.Advertising API(). When powered up,
   the Bluefruit module will start advertising for ADV_TIMEOUT seconds (by
   default 30 seconds in fast mode, the remaining time slow mode) and then
   stop advertising completely. The module will start advertising again if
   PIN_ADV is grounded.
*/
#include <bluefruit.h>

#define PIN_ADV       A0
#define ADV_TIMEOUT   20 // seconds

SoftwareTimer blinkTimer;

int counter = 0;

int val1 = 1;
int val2 = 1;
int val3 = 1;

void setup()
{
  // configure PIN_ADV as input with a pullup (pin is active low)
  pinMode(PIN_ADV, INPUT_PULLUP);

  Serial1.begin(115200);
  while ( !Serial1 ) delay(10);   // for nrf52840 with native usb

  Serial1.println("Bluefruit52 Advanced Advertising Example");
  Serial1.println("----------------------------------------\n");

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("Bluefruit52");

  // Set up and start advertising
  startAdv();

  Serial1.println("Advertising is started");
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  uint8_t CUSTOM_UUID[] =
  {
    val1++, val2++, val3++, 0x6A, 0x00, 0xA6, 0xF7, 0x8C,
    0xE7, 0x11, 0x8F, 0x71, 0x1A, 0xFF, 0x67, 0xDF
  };

  BLEUuid uuid = BLEUuid(CUSTOM_UUID);

  Bluefruit.Advertising.addUuid(uuid);

  uint8_t msd_payload[4]; // Two bytes are required for the CID, so we have 2 bytes user data, expand as needed
  uint16_t msd_cid = 0xFFFF;
  memset(msd_payload, 0, sizeof(msd_payload));
  memcpy(msd_payload, (uint8_t*)&msd_cid, sizeof(msd_cid));
  msd_payload[2] = 0x11;
  msd_payload[3] = counter++;
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, msd_payload, sizeof(msd_payload));
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)

     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.setStopCallback(adv_stop_callback);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in units of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(20);      // number of seconds in fast mode
  Bluefruit.Advertising.start(ADV_TIMEOUT);      // Stop advertising entirely after ADV_TIMEOUT seconds
}

static int flag = 0;
void loop()
{
  // Only check pin when advertising has already stopped
  if ( !Bluefruit.Advertising.isRunning() )
  {
    // Check if Pin is grounded
    //    if ( digitalRead(PIN_ADV) == 0 )
    if (flag != 0)
    {
      flag = 0;
      Bluefruit.Advertising.clearData();
      Bluefruit.ScanResponse.clearData(); // add this
      startAdv();
      Serial1.println("Advertising is started again");

    }
  }
  __WFI();
}

/**
   Callback invoked when advertising is stopped by timeout
*/
void adv_stop_callback(void)
{
  Serial1.println("Advertising time passed, advertising will now stop.");
  flag = 1;
}
