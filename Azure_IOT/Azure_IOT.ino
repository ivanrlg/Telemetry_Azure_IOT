#include "iot_configs.h"

//Azure Constants
static const char* host = IOT_CONFIG_IOTHUB_FQDN;
static const char* device_id = IOT_CONFIG_DEVICE_ID;
static const char* device_key = IOT_CONFIG_DEVICE_KEY;
static const char* ssid = IOT_CONFIG_WIFI_SSID;
static const char* password = IOT_CONFIG_WIFI_PASSWORD;

static const int port = 8883;

// Memory allocated for the sample's variables and structures.
static WiFiClientSecure wifi_client;
static X509List cert((const char*)ca_pem);
static PubSubClient mqtt_client(wifi_client);
static az_iot_hub_client client;
static char sas_token[200];
static uint8_t signature[512];
static unsigned char encrypted_signature[32];
static char base64_decoded_device_key[32];
static unsigned long next_telemetry_send_time_ms = 0;
static char telemetry_topic[128];
static uint8_t telemetry_payload[100];
static uint32_t telemetry_send_count = 0;

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
    ConnectToWiFi();
	
    SetupPin();

    SetupLCD();

    IniLCD();

    GetTelemetryPayload();
}

void loop()
{
    if (millis() > next_telemetry_send_time_ms)
    {
        if (!mqtt_client.connected())
        {
            Connect();
        }
        else
        {
            Send();
        }
		
        next_telemetry_send_time_ms = millis() + TELEMETRY_FREQUENCY_MILLISECS;
    }
	
    mqtt_client.loop();
    delay(500);
}

void SetupPin() 
{
    dht.begin();
    pinMode(LEDAZUL, OUTPUT);
    digitalWrite(LEDAZUL, HIGH);
}

void SetupLCD()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
    {
        Serial.println("OLED display not found");
    }
    display.clearDisplay();
}


static void ConnectToWiFi()
{
    Serial.begin(115200);
    Serial.println();
    Serial.print("Connecting to WIFI SSID ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
}

void IniLCD()
{
    display.clearDisplay();

    display.setCursor(20, 0);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("IvanSinglenton.dev");

    display.setCursor(0, 20);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("Initializing Device..");
    display.display();
}

void PublishLCD(float Temperature, float humedity)
{
    display.clearDisplay();

    display.setCursor(20, 0);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("IvanSinglenton.dev");

    display.setCursor(30, 15);  //oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print(float(Temperature));
    display.print(" C");

    display.setCursor(30, 37);  //oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print(humedity);
    display.print(" %");
    display.display();
}

void RestartingLCD()
{
    display.clearDisplay();

    display.setCursor(20, 0);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("IvanSinglenton.dev");

    display.setCursor(0, 20);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("Rebooting..");
    display.display();

    delay(2000);

    ESP.restart();
}

void receivedCallback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Received [");
    Serial.print(topic);
    Serial.print("]: ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println("");
}

static void Send()
{
    digitalWrite(LEDAZUL, HIGH);

    Serial.println("ESP8266 Sending telemetry . . . ");

    if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(
        &client, NULL, telemetry_topic, sizeof(telemetry_topic), NULL)))
    {
        Serial.println("Failed az_iot_hub_client_telemetry_get_publish_topic");
        return;
    }

    String Payload = GetTelemetryPayload();

    Serial.print("Sending: ");
    Serial.println(Payload);
    Serial.println();

    mqtt_client.publish(telemetry_topic, (char*)Payload.c_str(), false);

    Serial.println("Sent OK");
    Serial.println();

    delay(100);
    digitalWrite(LEDAZUL, LOW);
}

static void InitializeClients()
{
    az_iot_hub_client_options options = az_iot_hub_client_options_default();
    options.user_agent = AZ_SPAN_FROM_STR(AZURE_SDK_CLIENT_USER_AGENT);

    wifi_client.setTrustAnchors(&cert);
    if (az_result_failed(az_iot_hub_client_init(
        &client,
        az_span_create((uint8_t*)host, strlen(host)),
        az_span_create((uint8_t*)device_id, strlen(device_id)),
        &options)))
    {
        Serial.println("Failed initializing Azure IoT Hub client");
        return;
    }

    mqtt_client.setServer(host, port);
    mqtt_client.setCallback(receivedCallback);
}


static int ConnectToAzureIoTHub()
{
    size_t client_id_length;
    char mqtt_client_id[128];
    if (az_result_failed(az_iot_hub_client_get_client_id(
        &client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
    {
        Serial.println("Failed getting client id");
        return 1;
    }

    mqtt_client_id[client_id_length] = '\0';

    char mqtt_username[128];
    // Get the MQTT user name used to connect to IoT Hub
    if (az_result_failed(az_iot_hub_client_get_user_name(
        &client, mqtt_username, sizeofarray(mqtt_username), NULL)))
    {
        printf("Failed to get MQTT clientId, return code\n");
        return 1;
    }

    Serial.println();
    Serial.print("Client ID: ");
    Serial.println(mqtt_client_id);

    Serial.print("Username: ");
    Serial.println(mqtt_username);
	
    Serial.println();

    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

    while (!mqtt_client.connected())
    {
        time_t now = time(NULL);

        Serial.println("MQTT connecting ... ");

        if (mqtt_client.connect(mqtt_client_id, mqtt_username, sas_token))
        {
            Serial.println("Connected.");
        }
        else
        {
            Serial.print("Failed, status code =");
            Serial.print(mqtt_client.state());
            Serial.println(". Trying again in 5 seconds.");

            delay(5000);
        }
    }

    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

    return 0;
}

static void Connect()
{
    ConnectToWiFi();
    InitializeTime();
    InitializeClients();

    // The SAS token is valid for 1 hour by default in this sample.
    // After one hour the sample must be restarted, or the client won't be able
    // to connect/stay connected to the Azure IoT Hub.
    if (GenerateSasToken(sas_token, sizeofarray(sas_token)) != 0)
    {
        Serial.println("Failed generating MQTT password");
    }
    else
    {
        ConnectToAzureIoTHub();
    }

    digitalWrite(LEDAZUL, LOW);
}

static int GenerateSasToken(char* sas_token, size_t size)
{
    az_span signature_span = az_span_create((uint8_t*)signature, sizeofarray(signature));
    az_span out_signature_span;
    az_span encrypted_signature_span
        = az_span_create((uint8_t*)encrypted_signature, sizeofarray(encrypted_signature));

    uint32_t expiration = getSecondsSinceEpoch() + ONE_HOUR_IN_SECS;

    // Get signature
    if (az_result_failed(az_iot_hub_client_sas_get_signature(
        &client, expiration, signature_span, &out_signature_span)))
    {
        Serial.println("Failed getting SAS signature");
        return 1;
    }

    // Base64-decode device key
    int base64_decoded_device_key_length
        = base64_decode_chars(device_key, strlen(device_key), base64_decoded_device_key);

    if (base64_decoded_device_key_length == 0)
    {
        Serial.println("Failed base64 decoding device key");
        return 1;
    }

    // SHA-256 encrypt
    br_hmac_key_context kc;
    br_hmac_key_init(
        &kc, &br_sha256_vtable, base64_decoded_device_key, base64_decoded_device_key_length);

    br_hmac_context hmac_ctx;
    br_hmac_init(&hmac_ctx, &kc, 32);
    br_hmac_update(&hmac_ctx, az_span_ptr(out_signature_span), az_span_size(out_signature_span));
    br_hmac_out(&hmac_ctx, encrypted_signature);

    // Base64 encode encrypted signature
    String b64enc_hmacsha256_signature = base64::encode(encrypted_signature, br_hmac_size(&hmac_ctx));

    az_span b64enc_hmacsha256_signature_span = az_span_create(
        (uint8_t*)b64enc_hmacsha256_signature.c_str(), b64enc_hmacsha256_signature.length());

    // URl-encode base64 encoded encrypted signature
    if (az_result_failed(az_iot_hub_client_sas_get_password(
        &client,
        expiration,
        b64enc_hmacsha256_signature_span,
        AZ_SPAN_EMPTY,
        sas_token,
        size,
        NULL)))
    {
        Serial.println("Failed getting SAS token");
        return 1;
    }

    return 0;
}

static void InitializeTime()
{
    Serial.print("Setting time using SNTP");

    configTime(-4 * 3600, 0, NTP_SERVERS);
    time_t now = time(NULL);
    while (now < 1510592825)
    {
        delay(500);
        Serial.print(".");
        now = time(NULL);
    }
    Serial.println("done!");
}

static String GetTelemetryPayload()
{
    PrintCurrentTime();

    float Humidity = dht.readHumidity();
    float Temperature = dht.readTemperature();

    Serial.println("");

    Serial.print(Temperature);
    Serial.println(" C");

    Serial.print(Humidity);
    Serial.println(" %");

    Serial.println();

    String DeviceId = String(device_id);

    StaticJsonDocument<1024> doc;

    if (isnan(Humidity) || isnan(Temperature)) {
        Serial.println("Failed to read from DHT sensor!");

        doc["DeviceId"] = DeviceId;
        doc["Humidity"] = "Error";
        doc["Temperature"] = "Error";
        doc["Date"] = GetCurrentLocalTimeString();

        String Error;

        serializeJson(doc, Error);

        return Error;
    }

    PublishLCD(Temperature, Humidity);

    doc["DeviceId"] = DeviceId;
    doc["Humidity"] = floatAsString(Humidity);
    doc["Temperature"] = floatAsString(Temperature);
    doc["Date"] = GetCurrentLocalTimeString();

    String Message;

    serializeJson(doc, Message);

    return Message;
}

static String GetCurrentLocalTimeString()
{
    time_t now = time(NULL);
	
    tm* local_time = localtime(&now);

    char current_local_time[256];

	strftime(current_local_time, sizeof(current_local_time), "%Y-%m-%d %H:%M:%S", local_time);

	String CurrentLocal = String(current_local_time);
	
    return CurrentLocal;
}


static void PrintCurrentTime()
{
    Serial.print("Current time: ");
    Serial.println(GetCurrentLocalTimeString());
}

static uint32_t getSecondsSinceEpoch()
{
    return (uint32_t)time(NULL);
}

String floatAsString(float val) {
    char temp[6];
    dtostrf(val, 6, 2, temp);
    String fas = String(temp);
    fas.trim();
    return fas;
}