#include "../drivers/gfx.h"
#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../gui/desktop.h"

void kernel_main() {
    gfx_init();
    isr_install();
    irq_install();
    init_timer(50);
    init_keyboard();
    init_mouse();
    desktop_draw();
    desktop_loop();
}
