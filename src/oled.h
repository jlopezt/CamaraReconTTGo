#include <Global.h>

void inicializaOled(void);
void atiendeOled(void);

void pintaCadena(String cad,int8_t desp_x,int8_t desp_y);
void pintaCadena(String cad);
void pintaCadenaLimpia(String cad,int8_t desp_x,int8_t desp_y);
void pintaCadenaLimpia(String cad);
void pintaProgreso(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint8_t avance, uint8_t total);
void pintaProgresoTexto(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint8_t avance, uint8_t total,String cad, int8_t desp_x,int8_t desp_y);
void pintaProgresoTexto(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint8_t avance, uint8_t total,String cad);