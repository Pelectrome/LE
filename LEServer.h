#ifndef LEServer_H
#define LEServer_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <vector>

typedef enum
{
  onRead,
  onWrite,
  onNotify,
  onStatus
} LEState;

class LEUUID
{
private:
  BLEUUID LEuuid;

public:
  void set(BLEUUID uuid) { LEuuid = uuid; }
  bool equals(const char *uuid)
  {
    if (LEuuid.equals(BLEUUID(uuid)))
      return true;
    else
      return false;
  }
};
struct LEResponse
{
  LEState state;
  LEUUID uuid;
  String uuidStr;
  String clientAddress;
  String data;
  uint8_t *dataPtr;
  uint8_t size;
};

struct LEClient
{
  String address;
  uint16_t id;
  uint16_t count;
};

enum LEPropertie
{
  Read = 1 << 0,
  Write = 1 << 1,
  Notify = 1 << 2,
  Broadcast = 1 << 3,
  Indicate = 1 << 4,
  Write_NR = 1 << 5,
};

/**
 * @brief UserDescription mast enter in hex string.
 */
enum LEDescriptor
{
  ExtendedProperties = 0x2900,
  UserDescription = 0x2901,
  Configuration = 0x2902,
};

class LEServer
{
private:
  BLEServer *pServer = NULL;
  String _deviceName;
  bool _debug = false;
  
public:
  void createServer(const char *name);

  void addService(const char *uuid);

  void addCharacteristic(const char *service_uuid, const char *characteristic_uuid, uint32_t properties);

  void addDescriptor(const char *characteristic_uuid, uint16_t dicreptor_uuid, const char *descriptor_value = NULL);
  void addDescriptor(const char *characteristic_uuid, uint16_t dicreptor_uuid, uint8_t *data, size_t size);

  void updateDescriptor(uint16_t dicreptor_uuid, const char *descriptor_value);
  void updateDescriptor(uint16_t dicreptor_uuid, uint8_t *data, size_t size);

  void setOnConnectCallback(void (*callback)(LEClient LEClient));
  void setOnDisconnectCallback(void (*callback)(LEClient LEClient));

  void setAllCharacteristicCallback(void (*callback)(LEResponse LEResponse));
  void setCharacteristicCallback(const char *characteristic_uuid, void (*callback)(LEResponse LEResponse));

  void start();

  void notify(const char *characteristic_uuid, const char *data);
  void notify(const char *characteristic_uuid, uint8_t *data, uint8_t size);

  void setDebug(bool debug);

  BLEServer *getServer();
  BLEService *getService(const char *service_uuid);
  BLECharacteristic *getCharacteristic(const char *characteristic_uuid);
};

class ServerCallback : public BLEServerCallbacks
{
public:
  void setOnConnectCallback(void (*callback)(LEClient LEClient))
  {
    onConnectCallback = callback;
  };
  void setOnDisconnectCallback(void (*callback)(LEClient LEClient))
  {
    onDisconnectCallback = callback;
  };

  bool _debug = false;

private:
  uint16_t clientCount = 0;

  void (*onConnectCallback)(LEClient LEClient);
  void (*onDisconnectCallback)(LEClient LEClient);

  void onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param)
  {
    /* Get the MAC address of the connected LEClient */
    uint16_t ClientID = param->connect.conn_id;
    BLEAddress ClientAddress = param->connect.remote_bda;

    BLEDevice::startAdvertising();

    clientCount++;

    LEClient LEClient;
    LEClient.address = ClientAddress.toString().c_str();
    LEClient.id = ClientID;
    LEClient.count = clientCount;

    if (_debug)
    {
      Serial.print("Client Connected    , Id : ");
      Serial.print(LEClient.id);
      Serial.print(" , Adress : ");
      Serial.print(LEClient.address);
      Serial.print(" , Connected Devices : ");
      Serial.println(LEClient.count);
    }

    if (onConnectCallback != nullptr)
    {
      onConnectCallback(LEClient);
    }
  };

  void onDisconnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param)
  {
    uint16_t ClientID = param->connect.conn_id;
    BLEAddress ClientAddress = param->connect.remote_bda;

    clientCount--;

    LEClient LEClient;
    LEClient.address = ClientAddress.toString().c_str();
    LEClient.id = ClientID;
    LEClient.count = clientCount;

    if (_debug)
    {
      Serial.print("Client Disconnected , Id : ");
      Serial.print(LEClient.id);
      Serial.print(" , Adress : ");
      Serial.print(LEClient.address);
      Serial.print(" , Connected Devices : ");
      Serial.println(LEClient.count);
    }

    if (onDisconnectCallback != nullptr)
    {
      onDisconnectCallback(LEClient);
    }
  };
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
public:
  void setCharacteristicCallback(void (*callback)(LEResponse LEResponse))
  {
    characteristicCallback = callback;
  }

  bool _debug = false;

private:
  void (*characteristicCallback)(LEResponse LEResponse);
  int i=0;
  void onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param)
  {

    BLEAddress clientAddress = param->write.bda;

    LEResponse response;

    response.state = LEState::onWrite;
    response.clientAddress = clientAddress.toString().c_str();
    response.uuid.set(pCharacteristic->getUUID());
    response.uuidStr = pCharacteristic->getUUID().toString().c_str();
    response.data = pCharacteristic->getValue().c_str();
    response.dataPtr = pCharacteristic->getData();
    response.size = pCharacteristic->getLength();

    if (_debug)
    {
      //Serial.println();
      // Serial.println("Receiving New Data.");
      // Serial.print("Client Address: ");
      // Serial.println(response.clientAddress);
      // Serial.print("Characteristic uuid: ");
      // Serial.println(response.uuidStr);
      // Serial.print("Data Length: ");
      // Serial.println(response.size);
      // Serial.print("Data String: ");
      // Serial.println(response.data);

      // Serial.print("Data DEC: ");
      // for (size_t i = 0; i < response.size; i++)
      // {
      //   Serial.printf("%02d", *(response.dataPtr + i));
      //   if (i < response.size - 1)
      //     Serial.print("-");
      // }
      // Serial.println();
      
      //Serial.print("Data HEX: ");
      Serial.printf("uint8_t data%d[] = {",i);
      for (size_t i = 0; i < response.size; i++)
      {
        Serial.printf("0x%02X", *(response.dataPtr + i));
        if (i < response.size - 1)
          Serial.print(",");
      }
      Serial.print("};");
      Serial.println();
      i++;
    }

    if (characteristicCallback != nullptr)
    {
      characteristicCallback(response);
    }
  }

  void onRead(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param)
  {
    BLEAddress clientAddress = param->write.bda;
    LEResponse response;

    response.state = LEState::onRead;
    response.clientAddress = clientAddress.toString().c_str();
    response.uuid.set(pCharacteristic->getUUID());
    response.uuidStr = pCharacteristic->getUUID().toString().c_str();

    if (_debug)
    {
      Serial.println();
      Serial.println("Read Detected.");
      Serial.print("Client Address: ");
      Serial.println(response.clientAddress);
      Serial.print("Characteristic uuid: ");
      Serial.println(response.uuidStr);
    }

    if (characteristicCallback != nullptr)
    {
      characteristicCallback(response);
    }
  }

  // void onNotify(BLECharacteristic *pCharacteristic)
  // {
  //   LEResponse LEResponse;

  //   LEResponse.state = LEState::onNotify;
  //   LEResponse.uuid = pCharacteristic->getUUID();
  //   LEResponse.uuidStr = pCharacteristic->getUUID().toString().c_str();

  //   if (_debug)
  //   {
  //     Serial.println();
  //     Serial.println("Notify Detected.");
  //     Serial.print("Characteristic uuid: ");
  //     Serial.println(LEResponse.uuidStr);
  //   }

  //   if (characteristicCallback != nullptr)
  //   {
  //     characteristicCallback(LEResponse);
  //   }
  // }

  // void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code)
  // {
  //   LEResponse LEResponse;

  //   LEResponse.state = LEState::onStatus;
  //   LEResponse.uuid = pCharacteristic->getUUID();
  //   LEResponse.uuidStr = pCharacteristic->getUUID().toString().c_str();

  //   if (_debug)
  //   {
  //     Serial.println();
  //     Serial.println("Status Detected.");
  //     Serial.print("Characteristic uuid: ");
  //     Serial.println(LEResponse.uuidStr);
  //     Serial.print("Status: ");
  //     Serial.println(s);
  //     Serial.print("Code: ");
  //     Serial.println(code,HEX);
  //   }

  //   if (characteristicCallback != nullptr)
  //   {
  //     characteristicCallback(LEResponse);
  //   }
  // }
};

#endif // LEServer_H