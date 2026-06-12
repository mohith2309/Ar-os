#include "gfx.h"


typedef unsigned char  u8;
typedef unsigned short u16;

/* VBE mode info left at 0x600 by the bootloader */
static u8  *fb;
static int  fpitch, fbytes;
/* big buffers live above 1MB (A20 is on) */
static u32 *back = (u32 *)0x400000;   /* 800*600*4 = 1.92MB */
static u32 *wall = (u32 *)0x800000;   /* cached wallpaper   */

void gfx_init() {
    u8 *mi = (u8 *)0x600;
    fpitch = *(u16 *)(mi + 16);
    fbytes = mi[25] / 8;
    fb     = (u8 *)*(u32 *)(mi + 40);
}

void gfx_present() {
    for (int y = 0; y < GH; y++) {
        u32 *s = back + y * GW;
        u8  *d = fb + y * fpitch;
        if (fbytes == 4) {
            u32 *d4 = (u32 *)d;
            for (int x = 0; x < GW; x++) d4[x] = s[x];
        } else {
            for (int x = 0; x < GW; x++) {
                u32 c = s[x];
                d[x * 3]     = c;
                d[x * 3 + 1] = c >> 8;
                d[x * 3 + 2] = c >> 16;
            }
        }
    }
}

static u32 mix(u32 d, u32 s, int a) {
    int rb = (((d & 0xFF00FF) * (256 - a) + (s & 0xFF00FF) * a) >> 8) & 0xFF00FF;
    int g  = (((d & 0x00FF00) * (256 - a) + (s & 0x00FF00) * a) >> 8) & 0x00FF00;
    return rb | g;
}

void px(int x, int y, u32 c) {
    if ((unsigned)x >= GW || (unsigned)y >= GH) return;
    back[y * GW + x] = c;
}

void pxa(int x, int y, u32 c, int a) {
    if ((unsigned)x >= GW || (unsigned)y >= GH) return;
    u32 *p = back + y * GW + x;
    *p = (a >= 256) ? c : mix(*p, c, a);
}

void grect(int x, int y, int w, int h, u32 c) {
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++) px(i, j, c);
}

void garect(int x, int y, int w, int h, u32 c, int a) {
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++) pxa(i, j, c, a);
}

/* rounded rect with soft corners */
void garrect(int x, int y, int w, int h, int r, u32 c, int a) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            int dx = (i < r) ? r - 1 - i : (i >= w - r) ? i - (w - r) : -1;
            int dy = (j < r) ? r - 1 - j : (j >= h - r) ? j - (h - r) : -1;
            if (dx >= 0 && dy >= 0) {
                int d2 = dx * dx + dy * dy;
                if (d2 > r * r + r) continue;
                if (d2 > r * r - r) { pxa(x + i, y + j, c, a / 2); continue; }
            }
            pxa(x + i, y + j, c, a);
        }
}

void grrect(int x, int y, int w, int h, int r, u32 c) { garrect(x, y, w, h, r, c, 256); }

void gframe(int x, int y, int w, int h, u32 c) {
    for (int i = x; i < x + w; i++) { px(i, y, c); px(i, y + h - 1, c); }
    for (int j = y; j < y + h; j++) { px(x, j, c); px(x + w - 1, j, c); }
}

void gdisca(int cx, int cy, int r, u32 c, int a) {
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++) {
            int d2 = dx * dx + dy * dy;
            if (d2 > r * r + r) continue;
            int aa = (d2 > r * r - r) ? a / 2 : a;
            pxa(cx + dx, cy + dy, c, aa);
        }
}

void gdisc(int cx, int cy, int r, u32 c) { gdisca(cx, cy, r, c, 256); }

/* filled triangle (for vector icons) */
void gtri(int x0, int y0, int x1, int y1, int x2, int y2, u32 c) {
    int minx = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int maxx = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int miny = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int maxy = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
    int d = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    if (d == 0) return;
    int s = d > 0 ? 1 : -1;
    for (int y = miny; y <= maxy; y++)
        for (int x = minx; x <= maxx; x++) {
            int w0 = ((x1 - x0) * (y - y0) - (y1 - y0) * (x - x0)) * s;
            int w1 = ((x2 - x1) * (y - y1) - (y2 - y1) * (x - x1)) * s;
            int w2 = ((x0 - x2) * (y - y2) - (y0 - y2) * (x - x2)) * s;
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) px(x, y, c);
        }
}

/* layered soft drop shadow */
void gshadow(int x, int y, int w, int h, int r) {
    garrect(x - 4, y + 2, w + 8, h + 8, r + 6, 0x4A3870, 26);
    garrect(x - 2, y + 3, w + 4, h + 5, r + 3, 0x4A3870, 36);
    garrect(x,     y + 4, w,     h + 2, r,     0x4A3870, 48);
}

/* ---- text: embedded anti-aliased Helvetica alpha bitmaps ---- */
#include "fontdata.h"

/* draw one alpha bitmap at 1x */
static int blit_a(int x, int y, const u8 *bm, int w, int h, u32 c) {
    for (int r = 0; r < h; r++)
        for (int cl = 0; cl < w; cl++) {
            int a = bm[r * w + cl];
            if (a) pxa(x + cl, y + r, c, a == 255 ? 256 : a);
        }
    return w;
}

/* draw one alpha bitmap at 2x with bilinear smoothing */
static int blit_a2(int x, int y, const u8 *bm, int w, int h, u32 c) {
    for (int v = 0; v < h * 2; v++)
        for (int u = 0; u < w * 2; u++) {
            int x0 = u >> 1, y0 = v >> 1;
            int fx = u & 1, fy = v & 1;
            int x1 = (x0 + 1 < w) ? x0 + 1 : x0;
            int y1 = (y0 + 1 < h) ? y0 + 1 : y0;
            int A = bm[y0 * w + x0], B = bm[y0 * w + x1];
            int C = bm[y1 * w + x0], D = bm[y1 * w + x1];
            int a = (A * (2 - fx) * (2 - fy) + B * fx * (2 - fy) +
                     C * (2 - fx) * fy + D * fx * fy) >> 2;
            if (a) pxa(x + u, y + v, c, a >= 255 ? 256 : a);
        }
    return w * 2;
}

/* scale: 2 = 16px regular, 3 = 16px bold, 4 = 32px bold, 8 = 80px clock */
void gtext(int x, int y, const char *s, u32 c, int scale) {
    for (; *s; s++) {
        int ch = (unsigned char)*s;
        if (scale >= 8) {
            int i = (ch == ':') ? 10 : ch - '0';
            if (i < 0 || i > 10) { x += 20; continue; }
            x += blit_a2(x, y, f40_d + f40_g[i].off, f40_g[i].w, f40_h, c);
            continue;
        }
        if (ch < 32 || ch > 126) { x += 4 * (scale >= 4 ? 2 : 1); continue; }
        const Gl *g = (scale >= 3) ? &f16b_g[ch - 32] : &f16_g[ch - 32];
        const u8 *d = (scale >= 3) ? f16b_d : f16_d;
        if (scale >= 4) x += blit_a2(x, y, d + g->off, g->w, f16_h, c);
        else            x += blit_a(x, y, d + g->off, g->w, f16_h, c);
    }
}

int gtextw(const char *s, int scale) {
    int w = 0;
    for (; *s; s++) {
        int ch = (unsigned char)*s;
        if (scale >= 8) {
            int i = (ch == ':') ? 10 : ch - '0';
            w += (i < 0 || i > 10) ? 20 : f40_g[i].w * 2;
            continue;
        }
        if (ch < 32 || ch > 126) { w += 4 * (scale >= 4 ? 2 : 1); continue; }
        int gw = (scale >= 3) ? f16b_g[ch - 32].w : f16_g[ch - 32].w;
        w += (scale >= 4) ? gw * 2 : gw;
    }
    return w;
}

/* ---- wallpaper: gradient + soft radial blobs, cached ---- */
static void wblob(int cx, int cy, int r, u32 c, int amax) {
    int r2 = r * r;
    int x0 = cx - r, x1 = cx + r, y0 = cy - r, y1 = cy + r;
    if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0;
    if (x1 > GW - 1) x1 = GW - 1; if (y1 > GH - 1) y1 = GH - 1;
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            int dx = x - cx, dy = y - cy;
            int d2 = dx * dx + dy * dy;
            if (d2 >= r2) continue;
            int a = amax * (r2 - d2) / r2;
            a = a * (r2 - d2) / r2;           /* quadratic falloff */
            u32 *p = wall + y * GW + x;
            *p = mix(*p, c, a);
        }
}

static u32 lerp(u32 a, u32 b, int t, int n) {   /* t/n */
    int r = ((a >> 16 & 255) * (n - t) + (b >> 16 & 255) * t) / n;
    int g = ((a >> 8 & 255) * (n - t) + (b >> 8 & 255) * t) / n;
    int bl = ((a & 255) * (n - t) + (b & 255) * t) / n;
    return (r << 16) | (g << 8) | bl;
}

void gwallpaper(int dark) {
    u32 top    = dark ? 0x3A1B66 : 0xE9E0F8;
    u32 bottom = dark ? 0x1A0B30 : 0xCDA9EA;
    for (int y = 0; y < GH; y++) {
        u32 c = lerp(top, bottom, y, GH - 1);
        u32 *row = wall + y * GW;
        for (int x = 0; x < GW; x++) row[x] = c;
    }
    if (dark) {
        wblob(400, 330, 300, 0x8A4CC8, 220);
        wblob(250, 190, 170, 0xA766E0, 170);
        wblob(560, 440, 210, 0x6B34A8, 190);
        wblob(420, 320, 130, 0xB47AE8, 130);
    } else {
        wblob(200, 430, 260, 0xC79CE8, 180);
        wblob(640, 130, 210, 0xF2E8FD, 210);
        wblob(540, 520, 230, 0xDBA6E2, 160);
        wblob(120, 120, 150, 0xF0E4FA, 150);
    }
}

void gbg() {
    for (int i = 0; i < GW * GH; i++) back[i] = wall[i];
}
