#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  -1
#define TFT_BL   4

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define RX_PIN 3
#define TX_PIN 1
HardwareSerial mySerial(1);

float cpuUsage = -1.0;
float ramUsage = -1.0;
float gpuUsage = -1.0;

float prevCpuUsage = -1.0;
float prevRamUsage = -1.0;
float prevGpuUsage = -1.0;

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.init(135, 240);
  tft.setRotation(1);

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  drawAxes();
  Serial.println("UART Display Initialized. Waiting for data...");
}

void loop() {
  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
    Serial.print("Raw data received: ");
    Serial.println(data);

    if (data.length() > 0) {
      int parsedCount = sscanf(data.c_str(), "%f,%f,%f", &cpuUsage, &ramUsage, &gpuUsage);

      if (parsedCount == 3) {
        updateStats();
      } else {
        Serial.println("Error: Failed to parse data. Check input format.");
      }
    }
  }
}

void drawAxes() {
  int graphX = 50;
  int graphY = 30;
  int graphHeight = 80;
  int graphWidth = 150;

  // Draw Y-axis
  tft.drawFastVLine(graphX, graphY, graphHeight + 1, ST77XX_WHITE);

  // Draw X-axis with 1-pixel spacing from bars
  tft.drawFastHLine(graphX, graphY + graphHeight + 1, graphWidth, ST77XX_WHITE);
}

void drawBarWithGradient(int x, int y, int barWidth, int maxHeight, float value, float prevValue) {
  int prevHeight = (prevValue / 100.0) * maxHeight;
  int newHeight = (value / 100.0) * maxHeight;

  // Clear only the changed part of the bar
  if (newHeight < prevHeight) {
    tft.fillRect(x, y + maxHeight - prevHeight, barWidth, prevHeight - newHeight, ST77XX_BLACK);
  } else if (newHeight > prevHeight) {
    for (int i = prevHeight; i < newHeight; i++) {
      float normalizedValue = (float)i / maxHeight;
      uint16_t color = tft.color565(
          255 * normalizedValue,           // Red increases with height
          255 * (1 - normalizedValue),     // Green decreases with height
          0                                // No blue
      );
      tft.drawFastHLine(x, y + maxHeight - i, barWidth, color);
    }
  }
}

void updateStats() {
  int graphX = 50;
  int graphY = 30;
  int graphHeight = 80;
  int barWidth = 30;
  int barSpacing = 20;
  int barBaseX = graphX + 10;

  // CPU Bar
  if (cpuUsage != prevCpuUsage) {
    drawBarWithGradient(barBaseX, graphY, barWidth, graphHeight, cpuUsage, prevCpuUsage);
    updateBarText(barBaseX, graphY, graphHeight, barWidth, cpuUsage, "CPU");
    prevCpuUsage = cpuUsage;
  }

  // RAM Bar
  if (ramUsage != prevRamUsage) {
    drawBarWithGradient(barBaseX + barWidth + barSpacing, graphY, barWidth, graphHeight, ramUsage, prevRamUsage);
    updateBarText(barBaseX + barWidth + barSpacing, graphY, graphHeight, barWidth, ramUsage, "RAM");
    prevRamUsage = ramUsage;
  }

  // GPU Bar
  if (gpuUsage != prevGpuUsage) {
    drawBarWithGradient(barBaseX + 2 * (barWidth + barSpacing), graphY, barWidth, graphHeight, gpuUsage, prevGpuUsage);
    updateBarText(barBaseX + 2 * (barWidth + barSpacing), graphY, graphHeight, barWidth, gpuUsage, "GPU");
    prevGpuUsage = gpuUsage;
  }

  Serial.println("Display updated with new stats.");
}

void updateBarText(int x, int y, int maxHeight, int barWidth, float value, const char* label) {
  // Clear and update percentage text above the bar
  tft.fillRect(x, y - 15, barWidth, 10, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(x + 2, y - 15);
  tft.print(value, 1);
  tft.print("%");

  // Update label text below the bar
  tft.fillRect(x, y + maxHeight + 5, barWidth, 10, ST77XX_BLACK);
  tft.setCursor(x + 2, y + maxHeight + 5);
  tft.print(label);
}
