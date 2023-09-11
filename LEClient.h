#include <Arduino.h>
#include <BLEDevice.h>
#include <vector>


class LEAddress
{
private:
  BLEAddress *_address;

public:
  LEAddress(const char *address) { _address = new BLEAddress(address); }
  BLEAddress *get() { return _address; }
  bool equals(const char *address) { return (*_address).equals(BLEAddress(address)); }
};

class LEScanResults
{
private:
  struct LEScanResult
  {
    const char *name;
    const char *address;
    int rssi;
    uint32_t id;
  };

  std::vector<LEScanResult> LEScanResultsVector;

public:
  void set(std::string name, std::string address, int rssi, uint32_t id)
  {
    LEScanResult result;

    result.name = strdup(name.c_str());       // duplicate for new memory location
    result.address = strdup(address.c_str()); // duplicate for new memory location
    result.rssi = rssi;
    result.id = id;

    LEScanResultsVector.push_back(result);
  }
  LEScanResult get(uint32_t index)
  {
    return LEScanResultsVector[index];
  }
  void clear()
  {
    LEScanResultsVector.clear();
  }
  uint32_t count()
  {
    return LEScanResultsVector.size();
  }
};

class LECharacteristic
{
private:
  BLERemoteCharacteristic *_pCharacteristic;

public:
  void set(BLERemoteCharacteristic *characteristic) { _pCharacteristic = characteristic; }
  BLERemoteCharacteristic *get() { return _pCharacteristic; }
  const char *read() { return  strdup(_pCharacteristic->readValue().c_str()); }
  void write(const char *data) { _pCharacteristic->writeValue(data); }
  void write(uint8_t *pData, size_t length){_pCharacteristic->writeValue(pData,length);}
  const char *getUUID(){return _pCharacteristic->getUUID().toString().c_str();}
  void setNotifyCallback(std::function<void(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)> notifyCallback)
  {
    _pCharacteristic->registerForNotify(notifyCallback);
  };
  bool canRead() { return _pCharacteristic->canRead(); }
  bool canWrite() { return _pCharacteristic->canWrite(); }
  bool canNotify() { return _pCharacteristic->canNotify(); }
};

class LECharacteristics
{
private:
  std::vector<BLERemoteCharacteristic *> LECharacteristicsVector;
public:
  void set(BLERemoteCharacteristic * service)
  {
    LECharacteristicsVector.push_back(service);
  }
  LECharacteristic get(uint32_t index)
  {
    LECharacteristic characteristic;
    characteristic.set(LECharacteristicsVector[index]);
    return characteristic;
  }
  const char* getUUID(uint32_t index)
  {
    if(index < LECharacteristicsVector.size())
        return strdup(LECharacteristicsVector[index]->getUUID().toString().c_str());
    else
    {  
        Serial.println("Characteristics index out of range.");
        return "";
    }
  }
  void clear()
  {
    LECharacteristicsVector.clear();
  }
  uint32_t count()
  {
    return LECharacteristicsVector.size();
  }
};

class LEServices
{
private:
  std::vector<BLERemoteService *> LEServicesVector;

public:
  void set(BLERemoteService * service)
  {
    LEServicesVector.push_back(service);
  }
  BLERemoteService * get(uint32_t index){return LEServicesVector[index];}
  LECharacteristics getCharacteristics(const char* service_uuid);
  const char* getUUID(uint32_t index)
  {
    if(index < LEServicesVector.size())
      return strdup(LEServicesVector[index]->getUUID().toString().c_str());
    else
    {
        Serial.println("Services index out of range.");
        return "";
    }
  }
  void clear()
  {
    LEServicesVector.clear();
  }
  uint32_t count()
  {
    return LEServicesVector.size();
  }
};

class LEDescriptor
{
private:
  BLERemoteDescriptor *_pDescriptor;

public:
  void set(BLERemoteDescriptor *descriptor) { _pDescriptor = descriptor; }
  BLERemoteDescriptor *get() { return _pDescriptor; }
  const char *read() { return _pDescriptor->readValue().c_str(); }
  const char *getUUID(){return _pDescriptor->getUUID().toString().c_str();}

};

class LEClient
{
private:
  std::vector<BLERemoteService *> LEServicesVector;
  std::vector<BLERemoteCharacteristic *> LECharacteristicsVector;
  std::vector<BLERemoteDescriptor *> LEDescriptorVector;
  BLEScan *pBLEScan;
  bool _debug = false;
  void discover();

public:
  void begin();
  bool connect(const char *server_name, const uint8_t scan_duration = 5);
  bool connect(LEAddress server_address);

  bool isConnected();
  void disconnect();
  bool reconnect();
   
  
  LEServices getServices();
  LECharacteristics getCharacteristics(const char *service_uuid);

  LECharacteristic getCharacteristic(const char *service_uuid, const char *characteristic_uuid);
  LECharacteristic getCharacteristicByIndex(uint32_t index);
  
  LEDescriptor getDescriptorIndex(const char *service_uuid, const char *characteristic_uuid,const char *descriptor_uuid);
  LEDescriptor getDescriptorByIndex(uint32_t index);
  
  void setOnDisconnectCallback(void (*callback)());
  void setOnConnectCallback(void (*callback)());

  const char *getServerMacAdress();

  LEScanResults scan(const uint8_t scan_duration);
  void setDebug(bool debug);
};

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
public:
  bool _debug = false;
  void onResult(BLEAdvertisedDevice advertisedDevice);
};

class ClientCallbacks : public BLEClientCallbacks
{
public:
  bool _debug = false;
  void setOnDisconnectCallback(void (*callback)())
  {
    onDisconnectCallback = callback;
  }
  void setOnConnectCallback(void (*callback)())
  {
    onConnectCallback = callback;
  }

private:
  void (*onDisconnectCallback)();
  void (*onConnectCallback)();
  void onConnect(BLEClient *_pClient);
  void onDisconnect(BLEClient *_pClient);
};
