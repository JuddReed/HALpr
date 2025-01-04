#include "svgGraph.h"

int SvgGraph::load(DataBrane::OBSERVATION* data, int count) {
  reading = data;
  numSamp = count;
  min = reading[0].value;
  max = min;
  for(int index = 1;index<count;index++) {
    min = (min<reading[index].value)?min:reading[index].value;
    max = (max>reading[index].value)?max:reading[index].value;
  }
  return numSamp;
}
  
void SvgGraph::head(FILE* out) {
  fprintf(out,
    "   <svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 %d %d\">\n",
    width, height*2);

  fprintf(out,
    "    <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\""
    " style=\"fill:white;stroke:black;stroke-width:1\" />\n",
    width, height*((graphType == TABL)?2:1));
}
  
void SvgGraph::spew(FILE* out) {
  int i, x,y;
  if( graphType == TABL) {
    for(i=0;i<numSamp;i++) {
      if((numSamp-i)*14>2*height)continue;
      struct tm* when = localtime(&(reading[i].datetime));
      fprintf(out,
        "    <text x=\"%d\" y=\"%d\" style=\"stroke:black;fill=black;stroke-width:1\""
        ">%04d/%02d/%02d %02d:%02d %lf %s</text>\n", 3,14*(numSamp-i),
        1900+when->tm_year, 1+when->tm_mon, when->tm_mday, when->tm_hour, when->tm_min,
        reading[i].value, reading[i].units.c_str());
    }
    return;
  }
  fprintf(out,
    "    <text x=\"%d\" y=\"%d\" style=\"stroke:red;stroke-width:1\">%s</text>\n", 3,3+12,
    reading[0].units.c_str());

  for(i=0;i<=5;i++) {
    int level = (int)(max - (max-min)*(double)i / 5.0);
    int y = (height - 2 * kBorderY) * i / 5 + kBorderY;
    fprintf(out,
      "    <text x=\"%d\" y=\"%d\" style=\"stroke:red;stroke-width:1\">%d</text>\n", 3,y,level);
    fprintf(out,
      "    <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
    kBorderX-5, y, kBorderX, y);
  }
    

  float minStep = 999999;
  for(i = 1;i<numSamp;i++) {
    float step = reading[i].datetime - reading[i-1].datetime;
    if((step>0)&&(step < minStep))minStep = step;
  }
  int minTime = reading[0].datetime - minStep/2;
  int maxTime = reading[numSamp-1].datetime + minStep/2;
  int barWide = (width - 2*kBorderX)*minStep/(maxTime-minTime);
  barWide = (barWide<2)?2:barWide;
  if(barWide>kBorderY)barWide=kBorderY;
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
    fprintf(out, "    <polyline style=\"fill:none;stroke:black;stroke-width:2\" points=\"");
    for(i = 0;i<numSamp;i++) {
      x = kBorderX + (int)((double)(reading[i].datetime - minTime)*(double)(width - 2*kBorderX)/(double)(maxTime-minTime));
      y = height - kBorderY - (reading[i].value - min)*(height - kBorderY*2)/(max-min);
      fprintf(out,"%d,%d ",x,y);
    }
    fprintf(out,"\" />\n");
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

  fprintf(out,
    "    <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
    kBorderX, height-kBorderY, width-kBorderX, height-kBorderY);

  fprintf(out,
    "    <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
    kBorderX, kBorderY, kBorderX, height-kBorderY);

  int wasCol = -99;
  int wasDay = -99;
  for(i = 0;i<numSamp;i++) {
    x = kBorderX + (int)((double)(reading[i].datetime - minTime)*(double)(width - 2*kBorderX)/(double)(maxTime-minTime));
    if(x - wasCol > 32) { // if we've moved right by 32 pixels since last label
      struct tm* when = localtime(&(reading[i].datetime));
    fprintf(out,
      "    <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"stroke:red;stroke-width:2\" />\n",
      x, height - kBorderY, x, height - kBorderY+3);
      fprintf(out,
        " <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">%02d:%02d</text>\n",
        x-18, height - kBorderY + 15, when->tm_hour, when->tm_min);
      if(when->tm_mday!=wasDay)fprintf(out,
        " <text style=\"stroke:red;stroke-width:1\" x=\"%d\" y=\"%d\">%d/%02d</text>\n",
        x-18, height - kBorderY + 28, 1+when->tm_mon, when->tm_mday);
      wasCol = x;
      wasDay = when->tm_mday;
    }
  }
}

void SvgGraph::link(char *format, FILE* out) {
int i, x,y;
  if( graphType == TABL) {
    for(i=0;i<numSamp;i++) {
      if((numSamp-i)*14>2*height)continue;
      fprintf(out,"  <a xlink:href=\"");
      fprintf(out,format,reading[i].observation_id);
      fprintf(out,
        "\"><rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\""
        " style=\"opacity:0.000\"/></a>\n", 3,14*(numSamp-i-1),300,14);
    }
    return;
  }
  float minStep = 999999;
  for(i = 1;i<numSamp;i++) {
    float step = reading[i].datetime - reading[i-1].datetime;
    if((step>0)&&(step < minStep))minStep = step;
  }
  int minTime = reading[0].datetime - minStep/2;
  int maxTime = reading[numSamp-1].datetime + minStep/2;
  int barWide = (width - 2*kBorderX)*minStep/(maxTime-minTime);
  barWide = (barWide<2)?2:barWide;
  for(i = 0;i<numSamp;i++) {
    x = kBorderX + (int)((double)(reading[i].datetime - minTime)*(double)(width - 2*kBorderX)/(double)(maxTime-minTime));
    y = height - kBorderY - (reading[i].value - min)*(height - kBorderY*2)/(max-min);
    fprintf(out,"  <a xlink:href=\"");
    fprintf(out,format,reading[i].observation_id);
    fprintf(out,"\"><circle cx=\"%d\" cy=\"%d\" r=\"%d\" style=\"opacity:0.000\"/></a>\n",
      x, y, barWide/2);
  }
}

void SvgGraph::tail(FILE* out) { fprintf(out, "   </svg>\n"); }

