#!/usr/bin/env python3
import gi, subprocess, os, datetime
gi.require_version("Gtk","4.0"); gi.require_version("Adw","1")
from gi.repository import Gtk, Adw, Gdk
ICO=os.path.expanduser("~/.config/aros/icons")
APPS=[
 ("Files","files","nautilus"),("Terminal","terminal","kitty"),
 ("Photos","photos","loupe || nautilus"),("Music","music","gnome-text-editor"),
 ("Notes","notes","gnome-text-editor"),("Settings","settings","env XDG_CURRENT_DESKTOP=GNOME gnome-control-center"),
 ("App Store","store","env XDG_CURRENT_DESKTOP=GNOME gnome-software"),
 ("Personalize","personalize","env GSK_RENDERER=cairo python3 ~/.config/aros/aros-personalize.py"),
 ("Compatibility","compat","env GSK_RENDERER=cairo python3 ~/.config/aros/aros-compat.py"),
 ("About","about","kitty -e bash -c 'fastfetch||uname -a;read'"),
]
CSS=b"""
window{background:transparent;}
.bg{background:rgba(40,22,72,0.55);}
.search{background:rgba(255,255,255,0.95);border-radius:22px;min-height:50px;font-size:17px;padding:0 22px;color:#2A2433;}
.tile{background:transparent;border-radius:26px;padding:18px;}
.tile:hover{background:rgba(255,255,255,0.14);}
.lbl{color:#ffffff;font-size:15px;font-weight:500;margin-top:10px;}
.dot{min-width:8px;min-height:8px;border-radius:50%;background:rgba(255,255,255,0.35);}
.dot.on{background:#ffffff;}
"""
class LP(Adw.Application):
    def __init__(self): super().__init__(application_id="org.aros.Launchpad")
    def do_activate(self):
        p=Gtk.CssProvider(); p.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(),p,600)
        self.win=Adw.ApplicationWindow(application=self); self.win.fullscreen()
        bg=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=26); bg.add_css_class("bg")
        bg.set_margin_top(70); bg.set_margin_bottom(50); bg.set_margin_start(80); bg.set_margin_end(80)
        se=Gtk.SearchEntry(placeholder_text="Search",halign=Gtk.Align.CENTER); se.add_css_class("search"); se.set_size_request(460,-1)
        se.connect("search-changed",self.filt); bg.append(se)
        self.flow=Gtk.FlowBox(max_children_per_line=5,min_children_per_line=5,selection_mode=Gtk.SelectionMode.NONE,
                              halign=Gtk.Align.CENTER,valign=Gtk.Align.CENTER,column_spacing=30,row_spacing=26,homogeneous=True,vexpand=True)
        self.tiles=[]
        for name,ic,cmd in APPS:
            t=Gtk.Button(); t.add_css_class("tile")
            v=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,halign=Gtk.Align.CENTER)
            pic=Gtk.Picture.new_for_filename(os.path.join(ICO,ic+".png")); pic.set_size_request(96,96)
            l=Gtk.Label(label=name); l.add_css_class("lbl")
            v.append(pic); v.append(l); t.set_child(v)
            t.connect("clicked",self.launch,cmd)
            self.flow.append(t); self.tiles.append((name.lower(),t))
        bg.append(self.flow)
        dots=Gtk.Box(halign=Gtk.Align.CENTER,spacing=10)
        for i in range(2):
            d=Gtk.Box(); d.add_css_class("dot");
            if i==0: d.add_css_class("on")
            dots.append(d)
        bg.append(dots)
        self.win.set_content(bg)
        k=Gtk.EventControllerKey(); k.connect("key-pressed",self.onkey); self.win.add_controller(k)
        self.win.present()
    def filt(self,e):
        q=e.get_text().lower()
        for nm,t in self.tiles: t.set_visible(q in nm)
    def onkey(self,c,kv,kc,st):
        if kv==Gdk.KEY_Escape: self.quit()
    def launch(self,_,cmd):
        subprocess.Popen(["sh","-c",os.path.expanduser(cmd)]); self.quit()
LP().run(None)
