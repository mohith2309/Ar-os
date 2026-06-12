#ifndef GFX_H
#define GFX_H

#define GW 800
#define GH 600

typedef unsigned int u32;

/* design tokens — arOS 3 light glass theme (24-bit RGB) */
#define C_BLACK  0x000000
#define C_TEXT   0x2A2433
#define C_TEXT2  0x6F6685
#define C_MUTED  0xA89EBE
#define C_WHITE  0xFFFFFF
#define C_SURF   0xF7F4FC
#define C_RAISE  0xE9E2F6
#define C_PANEL  0xEFE9F8
#define C_ACCENT 0x8B5CF6
#define C_RED    0xFF5F57
#define C_GREEN  0x2BC840
#define C_YELLOW 0xFFC94D
#define C_BLUE   0x4D9DE0
#define C_PURPLE 0xB36CEB
#define C_ORANGE 0xFF9F43

void gfx_init();
void gfx_present();
void px(int x, int y, u32 c);
void pxa(int x, int y, u32 c, int a);                    /* a: 0..256 */
void grect(int x, int y, int w, int h, u32 c);
void garect(int x, int y, int w, int h, u32 c, int a);
void grrect(int x, int y, int w, int h, int r, u32 c);
void garrect(int x, int y, int w, int h, int r, u32 c, int a);
void gframe(int x, int y, int w, int h, u32 c);
void gdisc(int cx, int cy, int r, u32 c);
void gdisca(int cx, int cy, int r, u32 c, int a);
void gtri(int x0, int y0, int x1, int y1, int x2, int y2, u32 c);
void gshadow(int x, int y, int w, int h, int r);
void gtext(int x, int y, const char *s, u32 c, int scale);
int  gtextw(const char *s, int scale);
void gwallpaper(int dark);   /* render into wallpaper cache */
void gbg();                  /* blit wallpaper cache -> back buffer */

#endif
