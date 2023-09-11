#include <LEServer.h>

CharacteristicCallbacks characteristicCallbacks;
std::vector<CharacteristicCallbacks *> characteristicCallbacksVector;

ServerCallback serverCallback;

std::vector<BLECharacteristic *> pCharacteristics;
std::vector<BLEService *> pServices;

void LEServer::setDebug(bool debug)
{
  characteristicCallbacks._debug = debug;
  serverCallback._debug = debug;
  for (size_t i = 0; i < characteristicCallbacksVector.size(); i++)
  {
    characteristicCallbacksVector[i]->_debug = debug;
  }
  
}

void LEServer::setOnConnectCallback(void (*callback)(LEClient client))
{
  serverCallback.setOnConnectCallback(callback);
}

void LEServer::setOnDisconnectCallback(void (*callback)(LEClient client))
{
  serverCallback.setOnDisconnectCallback(callback);
}

void LEServer::setAllCharacteristicCallback(void (*callback)(LEResponse response))
{
  characteristicCallbacks.setCharacteristicCallback(callback);
}

void LEServer::setCharacteristicCallback(const char *characteristic_uuid, void (*callback)(LEResponse LEResponse))
{
   for (size_t i = 0; i < pServices.size(); i++)
  {
    BLECharacteristic *pCharacteristic = pServices[i]->getCharacteristic(characteristic_uuid);
    if (pCharacteristic != nullptr)
    {
      CharacteristicCallbacks* characteristicCallback;
      characteristicCallback = new CharacteristicCallbacks;
      characteristicCallback->setCharacteristicCallback(callback);
      characteristicCallbacksVector.push_back(characteristicCallback);

      pCharacteristic->setCallbacks(characteristicCallback);
      break;
    }
  }
}
void LEServer::createServer(const char *name)
{
  BLEDevice::init(name);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(&serverCallback);
}

void LEServer::addService(const char *uuid)
{
  BLEService *pService = pServer->createService(uuid);
  pServices.push_back(pService);
}

void LEServer::addCharacteristic(const char *service_uuid, const char *characteristic_uuid, uint32_t properties)
{
  BLEService *pService = pServer->getServiceByUUID(service_uuid);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(characteristic_uuid, properties);

  pCharacteristic->setCallbacks(&characteristicCallbacks);

  pCharacteristics.push_back(pCharacteristic);
}

void LEServer::addDescriptor(const char *characteristic_uuid, uint16_t dicreptor_uuid, const char *descriptor_value)
{
  for (size_t i = 0; i < pServices.size(); i++)
  {
    BLECharacteristic *pCharacteristic = pServices[i]->getCharacteristic(characteristic_uuid);
    if (pCharacteristic != nullptr)
    {
      BLEDescriptor *pDescriptor;
      pDescriptor = new BLEDescriptor(BLEUUID((uint16_t)dicreptor_uuid));

      if (descriptor_value != NULL)
      {
        pDescriptor->setValue(descriptor_value);
      }
      pCharacteristic->addDescriptor(pDescriptor);
      break;
    }
  }

  
}
void LEServer::addDescriptor(const char *characteristic_uuid, uint16_t dicreptor_uuid, uint8_t *data, size_t size)
{
   for (size_t i = 0; i < pServices.size(); i++)
  {
    BLECharacteristic *pCharacteristic = pServices[i]->getCharacteristic(characteristic_uuid);
    if (pCharacteristic != nullptr)
    {
      BLEDescriptor *pDescriptor;
      pDescriptor = new BLEDescriptor(BLEUUID((uint16_t)dicreptor_uuid));
      pDescriptor->setValue(data,size);
      
      pCharacteristic->addDescriptor(pDescriptor);
      break;
    }
  }
}
void LEServer::updateDescriptor(uint16_t dicreptor_uuid, const char *descriptor_value)
{
  for (size_t i = 0; i < pCharacteristics.size(); i++)
  {
    BLEDescriptor *descriptor = pCharacteristics[i]->getDescriptorByUUID(BLEUUID(dicreptor_uuid));
    if (descriptor != nullptr)
    {
      descriptor->setValue(descriptor_value);
      break;
    }
  }
}
void LEServer::updateDescriptor(uint16_t dicreptor_uuid, uint8_t *data, size_t size)
{
  for (size_t i = 0; i < pCharacteristics.size(); i++)
  {
    BLEDescriptor *descriptor = pCharacteristics[i]->getDescriptorByUUID(BLEUUID(dicreptor_uuid));
    if (descriptor != nullptr)
    {
      descriptor->setValue(data, size);
      break;
    }
  }
}
void LEServer::start()
{

  for (size_t i = 0; i < pServices.size(); i++)
  {
    pServices[i]->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pServices[i]->getUUID());
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
  }

  BLEDevice::startAdvertising();
}

void LEServer::notify(const char *characteristic_uuid, const char *data)
{

  for (size_t i = 0; i < pServices.size(); i++)
  {
    BLECharacteristic *pCharacteristic = pServices[i]->getCharacteristic(characteristic_uuid);
    if (pCharacteristic != nullptr)
    {
      pCharacteristic->setValue(data);
      pCharacteristic->notify();
      break;
    }
  }
}
void LEServer::notify(const char *characteristic_uuid, uint8_t *data, uint8_t size)
{
  for (size_t i = 0; i < pServices.size(); i++)
  {
    BLECharacteristic *pCharacteristic = pServices[i]->getCharacteristic(characteristic_uuid);
    if (pCharacteristic != nullptr)
    {
      pCharacteristic->setValue(data, size);
      pCharacteristic->notify();
      break;
    }
  }
}
BLEServer* LEServer::getServer()
{
    return pServer;
}
BLEService* LEServer::getService(const char *service_uuid)
{
    BLEService *pService = pServer->getServiceByUUID(service_uuid);
    if (pService != nullptr)
    {
      return pService;
    }
    return nullptr;
}
BLECharacteristic* LEServer::getCharacteristic(const char *characteristic_uuid)
{
  for (size_t i = 0; i < pServices.size(); i++)
  {
    BLECharacteristic *pCharacteristic = pServices[i]->getCharacteristic(characteristic_uuid);
    if (pCharacteristic != nullptr)
    {
      return pCharacteristic;
    }
  }
  return nullptr;
}