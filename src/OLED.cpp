#define SSD1306_ADDRESS 0x3c

#define I2C_SDA             21
#define I2C_SCL             22

#define SSD130_MODLE_TYPE   0       // 0 : GEOMETRY_128_64  // 1: GEOMETRY_128_32

#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#include "oled.h"

SSD1306 oled(SSD1306_ADDRESS, I2C_SDA, I2C_SCL, (OLEDDISPLAY_GEOMETRY)SSD130_MODLE_TYPE);
OLEDDisplayUi ui(&oled);

void inicializaUI(void)
    {
    static FrameCallback frames[] = {
    /*[](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setFont(ArialMT_Plain_10);

        display->drawString(64 + x, 9 + y, "SSID:" + RedWifi.nombreSSID());
        display->drawString(64 + x, 25 + y, "IP:" + RedWifi.getIP(false));

        },
    [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setFont(ArialMT_Plain_10);


        display->drawString( 64 + x, 5 + y, "Camera Ready! Use");
        display->drawString(64 + x, 25 + y, "http://" + RedWifi.getIP(false) );
        display->drawString(64 + x, 45 + y, "to connect");
        },*/
    [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setFont(ArialMT_Plain_10);


        display->drawString( 64 + x, 12 + y, String("Reconocimiento facial: ") + (getRecon()?"on":"off"));
        display->drawString( 64 + x, 48 + y, String("Detector: ") + (Entradas.estadoEntrada(0)?"on":"off"));
        }
    };

    ui.setTargetFPS(30);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, sizeof(frames) / sizeof(frames[0]));
    ui.setTimePerFrame(1000);
    ui.disableIndicator();    
    }

void inicializaOled(void)
    {
    Wire.begin(I2C_SDA, I2C_SCL);

    oled.init();
    
    inicializaUI();
    }

void atiendeOled(void) 
    {
    //ui.update();    
    }

/*****************************************Funciones auxiliares **********************************/
/*****************************************
 * enum OLEDDISPLAY_TEXT_ALIGNMENT {
  TEXT_ALIGN_LEFT = 0,
  TEXT_ALIGN_RIGHT = 1,
  TEXT_ALIGN_CENTER = 2,
  TEXT_ALIGN_CENTER_BOTH = 3
};
********************************************/
void pintaCadena(String cad,int8_t desp_x=0,int8_t desp_y=0)
    {
    const int8_t ALTO_TEXTO=20;    
    OLEDDISPLAY_COLOR colorInicial=oled.getColor();

    oled.setFont(ArialMT_Plain_16);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    oled.setColor(BLACK);
    oled.fillRect(0,0,oled.getWidth()-1,(oled.getHeight() / 2) + (ALTO_TEXTO/2) + desp_y);
    oled.setColor(WHITE);
    oled.drawString( oled.getWidth() / 2 + desp_x, oled.getHeight() / 2 - ALTO_TEXTO/2 + desp_y, cad.c_str());
    oled.setColor(colorInicial);
    oled.display();    
    }
void pintaCadena(String cad){pintaCadena(cad,0,0);};

void pintaCadenaLimpia(String cad,int8_t desp_x=0,int8_t desp_y=0)
    {
    oled.clear();
    pintaCadena(cad, desp_x, desp_y);
    }
void pintaCadenaLimpia(String cad) {pintaCadenaLimpia(cad, 0, 0);}

void pintaProgreso(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint8_t avance, uint8_t total)
    {
    uint8_t progreso=(uint8_t)(100*((float)avance/(float)total));
    OLEDDISPLAY_COLOR colorInicial=oled.getColor();

    oled.setColor(WHITE);
    //void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);
    oled.drawProgressBar(x, y, ancho, alto, progreso);
    oled.setColor(colorInicial);
    oled.display();        
    }

void pintaProgresoTexto(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint8_t avance, uint8_t total,String cad, int8_t desp_x,int8_t desp_y)
    {
    pintaCadena(cad, desp_x, desp_y);
    pintaProgreso(x, y, ancho, alto, avance, total);    
    }
void pintaProgresoTexto(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint8_t avance, uint8_t total,String cad)
    {pintaProgresoTexto(x, y, ancho, alto, avance, total,cad, 0,0);}


