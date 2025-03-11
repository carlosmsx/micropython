#include "msx_hw.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/regs/sio.h"
#include "hardware/structs/sio.h"
#include "hardware/sync.h"

uint8_t cursor_x = 0, cursor_y = 0;  // Coordenadas del cursor

void demora(){
    __nop(); __nop(); __nop(); __nop();
    __nop(); __nop(); __nop(); __nop();
    __nop(); __nop(); __nop(); __nop();
    __nop(); __nop(); __nop(); __nop();
}

void bus_as_output() {
    gpio_set_dir_out_masked(0b1111111100); //0xff << 2
}

void bus_as_input() {
    gpio_set_dir_in_masked(0b1111111100); //0xff << 2
}

void bus_write(uint8_t data) {
    //uint16_t data16 = data;
    //sio_hw->gpio_clr = 0b1111111100; //bus
    //sio_hw->gpio_set = data16 << 2; //bus
    //__nop(); __nop();
    //gpio_put_masked( 0b1111111100, data16<<2);

    gpio_put(BUS0, data & 1);
    gpio_put(BUS1, data & 2);
    gpio_put(BUS2, data & 4);
    gpio_put(BUS3, data & 8);
    gpio_put(BUS4, data & 16);
    gpio_put(BUS5, data & 32);
    gpio_put(BUS6, data & 64);
    gpio_put(BUS7, data & 128);

}

uint8_t bus_read() {
    return (sio_hw->gpio_in>>2) & 0xff;
}

void set_address(uint16_t addr) {
    bus_as_output();
    bus_write(addr & 0xff);
    __nop(); __nop();
    gpio_put(ADDR_LO, 1);
    __nop();
    gpio_put(ADDR_LO, 0);
    __nop(); __nop();
    bus_write(addr >> 8);
    __nop();
    gpio_put(ADDR_HI, 1);
    __nop();
    gpio_put(ADDR_HI, 0);
    __nop();
}

void z80_out(uint16_t port, uint8_t data) {
    set_address(port);
    bus_write(data);
    __nop(); __nop();
    gpio_put(CTRL_OE, 0);
    demora();
    gpio_put(CTRL_IORQ, 0);
    demora();
    gpio_put(CTRL_WR, 0);
    demora();
    gpio_put(CTRL_WR, 1);
    demora();
    gpio_put(CTRL_IORQ, 1);
    demora();
    gpio_put(CTRL_OE, 1);
    demora();
}

void z80_wr(uint16_t addr, uint8_t data) {
    set_address(addr);
    bus_write(data);
    __nop(); __nop();
    gpio_put(CTRL_OE, 0);
    demora();
    gpio_put(CTRL_MREQ, 0);
    demora();
    gpio_put(CTRL_WR, 0);
    demora();
    gpio_put(CTRL_WR, 1);
    demora();
    gpio_put(CTRL_MREQ, 1);
    demora();
    gpio_put(CTRL_OE, 1);
    demora();
}

uint8_t z80_in(uint16_t port) {
    set_address(port);
    bus_as_input();
    __nop(); __nop();
    gpio_put(CTRL_IORQ, 0);
    demora();
    gpio_put(CTRL_RD, 0);
    demora();
    gpio_put(CTRL_OE, 0);
    demora();
    uint8_t data = bus_read();
    //demora();
    gpio_put(CTRL_OE, 1);
    demora();
    gpio_put(CTRL_RD, 1);
    demora();
    gpio_put(CTRL_IORQ, 1);
    demora();
    return data;
}

uint8_t z80_rd(uint16_t addr) {
    set_address(addr);
    bus_as_input();
    __nop(); __nop();
    gpio_put(CTRL_MREQ, 0);
    demora();
    gpio_put(CTRL_RD, 0);
    demora();
    gpio_put(CTRL_OE, 0);
    demora();
    uint8_t data = bus_read();
    //demora();
    gpio_put(CTRL_OE, 1);
    demora();
    gpio_put(CTRL_RD, 1);
    demora();
    gpio_put(CTRL_MREQ, 1);
    demora();
    return data;
}

// Escribir un dato en la VRAM del TMS9918
void vpoke(uint16_t addr, uint8_t data)
{
    addr = addr & 0x3fff;
    z80_out(0x99, (addr & 0xff) );
    demora();
    z80_out(0x99, 0x40 | (addr >> 8) );
    demora();
    z80_out(0x98, data);
    demora();
}

uint8_t vpeek(uint16_t addr)
{
    addr = addr & 0x3fff;
    z80_out(0x99, addr & 0xff);
    demora();
    z80_out(0x99, addr >> 8);
    demora();
    return z80_in(0x98);
}

void init_ppi() {
    z80_out(0xab, 0x82);
    z80_out(0xaa, 0x50);
    z80_out(0xa8, 0xa0);
}

void charset(uint16_t base) {
    uint8_t c = z80_rd( CHARSET_ROM );
    demora();
    vpoke( base, c);
    demora();
    for (uint16_t i=1; i<2048; i++) {
        c = z80_rd( CHARSET_ROM + i );
        demora();
        z80_out(0x98, c);
        demora();
    }
}

// Escribir un valor en un registro del TMS9918
void tms9918_write_register(uint8_t reg, uint8_t value)
{
    reg = reg & 0x7;
    z80_out(0x99, value);
    demora();
    z80_out(0x99, 0x80 | reg);
    demora();
}

// Limpiar la pantalla del TMS9918
void tms9918_clear_screen(void)
{
    vpoke(0, 32);
    demora();
    for (uint16_t i=1; i<SCREEN_WIDTH*SCREEN_HEIGHT; i++) {
        z80_out(0x98, 32);
        demora();
    }
}

void tms9918_scroll_up() {
    uint16_t addr = 0;
    for (addr = 0; addr < SCREEN_WIDTH * (SCREEN_HEIGHT-1); addr++) {
        uint8_t c = vpeek(addr + SCREEN_WIDTH);
        vpoke(addr, c);
    }
    for (uint8_t i=0; i<SCREEN_WIDTH; i++, addr++) {
        vpoke(addr, 32);
    }
}

void screen0() {
    tms9918_write_register(0, 0);
    tms9918_write_register(1, 0xf0);
    tms9918_write_register(2, 0);
    tms9918_write_register(3, 0);
    tms9918_write_register(4, 0x01);
    tms9918_write_register(5, 0);
    tms9918_write_register(6, 0);
    tms9918_write_register(7, 0xf4);
}

void screen1() {
    tms9918_write_register(0, 0x00);
    tms9918_write_register(1, 0xe0);
    tms9918_write_register(2, 0x06);
    tms9918_write_register(3, 0x80);
    tms9918_write_register(4, 0x00);
    tms9918_write_register(5, 0x36);
    tms9918_write_register(6, 0x07);
    tms9918_write_register(7, 0x17);
}

void screen2() {
    tms9918_write_register(0, 2);
    tms9918_write_register(1, 224);
    tms9918_write_register(2, 6);
    tms9918_write_register(3, 255);
    tms9918_write_register(4, 3);
    tms9918_write_register(5, 54);
    tms9918_write_register(6, 7);
    tms9918_write_register(7, 4);
}

void tms9918_init() {
    // Configurar pines GPIO usados para el TMS9918
    for (uint8_t i=2; i<17; i++) {
        gpio_init(i);  // inicializo pin
        gpio_set_dir(i, GPIO_OUT);  // Configurarlo como salida
    }
    gpio_put(BUS0, 0);  // Inicializar en bajo
    gpio_put(BUS1, 0);  // Inicializar en bajo
    gpio_put(BUS2, 0);  // Inicializar en bajo
    gpio_put(BUS3, 0);  // Inicializar en bajo
    gpio_put(BUS4, 0);  // Inicializar en bajo
    gpio_put(BUS5, 0);  // Inicializar en bajo
    gpio_put(BUS6, 0);  // Inicializar en bajo
    gpio_put(BUS7, 0);  // Inicializar en bajo

    gpio_put(ADDR_LO, 0);  // Inicializar en bajo
    gpio_put(ADDR_HI, 0);  // Inicializar en bajo

    gpio_put(CTRL_IORQ, 1);  // Inicializar en alto
    gpio_put(CTRL_MREQ, 1);  // Inicializar en alto
    gpio_put(CTRL_WR, 1);  // Inicializar en alto
    gpio_put(CTRL_RD, 1);  // Inicializar en alto
    gpio_put(CTRL_OE, 1);  // Inicializar en alto

    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0,1);

    init_ppi();
    screen0();
    charset(0x800);
    tms9918_clear_screen();
}

void tms9918_write_char(uint8_t x, uint8_t y, uint8_t c) {
    uint16_t addr = x + y * SCREEN_WIDTH;
    vpoke(addr, c);
}

// Imprimir un carácter en la pantalla del TMS9918
void tms9918_putchar(char c) {
    // maneja secuencia de escape BS
    static uint8_t esc_seq = 0;
    if (c == '\b') {
        return;
    } else if (c == 27 && esc_seq == 0) {
        esc_seq=1;
        return;
    } else if (c == '[' && esc_seq == 1) {
        esc_seq=2;
        return;
    } else if (c == 'K' && esc_seq == 2) {
        esc_seq=0;
        if (cursor_x>0) cursor_x--;
        tms9918_write_char(cursor_x, cursor_y, ' ');  // borra el caracter
        return;
    } else
        esc_seq=0;

    // resto de caracteres
    if (c == '\n') {
        cursor_y++;
        cursor_x = 0;
    } else if (c == '\r') {
        cursor_x = 0;
    } else {
        tms9918_write_char(cursor_x, cursor_y, c);  // Función que escribe en la VRAM
        cursor_x++;
        if (cursor_x >= SCREEN_WIDTH) {
            cursor_y++;
            cursor_x = 0;
        }
    }

    // manejo de scroll
    if (cursor_y >= SCREEN_HEIGHT) {
        tms9918_scroll_up();
	cursor_y--;
    }
}

//--------------------------keyboard/teclado------------------

static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;


//uint8_t scan_keyboard() {
//}
const uint8_t scancode_to_ascii[2][11][8] =
{{
    {'0', '1', '2', '3', '4', '5', '6', '7' }, //row 0
    {'8', '9', '-', '=', '\\','[', ']', 0xa4}, //row 1
    {'\'',';', ',', '.', '/', ' ', 'a', 'b' }, //row 2
    {'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' }, //row 3
    {'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r' }, //row 4
    {'s', 't', 'u', 'v', 'w', 'x', 'y', 'z' }, //row 5
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }, //row 6
    {' ', ' ', ' ', ' ', ' ', '\b',' ', '\r'}, //row 7
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }, //row 8
    {' ', ' ', ' ', '0', '1', '2', '3', '4' }, //row 9
    {'5', '6', '7', '8', '9', '-', ',', '.' }  //row 10
},{
    {')', '!', '@', '#', '$', '%', '^', '&' }, //row 0
    {'*', '(', '_', '+', '|', '{', '}', 0xa5}, //row 1
    {'"', ':', '<', '>', '?', ' ', 'A', 'B' }, //row 2
    {'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J' }, //row 3
    {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R' }, //row 4
    {'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' }, //row 5
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }, //row 6
    {' ', ' ', ' ', ' ', ' ', '\b',' ', '\r'}, //row 7
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }, //row 8
    {' ', ' ', ' ', '0', '1', '2', '3', '4' }, //row 9
    {'5', '6', '7', '8', '9', '-', ',', '.' }  //row 10
}};

uint8_t key_matrix[11] = {255,255,255,255,255,255,255,255,255,255,255};
int msx_getkey(void) {
    for (uint8_t row=0; row<11; row++) {
	z80_out( 0xaa, 0xf0 | row); //TODO: flags de capslok, rele, tick y cas
        uint8_t raw = z80_in( 0xa9 );

        if (raw ^ key_matrix[row]) { //si hay cambios reviso bit por bit
            for (uint8_t col=0; col<8; col++) {
                uint8_t bit = (1<<col);
                uint8_t nbt = ~bit;

                if ((key_matrix[row] & bit) && !(raw & bit)) { //key down
                    key_matrix[row] &= nbt;
                    if ((row==6) && (col==0)) shift_pressed = 1;
                    else if ((row==6) && (col==1)) ctrl_pressed = 1;
                    else return scancode_to_ascii[shift_pressed][row][col];
                }
                else if (!(key_matrix[row] & bit) && (raw & bit)) { //key up
                    key_matrix[row] |= bit;
                    if ((row==6) && (col==0)) shift_pressed = 0;
                    else if ((row==6) && (col==1)) ctrl_pressed = 0;
                    //return scancode_to_ascii[row][col];
                }
            }
        }
    }
    return -1;
}
