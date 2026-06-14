#!/usr/bin/env python3
import gi, os, pathlib
gi.require_version("Gtk","4.0"); gi.require_version("Adw","1")
from gi.repository import Gtk, Adw, Gdk, GLib

CSS=b"""
window{background:linear-gradient(135deg,#E9E0F8,#CDA9EA);}
.title{font-size:36px;font-weight:800;color:#2A2433;}
.sub{font-size:15px;color:#6F6685;}
.logo{background:#8B5CF6;border-radius:30px;min-width:92px;min-height:92px;color:#fff;font-size:44px;font-weight:800;}
.card{background:rgba(255,255,255,0.88);border-radius:22px;padding:24px;}
.row{font-size:15px;color:#2A2433;}
button.suggested-action{background:#8B5CF6;color:#fff;border-radius:14px;padding:11px 30px;font-weight:700;}
button.flat{color:#6F6685;border-radius:14px;padding:11px 22px;}
entry,dropdown{border-radius:12px;min-height:42px;}
.dot{min-width:9px;min-height:9px;border-radius:50%;background:#C7B6E6;}
.dot.on{background:#8B5CF6;}
progressbar trough{min-height:10px;border-radius:8px;background:#E0D6F2;}
progressbar progress{min-height:10px;border-radius:8px;background:#8B5CF6;}
"""
class OOBE(Adw.Application):
    def __init__(self): super().__init__(application_id="org.aros.OOBE")
    def do_activate(self):
        pr=Gtk.CssProvider(); pr.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(),pr,600)
        self.win=Adw.ApplicationWindow(application=self); self.win.set_default_size(1000,720); self.win.fullscreen()
        self.stack=Gtk.Stack(transition_type=Gtk.StackTransitionType.SLIDE_LEFT,transition_duration=320,vexpand=True,hexpand=True)
        self.pages=[self.p_welcome(),self.p_lang(),self.p_account(),self.p_settings(),self.p_install(),self.p_done()]
        for i,p in enumerate(self.pages): self.stack.add_named(p,f"p{i}")
        self.idx=0
        self.dots=Gtk.Box(halign=Gtk.Align.CENTER,spacing=8); self.dw=[]
        for i in range(len(self.pages)):
            d=Gtk.Box(); d.add_css_class("dot")
            if i==0: d.add_css_class("on")
            self.dw.append(d); self.dots.append(d)
        self.back=Gtk.Button(label="Back"); self.back.add_css_class("flat"); self.back.connect("clicked",self.go_back); self.back.set_sensitive(False)
        self.next=Gtk.Button(label="Get started"); self.next.add_css_class("suggested-action"); self.next.connect("clicked",self.go_next)
        nav=Gtk.Box(spacing=12,margin_top=16,margin_bottom=30,margin_start=44,margin_end=44)
        nav.append(self.back); nav.append(Gtk.Box(hexpand=True)); nav.append(self.dots); nav.append(Gtk.Box(hexpand=True)); nav.append(self.next)
        root=Gtk.Box(orientation=Gtk.Orientation.VERTICAL); root.append(self.stack); root.append(nav)
        self.win.set_content(root); self.win.present()
    def base(self,logo,title,sub,body=None):
        b=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,halign=Gtk.Align.CENTER,valign=Gtk.Align.CENTER,spacing=14,vexpand=True)
        l=Gtk.Label(label=logo); l.add_css_class("logo"); b.append(l)
        t=Gtk.Label(label=title); t.add_css_class("title"); b.append(t)
        s=Gtk.Label(label=sub); s.add_css_class("sub"); s.set_wrap(True); b.append(s)
        if body: b.append(body)
        return b
    def p_welcome(self): return self.base("⚡","Welcome to arOS 3","the alternative operating system, built from scratch")
    def p_lang(self):
        c=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=8); c.add_css_class("card")
        self.lang=Gtk.DropDown.new_from_strings(["English (US)","English (UK)","Español","Français","Deutsch","日本語"]); c.append(self.lang)
        return self.base("\U0001F310","Language & Region","choose your language",c)
    def p_account(self):
        c=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=8); c.add_css_class("card")
        self.entry=Gtk.Entry(text="Abdi",placeholder_text="Your name"); c.append(self.entry)
        return self.base("\U0001F464","Create your account","what should arOS call you?",c)
    def p_settings(self):
        c=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=16); c.add_css_class("card")
        for lbl,on in [("Send usage diagnostics",False),("Location services",True),("Automatic updates",True),("Dark wallpaper",False)]:
            r=Gtk.Box(spacing=18); r.append(Gtk.Label(label=lbl,hexpand=True,halign=Gtk.Align.START,css_classes=["row"]))
            r.append(Gtk.Switch(active=on,valign=Gtk.Align.CENTER)); c.append(r)
        return self.base("⚙","Express settings","you can change these any time",c)
    def p_install(self):
        c=Gtk.Box(orientation=Gtk.Orientation.VERTICAL,spacing=12,halign=Gtk.Align.CENTER); 
        self.pbar=Gtk.ProgressBar(); self.pbar.set_size_request(420,-1)
        self.pstat=Gtk.Label(label="Preparing…",css_classes=["sub"])
        c.append(self.pbar); c.append(self.pstat)
        return self.base("✨","Installing arOS 3","sit back, this won't take long",c)
    def p_done(self): return self.base("✓","You're all set","welcome to arOS, enjoy")
    def refresh(self):
        self.stack.set_visible_child_name(f"p{self.idx}")
        for i,d in enumerate(self.dw): (d.add_css_class if i==self.idx else d.remove_css_class)("on")
        self.back.set_sensitive(0<self.idx<4)
        last=self.idx==len(self.pages)-1
        if self.idx==4:  # install page: auto-run, hide nav
            self.next.set_visible(False); self.back.set_visible(False)
            self.prog=0.0; GLib.timeout_add(60,self.tick)
        else:
            self.next.set_visible(True); self.back.set_visible(self.idx>0)
            self.next.set_label("Finish" if last else "Continue")
    def tick(self):
        self.prog+=0.02; self.pbar.set_fraction(min(self.prog,1.0))
        self.pstat.set_label(["Copying files…","Configuring…","Almost there…"][min(int(self.prog*3),2)])
        if self.prog>=1.0:
            self.idx=5; self.refresh(); return False
        return True
    def go_next(self,_):
        if self.idx>=len(self.pages)-1:
            os.makedirs(os.path.expanduser("~/.config/aros"),exist_ok=True)
            pathlib.Path(os.path.expanduser("~/.config/aros/.oobe-done")).touch(); self.quit(); return
        self.idx+=1; self.refresh()
    def go_back(self,_):
        if 0<self.idx<4: self.idx-=1; self.refresh()
OOBE().run(None)
