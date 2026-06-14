#!/usr/bin/env python3
import gi, os, pathlib
gi.require_version("Gtk", "4.0"); gi.require_version("Adw", "1")
from gi.repository import Gtk, Adw, Gdk, GLib

CSS = b"""
window { background: linear-gradient(135deg, #E9E0F8, #CDA9EA); }
.aros-title { font-size: 34px; font-weight: 800; color: #2A2433; }
.aros-sub { font-size: 15px; color: #6F6685; }
.logo { background:#8B5CF6; border-radius:28px; min-width:84px; min-height:84px; color:#fff; font-size:40px; font-weight:800; }
.card { background: rgba(255,255,255,0.85); border-radius: 22px; padding: 26px; }
button.suggested-action { background:#8B5CF6; color:#fff; border-radius:14px; padding:10px 28px; font-weight:700; }
button.flat { color:#6F6685; border-radius:14px; padding:10px 22px; }
entry { border-radius:12px; min-height:42px; }
.dot { min-width:9px; min-height:9px; border-radius:50%; background:#C7B6E6; }
.dot.on { background:#8B5CF6; }
"""

NAME = "Abdi"

class OOBE(Adw.Application):
    def __init__(self):
        super().__init__(application_id="org.aros.OOBE")
    def do_activate(self):
        disp = Gdk.Display.get_default()
        prov = Gtk.CssProvider(); prov.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_display(disp, prov, 600)
        self.win = Adw.ApplicationWindow(application=self)
        self.win.set_default_size(1000, 700); self.win.fullscreen()
        self.stack = Gtk.Stack(transition_type=Gtk.StackTransitionType.SLIDE_LEFT,
                               transition_duration=350, hexpand=True, vexpand=True)
        self.pages = [self._welcome(), self._account(), self._settings(), self._done()]
        for i, p in enumerate(self.pages): self.stack.add_named(p, f"p{i}")
        self.idx = 0
        # dots + nav
        self.dots = Gtk.Box(halign=Gtk.Align.CENTER, spacing=8)
        self.dotw = []
        for i in range(len(self.pages)):
            d = Gtk.Box(); d.add_css_class("dot"); 
            if i==0: d.add_css_class("on")
            self.dotw.append(d); self.dots.append(d)
        self.back = Gtk.Button(label="Back"); self.back.add_css_class("flat"); self.back.connect("clicked", self.go_back); self.back.set_sensitive(False)
        self.next = Gtk.Button(label="Get started"); self.next.add_css_class("suggested-action"); self.next.connect("clicked", self.go_next)
        nav = Gtk.Box(spacing=12, margin_top=18, margin_bottom=28, margin_start=40, margin_end=40)
        nav.append(self.back); 
        sp = Gtk.Box(hexpand=True); nav.append(sp); nav.append(self.dots)
        sp2 = Gtk.Box(hexpand=True); nav.append(sp2); nav.append(self.next)
        root = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        root.append(self.stack); root.append(nav)
        self.win.set_content(root); self.win.present()
    def _page(self, logo, title, sub, body=None):
        b = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, halign=Gtk.Align.CENTER, valign=Gtk.Align.CENTER, spacing=14, vexpand=True)
        l = Gtk.Label(label=logo); l.add_css_class("logo"); b.append(l)
        t = Gtk.Label(label=title); t.add_css_class("aros-title"); b.append(t)
        s = Gtk.Label(label=sub); s.add_css_class("aros-sub"); s.set_wrap(True); b.append(s)
        if body: b.append(body)
        return b
    def _welcome(self):
        return self._page("⚡", "Welcome to arOS 3", "the alternative operating system, built from scratch")
    def _account(self):
        card = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10); card.add_css_class("card")
        self.entry = Gtk.Entry(text=NAME, placeholder_text="Your name"); card.append(self.entry)
        return self._page("👤", "Create your account", "what should arOS call you?", card)
    def _settings(self):
        card = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=14); card.add_css_class("card")
        for lbl in ["Dark wallpaper", "System sounds", "Animations"]:
            row = Gtk.Box(spacing=16); row.append(Gtk.Label(label=lbl, hexpand=True, halign=Gtk.Align.START))
            sw = Gtk.Switch(active=(lbl!="Dark wallpaper")); row.append(sw); card.append(row)
        return self._page("⚙", "Make it yours", "tune arOS to your taste", card)
    def _done(self):
        return self._page("✓", "You're all set", "enjoy arOS 3")
    def _refresh(self):
        self.stack.set_visible_child_name(f"p{self.idx}")
        for i,d in enumerate(self.dotw):
            (d.add_css_class if i==self.idx else d.remove_css_class)("on")
        self.back.set_sensitive(self.idx>0)
        last = self.idx==len(self.pages)-1
        self.next.set_label("Finish" if last else ("Continue" if self.idx>0 else "Get started"))
    def go_next(self, _):
        if self.idx>=len(self.pages)-1:
            os.makedirs(os.path.expanduser("~/.config/aros"), exist_ok=True)
            pathlib.Path(os.path.expanduser("~/.config/aros/.oobe-done")).touch()
            self.quit(); return
        self.idx+=1; self._refresh()
    def go_back(self, _):
        if self.idx>0: self.idx-=1; self._refresh()

OOBE().run(None)
