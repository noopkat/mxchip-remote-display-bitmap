// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"
#include "RGB_LED.h"
#include <ArduinoJson.h>

#define INTERVAL 2000
#define MESSAGE_MAX_LEN 256
#define DEVICE_ID "AZ3166"

const int LED_COLOR_CODE = 204;
static bool hasWifi = false;

RGB_LED rgbLed;

static void InitWifi()
{
  Screen.print(3, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    hasWifi = true;
    Screen.print(3, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(3, "No Wi-Fi\r\n ");
  }
}

static void MessageCallback(const char* payLoad, int size)
{
  Screen.print(0,"message arrived");
  char temp[100];
  sprintf(temp, "%i", payLoad[0]);
  Screen.print(1, temp, true);
  Serial.println(temp);
  if (payLoad[0] == LED_COLOR_CODE) {
    rgbLed.setColor(payLoad[1], payLoad[2], payLoad[3]);
  }
}


static int DeviceMethodCallback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  const char *responseMessage = "\"Successfully invoke device method\"";
  int result = 200;
  
  if (strcmp(methodName, "ledon") == 0)
  { 
    LogInfo("payload is %s", payload);
    StaticJsonBuffer<100> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(payload);

    if (root["data"].success()) {
      int red = root["data"]["red"];
      int green = root["data"]["green"];
      int blue = root["data"]["blue"];

      char screenMsg[20];
      sprintf(screenMsg, "R:%i G:%i B:%i", red, green, blue);

      Screen.print(1, "Method: ledOn");
      Screen.print(2, screenMsg);
      rgbLed.setColor(red, green, blue);
    }
  }
  if (strcmp(methodName, "showBitmap") == 0)
  { 
    LogInfo("payload is %s", payload);
    DynamicJsonBuffer dBuffer;
    JsonObject &frameBuffer = dBuffer.parseObject(payload);

    if (frameBuffer["data"].success()) {
      //Screen.print(1, "Method: showBitmap");
      unsigned char bitmap[1024];
      frameBuffer["data"].asArray().copyTo(bitmap);
      Screen.draw(0, 0, 128, 8, bitmap);
    }
  }
  else if (strcmp(methodName, "ledoff") == 0)
  {
    Screen.print(1, "Method: ledOff");
    rgbLed.turnOff();
  }
  else
  {
    Screen.print(1, "no method found");
    responseMessage = "\"No method found\"";
    result = 404;
  }

  *response_size = strlen(responseMessage);
  *response = (unsigned char *)malloc(*response_size);
  strncpy((char *)(*response), responseMessage, *response_size);

  return result;
}

void setup()
{
  Screen.init();
  Screen.print(0, "(/.__.)/ <3");

  InitWifi();
  if (!hasWifi)
  {
    return;
  }

  DevKitMQTTClient_Init(true);
  DevKitMQTTClient_SetMessageCallback(MessageCallback);
  DevKitMQTTClient_SetDeviceMethodCallback(DeviceMethodCallback);

}

void loop()
{
  DevKitMQTTClient_Check();
  delay(10);
}
