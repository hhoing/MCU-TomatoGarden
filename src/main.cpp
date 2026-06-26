#include <Arduino.h>
#include "Config.h"
#include "Storage.h"
#include "Signature.h"
#include "AzureDpsClient.h"
#include "CliMode.h"
#include "Bitmap.h"

// #include <LIS3DHTR.h>
#include "Seeed_BME280.h"
#include <Wire.h>
#include "TFT_eSPI.h" 
#include "imgArray.h"



#include <rpcWiFiClientSecure.h>
#include <PubSubClient.h>

#include <WiFiUdp.h>
#include <NTP.h>

#include <az_json.h>
#include <az_result.h>
#include <az_span.h>
#include <az_iot_hub_client.h>

#define MQTT_PACKET_SIZE 1024

// LIS3DHTR<TwoWire> AccelSensor;
//VU
BME280 bme280;

int temp;
int humid;
int press;

int soil = 0;
unsigned long wetDuration = 0;
unsigned long lastCheckTime = 0;
const unsigned long CHECK_INTERVAL = 60000; // 1 minute
const unsigned long DANGEROUS_WET_TIME = 6UL * 60 * 60 * 1000; // 6 hours


const char CA_CERTS[] =
  //////////////////////////////////////
  // Common Name: Baltimore CyberTrust Root
  // Organizational Name: Baltimore
  // From Date: 2000-05-13 03:46:00
  // To Date: 2025-05-13 08:59:00
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n"
  "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n"
  "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n"
  "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n"
  "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n"
  "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n"
  "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n"
  "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n"
  "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n"
  "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n"
  "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n"
  "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n"
  "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n"
  "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n"
  "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n"
  "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n"
  "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n"
  "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n"
  "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n"
  "-----END CERTIFICATE-----\n"
  //////////////////////////////////////
  // Common Name: DigiCert Global Root G2
  // Organizational Name: DigiCert Inc
  // From Date: 2013-8-1 12:00:00
  // To Date: 2038-1-15 12:00:00
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
  "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
  "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
  "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
  "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
  "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
  "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
  "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
  "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
  "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
  "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
  "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
  "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
  "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
  "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
  "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
  "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
  "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
  "MrY=\n"
  "-----END CERTIFICATE-----\n"
  ;

WiFiClientSecure wifi_client;
PubSubClient mqtt_client(wifi_client);
WiFiUDP wifi_udp;
NTP ntp(wifi_udp);

std::string HubHost;
std::string DeviceId;

#define AZ_RETURN_IF_FAILED(exp) \
  do \
  { \
    az_result const _result = (exp); \
    if (az_result_failed(_result)) \
    { \
      return _result; \
    } \
  } while (0)

////////////////////////////////////////////////////////////////////////////////
// 

#define DLM "\r\n"

static String StringVFormat(const char* format, va_list arg)
{
    const int len = vsnprintf(nullptr, 0, format, arg);
    char str[len + 1];
    vsnprintf(str, sizeof(str), format, arg);

    return String{ str };
}

static void Abort(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    String str{ StringVFormat(format, arg) };
    va_end(arg);

    Serial.print(String::format("ABORT: %s" DLM, str.c_str()));

    while (true) {}
}

static void Log(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    String str{ StringVFormat(format, arg) };
    va_end(arg);

    Serial.print(str);
}

////////////////////////////////////////////////////////////////////////////////
// Display

// #include <LovyanGFX.hpp>

// static LGFX tft;
TFT_eSPI tft; //initialize TFT LCD 

static void DisplayPrintf(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    String str{ StringVFormat(format, arg) };
    va_end(arg);

    Log("%s\n", str.c_str());
    // tft.printf("%s\n", str.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// Button

#include <AceButton.h>
using namespace ace_button;

enum class ButtonId
{
    RIGHT = 0,
    CENTER,
    LEFT,
};
static const int ButtonNumber = 3;
static AceButton Buttons[ButtonNumber];
static bool ButtonsClicked[ButtonNumber];

static void ButtonEventHandler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
    const uint8_t id = button->getId();
    if (ButtonNumber <= id) return;

    switch (eventType)
    {
    case AceButton::kEventClicked:
        switch (static_cast<ButtonId>(id))
        {
        case ButtonId::RIGHT:
            DisplayPrintf("Right button was clicked");
            break;
        case ButtonId::CENTER:
            DisplayPrintf("Center button was clicked");
            break;
        case ButtonId::LEFT:
            DisplayPrintf("Left button was clicked");
            break;
        }
        ButtonsClicked[id] = true;
        break;
    }
}

static void ButtonInit()
{
    Buttons[static_cast<int>(ButtonId::RIGHT)].init(WIO_KEY_A, HIGH, static_cast<uint8_t>(ButtonId::RIGHT));
    Buttons[static_cast<int>(ButtonId::CENTER)].init(WIO_KEY_B, HIGH, static_cast<uint8_t>(ButtonId::CENTER));
    Buttons[static_cast<int>(ButtonId::LEFT)].init(WIO_KEY_C, HIGH, static_cast<uint8_t>(ButtonId::LEFT));

    ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(ButtonEventHandler);
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);

    for (int i = 0; i < ButtonNumber; ++i) ButtonsClicked[i] = false;
}

static void ButtonDoWork()
{
    for (int i = 0; static_cast<size_t>(i) < std::extent<decltype(Buttons)>::value; ++i)
    {
        Buttons[i].check();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Azure IoT DPS

static AzureDpsClient DpsClient;
static unsigned long DpsPublishTimeOfQueryStatus = 0;

static void MqttSubscribeCallbackDPS(char* topic, byte* payload, unsigned int length);

static int RegisterDeviceToDPS(const std::string& endpoint, const std::string& idScope, const std::string& registrationId, const std::string& symmetricKey, const uint64_t& expirationEpochTime, std::string* hubHost, std::string* deviceId)
{
    std::string endpointAndPort{ endpoint };
    endpointAndPort += ":";
    endpointAndPort += std::to_string(8883);

    if (DpsClient.Init(endpointAndPort, idScope, registrationId) != 0) return -1;

    const std::string mqttClientId = DpsClient.GetMqttClientId();
    const std::string mqttUsername = DpsClient.GetMqttUsername();

    const std::vector<uint8_t> signature = DpsClient.GetSignature(expirationEpochTime);
    const std::string encryptedSignature = GenerateEncryptedSignature(symmetricKey, signature);
    const std::string mqttPassword = DpsClient.GetMqttPassword(encryptedSignature, expirationEpochTime);

    const std::string registerPublishTopic = DpsClient.GetRegisterPublishTopic();
    const std::string registerSubscribeTopic = DpsClient.GetRegisterSubscribeTopic();

    Log("DPS:" DLM);
    Log(" Endpoint = %s" DLM, endpoint.c_str());
    Log(" Id scope = %s" DLM, idScope.c_str());
    Log(" Registration id = %s" DLM, registrationId.c_str());
    Log(" MQTT client id = %s" DLM, mqttClientId.c_str());
    Log(" MQTT username = %s" DLM, mqttUsername.c_str());
    //Log(" MQTT password = %s" DLM, mqttPassword.c_str());

    wifi_client.setCACert(CA_CERTS);
    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
    mqtt_client.setServer(endpoint.c_str(), 8883);
    mqtt_client.setCallback(MqttSubscribeCallbackDPS);
    DisplayPrintf("Connecting to Azure IoT Hub DPS...");
    if (!mqtt_client.connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) return -2;

    mqtt_client.subscribe(registerSubscribeTopic.c_str());
    mqtt_client.publish(registerPublishTopic.c_str(), "{payload:{\"modelId\":\"" IOT_CONFIG_MODEL_ID "\"}}");

    while (!DpsClient.IsRegisterOperationCompleted())
    {
        mqtt_client.loop();
        if (DpsPublishTimeOfQueryStatus > 0 && millis() >= DpsPublishTimeOfQueryStatus)
        {
            mqtt_client.publish(DpsClient.GetQueryStatusPublishTopic().c_str(), "");
            Log("Client sent operation query message" DLM);
            DpsPublishTimeOfQueryStatus = 0;
        }
    }

    if (!DpsClient.IsAssigned()) return -3;

    mqtt_client.disconnect();

    *hubHost = DpsClient.GetHubHost();
    *deviceId = DpsClient.GetDeviceId();

    Log("Device provisioned:" DLM);
    Log(" Hub host = %s" DLM, hubHost->c_str());
    Log(" Device id = %s" DLM, deviceId->c_str());

    return 0;
}

static void MqttSubscribeCallbackDPS(char* topic, byte* payload, unsigned int length)
{
    Log("Subscribe:" DLM " %s" DLM " %.*s" DLM, topic, length, (const char*)payload);

    if (DpsClient.RegisterSubscribeWork(topic, std::vector<uint8_t>(payload, payload + length)) != 0)
    {
        Log("Failed to parse topic and/or payload" DLM);
        return;
    }

    if (!DpsClient.IsRegisterOperationCompleted())
    {
        const int waitSeconds = DpsClient.GetWaitBeforeQueryStatusSeconds();
        Log("Querying after %u  seconds..." DLM, waitSeconds);

        DpsPublishTimeOfQueryStatus = millis() + waitSeconds * 1000;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Azure IoT Hub

static az_iot_hub_client HubClient;

static int SendCommandResponse(az_iot_hub_client_method_request* request, uint16_t status, az_span response);
static void MqttSubscribeCallbackHub(char* topic, byte* payload, unsigned int length);

static int ConnectToHub(az_iot_hub_client* iot_hub_client, const std::string& host, const std::string& deviceId, const std::string& symmetricKey, const uint64_t& expirationEpochTime)
{
    static std::string deviceIdCache;
    deviceIdCache = deviceId;

    const az_span hostSpan{ az_span_create((uint8_t*)&host[0], host.size()) };
    const az_span deviceIdSpan{ az_span_create((uint8_t*)&deviceIdCache[0], deviceIdCache.size()) };
    az_iot_hub_client_options options = az_iot_hub_client_options_default();
    options.model_id = AZ_SPAN_LITERAL_FROM_STR(IOT_CONFIG_MODEL_ID);
    if (az_result_failed(az_iot_hub_client_init(iot_hub_client, hostSpan, deviceIdSpan, &options))) return -1;

    char mqttClientId[128];
    size_t client_id_length;
    if (az_result_failed(az_iot_hub_client_get_client_id(iot_hub_client, mqttClientId, sizeof(mqttClientId), &client_id_length))) return -4;

    char mqttUsername[256];
    if (az_result_failed(az_iot_hub_client_get_user_name(iot_hub_client, mqttUsername, sizeof(mqttUsername), NULL))) return -5;

    char mqttPassword[300];
    uint8_t signatureBuf[256];
    az_span signatureSpan = az_span_create(signatureBuf, sizeof(signatureBuf));
    az_span signatureValidSpan;
    if (az_result_failed(az_iot_hub_client_sas_get_signature(iot_hub_client, expirationEpochTime, signatureSpan, &signatureValidSpan))) return -2;
    const std::vector<uint8_t> signature(az_span_ptr(signatureValidSpan), az_span_ptr(signatureValidSpan) + az_span_size(signatureValidSpan));
    const std::string encryptedSignature = GenerateEncryptedSignature(symmetricKey, signature);
    az_span encryptedSignatureSpan = az_span_create((uint8_t*)&encryptedSignature[0], encryptedSignature.size());
    if (az_result_failed(az_iot_hub_client_sas_get_password(iot_hub_client, expirationEpochTime, encryptedSignatureSpan, AZ_SPAN_EMPTY, mqttPassword, sizeof(mqttPassword), NULL))) return -3;

    Log("Hub:" DLM);
    Log(" Host = %s" DLM, host.c_str());
    Log(" Device id = %s" DLM, deviceIdCache.c_str());
    Log(" MQTT client id = %s" DLM, mqttClientId);
    Log(" MQTT username = %s" DLM, mqttUsername);
    //Log(" MQTT password = %s" DLM, mqttPassword);

    wifi_client.setCACert(CA_CERTS);
    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
    mqtt_client.setServer(host.c_str(), 8883);
    mqtt_client.setCallback(MqttSubscribeCallbackHub);

    if (!mqtt_client.connect(mqttClientId, mqttUsername, mqttPassword)) return -6;

    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC);
    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

    return 0;
}

static az_result SendTelemetry()
{
    // float accelX;
    // float accelY;
    // float accelZ;
    // AccelSensor.getAcceleration(&accelX, &accelY, &accelZ);

    // int light;
    // light = analogRead(WIO_LIGHT) * 100 / 1023;

    char creationTime[20 + 1];  // yyyy-mm-ddThh:mm:ss.sssZ
    {
        const time_t t = ntp.epoch();
        struct tm tm;
        gmtime_r(&t, &tm);
        const int len = snprintf(creationTime, sizeof(creationTime), "%d-%02d-%02dT%02d:%02d:%02dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        if (len != sizeof(creationTime) - 1)
        {
            Log("Failed snprintf" DLM);
            return AZ_ERROR_NOT_SUPPORTED;
        }
    }

    az_iot_message_properties props;
    uint8_t propsBuffer[128];
    if (az_result_failed(az_iot_message_properties_init(&props, az_span_create(propsBuffer, sizeof(propsBuffer)), 0)))
    {
        Log("Failed az_iot_message_properties_init" DLM);
        return AZ_ERROR_NOT_SUPPORTED;
    }
    if (az_result_failed(az_iot_message_properties_append(&props, AZ_SPAN_FROM_STR("iothub-creation-time-utc"), az_span_create(reinterpret_cast<uint8_t*>(creationTime), strlen(creationTime)))))
    {
        Log("Failed az_iot_message_properties_append" DLM);
        return AZ_ERROR_NOT_SUPPORTED;
    }

    char telemetry_topic[128];
    if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(&HubClient, &props, telemetry_topic, sizeof(telemetry_topic), NULL)))
    {
        Log("Failed az_iot_hub_client_telemetry_get_publish_topic" DLM);
        return AZ_ERROR_NOT_SUPPORTED;
    }

    az_json_writer json_builder;
    char telemetry_payload[200];
    AZ_RETURN_IF_FAILED(az_json_writer_init(&json_builder, AZ_SPAN_FROM_BUFFER(telemetry_payload), NULL));
    AZ_RETURN_IF_FAILED(az_json_writer_append_begin_object(&json_builder));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR("temp")));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, temp));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR("humid")));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, humid));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR("press")));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, press));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR("soil")));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, soil));
    // AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_LIGHT)));
    // AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, light));
    AZ_RETURN_IF_FAILED(az_json_writer_append_end_object(&json_builder));
    const az_span out_payload{ az_json_writer_get_bytes_used_in_destination(&json_builder) };

    static int sendCount = 0;
    if (!mqtt_client.publish(telemetry_topic, az_span_ptr(out_payload), az_span_size(out_payload), false))
    {
        // DisplayPrintf("ERROR: Send telemetry %d", sendCount);
        tft.setFreeFont(&FreeSans12pt7b);
        tft.drawString("Status", 200, 60);
        tft.setFreeFont(&FreeSans9pt7b);
        tft.fillRect(200,90, 120, 20, TFT_WHITE);
        tft.drawString("Er "+(String)(sendCount), 200, 90);
    }
    else
    {
        ++sendCount;
        // DisplayPrintf("Sent telemetry %d", sendCount);
        // tft.fillRect(200,60, 50, 60, TFT_BLUE);
        // tft.setFreeFont(&FreeSans12pt7b);
        // tft.drawString("Status", 200, 60);
        // tft.setFreeFont(&FreeSans9pt7b);
        // tft.fillRect(200,90, 120, 20, TFT_WHITE);
        // tft.drawString("Sent "+(String)(sendCount), 200, 90);

    }

    return AZ_OK;
}

static az_result SendButtonTelemetry(ButtonId id)
{
    char creationTime[20 + 1];  // yyyy-mm-ddThh:mm:ss.sssZ
    {
        const time_t t = ntp.epoch();
        struct tm tm;
        gmtime_r(&t, &tm);
        const int len = snprintf(creationTime, sizeof(creationTime), "%d-%02d-%02dT%02d:%02d:%02dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        if (len != sizeof(creationTime) - 1)
        {
            Log("Failed snprintf" DLM);
            return AZ_ERROR_NOT_SUPPORTED;
        }
    }

    az_iot_message_properties props;
    uint8_t propsBuffer[128];
    if (az_result_failed(az_iot_message_properties_init(&props, az_span_create(propsBuffer, sizeof(propsBuffer)), 0)))
    {
        Log("Failed az_iot_message_properties_init" DLM);
        return AZ_ERROR_NOT_SUPPORTED;
    }
    if (az_result_failed(az_iot_message_properties_append(&props, AZ_SPAN_FROM_STR("iothub-creation-time-utc"), az_span_create(reinterpret_cast<uint8_t*>(creationTime), strlen(creationTime)))))
    {
        Log("Failed az_iot_message_properties_append" DLM);
        return AZ_ERROR_NOT_SUPPORTED;
    }

    char telemetry_topic[128];
    if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(&HubClient, &props, telemetry_topic, sizeof(telemetry_topic), NULL)))
    {
        Log("Failed az_iot_hub_client_telemetry_get_publish_topic" DLM);
        return AZ_ERROR_NOT_SUPPORTED;
    }

    az_json_writer json_builder;
    char telemetry_payload[200];
    AZ_RETURN_IF_FAILED(az_json_writer_init(&json_builder, AZ_SPAN_FROM_BUFFER(telemetry_payload), NULL));
    AZ_RETURN_IF_FAILED(az_json_writer_append_begin_object(&json_builder));
    switch (id)
    {
    case ButtonId::RIGHT:
        AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_RIGHT_BUTTON)));
        AZ_RETURN_IF_FAILED(az_json_writer_append_string(&json_builder, AZ_SPAN_LITERAL_FROM_STR("click")));
        break;
    case ButtonId::CENTER:
        AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_CENTER_BUTTON)));
        AZ_RETURN_IF_FAILED(az_json_writer_append_string(&json_builder, AZ_SPAN_LITERAL_FROM_STR("click")));
        break;
    case ButtonId::LEFT:
        AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_LEFT_BUTTON)));
        AZ_RETURN_IF_FAILED(az_json_writer_append_string(&json_builder, AZ_SPAN_LITERAL_FROM_STR("click")));
        break;
    default:
        return AZ_ERROR_ARG;
    }
    AZ_RETURN_IF_FAILED(az_json_writer_append_end_object(&json_builder));
    const az_span out_payload{ az_json_writer_get_bytes_used_in_destination(&json_builder) };

    if (!mqtt_client.publish(telemetry_topic, az_span_ptr(out_payload), az_span_size(out_payload), false))
    {
        DisplayPrintf("ERROR: Send button telemetry");
    }
    else
    {
        DisplayPrintf("Sent button telemetry");
    }

    return AZ_OK;
}

static void HandleCommandMessage(az_span payload, az_iot_hub_client_method_request* command_request)
{
    int command_res_code = 200;
    az_result rc = AZ_OK;

    if (az_span_is_content_equal(AZ_SPAN_LITERAL_FROM_STR(COMMAND_RING_BUZZER), command_request->name))
    {
        // Parse the command payload (it contains a 'duration' field)
        Log("Processing command 'ringBuzzer'" DLM);
        char buffer[32];
        az_span_to_str(buffer, 32, payload);
        Log("Raw command payload: %s" DLM, buffer);

        az_json_reader json_reader;
        uint32_t duration;
        if (az_json_reader_init(&json_reader, payload, NULL) == AZ_OK)
        {
            if (az_json_reader_next_token(&json_reader) == AZ_OK)
            {
                if (az_result_failed(rc = az_json_token_get_uint32(&json_reader.token, &duration)))
                {
                    Log("Couldn't parse JSON token res=%d" DLM, rc);
                }
                else
                {
                    Log("Duration: %dms" DLM, duration);
                }
            }

            // Invoke command
            analogWrite(WIO_BUZZER, 128);
            delay(duration);
            analogWrite(WIO_BUZZER, 0);

            int rc;
            if (az_result_failed(rc = SendCommandResponse(command_request, command_res_code, AZ_SPAN_LITERAL_FROM_STR("{}"))))
            {
                Log("Unable to send %d response, status 0x%08x" DLM, command_res_code, rc);
            }
        }
    }
    else
    {
        // Unsupported command
        Log("Unsupported command received: %.*s." DLM, az_span_size(command_request->name), az_span_ptr(command_request->name));

        int rc;
        if (az_result_failed(rc = SendCommandResponse(command_request, 404, AZ_SPAN_LITERAL_FROM_STR("{}"))))
        {
            printf("Unable to send %d response, status 0x%08x\n", 404, rc);
        }
    }
}

static int SendCommandResponse(az_iot_hub_client_method_request* request, uint16_t status, az_span response)
{
    az_result rc = AZ_OK;
    // Get the response topic to publish the command response
    char commands_response_topic[128];
    if (az_result_failed(rc = az_iot_hub_client_methods_response_get_publish_topic(&HubClient, request->request_id, status, commands_response_topic, sizeof(commands_response_topic), NULL)))
    {
        Log("Unable to get method response publish topic" DLM);
        return rc;
    }

    Log("Status: %u\tPayload: '", status);
    char* payload_char = (char*)az_span_ptr(response);
    if (payload_char != NULL)
    {
        for (int32_t i = 0; i < az_span_size(response); i++)
        {
            Log("%c", *(payload_char + i));
        }
    }
    Log("'" DLM);

    // Send the commands response
    if (mqtt_client.publish(commands_response_topic, az_span_ptr(response), az_span_size(response), false))
    {
        Log("Sent response" DLM);
    }

    return rc;
}

static void MqttSubscribeCallbackHub(char* topic, byte* payload, unsigned int length)
{
    az_span topic_span = az_span_create((uint8_t *)topic, strlen(topic));
    az_iot_hub_client_method_request command_request;

    if (az_result_succeeded(az_iot_hub_client_methods_parse_received_topic(&HubClient, topic_span, &command_request)))
    {
        DisplayPrintf("Command arrived!");
        // Determine if the command is supported and take appropriate actions
        HandleCommandMessage(az_span_create(payload, length), &command_request);
    }

    Log(DLM);
}

////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{
    ////////////////////
    // Load storage

    Storage::Load();

    ////////////////////
    // Init I/O

    Serial.begin(115200);

    pinMode(WIO_BUZZER, OUTPUT);

    tft.begin(); //start TFT LCD 
    tft.setRotation(3);
    tft.fillScreen(TFT_WHITE); 
    tft.setFreeFont(&FreeSansBold12pt7b);
    tft.setTextColor(TFT_BLACK); 

    //     tft.fillRect(120,70, 50, 20, TFT_WHITE);
    // tft.fillRect(120,130, 50, 20, TFT_WHITE);
    // tft.fillRect(120,190, 80, 20, TFT_WHITE);

    // tft.setFreeFont(&FreeSans12pt7b);
    // tft.drawString((String)(temp), 120, 70);
    // tft.drawString((String)(humid), 120, 130);
    // tft.drawString((String)(press), 120, 190);

    ////////////////////
    // Init display

    // tft.begin();
    // tft.fillScreen(TFT_WHITE);
    // tft.pushImage((tft.width() - SeeedstudioBitmapWidth) / 2, (tft.height() - SeeedstudioBitmapHeight) / 2, SeeedstudioBitmapWidth, SeeedstudioBitmapHeight, SeeedstudioBitmap);
    delay(2000);

    // Enter configuration mode

    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);
    delay(100);

    if (digitalRead(WIO_KEY_A) == LOW &&
        digitalRead(WIO_KEY_B) == LOW &&
        digitalRead(WIO_KEY_C) == LOW   )
    {
        // DisplayPrintf("In configuration mode");
        tft.fillRect(20,20, 300, 20, TFT_WHITE);
        tft.drawString("In configuration mode", 20, 20);
        CliMode();
    }

    ButtonInit();

    // DisplayPrintf("Connecting to SSID: %s", IOT_CONFIG_WIFI_SSID);
    tft.fillRect(20,20, 300, 20, TFT_WHITE);
    tft.setFreeFont(&FreeSans9pt7b);
    tft.drawString("Connecting to SSID: " + (String)(IOT_CONFIG_WIFI_SSID), 20, 20);
    do
    {
        Log(".");
        WiFi.begin(IOT_CONFIG_WIFI_SSID, IOT_CONFIG_WIFI_PASSWORD);
        delay(500);
    }
    while (WiFi.status() != WL_CONNECTED);
    // DisplayPrintf("Connected");
     tft.setFreeFont(&FreeSans9pt7b);
    tft.drawString("Connected", 20, 50);
    delay(2000);

    ////////////////////
    // Sync time server
    
    ntp.begin();
    bme280.init();
    tft.fillScreen(TFT_WHITE); 
    tft.setFreeFont(&FreeSansBold12pt7b);
    tft.setTextColor(TFT_BLACK); 
    tft.drawString("Tomato Guard",70,20);  

    int imgW = 50; int imgH = 50;
    int row = 0; int col = 0;
    
    for (row = 0; row < imgH; row++) {
    for (col = 0; col < imgW; col++) {
        int pixelIndex = (row * imgW + col) * 2;
        uint16_t pixelValue = (baroTemperature[pixelIndex] << 8) | baroTemperature[pixelIndex + 1];
        tft.drawPixel(20 + col, 55 + row, pixelValue);
    }
    }

    for (row = 0; row < imgH; row++) {
    for (col = 0; col < imgW; col++) {
        int pixelIndex = (row * imgW + col) * 2;
        uint16_t pixelValue = (baroHumidity[pixelIndex] << 8) | baroHumidity[pixelIndex + 1];
        tft.drawPixel(20 + col, 115 + row, pixelValue);
    }
    }

    for (row = 0; row < imgH; row++) {
    for (col = 0; col < imgW; col++) {
        int pixelIndex = (row * imgW + col) * 2;
        uint16_t pixelValue = (baroAtmosphere[pixelIndex] << 8) | baroAtmosphere[pixelIndex + 1];
        tft.drawPixel(20 + col, 175 + row, pixelValue);
    }
    }



#if defined(USE_CLI) || defined(USE_DPS)

    if (RegisterDeviceToDPS(IOT_CONFIG_GLOBAL_DEVICE_ENDPOINT, IOT_CONFIG_ID_SCOPE, IOT_CONFIG_REGISTRATION_ID, IOT_CONFIG_SYMMETRIC_KEY, ntp.epoch() + TOKEN_LIFESPAN, &HubHost, &DeviceId) != 0)
    {
        Abort("RegisterDeviceToDPS()");
    }

#else

    HubHost = IOT_CONFIG_IOTHUB;
    DeviceId = IOT_CONFIG_DEVICE_ID;

#endif // USE_CLI || USE_DPS
}

bool isHappyDisplayed = false;
bool isWarningDisplayed = false;

void loop()
{
    ButtonDoWork();

    soil = analogRead(A1);  
    temp = bme280.getTemperature();
    humid = bme280.getHumidity();

    // 위험도 판단 
    int riskSoil = 0, riskTemp = 0, riskHumid = 0;

    // 온도 위험도 
    if (temp <= 17 || temp >= 31)          riskTemp = 2;
    else if ((temp >= 18 && temp < 21) ||
             (temp >  28 && temp <= 30))   riskTemp = 1;
    else                                   riskTemp = 0;

    // 습도 위험도 
    if (humid <= 44 || humid >= 76)        riskHumid = 2;
    else if ((humid >= 45 && humid <= 54) ||
             (humid >= 66 && humid <= 75)) riskHumid = 1;
    else                                   riskHumid = 0;

    // 토양 수분 위험도 
    if (soil >= 600 || soil <= 379)        riskSoil = 2;
    else if ((soil >= 500 && soil <= 599) ||
             (soil >= 380 && soil <= 429)) riskSoil = 1;
    else                                   riskSoil = 0;

    // 출력 
    auto drawRiskDotWithArrow = [](int x, int y, int level, const String& arrow) {
        uint16_t c = level == 2 ? TFT_RED : (level == 1 ? TFT_ORANGE : TFT_GREEN);
        tft.fillCircle(x, y, 6, c);
        if (level > 0 && arrow.length() > 0) {
            tft.setFreeFont(&FreeSans9pt7b);
            tft.setTextColor(c);
            tft.drawString(arrow, x + 10, y - 6);
        }
    };

    tft.fillRect(90, 70, 100, 20, TFT_WHITE);  
    tft.fillRect(90, 130, 100, 20, TFT_WHITE); 
    tft.fillRect(90, 190, 100, 20, TFT_WHITE); 

    tft.setFreeFont(&FreeSans12pt7b);
    tft.setTextColor(TFT_BLACK);
    tft.drawString(String(temp) + " °C", 100, 70);
    tft.drawString(String(humid) + " %", 100, 130);
    tft.drawString(String(soil), 100, 190);

    // 방향성 표시
    String arrowTemp = (temp <= 17) ? "^" : ((temp >= 31) ? "v" : (temp < 21 ? "^" : (temp > 28 ? "v" : "")));
    String arrowHumid = (humid <= 44) ? "^" : ((humid >= 76) ? "v" : (humid < 55 ? "^" : (humid > 65 ? "v" : "")));
    String arrowSoil = (soil <= 379) ? "^" : ((soil >= 600) ? "v" : (soil < 430 ? "^" : (soil > 499 ? "v" : "")));

    drawRiskDotWithArrow(160, 78, riskTemp, arrowTemp);
    drawRiskDotWithArrow(160, 138, riskHumid, arrowHumid);
    drawRiskDotWithArrow(160, 198, riskSoil, arrowSoil);

    // 상태 판단 
    enum class DisplayState { NORMAL, SAD, DANGER };
    static DisplayState lastState = DisplayState::NORMAL;
    static bool firstLoop = true;

    int dangerCount = (riskTemp == 2) + (riskHumid == 2) + (riskSoil == 2);
    int warningCount = (riskTemp == 1) + (riskHumid == 1) + (riskSoil == 1);

    DisplayState currentState;
    if (dangerCount >= 1)
        currentState = DisplayState::DANGER;
    else if (warningCount >= 2)
        currentState = DisplayState::SAD;
    else
        currentState = DisplayState::NORMAL;

    if (firstLoop || currentState != lastState)
    {
        lastState = currentState;
        firstLoop = false;

        tft.fillRect(200, 210, 120, 30, TFT_WHITE);  
        tft.fillRect(185, 40, 120, 160, TFT_WHITE);  

        if (currentState == DisplayState::DANGER)
        {
            tft.setTextColor(TFT_RED);
            tft.setFreeFont(&FreeSans12pt7b);
            tft.drawString("Warning!", 200, 210);

            for (int row = 0; row < 160; row++) {
                for (int col = 0; col < 120; col++) {
                    int pixelIndex = (row * 120 + col) * 2;
                    uint16_t pixelValue = (tomato[pixelIndex] << 8) | tomato[pixelIndex + 1];
                    tft.drawPixel(185 + col, 40 + row, pixelValue);
                }
            }
        }
        else if (currentState == DisplayState::SAD)
        {
            for (int row = 0; row < 160; row++) {
                for (int col = 0; col < 120; col++) {
                    int pixelIndex = (row * 120 + col) * 2;
                    uint16_t pixelValue = (sad[pixelIndex] << 8) | sad[pixelIndex + 1];
                    tft.drawPixel(185 + col, 40 + row, pixelValue);
                }
            }
        }
        else // NORMAL
        {
            for (int row = 0; row < 160; row++) {
                for (int col = 0; col < 120; col++) {
                    int pixelIndex = (row * 120 + col) * 2;
                    uint16_t pixelValue = (happy[pixelIndex] << 8) | happy[pixelIndex + 1];
                    tft.drawPixel(185 + col, 40 + row, pixelValue);
                }
            }
        }
    }

    // 전송 처리 
    static uint64_t reconnectTime;
    if (!mqtt_client.connected())
    {
        DisplayPrintf("Connecting to Azure IoT Hub...");
        const uint64_t now = ntp.epoch();
        if (ConnectToHub(&HubClient, HubHost, DeviceId, IOT_CONFIG_SYMMETRIC_KEY, now + TOKEN_LIFESPAN) != 0)
        {
            DisplayPrintf("> ERROR.");
            Log("> ERROR. Status code =%d. Try again in 5 seconds." DLM, mqtt_client.state());
            delay(5000);
            return;
        }

        DisplayPrintf("> SUCCESS.");
        reconnectTime = now + TOKEN_LIFESPAN * 0.85;
    }
    else
    {
        if ((uint64_t)ntp.epoch() >= reconnectTime)
        {
            DisplayPrintf("Disconnect");
            mqtt_client.disconnect();
            return;
        }

        mqtt_client.loop();

        static unsigned long nextTelemetrySendTime = 0;
        if (millis() > nextTelemetrySendTime)
        {
            SendTelemetry();  
            nextTelemetrySendTime = millis() + TELEMETRY_FREQUENCY_MILLISECS;
        }

        for (int i = 0; i < ButtonNumber; ++i)
        {
            if (ButtonsClicked[i])
            {
                SendButtonTelemetry(static_cast<ButtonId>(i));
                ButtonsClicked[i] = false;
            }
        }
    }

    delay(1000);
}
