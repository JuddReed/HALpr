#include <stdio.h>
#include <string.h>
#include <unistd.h>


const static int port = 502;
const static int MaxPacketSize = 1460 - 6;

class ModbusTcpPacket {
private:
  // MBAP: The Modbus Application Header
  short transaction;
  short protocol;
  short length;
  unsigned char unit;
  //
  unsigned char function;
  union {
    unsigned char bytes[(MaxPacketSize-2)];
    short         words[(MaxPacketSize-2)/2];
    int           longs[(MaxPacketSize-2)/4];
    float         reals[(MaxPacketSize-2)/4];
  } data;

public:
  ModbusTcpPacket() {
    protocol = 0x0000;
    transaction = 00;
    wipe();
  }

  void wipe() { length = 2; }

  int getSize() { return length;}

  void setFunction(int fun) {
    function = (unsigned char)fun;
  }

  void setTarget(int slave) {
    unit = (unsigned char)slave;
  }

  bool addBytes(int count, unsigned char* array) {
    int step = 1;
    if(length+count*step >= MaxPacketSize)return false;
    for(int i=0; i<count; ++i) {
      int a = (length - 2) / step;
      data.bytes[a] = array[i];
      length += step;
    }
  }

  bool addWords(int count, short* array) {
    int step = 2;
    if(length+count*step >= MaxPacketSize)return false;
    for(int i=0; i<count; ++i) {
      int a = (length - 2) / step;
      data.words[a] = array[i];
      length += step;
    }
  }

  bool addLongs(int count, int* array) {
    int step = 4;
    if(length+count*step >= MaxPacketSize)return false;
    for(int i=0; i<count; ++i) {
      int a = (length - 2) / step;
      data.longs[a] = array[i];
      length += step;
    }
  }

  bool addReals(int count, float* array) {
    int step = 4;
    if(length+count*step >= MaxPacketSize)return false;
    for(int i=0; i<count; ++i) {
      int a = (length - 2) / step;
      data.reals[a] = array[i];
      length += step;
    }
  }

};

bool alreadyRunning(char* myName) {
  char command[512];
  if(strchr(myName,'/'))myName = strrchr(myName,'/')+1;
  sprintf(command,"ps -A | grep %s | wc -l",myName);
  fprintf(stderr,"%s\n",command);
  FILE* list = popen(command, "r");
  int clones;
  int got = fscanf(list,"%d",&clones);
  fprintf(stderr,"%d\n",clones);
  return clones>1;
}

int main(int argc, char** argv) {
 if(alreadyRunning(argv[0]))return 0;
 sleep(60);
 return 0;
}

