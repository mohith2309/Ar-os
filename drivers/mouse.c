#include "mouse.h"
#include "ports.h"
#include "../cpu/isr.h"
#include "../gui/desktop.h"
#include "gfx.h"

static unsigned char cycle = 0;
static unsigned char pkt[3];
static int sx = GW / 2, sy = GH / 2;

static void mwait_w() { for (int i = 0; i < 100000; i++) if (!(port_byte_in(0x64) & 2)) return; }
static void mwait_r() { for (int i = 0; i < 100000; i++) if (port_byte_in(0x64) & 1) return; }

static void mouse_send(unsigned char b) {
    mwait_w(); port_byte_out(0x64, 0xD4);
    mwait_w(); port_byte_out(0x60, b);
    mwait_r(); port_byte_in(0x60);            /* consume ACK */
}

static void mouse_cb(registers_t *r) {
    (void)r;
    unsigned char b = port_byte_in(0x60);
    switch (cycle) {
    case 0:
        if (b & 0x08) { pkt[0] = b; cycle = 1; }  /* bit3 set on first byte */
        break;
    case 1:
        pkt[1] = b; cycle = 2;
        break;
    case 2: {
        pkt[2] = b; cycle = 0;
        if (pkt[0] & 0xC0) break;            /* drop overflowed packets */
        int dx = (pkt[0] & 0x10) ? (int)pkt[1] - 256 : (int)pkt[1];
        int dy = (pkt[0] & 0x20) ? (int)pkt[2] - 256 : (int)pkt[2];
        sx += dx * 2; sy -= dy * 2;          /* scaled for 800x600 */
        if (sx < 0) sx = 0;
        if (sx > GW - 1) sx = GW - 1;
        if (sy < 0) sy = 0;
        if (sy > GH - 1) sy = GH - 1;
        desktop_handle_mouse(sx, sy, pkt[0] & 7);
        break; }
    }
}

void init_mouse() {
    mwait_w(); port_byte_out(0x64, 0xA8);     /* enable aux device */
    mwait_w(); port_byte_out(0x64, 0x20);     /* read controller config */
    mwait_r();
    unsigned char cfg = port_byte_in(0x60);
    cfg |= 0x02;                               /* IRQ12 on */
    cfg &= ~0x20;                              /* aux clock on */
    mwait_w(); port_byte_out(0x64, 0x60);
    mwait_w(); port_byte_out(0x60, cfg);
    mouse_send(0xF6);                          /* set defaults */
    mouse_send(0xF4);                          /* enable streaming */
    register_interrupt_handler(IRQ12, mouse_cb);
}
