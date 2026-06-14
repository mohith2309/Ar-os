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
.sw{min-width:130px;min-height:80px;border-radius:14px;}
.acc{min-width:44px;min-height:44px;border-radius:50%;}
button{border-radius:12px;} button.pick{padding:10px 24px;font-weight:700;}
"""
ACCENTS=[("#8B5CF6","purple"),("#4D9DE0","blue"),("#2BC840","green"),("#FF9F43","orange"),("#FF5F57","red"),("#E0559B","pink")]
def apply(): subprocess.Popen(["bash",os.path.join(A,"apply-theme.sh")])
class P(Adw.Application):
    def __init__(self): super().__init__(application_id="org.aros.Personalize")
    def do_activate(self):
        pr=Gtk.CssProvider(); pr.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(),pr,600)
        w=Adw.ApplicationWindow(application=self); w.set_default_size(660,600)
        b=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=18,margin_top=28,margin_bottom=28,margin_start=30,margin_end=30)
        b.append(self._t("Personalize","h")); b.append(self._t("make arOS yours","sub"))
        # appearance
        ap=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=12); ap.add_css_class("card")
        ap.append(self._t("Appearance","h2"))
        ar=Gtk.Box(spacing=12,halign=Gtk.Align.CENTER)
        for lbl,m in [("Light","light"),("Dark","dark")]:
            btn=Gtk.Button(label=lbl); btn.add_css_class("pick"); btn.connect("clicked",self.setmode,m); ar.append(btn)
        ap.append(ar); b.append(ap)
        # wallpaper preview
        wc=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=12); wc.add_css_class("card")
        wc.append(self._t("Wallpaper","h2"))
        wr=Gtk.Box(spacing=16,halign=Gtk.Align.CENTER)
        for fn,m in [("wallpaper-light.png","light"),("wallpaper-dark.png","dark")]:
            t=Gtk.Button(); t.add_css_class("sw")
            pic=Gtk.Picture.new_for_filename(os.path.join(A,fn)); pic.set_content_fit(Gtk.ContentFit.COVER); pic.set_size_request(130,80)
            t.set_child(pic); t.connect("clicked",self.setmode,m); wr.append(t)
        wc.append(wr); b.append(wc)
        # accent
        ac=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=12); ac.add_css_class("card")
        ac.append(self._t("Accent color","h2"))
        arow=Gtk.Box(spacing=12,halign=Gtk.Align.CENTER)
        for hexc,name in ACCENTS:
            sw=Gtk.Button(); sw.add_css_class("acc")
            cp=Gtk.CssProvider(); cp.load_from_data(f"button{{background:{hexc};}}".encode()); sw.get_style_context().add_provider(cp,700)
            sw.connect("clicked",self.setaccent,hexc,name); arow.append(sw)
        ac.append(arow); b.append(ac)
        w.set_content(b); w.present()
    def _t(self,s,c):
        l=Gtk.Label(label=s,xalign=0)
        if c=="h": l.add_css_class("h")
        elif c=="sub": l.add_css_class("sub")
        else:
            p=Gtk.CssProvider(); p.load_from_data(b"label{font-size:16px;font-weight:700;color:#2A2433;}"); l.get_style_context().add_provider(p,700)
        return l
    def setmode(self,_,m):
        open(os.path.join(A,"mode"),"w").write(m); apply()
    def setaccent(self,_,hexc,name):
        open(os.path.join(A,"accent"),"w").write(hexc); open(os.path.join(A,"accent-name"),"w").write(name); apply()
P().run(None)
