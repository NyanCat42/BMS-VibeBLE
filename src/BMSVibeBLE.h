#pragma once
#include <NimBLEDevice.h>
#include <vector>

struct BMSData {
  float voltage = 0;
  float current = 0;
  uint16_t remainingCapacity = 0;
  uint16_t fullCapacity = 0;
  uint16_t cycleCount = 0;
  uint16_t balanceStatus = 0;
  uint16_t protectionStatus = 0;
  uint16_t software_version = 0;
  uint8_t stateOfCharge = 0;
  uint16_t mosfet_status = 0;
  uint8_t numCells = 0;
  uint8_t numNTCs = 0;
  std::vector<float> temperatures;
  String productionDate;
  bool valid = false;
};

class BMSVibeBLE {
 public:
  BMSVibeBLE(const char* macAddress);
  void begin();
  void update();
  bool isConnected() const;
  BMSData getData() const;

 private:
  void connectToBMS();
  void parseFrame(const std::vector<uint8_t>& frame);
  void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify);

  NimBLEAdvertisedDevice* advDevice = nullptr;
  NimBLEClient* pClient = nullptr;
  NimBLERemoteCharacteristic* pNotifyChar = nullptr;
  NimBLERemoteCharacteristic* pWriteChar = nullptr;

  std::vector<uint8_t> frameBuffer;
  std::string targetMAC;
  bool doConnect = false;
  bool connected = false;
  BMSData latestData;

  static void staticNotifyCallback(NimBLERemoteCharacteristic*, uint8_t*, size_t, bool);
  static BMSVibeBLE* instance;
};
