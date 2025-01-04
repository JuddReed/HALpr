#include <iostream>
#include <string.h>
#include "session.h"
#include "dataBrane.h"

using namespace std;
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

int main(int argc, char **argv, char **env) {
Session *sess = new Session(argc,argv);
char box[2048];
bool onServer = false;
   
  sess->spewTop("MDRS Habitat Activity Logger (prototype)");

  cout <<
    " <p>Welcome to version 0.15 of the "
    " <a href=/index.html>Habitat Activity Logging system (prototype)</a>.\n"
    "\n"
    " <p>You have successfully logged in and gained access to the shared portions of the system.\n"
    " You may also have additional privelages depending on your role.  \n";

  for ( char** a = env; *a; a++) {
    // printf("%s<br/>\n", *a);
    char* b = strtok(*a,"=");
    char* c = strtok(0,"=");
    if ( !strcmp(b,"HTTP_HOST") && !strcmp(c,"localhost") ) {
      printf("<H2>It is <b>STRONGLY</b> recommended that the system is"
        " accessed from another host!<br/>\n");
      onServer = true;

      thisHost(box);

      printf("Please browse to <i>http://%s/</i> from another computer on "
          "the HabActivityLogger wifi network.<br/></H2>\n", box );
    }
  }

  cout <<
    " <table><tr><td><ul>\n"
/*
    "  <li><a href=/cgi-bin/quickGuideWrapper?"
    << sess->getSessionArg() << "+menu+read>Quick Guides</a></li>\n"
*/
    "  <li>"
    "   <a href=/cgi-bin/tileServer?" << sess->getSessionArg() << "+518000+4254000+21+OUT>"
	"MDRS Map Server</a>\n"
    "  <a href=/cgi-bin/tileServer?" << sess->getSessionArg() << "+518500+4250500+8+OUT>"
	"(old start settings)</a></li>\n"
/*
    "  <li><a href=/cgi-bin/routePlanner?" << sess->getSessionArg() << ">"
        "EVA Route Planner</b>"
        "</a></li>\n"
*/
    "  <li><a href=/cgi-bin/pointEnter?" << sess->getSessionArg() << ">"
	"Way Points table</a></li>\n"
    "  <li><a href=/cgi-bin/reportGenWrapper?" << sess->getSessionArg() << "+menu>"
	"Report Management</a></li>\n"
    //"  <li><a href=/calendar.html >Report Review</a></li>\n"
    //"  <li><a href=/members/eventLogger.html>Event Logger</a></li>\n"
    " </ul></td><td><ul>\n"
    "  <li><a href=/weather.html>MDRS 7 day weather forecast</a></li>\n"
    "  <li><a href=/cgi-bin/libraryView?" << sess->getSessionArg() << "+menu>"
	"Some reference documents</a></li>\n"
/*
    "  <li><a href=/cgi-bin/engineerView?" << sess->getSessionArg() << "+menu>"
	"Engineer Data Tools</a></li>\n"
*/
    "  <li><a href=/cgi-bin/roster?" << sess->getSessionArg() << ">Enroll</a></li>\n"
    " </ul></td></tr</table>\n";

  delete sess;
}
