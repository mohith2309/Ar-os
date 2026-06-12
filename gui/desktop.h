#ifndef DESKTOP_H
#define DESKTOP_H
void desktop_draw();
void desktop_loop();
void desktop_handle_key(char c);
void desktop_handle_mouse(int x, int y, int buttons);
void desktop_tick();
#endif
