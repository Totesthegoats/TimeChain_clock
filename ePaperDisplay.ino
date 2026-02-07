#include <GxEPD2_BW.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// WiFi credentials
const char* ssid = "N-Pro Staff";
const char* password = "!!DontL0seYourHead!!";


// Display setup
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(/*CS=*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

// Timer variables
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30000; // Check every 30 seconds

// Data tracking
String currentBlockHeight = "";
String blocksToHalving = "";
String blocksToEpochEnd = "";
String supplyMined = "";
String percentMined = "";
String feeRate = "";

// First run flag
bool firstRun = true;

// Bitcoin logo bitmap - 40x40 pixels
const unsigned char bitcoin_logo [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 
  0x00, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xc0, 
  0x07, 0xff, 0xff, 0xe0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xc0, 0x03, 0xf0, 
  0x1f, 0x80, 0x01, 0xf8, 0x1f, 0x07, 0xc0, 0xf8, 0x3e, 0x0f, 0xe0, 0x7c, 
  0x3e, 0x1f, 0xf0, 0x7c, 0x3c, 0x1f, 0xf0, 0x3c, 0x7c, 0x1f, 0xf0, 0x3e, 
  0x7c, 0x1f, 0xf0, 0x3e, 0x78, 0x1f, 0xf0, 0x1e, 0x78, 0x1f, 0xe0, 0x1e, 
  0x78, 0x0f, 0xc0, 0x1e, 0x78, 0x0f, 0xe0, 0x1e, 0x78, 0x1f, 0xf0, 0x1e, 
  0x78, 0x1f, 0xf0, 0x1e, 0x7c, 0x1f, 0xf0, 0x3e, 0x7c, 0x1f, 0xf0, 0x3e, 
  0x3c, 0x1f, 0xf0, 0x3c, 0x3e, 0x1f, 0xf0, 0x7c, 0x3e, 0x0f, 0xe0, 0x7c, 
  0x1f, 0x07, 0xc0, 0xf8, 0x1f, 0x80, 0x01, 0xf8, 0x0f, 0xc0, 0x03, 0xf0, 
  0x0f, 0xf0, 0x0f, 0xf0, 0x07, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xc0, 
  0x01, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0x00, 0x00, 0x1f, 0xf8, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Forward declarations
void fetchAllData();
void updateBlockHeight(String newHeight);
void updateHalving(String newBlocks);
void updateEpoch(String newBlocks);
void updateSupply(String newSupply, String newPercent);
void updateFeeRate(String newFee);
void displayFullDashboard();
void displayError(String error);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Timechain Calendar Dashboard ===");
  
  display.init(115200);
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);  // Add this
  display.fillScreen(GxEPD_WHITE);     // Add this
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    fetchAllData();
    displayFullDashboard();
    firstRun = false;
    lastUpdate = millis();
  } else {
    Serial.println("\nWiFi connection failed!");
    displayError("WiFi Failed");
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate >= updateInterval) {
    Serial.println("\n--- Checking for updates ---");
    fetchAllData();
    lastUpdate = currentMillis;
  }
  
  delay(1000);
}

void fetchAllData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }
  
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  bool anyChange = false;
  
  // Fetch block height
  Serial.println("Fetching block height...");
  http.begin(client, "https://mempool.space/api/blocks/tip/height");
  if (http.GET() == 200) {
    String newBlockHeight = http.getString();
    newBlockHeight.trim();
    
    if (newBlockHeight != currentBlockHeight) {
      Serial.println("Block changed: " + currentBlockHeight + " -> " + newBlockHeight);
      currentBlockHeight = newBlockHeight;
      anyChange = true;
      
      // Recalculate halving, epoch, and supply when block changes
      if (currentBlockHeight.length() > 0) {
        long blockNum = currentBlockHeight.toInt();
        
        // Halving
        long nextHalving = 1050000;
        long halvingLeft = nextHalving - blockNum;
        blocksToHalving = String(halvingLeft);
        
        // Epoch
        long epochBlock = blockNum % 2016;
        long epochLeft = 2016 - epochBlock;
        blocksToEpochEnd = String(epochLeft);
        
        // Supply
        float btcMined;
        if (blockNum < 210000) {
          btcMined = blockNum * 50.0;
        } else if (blockNum < 420000) {
          btcMined = 210000 * 50.0 + (blockNum - 210000) * 25.0;
        } else if (blockNum < 630000) {
          btcMined = 210000 * 50.0 + 210000 * 25.0 + (blockNum - 420000) * 12.5;
        } else if (blockNum < 840000) {
          btcMined = 210000 * 50.0 + 210000 * 25.0 + 210000 * 12.5 + (blockNum - 630000) * 6.25;
        } else {
          btcMined = 210000 * 50.0 + 210000 * 25.0 + 210000 * 12.5 + 210000 * 6.25 + (blockNum - 840000) * 3.125;
        }
        
        float btcInMillions = btcMined / 1000000.0;
        supplyMined = String(btcInMillions, 2) + "M";
        
        float percent = (btcMined / 21000000.0) * 100.0;
        percentMined = String(percent, 2) + "%";
      }
    } else {
      Serial.println("No block change");
    }
  }
  http.end();
  
  // Fetch fee rates
  Serial.println("Fetching fees...");
  http.begin(client, "https://mempool.space/api/v1/fees/recommended");
  if (http.GET() == 200) {
    String payload = http.getString();
    int fastestStart = payload.indexOf("\"fastestFee\":") + 13;
    int fastestEnd = payload.indexOf(",", fastestStart);
    String newFeeRate = payload.substring(fastestStart, fastestEnd);
    
    if (newFeeRate != feeRate && newFeeRate.length() > 0) {
      Serial.println("Fee changed: " + feeRate + " -> " + newFeeRate);
      feeRate = newFeeRate;
      anyChange = true;
    } else {
      Serial.println("No fee change");
    }
  }
  http.end();
  
  // Only refresh display if something changed
  if (anyChange) {
    displayFullDashboard();
  }
}

void displayFullDashboard() {
  Serial.println("Drawing full dashboard...");
  
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    // Header with Bitcoin logo and centered title
    display.drawBitmap(10, 5, bitcoin_logo, 40, 40, GxEPD_BLACK);
    
    display.setFont(&FreeMonoBold9pt7b);
    String title = "TIMECHAIN CALENDAR";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int centerX = (400 - w) / 2;
    display.setCursor(centerX, 25);
    display.print(title);
    
    // Horizontal line under header
    display.drawLine(0, 50, 400, 50, GxEPD_BLACK);
    
    // "BLOCK HEIGHT" label above number
    display.setFont(&FreeMonoBold9pt7b);
    String label = "BLOCK HEIGHT";
    display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    centerX = (400 - w) / 2;
    display.setCursor(centerX, 75);
    display.print(label);
    
    // Block height - BIG in center
    display.setFont(&FreeMonoBold24pt7b);
    display.getTextBounds(currentBlockHeight, 0, 0, &x1, &y1, &w, &h);
    centerX = (400 - w) / 2;
    display.setCursor(centerX, 115);
    display.print(currentBlockHeight);
    
    // Horizontal line before stats
    display.drawLine(0, 130, 400, 130, GxEPD_BLACK);
    
    // Three column stats section
    int colWidth = 133;
    
    // Column 1: HALVING
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(15, 155);
    display.print("HALVING");
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(15, 185);
    display.print(blocksToHalving);
    display.setFont();
    display.setCursor(15, 200);
    display.print("blocks");
    
    // Vertical line
    display.drawLine(colWidth, 135, colWidth, 220, GxEPD_BLACK);
    
    // Column 2: EPOCH
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(colWidth + 25, 155);
    display.print("EPOCH");
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(colWidth + 25, 185);
    display.print(blocksToEpochEnd);
    display.setFont();
    display.setCursor(colWidth + 25, 200);
    display.print("blocks");
    
    // Vertical line
    display.drawLine(colWidth * 2, 135, colWidth * 2, 220, GxEPD_BLACK);
    
    // Column 3: SUPPLY
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(colWidth * 2 + 15, 155);
    display.print("SUPPLY");
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(colWidth * 2 + 15, 185);
    display.print(supplyMined);
    display.setFont();
    display.setCursor(colWidth * 2 + 15, 200);
    display.print(percentMined);
    
    // Horizontal line before footer
    display.drawLine(0, 225, 400, 225, GxEPD_BLACK);
    
    // Footer - fee rate with label
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(10, 250);
    display.print("FEE RATE");
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 275);
    display.print(feeRate + " sat/vB");
    
  } while (display.nextPage());
  
  Serial.println("Full dashboard displayed!");
  display.hibernate();
}

void updateBlockHeight(String newHeight) {
  Serial.println("Partial update: Block height");
  
  display.setPartialWindow(0, 55, 400, 80);
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    // "BLOCK HEIGHT" label
    display.setFont(&FreeMonoBold9pt7b);
    String label = "BLOCK HEIGHT";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    int centerX = (400 - w) / 2;
    display.setCursor(centerX, 75);
    display.print(label);
    
    // Block height
    display.setFont(&FreeMonoBold24pt7b);
    display.getTextBounds(newHeight, 0, 0, &x1, &y1, &w, &h);
    centerX = (400 - w) / 2;
    display.setCursor(centerX, 115);
    display.print(newHeight);
    
  } while (display.nextPage());
  
  display.hibernate();
}

void updateHalving(String newBlocks) {
  Serial.println("Partial update: Halving");
  
  display.setPartialWindow(0, 160, 133, 60);
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(15, 185);
    display.print(newBlocks);
    display.setFont();
    display.setCursor(15, 200);
    display.print("blocks");
    
  } while (display.nextPage());
  
  display.hibernate();
}

void updateEpoch(String newBlocks) {
  Serial.println("Partial update: Epoch");
  
  display.setPartialWindow(133, 160, 133, 60);
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(158, 185);
    display.print(newBlocks);
    display.setFont();
    display.setCursor(158, 200);
    display.print("blocks");
    
  } while (display.nextPage());
  
  display.hibernate();
}

void updateSupply(String newSupply, String newPercent) {
  Serial.println("Partial update: Supply");
  
  display.setPartialWindow(266, 160, 134, 60);
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(281, 185);
    display.print(newSupply);
    display.setFont();
    display.setCursor(281, 200);
    display.print(newPercent);
    
  } while (display.nextPage());
  
  display.hibernate();
}

void updateFeeRate(String newFee) {
  Serial.println("Partial update: Fee rate");
  
  display.setPartialWindow(0, 260, 200, 30);
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 275);
    display.print(newFee + " sat/vB");
    
  } while (display.nextPage());
  
  display.hibernate();
}

void displayError(String error) {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(20, 100);
    display.setTextSize(2);
    display.print("ERROR:");
    display.setCursor(20, 130);
    display.print(error);
  } while (display.nextPage());
  
  display.hibernate();
}
