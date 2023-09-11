#include <LEClient.h>

volatile bool AutoReconnectFlag = false;
const char *pServer_name;

BLEAddress *pServerAddress;
BLEScan *pBLEScan;
BLEClient *pClient;

AdvertisedDeviceCallbacks advertisedDeviceCallbacks;
ClientCallbacks clientCallbacks;

void LEClient::discover()
{
    if (_debug)
        Serial.println("\nDiscover Services and Characteristics.\n");

    int serviceIndex = 0;
    int characteristicIndex = 0;
    int descriptorIndex = 0;

    for (const auto &serverEntry : *pClient->getServices())
    {
        BLERemoteService *pService = serverEntry.second;
        LEServicesVector.push_back(pService);
        if (_debug)
        {
            String characteristicSizeStr = String(pService->getCharacteristics()->size());

                if (characteristicSizeStr.length() < 2)
                    characteristicSizeStr = "0" + characteristicSizeStr;

                if (characteristicSizeStr == "00")
                    characteristicSizeStr = "No";

            Serial.printf("\nService        (Index: %02d), UUID : %s, Characteristics(%s)\n", serviceIndex, pService->getUUID().toString().c_str(),characteristicSizeStr);
        }
        for (const auto &characteristicEntry : *pService->getCharacteristics())
        {
            BLERemoteCharacteristic *characteristic = characteristicEntry.second;
            LECharacteristicsVector.push_back(characteristic);
            if (_debug)
            {
                String descriptorSizeStr = String(characteristic->getDescriptors()->size());

                if (descriptorSizeStr.length() < 2)
                    descriptorSizeStr = "0" + descriptorSizeStr;

                if (descriptorSizeStr == "00")
                    descriptorSizeStr = "No";

                Serial.printf("Characteristic (Index: %02d), UUID : %s, Descriptors(%s), Properties(", characteristicIndex,
                              characteristic->getUUID().toString().c_str(),
                              descriptorSizeStr);
                bool slashFlag = false;
                if (characteristic->canRead())
                {
                    if (slashFlag)
                        Serial.print("/");
                    Serial.print("R");
                    slashFlag = true;
                }
                if (characteristic->canWrite())
                {
                    if (slashFlag)
                        Serial.print("/");
                    Serial.print("W");
                    slashFlag = true;
                }
                if (characteristic->canNotify())
                {
                    if (slashFlag)
                        Serial.print("/");
                    Serial.print("N");
                    slashFlag = true;
                }
                if (characteristic->canIndicate())
                {
                    if (slashFlag)
                        Serial.print("/");
                    Serial.print("I");
                    slashFlag = true;
                }
                if (characteristic->canBroadcast())
                {
                    if (slashFlag)
                        Serial.print("/");
                    Serial.print("B");
                    slashFlag = true;
                }
                if (characteristic->canWriteNoResponse())
                {
                    if (slashFlag)
                        Serial.print("/");
                    Serial.print("WNR");
                    slashFlag = true;
                }
                Serial.print(")");
            }
            Serial.println();
            characteristicIndex++;

            for (const auto &descriptorEntry : *characteristic->getDescriptors())
            {
                BLERemoteDescriptor *descriptor = descriptorEntry.second;
                LEDescriptorVector.push_back(descriptor);
                if(_debug)
                {
                      Serial.printf("   Descriptors (Index: %02d), UUID : %s\n", descriptorIndex,descriptor->getUUID().toString().c_str());
                }
                descriptorIndex++;
            }
        }

        serviceIndex++;
    }
    if (_debug)
        Serial.printf("\nDiscovered (%02d) Services, (%02d) Characteristics, (%02d) Descriptors.\n\n", serviceIndex, characteristicIndex,descriptorIndex);
}

void LEClient::setDebug(bool debug)
{
    _debug = debug;
    advertisedDeviceCallbacks._debug = debug;
    clientCallbacks._debug = debug;

    Serial.setDebugOutput(false);
}

void LEClient::setOnConnectCallback(void (*callback)())
{
    clientCallbacks.setOnConnectCallback(callback);
}

void LEClient::setOnDisconnectCallback(void (*callback)())
{
    clientCallbacks.setOnDisconnectCallback(callback);
}

void LEClient::begin()
{
    BLEDevice::init("LEClient");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(&advertisedDeviceCallbacks);
    pBLEScan->setActiveScan(true);
    pClient = BLEDevice::createClient();

    pClient->setClientCallbacks(&clientCallbacks);
}

bool LEClient::connect(const char *server_name, const uint8_t scan_duration)
{
    pServer_name = server_name;

    if (_debug)
        Serial.println("\nScanning begins.");

    pBLEScan->start(scan_duration);

    if (_debug)
        Serial.println("Scanning ends.");

    pBLEScan->clearResults();

    if (pServerAddress == nullptr)
    {
        Serial.println("Device not found.");
        return false;
    }
    else
    {
        if (_debug)
        {
            Serial.println("\nServer found.");
            Serial.print("Server Name: ");
            Serial.println(pServer_name);
            Serial.print("Server Address: ");
            Serial.println(pServerAddress->toString().c_str());
            Serial.println("\nConnecting...");
        }

        pClient->connect(*pServerAddress);

        pServer_name = nullptr; // clear the name to allow scan to work.

        if (pClient->isConnected())
        {
            if (_debug)
            {
                Serial.println("Successfully Connected.");
                discover();
            }
            return true;
        }
        else
        {
            if (_debug)
            {
                Serial.println("Couldn't Connect.");
                Serial.println("Rebooting...");
            }
            esp_restart();          
            return false;
        }
    }
}

bool LEClient::connect(LEAddress server_address)
{
    pServerAddress = server_address.get();
    pClient->connect(*pServerAddress);

    if (pClient->isConnected())
    {
        if (_debug)
        {
            Serial.println("Successfully Connected.");
            discover();
        }
        return true;
    }
    else
    {
        if (_debug)
            Serial.println("Couldn't Connect.");

        return false;
    }
}
bool LEClient::reconnect()
{
    if (!pClient->isConnected())
    {
        pClient->connect(*pServerAddress);

        if (pClient->isConnected())
        {
            if (_debug)
            {
                Serial.println("Successfully Rconnected.");
                discover();
            }
            return true;
        }
        else
        {
            if (_debug)
                Serial.println("Couldn't Rconnect.");

            return false;
        }
    }
    return false;
}
bool LEClient::isConnected()
{
    if (pClient->isConnected())
        return true;
    else
        return false;
}
void LEClient::disconnect()
{
    pClient->disconnect();
}

LEServices LEClient::getServices()
{
    LEServices services;
    for (const auto &entry : *pClient->getServices())
    {
        BLERemoteService *service = entry.second;
        services.set(service);
    }
    return services;
}
LECharacteristics LEClient::getCharacteristics(const char *service_uuid)
{
    LECharacteristics characteristics;
    for (const auto &entry : *pClient->getService(service_uuid)->getCharacteristics())
    {
        BLERemoteCharacteristic *characteristic = entry.second;
        characteristics.set(characteristic);
    }
    return characteristics;
}

LECharacteristic LEClient::getCharacteristic(const char *service_uuid, const char *characteristic_uuid)
{
    LECharacteristic characteristic;
    BLERemoteCharacteristic *pCharacteristic = pClient->getService(service_uuid)->getCharacteristic(characteristic_uuid);
    characteristic.set(pCharacteristic);

    return characteristic;
}

LECharacteristic LEClient::getCharacteristicByIndex(uint32_t index)
{
    LECharacteristic characteristic;
    characteristic.set(LECharacteristicsVector[index]);
    return characteristic;
}
LEDescriptor LEClient::getDescriptorByIndex(uint32_t index)
{
    LEDescriptor descriptor;
    descriptor.set(LEDescriptorVector[index]);
    return descriptor;
}
LEDescriptor LEClient::getDescriptorIndex(const char *service_uuid, const char *characteristic_uuid,const char *descriptor_uuid)
{
   uint16_t Vlength = LEDescriptorVector.size();
   LEDescriptor descriptor;
   for (size_t i = 0; i < Vlength; i++)
   {
     if(LEDescriptorVector[i]->getUUID().equals(BLEUUID(descriptor_uuid))&&
        LEDescriptorVector[i]->getRemoteCharacteristic()->getUUID().equals(BLEUUID(characteristic_uuid))&&
        LEDescriptorVector[i]->getRemoteCharacteristic()->getRemoteService()->getUUID().equals(BLEUUID(service_uuid)))
     {
        descriptor.set(LEDescriptorVector[i]);
     }
   }
   return descriptor;
   
}
LEScanResults LEClient::scan(const uint8_t scan_duration)
{
    if (_debug)
        Serial.println("\nScanning begins.\n");

    BLEScanResults scanResult = pBLEScan->start(scan_duration);

    if (_debug)
        Serial.printf("\nScanning ends, %d devices found.\n", scanResult.getCount());

    uint32_t scanResultCount = scanResult.getCount();
    LEScanResults customResults;

    for (size_t i = 0; i < scanResultCount; i++)
    {

        std::string name = scanResult.getDevice(i).getName().c_str();
        if (name == "")
            name = "N/A";

        std::string address = scanResult.getDevice(i).getAddress().toString().c_str();
        int rssi = scanResult.getDevice(i).getRSSI();

        customResults.set(name, address, rssi, i);
    }

    return customResults;
}
const char *LEClient::getServerMacAdress()
{
    return pServerAddress->toString().c_str();
}
void AdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{
    if (pServer_name != nullptr)
    {
        if (advertisedDevice.getName() == pServer_name)
        {                                                                   // Check if the name of the advertiser matches
            advertisedDevice.getScan()->stop();                             // Scan can be stopped, we found what we are looking for
            pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
        }
    }
    else
    {
        if (_debug)
        {
            String name = advertisedDevice.getName().c_str();
            if (name == "")
                name = "N/A";

            Serial.printf("Name : %s , Adress : %s , Rssi : %d\n", name,
                          advertisedDevice.getAddress().toString().c_str(),
                          advertisedDevice.getRSSI());
        }
    }
}

void ClientCallbacks::onConnect(BLEClient *_pClient)
{

    if (onConnectCallback != nullptr)
    {
        onConnectCallback();
    }
}

void ClientCallbacks::onDisconnect(BLEClient *_pClient)
{
    if (_debug)
        Serial.println("Disconnected.");

    if (onDisconnectCallback != nullptr)
    {
        onDisconnectCallback();
    }
}

LECharacteristics LEServices::getCharacteristics(const char *service_uuid)
{
    LECharacteristics characteristics;
    for (const auto &entry : *pClient->getService(service_uuid)->getCharacteristics())
    {
        BLERemoteCharacteristic *characteristic = entry.second;
        characteristics.set(characteristic);
    }
    return characteristics;
}
