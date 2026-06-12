#include "timer.h"
#include "isr.h"
#include "../drivers/ports.h"
#include "../gui/desktop.h"
static unsigned int tick=0;
static unsigned int hz=50;
static void timer_cb(registers_t *r){
    (void)r;
    tick++;
    if(tick%hz==0)desktop_tick();
}
void init_timer(unsigned int freq){
    hz=freq;
    register_interrupt_handler(IRQ0,timer_cb);
    unsigned int d=1193180/freq;
    port_byte_out(0x43,0x36);
    port_byte_out(0x40,(unsigned char)(d&0xff));
    port_byte_out(0x40,(unsigned char)((d>>8)&0xff));
}
