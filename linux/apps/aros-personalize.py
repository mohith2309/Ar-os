#!/usr/bin/env python3
import gi, subprocess, os
gi.require_version("Gtk","4.0"); gi.require_version("Adw","1")
from gi.repository import Gtk, Adw, Gdk
A=os.path.expanduser("~/.config/aros")
CSS=b"""
window{background:linear-gradient(135deg,#E9E0F8,#CDA9EA);}
.h{font-size:26px;font-weight:800;color:#2A2433;}
.sub{font-size:13px;color:#6F6685;}
.card{background:rgba(255,255,255,0.9);border-radius:18px;padding:18px;}
.sw{min-width:120px;min-height:74px;border-radius:14px;}
.acc{min-width:42px;min-height:42px;border-radius:50%;}
button{border-radius:12px;}
"""
ACCENTS=["#8B5CF6","#4D9DE0","#2BC840","#FF9F43","#FF5F57","#B36CEB"]
class P(Adw.Application):
    def __init__(self): super().__init__(application_id="org.aros.Personalize")
    def do_activate(self):
        pr=Gtk.CssProvider(); pr.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(),pr,600)
        w=Adw.ApplicationWindow(application=self); w.set_default_size(640,560)
        b=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=18,margin_top=28,margin_bottom=28,margin_start=30,margin_end=30)
        b.append(self._lbl("Personalize","h")); b.append(self._lbl("make arOS yours","sub"))
        # wallpaper
        wc=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=12); wc.add_css_class("card")
        wc.append(self._lbl("Wallpaper","h2"))
        row=Gtk.Box(spacing=16,halign=Gtk.Align.CENTER)
        for name,fn in [("Light","wallpaper.png"),("Dark","wallpaper-dark.png")]:
            t=Gtk.Button(); t.add_css_class("sw")
            pic=Gtk.Picture.new_for_filename(os.path.join(A,fn)); pic.set_content_fit(Gtk.ContentFit.COVER); pic.set_size_request(120,74)
            t.set_child(pic); t.connect("clicked",self.setwall,fn); row.append(t)
        wc.append(row); b.append(wc)
        # accent
        ac=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=12); ac.add_css_class("card")
        ac.append(self._lbl("Accent color","h2"))
        arow=Gtk.Box(spacing=12,halign=Gtk.Align.CENTER)
        for c in ACCENTS:
            sw=Gtk.Button(); sw.add_css_class("acc")
            cp=Gtk.CssProvider(); cp.load_from_data(f"button{{background:{c};}}".encode()); sw.get_style_context().add_provider(cp,700)
            sw.connect("clicked",self.setaccent,c); arow.append(sw)
        ac.append(arow); b.append(ac)
        w.set_content(b); w.present()
    def _lbl(self,t,c):
        l=Gtk.Label(label=t,xalign=0)
        if c=="h": l.add_css_class("h")
        elif c=="sub": l.add_css_class("sub")
        else:
            p=Gtk.CssProvider(); p.load_from_data(b"label{font-size:16px;font-weight:700;color:#2A2433;}"); l.get_style_context().add_provider(p,700)
        return l
    def setwall(self,_,fn):
        src=os.path.join(A,fn); dst=os.path.join(A,"wallpaper.png")
        if fn!="wallpaper.png":
            subprocess.run(["cp",src,dst])
        else:
            subprocess.run(["cp",os.path.join(A,"wallpaper.png"),dst])
        subprocess.run(["sh","-c","pkill swaybg; swaybg -i %s -m fill &"%dst])
    def setaccent(self,_,c):
        open(os.path.join(A,"accent"),"w").write(c)  # saved; applied on next login
P().run(None)
