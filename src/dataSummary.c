#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "timeString.h"

  
const int kStaticMaxDepth = 110;
const int kTrailerMaxDeep = 110;
/*
int parseDateTime(char* timestr) {
tm unixTime;
  char *leftOver = strptime(timestr, "%m/%d/%Y+%I:%M%p", &unixTime);
  unixTime.tm_sec = 0;
  time_t tick = mktime(&unixTime);
  return tick;
}

int parsePackedDateTime(char* timestr) {
tm unixTime;
  char *leftOver = strptime(timestr, "%Y%m%d%H%M%S", &unixTime);
  unixTime.tm_sec = 0;
  time_t tick = mktime(&unixTime);
  return tick;
}
*/
#define BIG_GRAPH 1024
struct Sample {
  long tick;
  float loftFrom, meterFrom, tankFrom, trailerFrom;
  float loftInto, meterInto, tankInto, trailerInto;
  float tankFinal;
} reading[BIG_GRAPH];


char* expectedtags[] = {
  (char*)"Water+Logging+Date+And+Time:",
  (char*)"Static+Tank+Level+Before:",
  (char*)"Water+Meter+Reading+Before:",
  (char*)"Loft+Tank+Level+Before:",
  (char*)"Water+Used+During+Fill:",
  (char*)"Water+Meter+Reading+After:",
  (char*)"Loft+Tank+Level+After:",
  (char*)"Static+Tank+Level+After:",
  (char*)"Static+Tank+Refilled:",
  (char*)"Trailer+Tank+Level+Before:",
  (char*)"Trailer+Tank+Level+After:",
  (char*)"Final+Static+Tank+Level:" };

long parseRestOfFile(FILE* in, long dateTime, int numReading) {
  //printf("%ld",dateTime);
  char buf[512], *p;
  reading[numReading].tick = dateTime;
  for(int col=1;col<12;col++) {
    for(p = fgets(buf,512,in);p;p= fgets(buf,512,in)) {
      if(0==strncmp(buf,expectedtags[col],strlen(expectedtags[col])))break;
    }
    if(!p){/*printf(",---\n");*/return numReading;}
    p = fgets(buf,512,in);
    p[strlen(p)-2]=0;
    //printf(",%s",p);
         if(col==1)sscanf(p,"%f",&(reading[numReading].tankFrom));
    else if(col==2)sscanf(p,"%f",&(reading[numReading].meterFrom));
    else if(col==3)sscanf(p,"%f",&(reading[numReading].loftFrom));

    else if(col==5)sscanf(p,"%f",&(reading[numReading].meterInto));
    else if(col==6)sscanf(p,"%f",&(reading[numReading].loftInto));
    else if(col==7)sscanf(p,"%f",&(reading[numReading].tankInto));

    else if(col==9)sscanf(p,"%f",&(reading[numReading].trailerFrom));
    else if(col==10)sscanf(p,"%f",&(reading[numReading].trailerInto));
    else if(col==11)sscanf(p,"%f",&(reading[numReading].tankFinal));

    if((col==8)&&(0==strcmp(p,"no"))) {
      reading[numReading].trailerFrom = reading[numReading-1].trailerInto;
      reading[numReading].tankFinal = reading[numReading].tankInto;
      reading[numReading].trailerInto = reading[numReading].trailerFrom;
      break;
    }
  }
  return numReading + 1;
    //if(col<12)for(p = fgets(buf,512,in);p;p= fgets(buf,512,in))printf("%s",p);
}

void spewSql(FILE* out, int count) {
  char spaceForTime[64];
  fprintf(out, "INSERT INTO OBSERVATION\n"
  "(OBSERVATION_ID, ACTIV_PARAM_ID, MEMBERSHIP_ID, DATETIME, VALUE, UNITS) VALUES\n");

  int j=0;
  for(int i=0;i<count;i++) {
    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'CM'),\n",
      ++j, 1, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].tankFrom);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'Gallons'),\n",
      ++j, 2, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].meterFrom);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'Gallons'),\n",
      ++j, 3, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].loftFrom);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'Gallons'),\n",
      ++j, 4, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].meterInto);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'Gallons'),\n",
      ++j, 5, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].loftInto);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'CM'),\n",
      ++j, 6, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].tankInto);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'CM'),\n",
      ++j, 7, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].trailerFrom);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'CM'),\n",
      ++j, 8, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].trailerInto);

    fprintf(out, "(%d, %d, %d, TIMESTAMP '%s', %8.2f, 'CM')%c\n",
      ++j, 9, 999,dbTimeString(reading[i].tick, spaceForTime),
      reading[i].tankFinal, (i+1==count)?';':',');
  }
}

void sketchCan(FILE* out, int centerX, int bottomY, int w, int h, char* styleText) {
int rX = w / 2;
int rY = rX / 2;
int x0 = centerX - rX;
int x1 = centerX + rX;
int y1 = bottomY - rY;
int y0 = bottomY - h + rY;
  fprintf(out, " <path %s d=\"M%d %d L%d %d"
      " A%d %d 0 1 1 %d %d L%d %d"
      " A%d %d 0 0 1 %d %d"
      " A%d %d 0 0 1 %d %d\" />\n", styleText,
      x1,y0, x1,y1,
      rX,rY, x0,y1, x0,y0,
      rX,rY, x1,y0,
      rX,rY, x0,y0);
}
  
void sketchTube(FILE* out, int centerX, int bottomY, int w, int h, float turn,
 float level, char* fluidStyle, char* tubeStyle) {
// turn is used to set the ratio of radii. So turn=0 is no end showing and
// turn=1 is circle ends.
/*
First convert the level to an angle measured from the bottom of the tank = 0
around to the top=Pi.  For a tank of radius R that is filled to angle A the
volume in it is tube length times the sector area R*R*A minus the triangle to
the center R*cosA * R*sinA. Length and R squared are constant so for a water
line at angle A the fraction full = A-cosA*sinA / Pi. The inverse of this
function is not easy so I'll just iterate.:
*/
  double ang = M_PI/2, halfAng= ang/2;
  for(int q=0;q<20;q++) {
    double f = (ang - cos(ang)*sin(ang)) / M_PI;
    if(f==level)break;
    if(f<level)ang+=halfAng;
    else ang-=halfAng;
    halfAng/=2;
  }

int rY = h / 2;
int rX = rY * turn;
int x0 = centerX - w/2 + rX;
int x1 = centerX + w/2 - rX;
int y0 = bottomY- h;
int y1 = bottomY;

//for 3D effect the edges waterlines are tilted a little:
// A       B
//  C       D
int xA = centerX - w/2 + rX - rX * sin(ang+.3);
int xB = xA + w - 2 * rX;
int yAB = bottomY + ( cos(ang+.3) - 1.0) * h /2;
int xC = centerX - w/2 + rX + rX * sin(ang-.3);
int xD = xC + w - 2 * rX;
int yCD = bottomY + ( cos(ang-.3) - 1.0) * h /2;

  fprintf(out,
      " <path %s d=\"M%d %d L%d %d L%d %d L%d %d L%d %d"
      " A%d %d 0 0 1 %d %d L%d %d"
      " A%d %d 0 0 1 %d %d\" />\n", 
      fluidStyle, xD,yCD, xC,yCD, xA,yAB, xB,yAB, xD,yCD,
      rX,rY, x1,y1, x0,y1,
      rX,rY, xA,yAB);

  fprintf(out, " <path %s d=\"M%d %d L%d %d"
      " A%d %d 0 1 1 %d %d L%d %d"
      " A%d %d 0 0 1 %d %d"
      " A%d %d 0 0 1 %d %d\" />\n", tubeStyle,
      x0,y0, x1,y0,
      rX,rY, x1,y1, x0,y1,
      rX,rY, x0,y0,
      rX,rY, x0,y1);
}
  
void sketchFractionTank(FILE* out, int centerX, int centerY, int size, float fillFract) {

  float fluidHigh = (1.0 + fillFract)*(float)size/2.0;
  sketchCan(out, centerX, centerY+size/2, size, size,
      (char*)"stroke=\"black\" fill=\"black\" stroke-width=\"1\"");
  sketchCan(out, centerX, centerY+size/2, size, (int)fluidHigh,
      (fillFract<0.1)
      ? (char*)"stroke=\"pink\" fill=\"red\" stroke-width=\"1\""
      : (char*)"stroke=\"cyan\" fill=\"blue\" stroke-width=\"1\"");
  sketchCan(out, centerX, centerY+size/2, size, size,
      (char*)"stroke=\"white\" fill=\"none\" stroke-width=\"3\"");

  sketchCan(out, centerX, centerY-size/4+size/32, size/4, size/8,
      (char*)"stroke=\"white\" fill=\"none\" stroke-width=\"3\"");
}

void sketchTanks(FILE* out, Sample* reading, int BlockSize) {
  int i;
  fprintf(out,
  " <rect x=\"800\" y=\"0\" width=\"%d\" height=\"%d\""
  " style=\"fill:black;stroke:black\" />\n", BlockSize, 4*BlockSize+60 );

  fprintf(out," <path stroke=\"white\" stroke-width=\"4\" d=\"M%d %d L%d %d L%d %d\" />\n",
      800+BlockSize/2, BlockSize/2,
      800+BlockSize-5, BlockSize * 11/6 - 5,
      800+BlockSize/2, BlockSize * 11/6 - 5);

  sketchCan(out, 800+BlockSize/2, BlockSize*5/6, BlockSize*5/6, BlockSize*2/3,
      (char*)"stroke=\"gold\" fill=\"gold\"");

  fprintf(out, "   <ellipse cx=\"%d\" cy=\"%d\" rx=\"%d\" ry=\"%d\""
  " style=\"fill:white;stroke:black;stroke-width:2\" />\n",
  800 + BlockSize/2, BlockSize/3, BlockSize*5/12, BlockSize/6 );

  fprintf(out, "   <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\""
  " style=\"fill:black;stroke:black;stroke-width:2\" />\n",
  800 + BlockSize/2-28, BlockSize/3-20, 56, 18);

  fprintf(out, "   <text x=\"%d\" y=\"%d\""
  " style=\"stroke:white;stroke-width:2\">%8.1f</text>\n",
  800 + BlockSize/2-26, BlockSize/3-6, reading->meterInto);

  // ten 1% ticks on small dial
  for(i=0;i<10;i++)
    fprintf(out, "   <ellipse cx=\"%d\" cy=\"%d\" rx=\"1\" ry=\"1\""
    " style=\"fill:red;stroke:red;stroke-width:1\" />\n",
    800 + BlockSize/2 + (int)(BlockSize/12.0 * sin(M_PI*(double)i*0.2)),
       BlockSize*5/12 - (int)(BlockSize/12.0 * cos(M_PI*(double)i*0.2)));

  fprintf(out, "   <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\""
    " style=\"stroke:black;stroke-width:1\" />\n",
    800 + BlockSize/2, BlockSize*5/12,
    800 + BlockSize/2 + (int)(BlockSize/12.0 * sin(M_PI*20.0*reading->meterInto)),
       BlockSize*5/12 - (int)(BlockSize/12.0 * cos(M_PI*20.0*reading->meterInto)));

  // sketch the partially filled loft tank
  sketchTube(out, 800+BlockSize/2, 3*BlockSize/2+BlockSize/3, BlockSize*3/4, BlockSize*2/3,
      0.5, reading->loftInto / 60.0,
      (char*)"stroke=\"cyan\" fill=\"blue\" stroke-width=\"1\"",
      (char*)"stroke=\"white\" fill=\"none\" stroke-width=\"3\"");

  sketchFractionTank(out, 800+BlockSize/2, 5*BlockSize/2, BlockSize*2/3,
      1.0 - reading->tankFinal/kStaticMaxDepth );

  // Draw the hidden wheel for the trailer
  for(i=-8;i<0;i++)fprintf(out, "   <ellipse cx=\"%d\" cy=\"%d\" rx=\"%d\" ry=\"%d\""
    " style=\"fill:none;stroke:white;stroke-width:5\" />\n",
    800 + BlockSize/4+i, 21*BlockSize/6+i+8, BlockSize/12-2, BlockSize/6-4);
  fprintf(out, "   <ellipse cx=\"%d\" cy=\"%d\" rx=\"%d\" ry=\"%d\""
    " style=\"fill:gray;stroke:white;stroke-width:5\" />\n",
    800 + BlockSize/4, 21*BlockSize/6+8+8, BlockSize/12-2, BlockSize/6-4);

  // draw the trailer's bed and tongue.
  fprintf(out, "   <path stroke=\"white\" fill=\"gray\" stroke-width=\"2\" d=\"");
  fprintf(out, " M%d %d", 800,4*BlockSize);
  fprintf(out, " L%d %d", 800+BlockSize/4,4*BlockSize - BlockSize/6);
  fprintf(out, " L%d %d", 800+BlockSize/2,4*BlockSize);
  fprintf(out, " L%d %d", 800+BlockSize  ,4*BlockSize - BlockSize/3);
  fprintf(out, " L%d %d", 800+BlockSize/2,4*BlockSize - 2*BlockSize/3);
  fprintf(out, " L%d %d", 800,4*BlockSize - BlockSize/3);
  fprintf(out, " L%d %d", 800+BlockSize/4,4*BlockSize - BlockSize/6);
  fprintf(out, "\"/>\n");

  sketchFractionTank(out, 800+BlockSize/2, 7*BlockSize/2, BlockSize*2/3,
      reading->trailerInto/kTrailerMaxDeep );

  // add a wheel to the trailer
  for(i=0;i<8;i++)fprintf(out, "   <ellipse cx=\"%d\" cy=\"%d\" rx=\"%d\" ry=\"%d\""
    " style=\"fill:none;stroke:white;stroke-width:5\" />\n",
    800 + 3*BlockSize/4+i, 23*BlockSize/6+i+8, BlockSize/12-2, BlockSize/6-4);
  fprintf(out, "   <ellipse cx=\"%d\" cy=\"%d\" rx=\"%d\" ry=\"%d\""
    " style=\"fill:gray;stroke:white;stroke-width:5\" />\n",
    800 + 3*BlockSize/4+8, 23*BlockSize/6+8+8, BlockSize/12-2, BlockSize/6-4);

  // add refill hoses:
  fprintf(out, "    <path stroke=\"white\" fill=\"none\" stroke-width=\"3\""
      " d=\"M%d %d A%d %d 0 1 0 %d %d\" />\n", 880, 186, 48, 126, 828, 425);
  fprintf(out, "    <path stroke=\"white\" fill=\"none\" stroke-width=\"3\""
      " d=\"M%d %d A%d %d 0 1 1 %d %d\" />\n", 878, 375, 45, 128, 933, 570);
  /*
   <path stroke="white" fill="none" stroke-width="3" d="M878 375 A45 128 0 1 1 933 570" />
  */
 
}

void drawConstReadingHashes(FILE* out, double loGal, double hiGal, double slope, double duration,
  int loX = 25, int loY = 00, int wide = 750, int high = 160) {
    
  // Lower left is at t = minTime and y = minRate,  Upper left is minTime,maxRate
  int kGallStep = 50;
  int iLoGal = (int)loGal + kGallStep - ((int)loGal%kGallStep);
  int iHiGal = (int)(hiGal + (slope*duration));
  for(int gal=iLoGal; gal < iHiGal; gal += kGallStep) {
    double lx = 0;
    double ly = gal-loGal;
    double hx = duration;
    double hy = ly - duration*slope; // - slope*duration - loGal;
    if (ly > (hiGal-loGal)) {lx = ((double)gal-hiGal)/slope; ly = hiGal-loGal; }
    if (hy < 0 ) {
      hx += hy/slope;
      hy = 0;
      fprintf(out," <text style=\"stroke:red;stroke-width:1\""
          " x=\"%d\" y=\"%d\">%d</text>\n",
          loX+(int)((double)wide*hx/duration)-25, loY+high+15, gal);
    } 
    int x0 = loX + (int)(lx*wide/duration);
    int y0 = loY + high - (int)(ly*high/(hiGal-loGal));
    int x1 = loX + (int)(hx*wide/duration);
    int y1 = loY + high - (int)(hy*high/(hiGal-loGal));

    fprintf(out," <line style=\"stroke:red;stroke-width:1\""
        " x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"/>\n",x0,y0,x1,y1);
  }
}


void spewSvg(FILE* out, int numReading, int period=14) {
  const int kGraphRowHigh = 640 / 4;
  const int kMeterBaseRow = 1 * kGraphRowHigh;
  const int kLoftTBaseRow = 2 * kGraphRowHigh;
  const int kStaticTankTopRow = 2 * kGraphRowHigh;
  const int kTrailBaseRow = 4 * kGraphRowHigh;

  fprintf(out,
    "   <svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 %d %d\">\n",
    800+kGraphRowHigh, 4*kGraphRowHigh+60);

  fprintf(out,
  " <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\""
  " style=\"fill:white;stroke:black;stroke-width:2\" />\n", 800, 4*kGraphRowHigh+60);
  for(int i=1;i<=4;++i)fprintf(out, 
  " <line x1=\"25\" y1=\"%d\" x2=\"775\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
  kGraphRowHigh*i, kGraphRowHigh*i);

  int t;
  for(t=0;t<numReading;++t) {
    if( (reading[numReading-1].tick - (reading[t].tick ) < 3600*24*period) ) break;
  }

  int loT = t;
  double miniTime = reading[t].tick;
  double maxiTime = reading[numReading-1].tick;
  double multTime = 750.0 / (reading[numReading-1].tick - miniTime);
  double rateMeter = (reading[numReading-1].meterInto - reading[t].meterFrom) 
                   / (maxiTime - miniTime);
  fprintf(out," <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">"
	"Mean use = %d Gal/day</text>\n", 150,25, (int)(0.5+(rateMeter*24*3600)));
  double minRate = reading[t].meterFrom;
  double maxRate = reading[t].meterFrom;

  // Draw time hashes and labels
  int dom = 0;
  int wasCol = -1000;
  for(;t<numReading;++t) {
    struct tm* when = localtime(&(reading[t].tick));
    int col = 25+(int)((reading[t].tick - miniTime)*multTime);

    fprintf(out," <line style=\"stroke:red;stroke-width:1\""
        " x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"/>\n",
        col,000, col, 4*kGraphRowHigh);
    if(col - wasCol > 32) { // if we've moved right by 32 pixels since last label
      fprintf(out," <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">%2d:%02d</text>\n",
        col-16, 4*kGraphRowHigh + 14, when->tm_hour, when->tm_min);
      wasCol = col;
    }
    if(dom!=when->tm_mday) {
      dom=when->tm_mday;
      fprintf(out,
        " <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">%d/%02d</text>\n",
        col-16, 4*kGraphRowHigh + 28, 1+when->tm_mon, when->tm_mday);
        /*" <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">%d/%02d/%02d</text>\n",
        col-16, 4*kGraphRowHigh + 28, 1+when->tm_mon, when->tm_mday, when->tm_year%100);*/
    }
  }
  // Plot loft tank level over time (and compute meter graph vertical range)
  fprintf(out, " <polyline style=\"fill:none;stroke:black;stroke-width:3\" points=\"");
  for(t=loT;t<numReading;t++) {
    double meter = reading[t].meterFrom - (reading[t].tick - miniTime)*rateMeter;
    if(meter < minRate )minRate = meter;
    if(meter > maxRate )maxRate = meter;
    meter = reading[t].meterInto - (reading[t].tick - miniTime)*rateMeter;
    if(meter < minRate )minRate = meter;
    if(meter > maxRate )maxRate = meter;

    fprintf(out, " %d,%d",
      (int)(25.0 + (reading[t].tick - miniTime) * multTime),
      kLoftTBaseRow - (int)(reading[t].loftFrom*kGraphRowHigh/64) );
    fprintf(out, " %d,%d",
      (int)(25.0 + (reading[t].tick - miniTime) * multTime),
      kLoftTBaseRow - (int)(reading[t].loftInto*kGraphRowHigh/64) );
  }
  fprintf(out, "\" />\n");

  drawConstReadingHashes(out, minRate, maxRate, rateMeter, maxiTime-miniTime);
  
  fprintf(out, " <polyline style=\"fill:none;stroke:black;stroke-width:3\" points=\"");
  int newTick, oldTick = (int)(25.0 + (reading[loT].tick - miniTime) * multTime);
  for(t=loT;t<numReading;t++) {
    newTick = (int)(25.0 + (reading[t].tick - miniTime) * multTime);
    if(newTick < oldTick)newTick = oldTick;
    oldTick = newTick;
    double meter = reading[t].meterFrom - (reading[t].tick - miniTime)*rateMeter;
    fprintf(out, " %d,%d", newTick,
      kMeterBaseRow - (int)(kGraphRowHigh*(meter-minRate)/(maxRate-minRate)) );
    meter = reading[t].meterInto - (reading[t].tick - miniTime)*rateMeter;
    fprintf(out, " %d,%d", newTick,
      kMeterBaseRow - (int)(kGraphRowHigh*(meter-minRate)/(maxRate-minRate)) );
  }
  fprintf(out, "\" />\n");

  fprintf(out, " <polyline style=\"fill:none;stroke:black;stroke-width:3\" points=\"");
  oldTick = (int)(25.0 + (reading[loT].tick - miniTime) * multTime);
  for(t=loT;t<numReading;t++) {
    newTick = (int)(25.0 + (reading[t].tick - miniTime) * multTime);
    if(newTick < oldTick)newTick = oldTick;
    oldTick = newTick;
    fprintf(out, " %d,%d", newTick,
      kStaticTankTopRow + (int)(reading[t].tankFrom*kGraphRowHigh/kStaticMaxDepth) );
    fprintf(out, " %d,%d", newTick,
      kStaticTankTopRow + (int)(reading[t].tankInto*kGraphRowHigh/kStaticMaxDepth) );
    fprintf(out, " %d,%d", newTick,
      kStaticTankTopRow + (int)(reading[t].tankFinal*kGraphRowHigh/kStaticMaxDepth) );
  }
  fprintf(out, "\" />\n");

  fprintf(out, "   <polyline style=\"fill:none;stroke:black;stroke-width:3\" points=\"");
  oldTick = (int)(25.0 + (reading[loT].tick - miniTime) * multTime);
  for(t=loT;t<numReading;t++) {
    newTick = (int)(25.0 + (reading[t].tick - miniTime) * multTime);
    if(newTick < oldTick)newTick = oldTick;
    oldTick = newTick;
    fprintf(out, " %d,%d", newTick,
      kTrailBaseRow - (int)(reading[t].trailerFrom*kGraphRowHigh/kTrailerMaxDeep) );
    fprintf(out, " %d,%d", newTick,
      kTrailBaseRow - (int)(reading[t].trailerInto*kGraphRowHigh/kTrailerMaxDeep) );
  }
  fprintf(out, "\" />\n");

  for(int t=0;t<4;++t) fprintf(out,
      "   <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\""
      " style=\"fill:white;stroke:white;stroke-width:2\" />\n",
      28, 10 + kGraphRowHigh * t, 80, 18) ;

  fprintf(out,
      "   <text style=\"fill:black;stroke:black;stroke-width:1\""
      " x=\"%d\" y=\"%d\">%s</text>\n", 30, 25 + kGraphRowHigh *
      0, (char *)"Water Meter" );

  fprintf(out,
      "   <text style=\"fill:black;stroke:black;stroke-width:1\""
      " x=\"%d\" y=\"%d\">%s</text>\n", 30, 25 + kGraphRowHigh *
      1, (char *)"Loft Tank" );

  fprintf(out,
      "   <text style=\"fill:black;stroke:black;stroke-width:1\""
      " x=\"%d\" y=\"%d\">%s</text>\n", 30, 25 + kGraphRowHigh *
      2, (char *)"Static Tank" );

  fprintf(out,
      "   <text style=\"fill:black;stroke:black;stroke-width:1\""
      " x=\"%d\" y=\"%d\">%s</text>\n", 30, 25 + kGraphRowHigh *
      3, (char *)"Trailer Tank" );

  sketchTanks(out, &reading[numReading-1], kGraphRowHigh);

  fprintf(out, "  </svg>\n");
}

int loadWaterLogs() {
char buf[512];
char* p;
int numReading = 0;

FILE* list = popen("ls /usr/lib/cgi-bin/queue/EngSupWater_*","r");
  for(p = fgets(buf,512,list);p;p= fgets(buf,512,list)) {
    p[strlen(p)-1]=0;
printf("<!-- %s ", p); char spaceForTime[64];
    FILE* in = fopen(p,"r");
    for(p = fgets(buf,512,in);p;p= fgets(buf,512,in)) {
      if(0==strncmp(buf,expectedtags[0],strlen(expectedtags[0])))break;
    }
    if(!p){printf("---\n");fclose(in);continue;}
    p = fgets(buf,512,in);
    p[strlen(p)-2]=0;
    long dateTime = parseDateTime(p);

    numReading = parseRestOfFile(in, dateTime, numReading);

    fclose(in);
printf(" at %ld = %s -->\n", dateTime, dbTimeString(dateTime, spaceForTime));
  }
  pclose(list);
  return numReading;
}

struct PERCENT_SERIES {
 long dateTime;
 double percent;
} series[3*365];//up to three years of daily samples

void spewPercentSeriesSvg(FILE* out, char* title, int numReading, struct PERCENT_SERIES *series) {
  int kGraphRowHigh = 200;

  int i = numReading - 1;
  double xm = 750.0 /(series[i].dateTime-series[0].dateTime);
  fprintf(out,
    "   <svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 %d %d\">\n",
    800, kGraphRowHigh+500);

  fprintf(out,
    " <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\""
    " style=\"fill:white;stroke:black;stroke-width:2\" />\n", 800, kGraphRowHigh+50);

  fprintf(out, 
    " <line x1=\"25\" y1=\"%d\" x2=\"775\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
    kGraphRowHigh+25, kGraphRowHigh+25);

  fprintf(out, 
    " <line x1=\"25\" y1=\"%d\" x2=\"25\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
    25, kGraphRowHigh+25);

  for(i=0;i<10;i++) {
    fprintf(out, 
      " <line x1=\"20\" y1=\"%d\" x2=\"25\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
      (10-i)*kGraphRowHigh/10+25, (10-i)*kGraphRowHigh/10+25);
    fprintf(out, 
      " <text x=\"2\" y=\"%d\" style=\"stroke:red;stroke-width:2\">%d</text>\n",
      (10-i)*kGraphRowHigh/10+25, 10*i);
  }
  fprintf(out, 
    " <text x=\"2\" y=\"%d\" style=\"stroke:red;stroke-width:2\">%%</text>\n",
      20);
  fprintf(out, 
    " <text x=\"%d\" y=\"%d\" style=\"stroke:red;stroke-width:2\">%s</text>\n",
    800/2 - 75,  15, title);

  int wasCol = -1000;
  int wasRow = 10000;
  for(i=0;i<numReading;i++) {
    int column = 25+(int)((double)(series[i].dateTime - series[0].dateTime)*xm);

    int row = 25 + (int)((float)kGraphRowHigh * (1.0 - series[i].percent));

    if((row<wasRow)||(row>wasRow+20)) {
      fprintf(out, 
        " <text x=\"%d\" y=\"%d\""
        " style=\"fill:white;stroke:black;stroke-width:2\">%d%%</text>\n",
        column, row - 3, (int)(100.0 * series[i].percent));
      wasRow = row;
    }

    fprintf(out, 
      " <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\""
      " style=\"fill:blue;stroke:black;stroke-width:2\" />\n",
      column,row,
      (int)(43200.0 * xm),
      (int)((float)kGraphRowHigh * series[i].percent) );

    struct tm* when = localtime(&(series[i].dateTime));
    if(column - wasCol > 32) { // if we've moved right by 32 pixels since last label
      fprintf(out,
        " <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">%d/%02d</text>\n",
        column, 30+kGraphRowHigh + 14, 1+when->tm_mon, when->tm_mday, when->tm_year%100);
      wasCol = column;
    }
  }
  fprintf(out, "  </svg>\n");
}

int loadPercentLog(char* param, struct PERCENT_SERIES *series) {
char buf[512];
char* p;
int numReading = 0;
sprintf(buf,"grep %s -A1 /usr/lib/cgi-bin/queue/Engineering_* "
    " | grep Engineering | sed -e 's:.*Engineering_::' | grep '-'| sed -e 's:-: :'", 
    param);
FILE* samples = popen(buf, "r");
  for(p = fgets(buf,512,samples);p;p= fgets(buf,512,samples)) {
  long dt;
  double perc;
    int g = sscanf(p,"%ld %lf",&dt,&perc);
    if(2 != g)continue;
    series[numReading].dateTime = parsePackedDateTime(p);
    series[numReading].percent = perc;
    numReading++;
  }
  pclose(samples);
  return numReading;
}

int loadPropaneLog() { return loadPercentLog((char*)"Propane+Fuel+Level", series); };
int loadDieselLog() { return loadPercentLog((char*)"Diesel+Fuel+Level", series); };
int loadDownloadLog() { return loadPercentLog((char*)"Network+Download+Allowance", series); };
int loadSignalLog() { return loadPercentLog((char*)"Network+Signal+Strength", series); };

void spewLpSvg(FILE* out, int numReading) {
  spewPercentSeriesSvg(out, (char*)"Propane Tank Level", numReading, series);
}
void spewDieSvg(FILE* out, int numReading) {
  spewPercentSeriesSvg(out, (char*)"Generator Fuel Level", numReading, series);
}
void spewDownSvg(FILE* out, int numReading) {
  for(int i=0;i<numReading;i++) series[i].percent /= 100.0;
  spewPercentSeriesSvg(out, (char*)"Satellite Download Allowance", numReading, series);
}
void spewSignalSvg(FILE* out, int numReading) {
  double max = 0; int i;
  for(i=0;i<numReading;i++) max = (max>series[i].percent)?max:series[i].percent;
  for(i=0;i<numReading;i++) series[i].percent /= max;
  spewPercentSeriesSvg(out, (char*)"Satellite Signal Strength", numReading, series);
}

 
struct TEMPERATURES {
 long dateTime;
 double deckOneTemp, deckTwoTemp;
} temperature[365];//up to one year of daily samples

void spewHeatSvg(FILE* out, int numReading) {
  int kGraphRowHigh = 100;
  int GraphGap = 25;
  int i = numReading - 1;
  int GraphWidth = 900;
  double xm = (double)GraphWidth /(3600*24*15);
  int xb = temperature[numReading-1].dateTime - 3600*24*15;

  fprintf(out,
    "   <svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 %d %d\">\n",
    2*GraphGap+GraphWidth, kGraphRowHigh*2+3*GraphGap);

  fprintf(out,
    " <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\""
    " style=\"fill:white;stroke:black;stroke-width:2\" />\n",
    2*GraphGap+GraphWidth, 2*kGraphRowHigh+3*GraphGap);

  for(i=1;i<=2;i++) {
    fprintf(out, 
      " <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
      GraphGap, i*(kGraphRowHigh+GraphGap), GraphGap+GraphWidth, i*(kGraphRowHigh+GraphGap));
    fprintf(out, 
      " <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
      GraphGap, i*(kGraphRowHigh+GraphGap), GraphGap, (i-1)*kGraphRowHigh+i*GraphGap);
  }

  for(i=32;i<100;i+=16) {
    fprintf(out, 
      " <line x1=\"20\" y1=\"%d\" x2=\"25\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
      (96-i)*kGraphRowHigh/64+GraphGap, (96-i)*kGraphRowHigh/64+GraphGap);
    fprintf(out, 
      " <line x1=\"20\" y1=\"%d\" x2=\"25\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
      (96-i)*kGraphRowHigh/64+GraphGap*2+kGraphRowHigh,
      (96-i)*kGraphRowHigh/64+GraphGap*2+kGraphRowHigh);
    fprintf(out, 
      " <text x=\"2\" y=\"%d\" style=\"stroke:red;stroke-width:2\">%d</text>\n",
      (96-i)*kGraphRowHigh/64+GraphGap, i);
    fprintf(out, 
      " <text x=\"2\" y=\"%d\" style=\"stroke:red;stroke-width:2\">%d</text>\n",
      (96-i)*kGraphRowHigh/64+GraphGap*2+kGraphRowHigh, i);
  }

  fprintf(out, " <text x=\"27\" y=\"%d\" style=\"stroke:red;stroke-width:2\">F</text>\n", 
      GraphGap);
  fprintf(out, " <text x=\"27\" y=\"%d\" style=\"stroke:red;stroke-width:2\">F</text>\n", 
      kGraphRowHigh+2*GraphGap);

  fprintf(out, 
    " <text x=\"%d\" y=\"%d\" style=\"stroke:red;stroke-width:2\">Upper Deck Temperature</text>\n",
    GraphGap+GraphWidth/2 - 80,  15);
  fprintf(out, 
    " <text x=\"%d\" y=\"%d\" style=\"stroke:red;stroke-width:2\">Lower Deck Temperature</text>\n",
    GraphGap+GraphWidth/2 - 80,  15+GraphGap+kGraphRowHigh);

  int wasCol = -1000;
  int wasRowTwo = 10000;
  int wasRowOne = 10000;
  for(i=0;i<numReading;i++) {
    if(temperature[i].dateTime < xb)continue;

    int column = GraphGap+(int)((double)(temperature[i].dateTime - xb)*xm);

    int row = GraphGap+kGraphRowHigh
        - (int)((float)kGraphRowHigh * (temperature[i].deckTwoTemp-32.0)/64.0);

    if((row<wasRowTwo)||(row>wasRowTwo+20)) {
      fprintf(out, 
        " <text x=\"%d\" y=\"%d\""
        " style=\"fill:white;stroke:black;stroke-width:2\">%d</text>\n",
        column, row - 3, (int)temperature[i].deckTwoTemp);
      wasRowTwo = row;
    }
    fprintf(out, 
      " <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\""
      " style=\"fill:blue;stroke:black;stroke-width:2\" />\n",
      column,row, (int)(3600 * 6 * xm), GraphGap+kGraphRowHigh - row);

    row = 2*(GraphGap+kGraphRowHigh)
        - (int)((float)kGraphRowHigh*(temperature[i].deckOneTemp-32.0)/64.0);
    if((row<wasRowOne)||(row>wasRowOne+20)) {
      fprintf(out, 
        " <text x=\"%d\" y=\"%d\""
        " style=\"fill:white;stroke:black;stroke-width:2\">%d</text>\n",
        column, row - 3, (int)temperature[i].deckOneTemp);
      wasRowTwo = row;
    }
    fprintf(out, 
      " <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\""
      " style=\"fill:blue;stroke:black;stroke-width:2\" />\n",
      column,row, (int)(3600 * 6 * xm), 2*(GraphGap+kGraphRowHigh) - row);

    struct tm* when = localtime(&(temperature[i].dateTime));
    if(column - wasCol > 32) { // if we've moved right by 32 pixels since last label
      fprintf(out,
        " <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">%d/%02d</text>\n",
        column, 2*(GraphGap+kGraphRowHigh) + 14, 1+when->tm_mon, when->tm_mday);
      wasCol = column;
    }
  }
  fprintf(out, "  </svg>\n");
}

char* keyHvacTags[] = {
  (char*)"Hvac+Event+Date",
  (char*)"Hvac+Event+Time",
  (char*)"Hvac+Lower+Deck+Temperature+Reading:",
  (char*)"Hvac+Upper+Deck+Temperature+Reading:",
};

int loadHvacLogs() {
char buf[512];
char dateTime[1024];
double tU, tL;

char* p;
int numReading = 0;

FILE* list = popen("ls /usr/lib/cgi-bin/queue/EngSupHVAC_*","r");
  for(p = fgets(buf,512,list);p;p= fgets(buf,512,list)) {
    p[strlen(p)-1]=0;
    FILE* in = fopen(p,"r");
    int element = 0;
    tL = tU = -9999;
    *dateTime=0;
    for(p = fgets(buf,512,in);p;p= fgets(buf,512,in)) {
      if(0==strncmp(buf,keyHvacTags[element],strlen(keyHvacTags[element]))) {
        p = fgets(buf,512,in);
    	p[strlen(p)-1]=0;
    	if(p[strlen(p)-1]<'0')p[strlen(p)-1]=0;
        switch(element) {
        case 0:
          strcpy(dateTime,p);
          strcat(dateTime,"+");
          break;
        case 1:
          strcat(dateTime,p);
          break;
        case 2:
          sscanf(p,"%lf",&tL);
          break;
        case 3:
          sscanf(p,"%lf",&tU);
          break;
        }
        element++;
      }
      if(element>=4)break;
    }
    fclose(in);
    if(element!=4)continue;

    temperature[numReading].dateTime = parseDateTime(dateTime);
    temperature[numReading].deckOneTemp = tL;
    temperature[numReading].deckTwoTemp = tU;
    numReading++;
  }
  pclose(list);
  return numReading;
}
