#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "session.h"
#include "dataBrane.h"

const int fontSize[4] = {32, 16, 24, 16};
const int prime = 4999;
const int period = 300;	// time to go through whole list in seconds (5 minutes)
const int cardHigh = fontSize[0]+fontSize[1]+fontSize[2]+fontSize[3]+5;
const int cardWide = 400;

const char* BLACK = (char*)"black";
const char* RED   = (char*)"red";
const char* GREEN = (char*)"green";
const char* BLUE  = (char*)"blue";

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

void scorllingTextLine(const char* color, int size, int xc, int y0, const char* line, int high) {
    printf(
      " <text text-anchor=\"middle\" style=\"stroke:%s;font-size:%d\" x=\"%d\" y=\"%d\">\n"
      "  %s\n"
      "  <animate attributeName=\"y\" attributeType=\"XML\" "
      "   begin=\"0s\" dur=\"%ds\" fill=\"freeze\" from=\"%d\" to=\"%d\" />\n"
      " </text>\n",
      color, size, xc, y0, line, period, y0, y0-high);
}

int main(int argc, char **argv) {
char space[128], extra[128];
  DataBrane *db = new DataBrane();
  DataBrane::MEMBERS *member = db->getActiveMembers(false);
  int choices = db->memberCount();

  spewHtmlTop();

  printf(
    "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
    " width=\"%dpx\" height=\"%dpx\" viewBox=\"0 0 %d %d\">\n",
     cardWide+10, cardHigh+10, cardWide+10, cardHigh+10);

  for(int r=0;r<choices;r++) {
    const int n = (prime*r)%choices;
    int y = r*(cardHigh+10)+5;
    int x = cardWide/2 + 5;
    const int high = (cardHigh+10)*choices;
    char dateRange[128];
    printf("\n"
      " <rect x=\"5\" y=\"%d\" width=\"%d\" height=\"%d\""
      " style=\"fill:white;stroke:black;stroke-width:2\" >\n"
      "  <animate attributeName=\"y\" attributeType=\"XML\" "
      "   begin=\"0s\" dur=\"%ds\" fill=\"freeze\" from=\"%d\" to=\"%d\" />\n"
      " </rect>\n",
      y,cardWide, cardHigh, period, y, y-high);

    scorllingTextLine( BLACK, fontSize[0], x, y+=fontSize[0], member[n].name.c_str(), high);
    scorllingTextLine( RED  , fontSize[1], x, y+=fontSize[1], member[n].role.c_str(), high);
    scorllingTextLine( BLUE , fontSize[2], x, y+=fontSize[2], member[n].team.c_str(), high);
    sprintf(dateRange,
      "%s through %s",
      strtok(strcpy(space,member[n].enterDate.c_str())," "),
      strtok(strcpy(extra,member[n].leaveDate.c_str())," "));
    scorllingTextLine( GREEN, fontSize[3], x, y+=fontSize[3], dateRange, high);
  }
  printf("</svg>\n");
  spewHtmlBot();
}
