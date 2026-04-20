/**
 * Arduino Uno R4 WiFi - Button Signal Receiver Endpoint
 * 
 * This is an example endpoint handler that receives button press signals
 * from the ESP32 button controller via HTTP POST.
 * 
 * Expected HTTP POST requests:
 * POST /button
 * Content-Type: text/plain
 * Body: "BUTTON_PRESSED" or "BUTTON_RELEASED"
 * 
 * This can be integrated into the existing Arduino sketch or run separately.
 */

#ifdef ESP32
  // This file is for Arduino Uno R4 only
  #error "This file is for Arduino Uno R4 WiFi, not ESP32"
#endif

#include <WiFiS3.h>
#include <ArduinoHttpServer.h>

// WiFi Configuration
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Server setup
WiFiServer buttonServer(8080);

// Flag for button press event
volatile bool remoteButtonPressed = false;
unsigned long lastButtonEventTime = 0;

void setup() {
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n\n=== Arduino Uno R4 WiFi - Button Signal Receiver ===");
    
    // Initialize WiFi
    setupWiFi();
    
    // Start HTTP server for button signals
    buttonServer.begin();
    Serial.print("Button signal server listening on port 8080");
    Serial.println();
}

void loop() {
    // Check for incoming button signal HTTP requests
    WiFiClient client = buttonServer.available();
    if (client) {
        handleButtonSignalRequest(client);
    }
    
    // Check if we received a remote button press
    if (remoteButtonPressed && (millis() - lastButtonEventTime > 100)) {
        Serial.println("Remote button press event from ESP32!");
        remoteButtonPressed = false;
    }
    
    delay(50);
}

void handleButtonSignalRequest(WiFiClient client) {
    Serial.println("\n--- Incoming button signal request ---");
    
    // Read request line
    String requestLine = client.readStringUntil('\n');
    Serial.print("Request: ");
    Serial.println(requestLine);
    
    // Skip headers
    String line;
    int contentLength = 0;
    while (client.available()) {
        line = client.readStringUntil('\n');
        if (line.startsWith("Content-Length: ")) {
            contentLength = line.substring(16).toInt();
        }
        if (line == "\r") break;  // End of headers
    }
    
    // Read body
    String body;
    if (contentLength > 0) {
        char buffer[128];
        int bytesRead = client.readBytes(buffer, min(contentLength, 127));
        buffer[bytesRead] = '\0';
        body = String(buffer);
    }
    
    // Process button signal
    Serial.print("Button Signal: ");
    Serial.println(body);
    
    if (body == "BUTTON_PRESSED") {
        remoteButtonPressed = true;
        lastButtonEventTime = millis();
        Serial.println(">>> BUTTON PRESSED signal received <<<");
    } else if (body == "BUTTON_RELEASED") {
        Serial.println(">>> BUTTON RELEASED signal received <<<");
    }
    
    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Content-Length: 2");
    client.println();
    client.println("OK");
    
    client.stop();
}

void setupWiFi() {
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        while (true);
    }
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    int status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}
