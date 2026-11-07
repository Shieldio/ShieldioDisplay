#include "ShieldioDisplay.h"
#include <cassert>
#include <cstdio>
FakeWire Wire;
bool lit(int x,int y){return Wire.ram[y/8*128+x] & (1<<(y%8));}
int main(){
  ShieldioDisplay text;
  text.begin();text.text(0,0,"A");text.render();
  int sent=Wire.dataBytes;text.render();assert(Wire.dataBytes==sent);
  ShieldioCanvas d;d.begin();
  d.pixel(127,63);d.pixel(-1,0);d.pixel(128,0);d.pixel(0,64);d.render();
  assert(lit(127,63));assert(!lit(0,0));
  sent=Wire.dataBytes;d.render();assert(sent==Wire.dataBytes);
  d.clear();d.render();for(auto b:Wire.ram)assert(b==0);
  d.line(-10,10,10,10);d.render();for(int x=0;x<=10;++x)assert(lit(x,10));
  d.clear();d.rect(-2,-2,5,5,false);d.render();
  assert(lit(2,0)&&lit(0,2)&&!lit(0,0));
  d.clear();d.rect(126,62,10,10,true);d.render();
  assert(lit(126,62)&&lit(127,63)&&!lit(125,62));
  d.clear();d.circle(64,32,12);d.render();
  assert(lit(52,32)&&lit(76,32)&&lit(64,20)&&lit(64,44)&&!lit(64,32));
  d.circle(64,32,12,true);d.render();assert(lit(64,32));
  d.clear();d.circle(0,0,0);d.render();assert(lit(0,0));
  d.circle(-32768,32767,143,true);d.rect(32767,32767,32767,32767,true);d.render();
  d.pixel(127,63);d.text(0,0,"A");d.render();assert(lit(127,63));
  d.pixel(127,63,false);d.render();assert(!lit(127,63));
  d.clear();d.render();sent=Wire.dataBytes;d.clear();d.render();assert(sent==Wire.dataBytes);
  puts("Geometry, clipping, text overlay and differential Wire transfers passed.");
  using Animal = ShieldioDisplay::Animal;
  using Mood = ShieldioDisplay::Mood;
  text.clear();text.render();
  auto blank = Wire.ram;
  for(int a=0;a<3;++a) for(int m=0;m<5;++m) for(int b=0;b<2;++b) {
    text.face((Animal)a,(Mood)m,b);
    auto streamed = Wire.ram;
    assert(streamed != blank);
    d.face((Animal)a,(Mood)m,b);
    assert(Wire.ram == streamed); // Canvas and low-RAM paths agree.
    text.render();assert(Wire.ram == blank); // Restore even unchanged text.
  }
  text.face(Animal::Cat,Mood::Happy);auto happy=Wire.ram;
  text.face(Animal::Cat,Mood::Sad);assert(Wire.ram!=happy);
  text.face(Animal::Dog,Mood::Happy);assert(Wire.ram!=happy);
  text.face(Animal::Cat,Mood::Happy,true);assert(Wire.ram!=happy);
  d.face(Animal::Lion,Mood::Happy);d.pixel(0,0);d.render();assert(lit(0,0));
  d.clear();d.render();assert(Wire.ram==blank);
  puts("30 animal/mood/blink frames, canvas parity and text restoration passed.");
}

