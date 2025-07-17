#include "BMSVibeBLE.h"

static const NimBLEUUID serviceUUID("0000FF00-0000-1000-8000-00805F9B34FB");
static const NimBLEUUID notifyCharUUID("0000FF01-0000-1000-8000-00805F9B34FB");
static const NimBLEUUID writeCharUUID("0000FF02-0000-1000-8000-00805F9B34FB");
static const uint8_t readBasicInfo[] = { 0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77 };

BMSVibeBLE* BMSVibeBLE::instance = nullptr;

BMSVibeBLE::BMSVibeBLE(const char* macAddress) {
  targetMAC = std::string(macAddress);
  std::transform(targetMAC.begin(), targetMAC.end(), targetMAC.begin(), ::tolower);
  instance = this;
}

void BMSVibeBLE::begin() {
  NimBLEDevice::init("");
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(45);
  scan->setWindow(15);
  NimBLEScanResults results = scan->getResults(10000, false);

  for (int i = 0; i < results.getCount(); ++i) {
    const NimBLEAdvertisedDevice* d = results.getDevice(i);
    String addr = d->getAddress().toString().c_str();
    addr.toLowerCase();
    if (addr == targetMAC.c_str()) {
      advDevice = new NimBLEAdvertisedDevice(*d);
      doConnect = true;
      break;
    }
  }

  if (doConnect) connectToBMS();
}

void BMSVibeBLE::connectToBMS() {
  pClient = NimBLEDevice::createClient();
  if (!pClient->connect(advDevice)) return;

  auto svc = pClient->getService(serviceUUID);
  if (!svc) return;

  pNotifyChar = svc->getCharacteristic(notifyCharUUID);
  pWriteChar = svc->getCharacteristic(writeCharUUID);

  if (pNotifyChar && pNotifyChar->canNotify()) {
    pNotifyChar->subscribe(true, staticNotifyCallback);
  }

  if (pWriteChar && pWriteChar->canWrite()) {
    pWriteChar->writeValue(readBasicInfo, sizeof(readBasicInfo), false);
  }

  connected = true;
}

void BMSVibeBLE::update() {
  if (connected && pWriteChar) {
    pWriteChar->writeValue(readBasicInfo, sizeof(readBasicInfo), false);
  }
}

void BMSVibeBLE::staticNotifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* data, size_t len, bool isNotify) {
  if (instance) instance->notifyCallback(pChar, data, len, isNotify);
}

void BMSVibeBLE::notifyCallback(NimBLERemoteCharacteristic*, uint8_t* data, size_t len, bool) {
  frameBuffer.insert(frameBuffer.end(), data, data + len);
  if (frameBuffer.size() >= 4 && frameBuffer[0] == 0xDD && frameBuffer[1] == 0x03) {
    uint16_t dataLen = (frameBuffer[2] << 8) | frameBuffer[3];
    size_t totalLen = 4 + dataLen + 2 + 1;
    if (frameBuffer.size() >= totalLen) {
      parseFrame(frameBuffer);
      frameBuffer.clear();
    }
  } else if (frameBuffer.size() >= 4) {
    frameBuffer.clear();
  }
}

void BMSVibeBLE::parseFrame(const std::vector<uint8_t>& f) {
  if (f.size() < 40) return;

  latestData.valid = true;
  latestData.voltage = ((f[4] << 8) | f[5]) / 100.0f;
  latestData.current = ((int16_t)((f[6] << 8) | f[7])) / 100.0f;
  latestData.remainingCapacity = (f[8] << 8) | f[9];
  latestData.fullCapacity = (f[10] << 8) | f[11];
  latestData.cycleCount = (f[12] << 8) | f[13];
  uint16_t prodDate = (f[14] << 8) | f[15];
  uint16_t year = (prodDate >> 9) + 2000;
  uint8_t month = (prodDate >> 5) & 0x0F;
  uint8_t day = prodDate & 0x1F;
  latestData.productionDate = String(year) + "-" + String(month) + "-" + String(day);
  latestData.balanceStatus = (f[16] << 8) | f[17];
  latestData.protectionStatus = (f[18] << 8) | f[19];
  latestData.stateOfCharge = f[22];
  latestData.numCells = f[25];
  latestData.numNTCs = f[26];

  latestData.temperatures.clear();
  for (int i = 0; i < latestData.numNTCs; i++) {
    int base = 27 + 2 * i;
    if (base + 1 >= f.size()) break;
    uint16_t tRaw = (f[base] << 8) | f[base + 1];
    latestData.temperatures.push_back((tRaw - 2731) / 10.0f);
  }
}

bool BMSVibeBLE::isConnected() const { return connected; }
BMSData BMSVibeBLE::getData() const { return latestData; }
