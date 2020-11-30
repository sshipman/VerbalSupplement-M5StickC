
/*
    Based on BLE_UART example, and battery level example from 
    https://circuitdigest.com/microcontroller-projects/esp32-ble-server-how-to-use-gatt-services-for-battery-level-indication

    Create a BLE server that provides 2 services:
    1) UART serial, which will mostly be used to receive messages from the phone
     The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
     Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
     Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"
    2) Battery level, for convenience


   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   The rxValue is the data received, which will eventually be displayed on the screen.

   
*/
#include <Arduino.h>
#include <M5StickC.h>
#include <tb_display.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

long loopTime, startTime = 0;

// battery level definitions
#define BatteryService BLEUUID((uint16_t)0x180F) 
BLECharacteristic BatteryLevelCharacteristic(BLEUUID((uint16_t)0x2A19), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define UART_SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyUARTCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
        tb_display_print_String(rxValue.c_str());
      }
    }
};

double getBatteryLevel()
{
  uint16_t vbatData = M5.Axp.GetVbatData();
  double vbat = vbatData * 1.1 / 1000;
  return 100.0 * ((vbat - 3.0) / (4.07 - 3.0));
}

void initBLE() {
  // Create the BLE Device
  BLEDevice::init("VerbalSupplement");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the UART BLE Service
  BLEService *pUARTService = pServer->createService(UART_SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pUARTService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pUARTService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyUARTCallbacks());

  // Start the UART service
  pUARTService->start();

  // Create the Battery service
  BLEService *pBattery = pServer->createService(BatteryService);

  pBattery->addCharacteristic(&BatteryLevelCharacteristic);
  BatteryLevelDescriptor.setValue("Percentage 0 - 100");
  BatteryLevelCharacteristic.addDescriptor(&BatteryLevelDescriptor);
  BatteryLevelCharacteristic.addDescriptor(new BLE2902());

  pServer->getAdvertising()->addServiceUUID(BatteryService);

  pBattery->start();
  
  // Start advertising
  pServer->getAdvertising()->start();  
  
}

void setup() {
  M5.begin();
  M5.Axp.SetAdcState(false);
  M5.Axp.ScreenBreath(10);
  Serial.begin(115200);

  tb_display_init(1, 3);
  tb_display_word_wrap = false;

  initBLE();

  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  loopTime = millis();

    if (deviceConnected) {
      if (startTime < loopTime - 5000) {
        uint8_t level = (int) getBatteryLevel();
        BatteryLevelCharacteristic.setValue(&level, 1);
        BatteryLevelCharacteristic.notify();
        startTime = loopTime;
      }
	  } else {
      if (startTime < loopTime - 10000) {
        Serial.println("Nobody connected.  Going back to sleep");
        M5.Axp.DeepSleep(0);
      }
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
        Serial.println("disconnected, turning screen off");
        M5.Axp.SetLDO2(false);
        M5.Axp.DeepSleep(0);
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        Serial.println("connected, turning screen on");
        tb_display_clear();
        tb_display_show();
        M5.Axp.SetLDO2(true);
    }
    delay(20);
}
