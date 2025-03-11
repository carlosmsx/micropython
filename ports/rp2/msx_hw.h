#ifndef TMS9918_H
#define TMS9918_H

#include <stdint.h>

// Definición de pines GPIO usados para la comunicación con el TMS9918
#define BUS0 2
#define BUS1 3
#define BUS2 4
#define BUS3 5
#define BUS4 6
#define BUS5 7
#define BUS6 8
#define BUS7 9
#define ADDR_LO 10
#define ADDR_HI 11
#define CTRL_IORQ 12
#define CTRL_MREQ 13
#define CTRL_WR 14
#define CTRL_RD 15
#define CTRL_OE 16

#define CHARSET_ROM 7103
#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 24

// Inicialización del TMS9918
void tms9918_init(void);

// Escribir un dato en la VRAM del TMS9918
void tms9918_write_data(uint8_t data);

// Escribir un valor en un registro del TMS9918
void tms9918_write_register(uint8_t reg, uint8_t value);

// Imprimir un carácter en la pantalla del TMS9918
void tms9918_putchar(char c);

// Limpiar la pantalla del TMS9918
void tms9918_clear_screen(void);

void vpoke(uint16_t, uint8_t);
uint8_t vpeek(uint16_t);
void z80_out(uint16_t, uint8_t);
uint8_t z80_in(uint16_t);

#endif
