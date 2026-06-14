#!/usr/bin/env python3
import gi, subprocess, os, datetime
gi.require_version("Gtk","4.0"); gi.require_version("Adw","1")
from gi.repository import Gtk, Adw, Gdk, GLib

ICO=os.path.expanduser("~/.config/aros/icons")
APPS=[
 ("Files","files","nautilus"),
 ("Terminal","terminal","kitty"),
 ("Photos","photos","loupe || nautilus"),
 ("Music","music","gnome-text-editor"),
 ("Notes","notes","gnome-text-editor"),
 ("Settings","settings","nm-connection-editor"),
 ("App Store","store","gnome-software || flatpak run flathub"),
 ("Compatibility","compat","env GSK_RENDERER=cairo python3 ~/.config/aros/aros-compat.py"),
 ("About","about","kitty -e bash -c 'fastfetch||uname -a;read'"),
]
CSS=b"""
window{background:transparent;}
.bg{background:linear-gradient(160deg, rgba(58,27,102,0.92), rgba(46,20,82,0.92));}
.greet{font-size:40px;font-weight:800;color:#ffffff;}
.search{background:rgba(255,255,255,0.92);border-radius:20px;min-height:46px;font-size:16px;padding:0 18px;color:#2A2433;}
.tile{background:transparent;border-radius:22px;padding:16px;}
.tile:hover{background:rgba(255,255,255,0.12);}
.lbl{color:#F2ECFC;font-size:15px;font-weight:600;margin-top:8px;}
"""
class LP(Adw.Application):
    def __init__(self): super().__init__(application_id="org.aros.Launchpad")
    def do_activate(self):
        p=Gtk.CssProvider(); p.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(),p,600)
        self.win=Adw.ApplicationWindow(application=self); self.win.fullscreen()
        bg=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=20); bg.add_css_class("bg")
        bg.set_margin_top(60); bg.set_margin_bottom(60); bg.set_margin_start(120); bg.set_margin_end(120)
        h=datetime.datetime.now().hour
        g="Good Morning" if h<12 else "Good Afternoon" if h<18 else "Good Evening"
        lab=Gtk.Label(label=f"{g}, Abdi!",halign=Gtk.Align.CENTER); lab.add_css_class("greet"); bg.append(lab)
        se=Gtk.SearchEntry(placeholder_text="Search apps…",halign=Gtk.Align.CENTER); se.add_css_class("search"); se.set_size_request(420,-1)
        se.connect("search-changed",self.filt); bg.append(se)
        self.flow=Gtk.FlowBox(max_children_per_line=4,min_children_per_line=4,selection_mode=Gtk.SelectionMode.NONE,
                              halign=Gtk.Align.CENTER,valign=Gtk.Align.START,column_spacing=24,row_spacing=20,homogeneous=True)
        self.tiles=[]
        for name,ic,cmd in APPS:
            t=Gtk.Button(); t.add_css_class("tile")
            v=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,halign=Gtk.Align.CENTER)
            pic=Gtk.Picture.new_for_filename(os.path.join(ICO,ic+".png")); pic.set_size_request(84,84)
            l=Gtk.Label(label=name); l.add_css_class("lbl")
            v.append(pic); v.append(l); t.set_child(v)
            t.connect("clicked",self.launch,cmd)
            self.flow.append(t); self.tiles.append((name.lower(),t))
        sc=Gtk.ScrolledWindow(vexpand=True); sc.set_child(self.flow); bg.append(sc)
        self.win.set_content(bg)
        key=Gtk.EventControllerKey(); key.connect("key-pressed",self.onkey); self.win.add_controller(key)
        self.win.present()
    def filt(self,e):
        q=e.get_text().lower()
        for nm,t in self.tiles: t.set_visible(q in nm)
    def onkey(self,c,kv,kc,st):
        if kv==Gdk.KEY_Escape: self.quit()
    def launch(self,_,cmd):
        subprocess.Popen(["sh","-c",os.path.expanduser(cmd)]); self.quit()
LP().run(None)
