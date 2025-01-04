#include <iostream>
#include <string.h>
#include "dataBrane.h"

const int GraphWidth = 850;
const int GraphHeight = 350;
using namespace std;

char* greeting  = (char*)"Welcome to the HALpr dashboard.<br/>\n"
  "This screen is intended to display information about HALpr.<br/>\n"
  "Who has been here before:\n"
  "<iframe align=top width=440 height=150 src=\"/cgi-bin/everyone\"></iframe><br/>\n"
  "<br/>The following graphs are updated peiodically. They show the last several "
  "readings from all of the sensors that are automatically monitored.<br/>"
  "<center>\n";

char* thisHost(char *buff) {
FILE* c = popen("/sbin/ifconfig wlan0 | grep 'inet add'| sed -e 's-.*addr:--' -e 's: .*::'","r");
//FILE* c = popen("ifconfig eth0 | grep 'inet add' |sed -e 's: Bcast.*::' -e 's-.*:--'","r");
  sprintf(buff,"unknown");
  if(c) {
    int got = fscanf(c,"%s",buff);
    pclose(c);
  }
  return strdup(buff);
}

void spewTop(char* thisPage, int age) {
  printf(
    "Content-type: text/html\n\n"
    "<html lang=\"en-US\" xml:lang=\"en-US\" xmlns =\"http://www.w3.org/1999/xhtml\">\n"
    " <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\" />\n"
    " <head>\n"
    "  <title>Habitat Activity Logger PRototype</title>\n");
  printf( "  <meta http-equiv=\"refresh\" content=\"%d;url=http://%s/?%d\">\n", 300, thisPage, age+1);
  printf(
    " </head>\n"
    " <body>\n"
    "  <table>\n"
    "   <tr><th colspan=4>\n"
    "    <h1>MDRS"
    " <font color=red>H</font>abitat"
    " <font color=red>A</font>ctivity"
    " <font color=red>L</font>ogger"
    " <font color=red>pr</font>ototype</h1>\n"
    "   </th></tr>\n"
    "   <tr><td align=center valign=top rowspan=2>\n"
    "    <a href=/cgi-bin/home>\n"
    "     <img src=/marvin.jpg width=256>\n"
    "    </a><br/><a href=/cgi-bin/home>\n"
    "     Sign In to access crew specific pages\n"
    "    </a>\n"
    "    <br><br><a href=/weather.html>MDRS 7 day weather forecast</a>\n"
    "   </td><td>\n");
  printf("%s", greeting);
  printf(
    "<iframe align=top src=\"/cgi-bin/temperatureGraph\" width=\"%d\" height=\"%d\" "
    "scrolling=\"yes\"><p>Your browser does not support iframes.</p></iframe/>\n"
    "</center><br/>\n", GraphWidth, GraphHeight);
  printf(
    "   </td></tr>\n"
    "  </table>\n");
}

void spewBot() {
  printf(
    " </body>\n"
    "</html>\n"
  );
}

int main(int argc, char **argv, char **env) {
char box[2048], whoAmI[1024];
bool onServer = false;
int age = 0;
  if(argc>1)sscanf(argv[1],"%d",&age);

  for ( char** a = env; *a; a++) {
    char* b = strtok(*a,"=");
    char* c = strtok(0,"=");
    if ( !strcmp(b,"HTTP_HOST") && !strcmp(c,"localhost") ) {
      onServer = true;
      thisHost(box);
    }
    if ( !strcmp(b,"HTTP_HOST") ) {
      strcpy(whoAmI, c);
    }
    if ( !strcmp(b,"REQUEST_URI") ) {
      strcat(whoAmI, c);
    }
  }
  
  spewTop(whoAmI, age);

  if(onServer) {
      printf("Please browse to <i>http://%s/</i> from another computer on "
          "the <font color=green>HabActivityLogger</font> wifi network.<br/></H2>\n", box );
  }

  spewBot();
}
