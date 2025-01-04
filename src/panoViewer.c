#include <iostream>
#include <math.h>
#include <string.h>
#include "session.h"
#include "dataBrane.h"

char* dir[17] = {(char*)"S", (char*)"SW", (char*)"W", (char*)"NW", (char*)"N",
    (char*)"NE", (char*)"E", (char*)"SE", (char*)"S", (char*)"SW", (char*)"W",
    (char*)"NW", (char*)"N", (char*)"NE", (char*)"E", (char*)"SE", (char*)"E"};
  int high = 360-16;
  int wide = 8*high;

void showPano(char* easting, char* northing) {
  printf("    <image x=\"%d\" y=\"0\" width=\"%d\" height=\"%d\""
         " xlink:href=\"/panorama/pano_%s_%s.bmp\"/>\n"
         "    <image x=\"%d\" y=\"0\" width=\"%d\" height=\"%d\""
         " xlink:href=\"/panorama/pano_%s_%s.bmp\"/>\n",
    0000, wide, high, easting, northing,
    wide, wide, high, easting, northing
    );
  for(int i=0;i<16;i++) {
    int x = i*wide/8;
    printf("    <text x=\"%d\" y=\"12\" fill=\"red\">%s</text>\n",x,dir[i]);
    printf("    <line x1=\"%d\" y1=\"12\" x2=\"%d\" y2=\"32\" "
	"style=\"stroke:RED;stroke-width:2\"/>\n", x,x);
  }
}

int polyX[9] = {0, -10,-10,-25,0,25,10,10,0};
int polyY[9] = {70, 70,85,85,100,85,85,70,70};

void showExitDir(char *bearOut) {
  int x;
  sscanf(bearOut,"%d",&x);
  x = (wide * (x-180)) / 360;

  if(x<0)x+=wide;
  printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">EXIT</text>\n",x-20, 36+0*13);
  printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">HERE</text>\n",x-22, 36+1*13);
  printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">(%s)</text>\n",x-17, 36+2*13, bearOut);
  printf("    <polyline style=\"fill:green;stroke:green\" points=\"");
  for(int p=0;p<9;p++)printf(" %d,%d", x+polyX[p],polyY[p]);
  printf("\"/>\n");

  x += wide;
  printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">EXIT</text>\n",x-20, 36+0*13);
  printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">HERE</text>\n",x-22, 36+1*13);
  printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">(%s)</text>\n",x-17, 36+2*13, bearOut);
  printf("    <polyline style=\"fill:green;stroke:green\" points=\"");
  for(int p=0;p<9;p++)printf(" %d,%d", x+polyX[p],polyY[p]);
  printf("\"/>\n");
}

void extraArgs(int argc, char **argv) {
  if(argc<=4) return;
  if(argc==5) showExitDir(argv[4]);
  else {
    long atE, atN, toE, toN;
    sscanf(argv[2],"%ld",&atE);
    sscanf(argv[3],"%ld",&atN);
    for(int n=4;n<argc-1;n+=2){
      sscanf(argv[n],"%ld",&toE);
      sscanf(argv[n+1],"%ld",&toN);
      double head = 180*atan2((double)(toE-atE),(double)(toN-atN))/M_PI;
      int x = (wide * (head+180)) / 360;
      x %= wide;
      printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">%ld E</text>\n",x-20, 36+0*13, toE);
      printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">%ld N</text>\n",x-22, 36+1*13, toN);
      printf("    <polyline style=\"fill:green;stroke:green\" points=\"");
      for(int p=0;p<9;p++)printf(" %d,%d", x+polyX[p],polyY[p]);
      printf("\"/>\n");
      x += wide;
      printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">%ld E</text>\n",x-20, 36+0*13, toE);
      printf("    <text x=\"%d\" y=\"%d\" fill=\"green\">%ld N</text>\n",x-22, 36+1*13, toN);
      printf("    <polyline style=\"fill:green;stroke:green\" points=\"");
      for(int p=0;p<9;p++)printf(" %d,%d", x+polyX[p],polyY[p]);
      printf("\"/>\n");
    }
  }
}

int main(int argc, char **argv) {
char buffer[1024];
Session* session = new Session(argc, argv);
  session->spewTop();
  printf("  <center><strong>12S %se %sn NAD27</strong></center>\n",argv[2],argv[3]);
  printf("  <div class=\"scrollable\">\n"
         "   <svg x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" >\n", wide*2, high);
  showPano(argv[2],argv[3]);
  extraArgs(argc, argv);
  printf("   </svg>\n"
         "  </div>\n");
  session->spewEnd();
}
