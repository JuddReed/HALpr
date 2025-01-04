#ifndef HABACTLOG_SVGGRAPH_H_
#define HABACTLOG_SVGGRAPH_H_
#include "dataBrane.h"

class SvgGraph {
private:
int width, height;
long begin, finish;	// unix date times
double min, max;
int numSamp;
DataBrane::OBSERVATION* reading;
const static int kBorderX = 50;
const static int kBorderY = 30;
public:
  enum GraphType {BARS, LINE, DOTS, TABL} graphType;
  void setSize(int w = 640, int h = 0) {
    if(w<=0)w=128;
    if(h<=0)h = w;
    width = w;
    height = h;
  }
  SvgGraph(int w = 640, int h = 0){
    setSize(w,h);
    min = max = 0;
    begin = finish = 0;
    graphType = BARS;
  }
  int load(DataBrane::OBSERVATION* data, int count);
  void head(FILE* out = stdout);
  void spew(FILE* out = stdout);
  void link(char* format, FILE* out = stdout);
  void tail(FILE* out = stdout);
};

#endif // HABACTLOG_SVGGRAPH_H_
