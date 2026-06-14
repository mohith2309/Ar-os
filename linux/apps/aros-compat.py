#!/usr/bin/env python3
import gi, subprocess
gi.require_version("Gtk","4.0"); gi.require_version("Adw","1")
from gi.repository import Gtk, Adw, Gdk, GLib

CSS=b"""
window{background:linear-gradient(135deg,#E9E0F8,#CDA9EA);}
.h{font-size:26px;font-weight:800;color:#2A2433;}
.sub{font-size:14px;color:#6F6685;}
.card{background:rgba(255,255,255,0.9);border-radius:18px;padding:18px;}
.badge-ok{background:#2BC840;color:#fff;border-radius:10px;padding:3px 10px;font-weight:700;font-size:12px;}
.badge-exp{background:#FF9F43;color:#fff;border-radius:10px;padding:3px 10px;font-weight:700;font-size:12px;}
.badge-no{background:#C7B6E6;color:#4A3870;border-radius:10px;padding:3px 10px;font-weight:700;font-size:12px;}
.ic{min-width:46px;min-height:46px;border-radius:13px;color:#fff;font-size:22px;font-weight:800;}
button.go{background:#8B5CF6;color:#fff;border-radius:12px;padding:8px 18px;font-weight:700;}
"""

LAYERS=[
 ("🐧","#4D9DE0","Linux apps","Flatpak + Flathub — thousands of apps","ok","Open store","store"),
 ("🪟","#5566FF","Windows apps","Wine + box64/FEX x86 emulation (ARM: experimental)","exp","Set up","win"),
 ("","#2A2433","macOS apps","No technology runs macOS apps on Linux","no","Unavailable","none"),
 ("🤖","#2BC840","Android apps","Waydroid — needs kernel binder (not in this VM)","no","Unavailable","none"),
]

class App(Adw.Application):
    def __init__(self): super().__init__(application_id="org.aros.Compat")
    def do_activate(self):
        p=Gtk.CssProvider(); p.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(),p,600)
        w=Adw.ApplicationWindow(application=self); w.set_default_size(720,640)
        box=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=16,margin_top=30,margin_bottom=30,margin_start=34,margin_end=34)
        t=Gtk.Label(label="Compatibility Center",xalign=0); t.add_css_class("h"); box.append(t)
        s=Gtk.Label(label="Choose which apps arOS can run",xalign=0); s.add_css_class("sub"); box.append(s)
        for emo,col,name,desc,badge,btn,act in LAYERS:
            c=Gtk.Box(spacing=14); c.add_css_class("card")
            ic=Gtk.Label(label=emo); ic.add_css_class("ic")
            ic.set_css_classes(["ic"]); ic.get_style_context()  # color via inline
            icb=Gtk.Box(); icb.append(ic)
            ic_prov=Gtk.CssProvider(); ic_prov.load_from_data(f"label{{background:{col};}}".encode())
            ic.get_style_context().add_provider(ic_prov,700)
            c.append(ic)
            txt=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=3,hexpand=True,valign=Gtk.Align.CENTER)
            nm=Gtk.Label(label=name,xalign=0); nm.add_css_class("h"); nm_p=Gtk.CssProvider(); nm_p.load_from_data(b"label{font-size:17px;}"); nm.get_style_context().add_provider(nm_p,700)
            ds=Gtk.Label(label=desc,xalign=0,wrap=True); ds.add_css_class("sub")
            txt.append(nm); txt.append(ds); c.append(txt)
            bd=Gtk.Label(label={"ok":"Ready","exp":"Experimental","no":"N/A"}[badge],valign=Gtk.Align.CENTER)
            bd.add_css_class({"ok":"badge-ok","exp":"badge-exp","no":"badge-no"}[badge]); c.append(bd)
            b=Gtk.Button(label=btn,valign=Gtk.Align.CENTER); b.add_css_class("go")
            b.set_sensitive(act!="none")
            b.connect("clicked",self.on_click,act)
            c.append(b); box.append(c)
        sc=Gtk.ScrolledWindow(); sc.set_child(box); w.set_content(sc); w.present()
    def on_click(self,_,act):
        if act=="store": subprocess.Popen(["sh","-c","gnome-software || flatpak run flathub 2>/dev/null || true"])
        elif act=="win": subprocess.Popen(["sh","-c","kitty -e bash -c 'echo Installing box64 x86 layer...; sudo dnf -y install box64; echo Wine on ARM needs manual hangover build - see docs; read' || true"])

App().run(None)
