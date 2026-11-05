#pragma once
#include <stdint.h>
#include <vector>
#include <array>
struct FakeWire {
  std::array<uint8_t,1024> ram{};
  std::vector<uint8_t> tx;
  int cmd=0, arg=0, start=0, end=127, page=0, x=0, dataBytes=0;
  void begin() {}
  void setClock(unsigned long) {}
  void beginTransmission(uint8_t) {tx.clear();}
  void write(uint8_t v) {tx.push_back(v);}
  void endTransmission() {
    if(tx[0]==0x00) {
      int v=tx[1];
      if(cmd) {
        if(cmd==0x21) {if(arg==0)start=x=v;else end=v;}
        if(cmd==0x22 && arg==0)page=v;
        if(++arg==2){cmd=0;arg=0;}
      } else if(v==0x21 || v==0x22)cmd=v;
    } else if(tx[0]==0x40) {
      for(size_t i=1;i<tx.size();++i){ram.at(page*128+x)=tx[i];++dataBytes;if(++x>end)x=start;}
    }
  }
};
extern FakeWire Wire;
