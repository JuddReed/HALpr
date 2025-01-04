#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "session.h"
#include "dataBrane.h"

const int maxCount = 1000; // maximum number of data points to graph
char **sensorName;
const int boxHigh = 200;	// height of a graph in pixels
const int boxWide = 800;	// width of a graph in pixels

const char* BLACK = (char*)"black";
const char* RED   = (char*)"red";
const char* GREEN = (char*)"green";
const char* BLUE  = (char*)"blue";

enum GraphType {BAR, LINE, DOT};
struct SIGNAL {
public:
  long timestamp;
  char yearMoDyHrMn[16]; double sample;
};


int sensorList(char* wildcard) {
  int count = 0;
  char buffer[1024];
  sprintf(buffer,"cat %s",wildcard);
  FILE* list = popen(buffer,"r");
  if(NULL == fgets(buffer,1024,list))return 0;
  strtok(buffer," ");
  strtok(0," ");
  char* first = strdup(strtok(0,":"));
//fprintf(stderr,"%d:%s\n",0,first);
  for(count=1;(char*)EOF != fgets(buffer,1024,list);count++){
    strtok(buffer," ");
    strtok(0," ");
    char* name = strtok(0,":");
    if(0 == strcmp(name,first))break;
//fprintf(stderr,"%d:%s\n",count,name);
  }
  sensorName = new char*[count];
  //sensorName[0] = first;
  int n;for(n=0;(char*)EOF != fgets(buffer,1024,list);n++){
    if(n>=count)break;
    strtok(buffer," ");
    strtok(0," ");
    char* name = strtok(0,":");
    if((n)&&(0 == strcmp(name,first)))break;
    sensorName[n] = strdup(name);
    char* word = strtok(0,"\n");
    double howHot;
    int valid = sscanf(word,"%lf", &howHot);
    if(!valid){
      n--;
      //fprintf(stderr,"'%s' is not a number\n",word);
    }
    //else fprintf(stderr,"%d:%s\n",n,sensorName[n]);
  }
  pclose(list);
  return n;
}

int getLastReadings(int maxCount, char* wildcard,
        char* sensor, SIGNAL* signal, double &minValue, double &maxValue) {
  //fprintf(stderr,"get last %d readings from %s\n", maxCount, sensor);
  int count = 0;
  char buffer[1024];
  sprintf(buffer,"cat %s|grep '%s' | tail -%d", wildcard,sensor, maxCount);
  FILE* list = popen(buffer,"r");
  long tick; char tock[16]; double howHot;
  int r; for(r=0;r<maxCount;r++) {
    char* got = fgets(buffer,1024,list);
    if((char*)EOF == got) break;
    if(NULL == got) break;
  //fprintf(stderr,"%d:  %s", r, buffer);
    int valid = sscanf(buffer,"%ld %s", &tick, tock);
  //fprintf(stderr,"%d:  (%d) ", r, valid);
    signal[r].timestamp = tick;
    strcpy(signal[r].yearMoDyHrMn, tock);
    strtok(buffer,":");
    char* word = strtok(0,"\n");
    valid = sscanf(word,"%lf", &howHot);
    if(!valid){
      //fprintf(stderr,"'%s' is not a number\n",word);
      return 0;
    } else {
      signal[r].sample = howHot;
      if((0==r)||(minValue>howHot))minValue = howHot;
      if((0==r)||(maxValue<howHot))maxValue = howHot;

      //fprintf(stderr,"%d:  %ld %s %lf\n", r, signal[r].timestamp, signal[r].yearMoDyHrMn, signal[r].sample);
    }
  }
  pclose(list);
  return r;
}

void spewHtmlTop() {
  printf(
    "Content-type: text/html\n\n"
    "<html lang=\"en-US\" xml:lang=\"en-US\" xmlns =\"http://www.w3.org/1999/xhtml\">\n"
    " <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\" />\n"
    " <body>\n"
  );
}

void spewHtmlBot() {
  printf(
    " </body>\n"
    "</html>\n"
  );
}

       
void spewGraph(int topY, int high, int wide, char *sensorName, GraphType graphType, int count,
      double min, double max, SIGNAL* signal) {
  printf("\n"
    "   <rect x=\"2\" y=\"%d\" width=\"%d\" height=\"%d\""
    " style=\"fill:white;stroke:black;stroke-width:2\" ></rect>\n",
    2+topY,wide-4, high-4);
  printf(
    "   <text text-anchor=\"left\" style=\"stroke:black;font-size:10\" x=\"%d\" y=\"%d\">"
    "    %s</text>\n"
    "   <text text-anchor=\"left\" style=\"stroke:black;font-size:10\" x=\"%d\" y=\"%d\">"
    "    %d</text>\n"
    "   <text text-anchor=\"left\" style=\"stroke:black;font-size:10\" x=\"%d\" y=\"%d\">"
    "    %d</text>\n",
    6,4+topY+10,sensorName,
    6,4+topY+20,(int)max, //(max*1.1 - 0.1*min),
    6,4+topY+high-20,(int)min //(min*1.1 - 0.1*max)
  );
  printf(
    "   <polyline style=\"fill:none;stroke:black;stroke-width:2\" "
    "points=\"%d,%d %d,%d %d,%d\" />\n",
    25,topY+20, 25,topY+high-20, wide-14,topY+high-20);
  if(min == max) {
    return;
  }
  double vertM = (double)(40 - high) / (max-min);
  double vertB = topY+high-20.0 - vertM * min;
  double horiM =  (double)(wide-14-25) / (signal[count-1].timestamp - signal[0].timestamp);
  double horiB = (double)25 - horiM *  signal[0].timestamp;
  int i,j=0;
  for(i = 0;i<count;i++) {
    int at = (int)((double)signal[i].timestamp*horiM + horiB);
    if(at>64+j) {
      printf(
        "   <text text-anchor=\"middle\" style=\"stroke:black;font-size:10\" x=\"%d\" y=\"%d\">"
        "    %s</text>\n",
        at, topY+high-20+8, &(signal[i].yearMoDyHrMn[4]));
      j = at;
    }
  }
/****

  if( graphType == BARS) {
    for(i = 0;i<numSamp;i++) {
      x = kBorderX + (int)((double)(reading[i].datetime - minTime)*(double)(width - 2*kBorderX)/(double)(maxTime-minTime));
      y = height - kBorderY - (reading[i].value - min)*(height - kBorderY*2)/(max-min);
      fprintf(out,
        "    <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\""
        " style=\"fill:blue;stroke:black;stroke-width:2\" />\n",
        x-barWide/2, y, barWide, (height - kBorderY - y) );
    }
  } else if(graphType == LINE) {
*/
    printf(
      "   <polyline style=\"fill:none;stroke:black;stroke-width:2\" "
      "points=\"%d,%d",
      (int)((double)signal[0].timestamp*horiM + horiB),
      (int)((double)signal[0].sample*vertM + vertB));

    for(i = 1;i<count;i++) {
      printf(" %d,%d",
        (int)((double)signal[i].timestamp*horiM + horiB),
        (int)((double)signal[i].sample*vertM + vertB));
    }
    printf("\" />\n");
/***
  } else if(graphType == DOTS) {
    for(i = 0;i<numSamp;i++) {
      x = kBorderX + (int)((double)(reading[i].datetime - minTime)*(double)(width - 2*kBorderX)/(double)(maxTime-minTime));
      y = height - kBorderY - (reading[i].value - min)*(height - kBorderY*2)/(max-min);
      fprintf(out,
        "    <circle cx=\"%d\" cy=\"%d\" r=\"%d\""
        " style=\"fill:blue;stroke:black;stroke-width:2\" />\n",
        x, y, barWide/2);
    }
  }
***************/

}

int main(int argc, char **argv) {
  // char srting with extra room (I hope)
  char wildcard[2048];
  sprintf(wildcard,"/usr/lib/cgi-bin/queue/Monitor_%s", (argc>1)?argv[argc-1]:"20*");

  int sensorCount = sensorList(wildcard);
  SIGNAL signal[maxCount];
  double minValue, maxValue;
  int row=0;
  spewHtmlTop();

  printf(
    "  <svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
    " width=\"%dpx\" height=\"%dpx\""
    " viewBox=\"0 0 %d %d\">\n",
    boxWide, boxHigh * sensorCount,
    boxWide, boxHigh * sensorCount);
    
  printf("\n"
    "   <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\""
    " style=\"fill:black;stroke:black\" ></rect>\n",
    boxWide, boxHigh*sensorCount);

  for(int s=0;s<sensorCount;s++) {
     int got = getLastReadings(maxCount, wildcard, sensorName[s], signal, minValue, maxValue);
     if (got>0) {
       spewGraph(row*boxHigh, boxHigh, boxWide, sensorName[s], BAR, got,minValue,maxValue,signal);
       row++;
     }
  }
/*
char space[128], extra[128];
  DataBrane *db = new DataBrane();
  DataBrane::MEMBERS *member = db->getActiveMembers(false);
  int choices = db->memberCount();

  for(int r=0;r<choices;r++) {
    const int n = (prime*r)%choices;
    int y = r*(cardHigh+10)+5;
    int x = cardWide/2 + 5;
    const int high = (cardHigh+10)*choices;
    char dateRange[128];
    scorllingTextLine( BLACK, fontSize[0], x, y+=fontSize[0], member[n].name.c_str(), high);
    scorllingTextLine( RED  , fontSize[1], x, y+=fontSize[1], member[n].role.c_str(), high);
    scorllingTextLine( BLUE , fontSize[2], x, y+=fontSize[2], member[n].team.c_str(), high);
    sprintf(dateRange,
      "%s through %s",
      strtok(strcpy(space,member[n].enterDate.c_str())," "),
      strtok(strcpy(extra,member[n].leaveDate.c_str())," "));
    scorllingTextLine( GREEN, fontSize[3], x, y+=fontSize[3], dateRange, high);
  }
*/
  printf("   </svg>\n");
  spewHtmlBot();
}
