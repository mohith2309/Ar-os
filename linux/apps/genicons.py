#!/usr/bin/env python3
from PIL import Image, ImageDraw
import os, math
S=88; SS=4; OUT=os.path.expanduser("~/.config/aros/icons"); os.makedirs(OUT,exist_ok=True)

def grad(w,h,c1,c2):
    img=Image.new("RGB",(w,h))
    px=img.load()
    for y in range(h):
        t=y/(h-1)
        c=tuple(int(c1[i]+(c2[i]-c1[i])*t) for i in range(3))
        for x in range(w): px[x,y]=c
    return img

def rounded_mask(w,h,r):
    m=Image.new("L",(w,h),0); d=ImageDraw.Draw(m)
    d.rounded_rectangle([0,0,w-1,h-1],radius=r,fill=255)
    return m

def icon(name,c1,c2,draw_glyph):
    w=h=S*SS
    base=grad(w,h,c1,c2)
    glyph=Image.new("RGBA",(w,h),(0,0,0,0))
    dd=ImageDraw.Draw(glyph)
    draw_glyph(dd,w)
    base=base.convert("RGBA"); base.alpha_composite(glyph)
    mask=rounded_mask(w,h,int(w*0.26))
    base.putalpha(mask)
    base=base.resize((S,S),Image.LANCZOS)
    base.save(os.path.join(OUT,name+".png"))

W=(255,255,255,255)
def bolt(d,w):
    u=w/100
    d.polygon([(58*u,16*u),(34*u,54*u),(50*u,54*u),(42*u,84*u),(68*u,44*u),(50*u,44*u)],fill=W)
def folder(d,w):
    u=w/100
    d.rounded_rectangle([24*u,34*u,40*u,42*u],radius=3*u,fill=W)
    d.rounded_rectangle([22*u,40*u,78*u,74*u],radius=7*u,fill=W)
def term(d,w):
    u=w/100
    d.line([(30*u,34*u),(48*u,50*u),(30*u,66*u)],fill=W,width=int(7*u),joint="curve")
    d.rounded_rectangle([52*u,60*u,72*u,67*u],radius=3*u,fill=W)
def photos(d,w):
    u=w/100
    d.rounded_rectangle([24*u,26*u,76*u,74*u],radius=9*u,outline=W,width=int(6*u))
    d.ellipse([56*u,36*u,66*u,46*u],fill=W)
    d.polygon([(30*u,70*u),(48*u,46*u),(70*u,70*u)],fill=W)
def note(d,w):
    u=w/100
    d.ellipse([28*u,58*u,42*u,72*u],fill=W); d.ellipse([56*u,52*u,70*u,66*u],fill=W)
    d.rectangle([39*u,28*u,42*u,66*u],fill=W); d.rectangle([67*u,22*u,70*u,60*u],fill=W)
    d.polygon([(39*u,28*u),(70*u,22*u),(70*u,32*u),(39*u,38*u)],fill=W)
def lines(d,w):
    u=w/100
    for i in range(3): d.rounded_rectangle([28*u,(34+i*12)*u,72*u,(38+i*12)*u],radius=2*u,fill=W)
def gear(d,w):
    u=w/100; cx=cy=50*u
    for a in range(0,360,45):
        rad=math.radians(a); x=cx+26*u*math.cos(rad); y=cy+26*u*math.sin(rad)
        d.ellipse([x-9*u,y-9*u,x+9*u,y+9*u],fill=W)
    d.ellipse([cx-22*u,cy-22*u,cx+22*u,cy+22*u],fill=W)
    d.ellipse([cx-10*u,cy-10*u,cx+10*u,cy+10*u],fill=(0,0,0,0))
def info(d,w):
    u=w/100; cx=50*u
    d.ellipse([cx-5*u,26*u,cx+5*u,36*u],fill=W)
    d.rounded_rectangle([cx-5*u,42*u,cx+5*u,74*u],radius=4*u,fill=W)

icon("launcher",(160,124,248),(124,77,240),bolt)
icon("files",(255,214,107),(245,166,35),folder)
icon("terminal",(58,53,80),(30,27,46),term)
icon("photos",(201,139,240),(139,92,246),photos)
icon("music",(255,138,138),(255,95,87),note)
icon("notes",(107,224,138),(43,200,64),lines)
icon("settings",(111,182,240),(77,157,224),gear)
icon("about",(255,192,122),(255,159,67),info)
print("icons:",os.listdir(OUT))
