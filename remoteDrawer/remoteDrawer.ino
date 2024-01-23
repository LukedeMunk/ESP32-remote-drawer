/*
 * File:      remoteDrawer.ino
 * Author:    Luke de Munk
 * 
 * Code for a web interface with a canvas to draw on.
 * Shows canvas (near) real-time on a ST7735 display.
 * For more info, check out:
 * https://github.com/LukedeMunk/ESP32-remote-drawer
 */
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <Adafruit_GFX.h>                                                   //Core graphics library
#include <Adafruit_ST7735.h>                                                //Hardware-specific library for ST7735
#include <Fonts/FreeSans9pt7b.h>
#include "Debugger.h"

/* Pins */
#define SCL_PIN             8                                               //Use hardware SPI GPIO clock pin for your hardware
#define SDA_PIN             23                                              //Use hardware SPI GPIO data pin for your hardware

#define RST_PIN             4                                               //Display reset pin
#define DC_PIN              16                                              //Register select (stands for Data Control perhaps) (0 = command, 1 = display)
#define CS_PIN              17                                              //Display enable (Chip select), if not enabled will not talk on SPI bus
#define BLK_PIN             5                                               //Backlight control pin (0 = off, 1 = on)

#define WIDTH               160
#define HEIGHT              80

/* Default colors */
#define BACKGROUND_COLOR    ST77XX_BLACK

/* Pencil types */
#define PENCIL              0
#define ERASER              1

/* Network */
#define SSID                "XX"
#define PASSWORD            "XX"
AsyncWebServer server(80);                                                  //Create AsyncWebServer object on port 80

Adafruit_ST7735 tft = Adafruit_ST7735(CS_PIN, DC_PIN, RST_PIN);             //Initialise display object

uint32_t color = tft.color565(255, 255, 255);
uint8_t lineWidth = 3;
uint8_t pencilType = PENCIL;

/**************************************************************************/
/*!
  @brief    Replaces placeholders with actual data.
*/
/**************************************************************************/
String processor(const String& var) {
    if (var == "COLOR") {
        char colorString[6];
        sprintf(colorString, "%06x", color);                                //Contruct color string

        return String(colorString);
    } else if (var == "LINE_WIDTH") {
        return String(lineWidth);
    } else {
        return " placeholder_error ";
    }
    return String();
}

/**************************************************************************/
/*!
  @brief    Setup the controller.
*/
/**************************************************************************/
void setup() {
    Serial.begin(115200);                                                   //Serial port for debugging purposes
  
    /* Initialize tft display */
    tft.initR(INITR_MINI160x80);                                            //Init ST7735S mini display
    tft.setColRowStart(26, 1);
    tft.setRotation(3);
    tft.invertDisplay(true);
    
    /* Draw loading screen */
    tft.fillScreen(tft.color565(255, 255, 255));
    delay(1000);
    tft.fillScreen(BACKGROUND_COLOR);

    tft.setCursor(6, HEIGHT/2);
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(tft.color565(255, 255, 255));
    tft.print("Initialising SPIFFS");

    
    /* Initialize SPIFFS */
    if(!SPIFFS.begin(true)){
        debugln("An Error has occurred while mounting SPIFFS");
        return;
    }
    
    /* Connect to Wi-Fi */
    WiFi.begin(SSID, PASSWORD);
    
    debug("Connecting to WiFi.");
    
    tft.setCursor(6, HEIGHT/2);
    tft.fillScreen(BACKGROUND_COLOR);
    tft.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        debug(".");
    }
    
    debugln("");
    debug("IP: ");
    debugln(WiFi.localIP());

    tft.setCursor(6, HEIGHT/2);
    tft.fillScreen(BACKGROUND_COLOR);
    tft.print("Initialising server");
    /*
    *  Routes for loading all the necessary files
    */
    /* Load index.html file */
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    
    /* Load style.css file */
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });
    
    /* Load style_mobile.css file */
    server.on("/style_mobile.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style_mobile.css", "text/css");
    });

    /* Load favicon.ico file */
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/favicon.ico", "image/x-icon");
    });

    /* Load jquery.min.js file */
    server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/jquery.min.js", "text/script");
    });

    /* Load base.js file */
    server.on("/base.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/base.js", "text/script");
    });
    /*
    * End of file loading
    */

    /*
    * Routes for receiving data
    */
    /* Route for updating xy values */
    server.on("/update_xy", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("x") && request->hasParam("y")) {
            uint16_t x = (uint16_t) atoi(request->getParam("x")->value().c_str());
            uint16_t y = (uint16_t) atoi(request->getParam("y")->value().c_str());

            if (pencilType == PENCIL) {
                tft.fillCircle(x, y, lineWidth, color);
            } else {
                tft.fillCircle(x, y, lineWidth, BACKGROUND_COLOR);
            }
        }
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    /* Route for updating color */
    server.on("/update_color", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("color")) {
            color = (uint32_t) strtol(request->getParam("color")->value().c_str(), NULL, 16);
        }
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    /* Route for updating line width */
    server.on("/update_line_width", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("width")) {
            lineWidth = (uint8_t) atoi(request->getParam("width")->value().c_str());
        }
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    
    /* Route for updating pencil type */
    server.on("/update_pencil_type", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("pencil")) {
            pencilType = (uint8_t) atoi(request->getParam("pencil")->value().c_str());
        }
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    /* Route for clearing screen */
    server.on("/erase_canvas", HTTP_GET, [](AsyncWebServerRequest *request){
        tft.fillScreen(BACKGROUND_COLOR);
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    
    server.begin();                                                         //Start server

    tft.setCursor(6, HEIGHT/2);
    tft.fillScreen(BACKGROUND_COLOR);
    tft.print("Ready");
    delay(1000);
    
    tft.setCursor(6, HEIGHT/2);
    tft.fillScreen(BACKGROUND_COLOR);
    tft.print("Go to:");
    delay(1000);
    
    tft.setCursor(6, HEIGHT/2);
    tft.fillScreen(BACKGROUND_COLOR);
    tft.print(WiFi.localIP().toString().c_str());
    delay(4000);
    
    tft.fillScreen(BACKGROUND_COLOR);
}

/**************************************************************************/
/*!
  @brief    Mainloop.
*/
/**************************************************************************/
void loop() {

}
