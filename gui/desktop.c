#include "desktop.h"
#include "../drivers/gfx.h"
#include "../drivers/ports.h"

#define APP_NONE   0
#define APP_ABOUT  1
#define APP_FILES  2
#define APP_TERM   3
#define APP_SETT   4
#define APP_NOTES  5
#define APP_PHOTOS 6
#define APP_MUSIC  7

/* phase: 0 = boot splash, 1 = lock screen, 2 = desktop */
static int phase = 0;
static unsigned int bootsecs = 0;
static int ready = 0;
static volatile int dirty = 1;
static int app = APP_NONE;
static int wx = 80, wy = 64;
static const int ww = 640, wh = 430;
static int launcher = 0;
static int notif = 0;
static int notif_count = 3;
static int mx = GW / 2, my = GH / 2, mbtn = 0;
static int drag = 0, drag_ox = 0, drag_oy = 0;
static int hover_dock = -1, hover_launch = -1;
/* clock starts at 20:26 like the concept video */
static unsigned int secs = 20u * 3600 + 26u * 60;

static int s_dark = 0, s_sound = 1, s_anim = 1;

static char tline[30]; static int tlen = 0;
static char tout[14][36]; static int tcount = 0;

/* ---- icon kinds ---- */
#define IC_LOGO   0
#define IC_FILES  1
#define IC_TERM   2
#define IC_PHOTOS 3
#define IC_MUSIC  4
#define IC_NOTES  5
#define IC_SETT   6
#define IC_ABOUT  7
#define IC_POWER  8
static const u32 ic_col[9] = {C_ACCENT, C_YELLOW, 0x2A2433, C_PURPLE, C_RED,
                              C_GREEN, C_BLUE, C_ORANGE, 0x9A93AC};

/* ---- dock: 8 round icons like the video ---- */
#define NDOCK 8
#define DOCK_W 388
#define DOCK_H 52
#define DOCK_X ((GW - DOCK_W) / 2)
#define DOCK_Y 532
#define DCX(i) (DOCK_X + 30 + (i) * 47)
#define DCY    (DOCK_Y + DOCK_H / 2)
static const char *dock_name[NDOCK] = {"Launcher", "Files", "Terminal", "Photos",
                                       "Music", "Notes", "Settings", "About"};
static const int dock_ic[NDOCK]  = {IC_LOGO, IC_FILES, IC_TERM, IC_PHOTOS,
                                    IC_MUSIC, IC_NOTES, IC_SETT, IC_ABOUT};
static const int dock_app[NDOCK] = {-1, APP_FILES, APP_TERM, APP_PHOTOS,
                                    APP_MUSIC, APP_NOTES, APP_SETT, APP_ABOUT};

/* ---- fullscreen launcher grid ---- */
#define NLAUNCH 8
static const char *l_lbl[NLAUNCH] = {"Files", "Terminal", "Photos", "Music",
                                     "Notes", "Settings", "About", "Reboot"};
static const int   l_ic[NLAUNCH]  = {IC_FILES, IC_TERM, IC_PHOTOS, IC_MUSIC,
                                     IC_NOTES, IC_SETT, IC_ABOUT, IC_POWER};
static const int   l_app[NLAUNCH] = {APP_FILES, APP_TERM, APP_PHOTOS, APP_MUSIC,
                                     APP_NOTES, APP_SETT, APP_ABOUT, -2};
#define LGX 100
#define LGY 210
#define LICX(i) (LGX + ((i) % 4) * 150 + 75)
#define LICY(i) (LGY + ((i) / 4) * 135 + 42)

static const char *titles[8] = {"", "About arOS", "Files", "Terminal",
                                "Settings", "Notes", "Photos", "Music"};

/* ---- helpers ---- */
static int seq(const char *a, const char *b) {
    int i = 0;
    for (; a[i] && b[i]; i++) if (a[i] != b[i]) return 0;
    return a[i] == b[i];
}
static void u2(char *b, int i, unsigned v) { b[i] = '0' + v / 10; b[i + 1] = '0' + v % 10; }

static void reboot_now() {
    for (int i = 0; i < 100000; i++) if (!(port_byte_in(0x64) & 2)) break;
    port_byte_out(0x64, 0xFE);
}

static u32 lerpc(u32 a, u32 b, int t, int n) {
    int r = ((a >> 16 & 255) * (n - t) + (b >> 16 & 255) * t) / n;
    int g = ((a >> 8 & 255) * (n - t) + (b >> 8 & 255) * t) / n;
    int bl = ((a & 255) * (n - t) + (b & 255) * t) / n;
    return (r << 16) | (g << 8) | bl;
}

/* ---- vector icons: colored disc + white glyph ---- */
static void note_glyph(int cx, int cy, int r, u32 c) {
    gdisc(cx - r / 3, cy + r / 3, r / 5 + 1, c);
    gdisc(cx + r / 4, cy + r / 5, r / 5 + 1, c);
    grect(cx - r / 3 + r / 5 - 1, cy - r / 3, 2, r * 2 / 3, c);
    grect(cx + r / 4 + r / 5 - 1, cy - r / 2, 2, r * 2 / 3, c);
    gtri(cx - r / 3 + r / 5 - 1, cy - r / 3 - 2, cx + r / 4 + r / 5 + 1, cy - r / 2 - 2,
         cx + r / 4 + r / 5 + 1, cy - r / 2 + 4, c);
    gtri(cx - r / 3 + r / 5 - 1, cy - r / 3 - 2, cx - r / 3 + r / 5 - 1, cy - r / 3 + 4,
         cx + r / 4 + r / 5 + 1, cy - r / 2 + 4, c);
}

static void icon(int k, int cx, int cy, int r) {
    u32 base = ic_col[k];
    gdisc(cx, cy, r, base);
    switch (k) {
    case IC_LOGO: {
        gtri(cx + r * 2 / 5, cy - r * 3 / 5, cx - r * 2 / 5, cy + r / 8,
             cx + r / 12, cy + r / 8, C_WHITE);
        gtri(cx - r * 2 / 5, cy + r * 3 / 5, cx + r * 2 / 5, cy - r / 8,
             cx - r / 12, cy - r / 8, C_WHITE);
        break; }
    case IC_FILES:
        grrect(cx - r / 2, cy - r * 2 / 5, r * 2 / 5, r / 4, 2, C_WHITE);
        grrect(cx - r / 2, cy - r / 5, r, r * 3 / 5, r / 6 + 1, C_WHITE);
        break;
    case IC_TERM:
        gtri(cx - r / 2, cy - r * 2 / 5, cx - r / 2, cy + r * 2 / 5,
             cx, cy, C_GREEN);
        grect(cx + r / 8, cy + r / 4, r / 2, 2, C_WHITE);
        break;
    case IC_PHOTOS:
        grrect(cx - r / 2, cy - r / 2, r, r, r / 5, C_WHITE);
        gdisc(cx + r / 5, cy - r / 5, r / 8 + 1, base);
        gtri(cx - r / 2 + 2, cy + r / 2 - 2, cx - r / 8, cy - r / 8,
             cx + r / 3, cy + r / 2 - 2, base);
        break;
    case IC_MUSIC:
        note_glyph(cx, cy, r, C_WHITE);
        break;
    case IC_NOTES:
        grrect(cx - r * 2 / 5, cy - r / 2, r * 4 / 5, r, r / 6 + 1, C_WHITE);
        for (int i = 0; i < 3; i++)
            grect(cx - r / 4, cy - r / 4 + i * (r / 4), r / 2, 2, base);
        break;
    case IC_SETT: {
        int d = r * 3 / 5, d7 = d * 7 / 10;
        int ox[8] = {d, -d, 0, 0, d7, d7, -d7, -d7};
        int oy[8] = {0, 0, d, -d, d7, -d7, d7, -d7};
        for (int i = 0; i < 8; i++)
            gdisc(cx + ox[i], cy + oy[i], r / 5, C_WHITE);
        gdisc(cx, cy, r * 11 / 20, C_WHITE);
        gdisc(cx, cy, r / 4, base);
        break; }
    case IC_ABOUT:
        gdisc(cx, cy - r * 2 / 5, r / 8 + 1, C_WHITE);
        grect(cx - r / 10, cy - r / 6, r / 5 + 1, r * 3 / 5, C_WHITE);
        break;
    case IC_POWER:
        gdisc(cx, cy + r / 10, r / 2, C_WHITE);
        gdisc(cx, cy + r / 10, r / 3, base);
        grect(cx - r / 6, cy - r * 3 / 5, r / 3, r / 3, base);
        grect(cx - r / 12, cy - r * 3 / 5, r / 6 + 1, r / 2, C_WHITE);
        break;
    }
}

/* ---- terminal ---- */
static void term_print(const char *s) {
    if (tcount == 14) {
        for (int i = 0; i < 13; i++)
            for (int j = 0; j < 36; j++) tout[i][j] = tout[i + 1][j];
        tcount = 13;
    }
    int j = 0;
    for (; s[j] && j < 35; j++) tout[tcount][j] = s[j];
    tout[tcount][j] = 0;
    tcount++;
}

static void term_exec() {
    char echo[36] = "> ";
    for (int i = 0; i < tlen && i < 33; i++) echo[2 + i] = tline[i];
    echo[2 + (tlen < 33 ? tlen : 33)] = 0;
    term_print(echo);
    if (tlen == 0) {}
    else if (seq(tline, "help"))   { term_print("help about uptime"); term_print("clear reboot lock"); }
    else if (seq(tline, "about"))  term_print("arOS 3.0");
    else if (seq(tline, "clear"))  tcount = 0;
    else if (seq(tline, "lock"))   { phase = 1; app = APP_NONE; }
    else if (seq(tline, "reboot")) reboot_now();
    else if (seq(tline, "uptime")) {
        char b[20] = "up ";
        unsigned v = secs - (20u * 3600 + 26u * 60); int p = 3;
        char d[10]; int n = 0;
        if (v == 0) d[n++] = '0';
        while (v) { d[n++] = '0' + v % 10; v /= 10; }
        while (n) b[p++] = d[--n];
        b[p++] = 's'; b[p] = 0;
        term_print(b);
    }
    else term_print("unknown cmd, try help");
    tlen = 0;
    for (int i = 0; i < 30; i++) tline[i] = 0;
}

/* ---- shared widgets ---- */
static void clock_str(char *clk) {
    u2(clk, 0, (secs / 3600) % 24);
    u2(clk, 3, (secs / 60) % 60);
}

static void battery(int x, int y, u32 c) {
    gframe(x, y, 22, 11, c);
    grect(x + 22, y + 3, 2, 5, c);
    grect(x + 3, y + 3, 14, 5, c);
}

static void signal_bars(int x, int y, u32 c) {
    grect(x, y + 7, 3, 4, c);
    grect(x + 5, y + 4, 3, 7, c);
    grect(x + 10, y, 3, 11, c);
}

static void speaker(int x, int y, u32 c) {
    grect(x, y + 3, 4, 5, c);
    gtri(x + 3, y + 5, x + 9, y - 1, x + 9, y + 11, c);
}

/* ---- boot splash ---- */
static void draw_splash() {
    grect(0, 0, GW, GH, 0x150A24);
    icon(IC_LOGO, GW / 2 - 64, GH / 2 - 14, 26);
    gtext(GW / 2 - 24, GH / 2 - 30, "arOS", C_WHITE, 4);
    for (int i = 0; i < 3; i++)
        gdisca(GW / 2 - 20 + i * 20, GH / 2 + 60, 5,
               C_WHITE, (int)(bootsecs % 3) == i ? 256 : 80);
}

/* ---- lock screen ---- */
static void draw_lock() {
    const char *d = "Friday 14 November";
    gtext((GW - gtextw(d, 2)) / 2, 168, d, C_SURF, 2);
    char clk[6] = "00:00";
    clock_str(clk);
    int cw = gtextw(clk, 8);
    gtext((GW - cw) / 2 + 3, 213, clk, 0x5B3E8E, 8);
    gtext((GW - cw) / 2, 210, clk, 0xF2ECFC, 8);
    const char *h = "click anywhere to unlock";
    gtext((GW - gtextw(h, 2)) / 2, 470, h, 0xC9BBE2, 2);
}

/* ---- top pills ---- */
#define SPILL_X 136
#define SPILL_W 470
#define TPILL_X (GW - 166)
static void draw_toppills() {
    int sx = SPILL_X, sy = 12, sh = 36;
    gshadow(sx, sy, SPILL_W, sh, 18);
    garrect(sx, sy, SPILL_W, sh, 18, C_WHITE, 225);
    icon(IC_LOGO, sx + 22, sy + 18, 11);
    gdisc(sx + 48, sy + 18, 7, C_RED);
    gtext(sx + 64, sy + 10, "What do you want to do?", C_MUTED, 2);
    gdisca(sx + SPILL_W - 52, sy + 18, 9, C_RAISE, 256);
    grect(sx + SPILL_W - 53, sy + 13, 3, 7, C_TEXT2);
    gdisc(sx + SPILL_W - 52, sy + 22, 2, C_TEXT2);
    gdisca(sx + SPILL_W - 24, sy + 18, 9, C_ACCENT, 256);
    gtri(sx + SPILL_W - 27, sy + 13, sx + SPILL_W - 27, sy + 23,
         sx + SPILL_W - 18, sy + 18, C_WHITE);
    /* status pill */
    int tw = 150, tx = TPILL_X, ty = 12;
    gshadow(tx, ty, tw, sh, 18);
    garrect(tx, ty, tw, sh, 18, C_WHITE, 225);
    char clk[6] = "00:00";
    clock_str(clk);
    gtext(tx + 14, ty + 10, clk, C_TEXT, 2);
    signal_bars(tx + 62, ty + 13, C_TEXT2);
    speaker(tx + 84, ty + 13, C_TEXT2);
    battery(tx + 104, ty + 13, C_TEXT2);
}

/* ---- notification center ---- */
static void draw_notif() {
    if (!notif) return;
    int nw = 320, nx = GW - nw - 16, ny = 58;
    int nh = notif_count ? 78 + notif_count * 86 : 140;
    gshadow(nx, ny, nw, nh, 16);
    garrect(nx, ny, nw, nh, 16, C_WHITE, 240);
    gtext(nx + 20, ny + 14, "Notifications", C_TEXT, 3);
    garrect(nx + nw - 90, ny + 12, 74, 24, 12, C_RAISE, 256);
    gtext(nx + nw - 78, ny + 16, "Clear all", C_TEXT2, 2);
    if (!notif_count) {
        gtext(nx + (nw - gtextw("No notifications", 2)) / 2, ny + 70,
              "No notifications", C_MUTED, 2);
        return;
    }
    static const int   nic[3]   = {IC_LOGO, IC_PHOTOS, IC_ABOUT};
    static const char *ntit[3]  = {"Cloudbay is running", "Screenshot saved", "Powerpoint installed"};
    static const char *nbody[3] = {"Files sync to your cloud", "Click to edit and share", "Click to launch it now"};
    for (int i = 0; i < notif_count; i++) {
        int cy = ny + 50 + i * 86;
        garrect(nx + 12, cy, nw - 24, 76, 12, C_PANEL, 256);
        icon(nic[i], nx + 36, cy + 38, 14);
        gtext(nx + 60, cy + 14, ntit[i], C_TEXT, 3);
        gtext(nx + 60, cy + 38, nbody[i], C_TEXT2, 2);
    }
}

static void draw_dock() {
    gshadow(DOCK_X, DOCK_Y, DOCK_W, DOCK_H, 26);
    garrect(DOCK_X, DOCK_Y, DOCK_W, DOCK_H, 26, C_WHITE, 230);
    for (int i = 0; i < NDOCK; i++) {
        int cx = DCX(i), cy = DCY;
        if (i == hover_dock) gdisca(cx, cy, 21, C_RAISE, 256);
        icon(dock_ic[i], cx, cy, 16);
        if (i == hover_dock) {
            int lw = gtextw(dock_name[i], 2);
            int lx = cx - lw / 2;
            garrect(lx - 12, DOCK_Y - 38, lw + 24, 30, 14, C_WHITE, 235);
            gtext(lx, DOCK_Y - 31, dock_name[i], C_TEXT, 2);
        }
        if (app != APP_NONE && dock_app[i] == app)
            gdisc(cx, DOCK_Y + DOCK_H - 5, 2, C_ACCENT);
    }
}

/* ---- fullscreen launcher ---- */
static void draw_launcher() {
    if (!launcher) return;
    garect(0, 0, GW, GH, 0x2E1452, 200);
    const char *g = "Good Evening, Abdi!";
    gtext((GW - gtextw(g, 4)) / 2, 64, g, C_WHITE, 4);
    int sw = 320, sx = (GW - sw) / 2, sy = 136;
    garrect(sx, sy, sw, 32, 16, C_WHITE, 230);
    gtext(sx + 22, sy + 8, "Search", C_MUTED, 2);
    for (int i = 0; i < NLAUNCH; i++) {
        int cx = LICX(i), cy = LICY(i);
        if (i == hover_launch) gdisca(cx, cy, 36, 0xFFFFFF, 70);
        icon(l_ic[i], cx, cy, 29);
        int lw = gtextw(l_lbl[i], 2);
        gtext(cx - lw / 2, cy + 40, l_lbl[i], C_SURF, 2);
    }
}

static void draw_toggle(int x, int y, int on, const char *label) {
    garrect(x, y, 56, 26, 13, on ? C_ACCENT : C_MUTED, 256);
    gdisc(on ? x + 42 : x + 14, y + 13, 9, C_WHITE);
    gtext(x + 72, y + 5, label, C_TEXT, 2);
}

static void draw_folder(int x, int y, const char *label) {
    grrect(x + 4, y, 22, 8, 3, 0xE8B43C);
    grrect(x, y + 5, 56, 36, 6, C_YELLOW);
    int lw = gtextw(label, 2);
    gtext(x + 28 - lw / 2, y + 48, label, C_TEXT2, 2);
}

/* window chrome */
static void draw_chrome() {
    gshadow(wx, wy, ww, wh, 14);
    garrect(wx, wy, ww, wh, 14, C_SURF, 248);
    gdisc(wx + 24, wy + 18, 6, C_ORANGE);
    gtext(wx + 42, wy + 11, titles[app], C_TEXT, 3);
    gtext(wx + ww - 88, wy + 11, "-", C_TEXT2, 2);
    gframe(wx + ww - 60, wy + 13, 12, 12, C_TEXT2);
    gtext(wx + ww - 32, wy + 11, "x", C_TEXT2, 2);
}

static void draw_window() {
    draw_chrome();
    int by = wy + 44;
    if (app == APP_ABOUT) {
        icon(IC_LOGO, wx + ww / 2, by + 70, 38);
        gtext(wx + (ww - gtextw("arOS 3", 4)) / 2, by + 130, "arOS 3", C_TEXT, 4);
        gtext(wx + (ww - gtextw("alternative operating system", 2)) / 2, by + 180,
              "alternative operating system", C_TEXT2, 2);
        gtext(wx + (ww - gtextw("i686 | vesa 800x600 | ps/2", 2)) / 2, by + 215,
              "i686 | vesa 800x600 | ps/2", C_MUTED, 2);
        gtext(wx + (ww - gtextw("built from scratch, zero deps", 2)) / 2, by + 245,
              "built from scratch, zero deps", C_MUTED, 2);
    } else if (app == APP_FILES) {
        gtext(wx + 22, by - 2, "<", C_TEXT2, 2);
        gtext(wx + 48, by - 2, ">", C_MUTED, 2);
        garrect(wx + (ww - 260) / 2, by - 6, 260, 26, 13, C_RAISE, 256);
        gtext(wx + (ww - 260) / 2 + 20, by - 1, "Search", C_MUTED, 2);
        int cy0 = by + 32;
        garrect(wx + 8, cy0, 150, wh - 100, 10, C_PANEL, 256);
        gtext(wx + 24, cy0 + 10, "Folders", C_MUTED, 3);
        static const char *side[4] = {"Home", "Docs", "Pics", "Music"};
        for (int i = 0; i < 4; i++) {
            int iy = cy0 + 38 + i * 32;
            if (i == 2) garrect(wx + 14, iy - 6, 138, 28, 12, C_RAISE, 256);
            gdisc(wx + 30, iy + 7, 5, (i == 2) ? C_ACCENT : C_MUTED);
            gtext(wx + 46, iy, side[i], (i == 2) ? C_TEXT : C_TEXT2, 2);
        }
        static const char *tags[3] = {"Urgent", "Fun", "Works"};
        static const u32 tagc[3] = {C_RED, C_YELLOW, C_BLUE};
        for (int i = 0; i < 3; i++) {
            int iy = cy0 + 190 + i * 30;
            gdisc(wx + 30, iy + 7, 5, tagc[i]);
            gtext(wx + 46, iy, tags[i], C_TEXT2, 2);
        }
        gtext(wx + 186, cy0 + 6, "Pictures", C_TEXT, 3);
        int gx = wx + 190, gy = cy0 + 40;
        static const char *dirs[3] = {"Walls", "Shots", "Trips"};
        for (int i = 0; i < 3; i++)
            draw_folder(gx + i * 150, gy, dirs[i]);
        static const u32 ph[3] = {C_BLUE, C_PURPLE, C_ORANGE};
        static const char *pn[3] = {"sea", "city", "dunes"};
        for (int i = 0; i < 3; i++) {
            grrect(gx + i * 150, gy + 105, 56, 40, 6, ph[i]);
            gdisc(gx + i * 150 + 14, gy + 117, 6, C_WHITE);
            int lw = gtextw(pn[i], 2);
            gtext(gx + i * 150 + 28 - lw / 2, gy + 152, pn[i], C_TEXT2, 2);
        }
        /* bottom tab bar like the video */
        garrect(wx + 170, wy + wh - 38, 100, 26, 13, C_RAISE, 256);
        gtext(wx + 186, wy + wh - 34, "Pictures", C_TEXT2, 2);
        garrect(wx + 276, wy + wh - 38, 26, 26, 13, C_RAISE, 256);
        gtext(wx + 284, wy + wh - 34, "+", C_TEXT2, 2);
        gtext(wx + 420, wy + wh - 34, "33 items", C_MUTED, 2);
    } else if (app == APP_TERM) {
        garrect(wx + 16, by, ww - 32, wh - 64, 10, 0x1E1828, 250);
        for (int i = 0; i < tcount; i++)
            gtext(wx + 32, by + 14 + i * 20, tout[i], C_GREEN, 2);
        int row = by + 14 + tcount * 20;
        int px0 = wx + 32 + gtextw("arOS> ", 2);
        gtext(wx + 32, row, "arOS>", C_ACCENT, 2);
        gtext(px0, row, tline, C_SURF, 2);
        grect(px0 + gtextw(tline, 2) + 2, row, 9, 16, C_GREEN);
    } else if (app == APP_SETT) {
        gtext(wx + 40, by + 6, "System Settings", C_MUTED, 3);
        draw_toggle(wx + 40, by + 44,  s_dark,  "dark wallpaper");
        draw_toggle(wx + 40, by + 100, s_sound, "system sounds");
        draw_toggle(wx + 40, by + 156, s_anim,  "animations");
        gtext(wx + 40, by + 220, "click a switch to flip it", C_MUTED, 2);
    } else if (app == APP_PHOTOS) {
        int ax = wx + 16, ay = by + 4, aw = ww - 32, ah = wh - 160;
        for (int r = 0; r < ah; r++)
            grect(ax, ay + r, aw, 1, lerpc(0x8CC6F2, 0xF6C9A2, r, ah));
        gdisc(ax + aw - 90, ay + 52, 28, 0xFFF2C8);
        gtri(ax + 30, ay + ah, ax + 210, ay + 56, ax + 390, ay + ah, 0x4A3A6E);
        gtri(ax + 188, ay + 78, ax + 210, ay + 56, ax + 232, ay + 78, 0xEDE8F4);
        gtri(ax + 250, ay + ah, ax + 430, ay + 96, ax + 610, ay + ah, 0x5E4A86);
        gtri(ax + 410, ay + 116, ax + 430, ay + 96, ax + 450, ay + 116, 0xE5DEF0);
        gtext(wx + (ww - gtextw("matterhorn.jpg - 1 of 33", 2)) / 2, ay + ah + 10,
              "matterhorn.jpg - 1 of 33", C_TEXT2, 2);
        static const u32 th[6] = {C_BLUE, C_PURPLE, C_ORANGE, C_GREEN, C_RED, C_YELLOW};
        for (int i = 0; i < 6; i++) {
            int txx = wx + 90 + i * 80;
            grrect(txx, ay + ah + 40, 56, 38, 6, th[i]);
            if (i == 0) gframe(txx - 2, ay + ah + 38, 60, 42, C_ACCENT);
        }
    } else if (app == APP_NOTES) {
        garrect(wx + 16, by, ww - 32, 38, 10, 0x2EC4BC, 256);
        gtext(wx + 36, by + 10, "note.txt", C_WHITE, 3);
        for (int i = 0; i < 3; i++)
            gdisc(wx + ww - 50 - i * 18, by + 19, 4, C_WHITE);
        static const char *lines[5] = {"Welcome to arOS Notes",
                                       "- glassmorphism UI",
                                       "- builtin AI search",
                                       "- zero dependencies",
                                       "- made from scratch"};
        for (int i = 0; i < 5; i++) {
            gtext(wx + 40, by + 60 + i * 34, lines[i], i ? C_TEXT2 : C_TEXT, i ? 2 : 3);
            grect(wx + 36, by + 84 + i * 34, ww - 96, 1, C_RAISE);
        }
    } else if (app == APP_MUSIC) {
        garrect(wx + 40, by + 16, 150, 150, 16, C_PURPLE, 256);
        gdisca(wx + 95, by + 66, 50, 0xD9A6F2, 130);
        note_glyph(wx + 115, by + 91, 40, C_WHITE);
        gtext(wx + 220, by + 36, "Midnight Drive", C_TEXT, 3);
        gtext(wx + 220, by + 62, "Sonora - arOS Music", C_TEXT2, 2);
        garrect(wx + 220, by + 100, 330, 8, 4, C_RAISE, 256);
        garrect(wx + 220, by + 100, 132, 8, 4, C_ACCENT, 256);
        gdisc(wx + 352, by + 104, 7, C_WHITE);
        gtext(wx + 220, by + 116, "1:24", C_MUTED, 2);
        gtext(wx + 516, by + 116, "3:30", C_MUTED, 2);
        int ccy = by + 200;
        gdisca(wx + 300, ccy, 20, C_RAISE, 256);
        gtri(wx + 308, ccy - 8, wx + 308, ccy + 8, wx + 294, ccy, C_TEXT2);
        grect(wx + 292, ccy - 8, 3, 16, C_TEXT2);
        gdisc(wx + 370, ccy, 26, C_ACCENT);
        gtri(wx + 363, ccy - 11, wx + 363, ccy + 11, wx + 382, ccy, C_WHITE);
        gdisca(wx + 440, ccy, 20, C_RAISE, 256);
        gtri(wx + 448, ccy - 8, wx + 448, ccy + 8, wx + 462, ccy, C_TEXT2);
        grect(wx + 461, ccy - 8, 3, 16, C_TEXT2);
    }
}

static void draw_cursor() {
    static const unsigned char fill[12] = {
        0x00,0x02,0x06,0x0E,0x1E,0x3E,0x7E,0x1E,0x36,0x24,0x00,0x00};
    static const unsigned char line[12] = {
        0x01,0x05,0x09,0x11,0x21,0x41,0x81,0x61,0x49,0x5A,0x24,0x18};
    for (int r = 0; r < 12; r++)
        for (int b = 0; b < 8; b++) {
            int X = mx + b * 2, Y = my + r * 2;
            if (fill[r] & (1 << b)) {
                px(X, Y, C_BLACK); px(X + 1, Y, C_BLACK);
                px(X, Y + 1, C_BLACK); px(X + 1, Y + 1, C_BLACK);
            }
            if (line[r] & (1 << b)) {
                px(X, Y, C_WHITE); px(X + 1, Y, C_WHITE);
                px(X, Y + 1, C_WHITE); px(X + 1, Y + 1, C_WHITE);
            }
        }
}

static int wall_dark = -1;
static void ensure_wall(int dark) {
    if (wall_dark != dark) { gwallpaper(dark); wall_dark = dark; }
}

static void render() {
    if (phase == 0) {
        draw_splash();
        gfx_present();
        return;
    }
    if (phase == 1) {
        ensure_wall(1);
        gbg();
        draw_lock();
        draw_cursor();
        gfx_present();
        return;
    }
    ensure_wall(s_dark);
    gbg();
    if (app != APP_NONE) draw_window();
    draw_launcher();
    if (!launcher) draw_toppills();
    draw_notif();
    draw_dock();
    draw_cursor();
    gfx_present();
}

/* ---- input ---- */
static void open_app(int a) {
    if (a == -2) { reboot_now(); return; }
    app = a;
    launcher = 0;
    notif = 0;
}

static void on_press() {
    if (phase == 0) return;
    if (phase == 1) { phase = 2; return; }
    if (notif) {
        int nw = 320, nx = GW - nw - 16, ny = 58;
        int nh = notif_count ? 78 + notif_count * 86 : 140;
        if (mx >= nx && mx < nx + nw && my >= ny && my < ny + nh) {
            if (mx >= nx + nw - 90 && mx < nx + nw - 16 &&
                my >= ny + 12 && my < ny + 36) notif_count = 0;
            return;
        }
        notif = 0;
        return;
    }
    /* status pill toggles notifications; search pill opens launcher */
    if (my >= 12 && my < 48 && !launcher) {
        if (mx >= TPILL_X && mx < TPILL_X + 150) { notif = 1; return; }
        if (mx >= SPILL_X && mx < SPILL_X + SPILL_W) { launcher = 1; return; }
    }
    if (my >= DOCK_Y && my < DOCK_Y + DOCK_H) {
        for (int i = 0; i < NDOCK; i++)
            if (mx >= DCX(i) - 22 && mx < DCX(i) + 22) {
                if (dock_app[i] == -1) launcher = !launcher;
                else open_app(dock_app[i]);
                return;
            }
        return;
    }
    if (launcher) {
        for (int i = 0; i < NLAUNCH; i++) {
            int dx = mx - LICX(i), dy = my - LICY(i);
            if (dx > -40 && dx < 40 && dy > -40 && dy < 56) { open_app(l_app[i]); return; }
        }
        launcher = 0;
        return;
    }
    if (app != APP_NONE && my >= wy && my < wy + 36 && mx >= wx && mx < wx + ww) {
        if (mx >= wx + ww - 44) { app = APP_NONE; return; }
        if (mx >= wx + ww - 100) return;
        drag = 1; drag_ox = mx - wx; drag_oy = my - wy;
        return;
    }
    if (app == APP_SETT) {
        int bx = wx + 40, by = wy + 44;
        if (mx >= bx && mx < bx + 340) {
            if (my >= by + 40  && my < by + 76)  { s_dark  = !s_dark;  return; }
            if (my >= by + 96  && my < by + 132) { s_sound = !s_sound; return; }
            if (my >= by + 152 && my < by + 188) { s_anim  = !s_anim;  return; }
        }
    }
}

void desktop_handle_mouse(int x, int y, int buttons) {
    int pressed  = (buttons & 1) && !(mbtn & 1);
    int released = !(buttons & 1) && (mbtn & 1);
    mx = x; my = y;
    if (pressed) on_press();
    if (released) drag = 0;
    if ((buttons & 1) && drag) {
        wx = mx - drag_ox;
        wy = my - drag_oy;
        if (wx < 0) wx = 0;
        if (wx > GW - ww) wx = GW - ww;
        if (wy < 4) wy = 4;
        if (wy > DOCK_Y - 50) wy = DOCK_Y - 50;
    }
    hover_dock = -1;
    if (phase == 2 && my >= DOCK_Y - 4 && my < DOCK_Y + DOCK_H)
        for (int i = 0; i < NDOCK; i++)
            if (mx >= DCX(i) - 22 && mx < DCX(i) + 22) hover_dock = i;
    hover_launch = -1;
    if (launcher)
        for (int i = 0; i < NLAUNCH; i++) {
            int dx = mx - LICX(i), dy = my - LICY(i);
            if (dx > -40 && dx < 40 && dy > -40 && dy < 56) hover_launch = i;
        }
    mbtn = buttons;
    dirty = 1;
}

void desktop_handle_key(char c) {
    if (phase == 0) return;
    if (phase == 1) { phase = 2; dirty = 1; return; }
    if (app != APP_TERM) return;
    if (c == '\b') { if (tlen > 0) tline[--tlen] = 0; }
    else if (c == '\n') term_exec();
    else if (tlen < 28) { tline[tlen++] = c; tline[tlen] = 0; }
    dirty = 1;
}

void desktop_tick() {
    secs++;
    if (phase == 0) {
        bootsecs++;
        if (bootsecs >= 3) phase = 1;
    }
    dirty = 1;
}

void desktop_draw() {
    ready = 1;
    render();
}

/* main loop: interrupts only flag work, rendering happens here */
void desktop_loop() {
    for (;;) {
        __asm__ volatile("hlt");
        if (ready && dirty) {
            dirty = 0;
            render();
        }
    }
}
