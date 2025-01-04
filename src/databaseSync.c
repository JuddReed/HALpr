#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "session.h"
#include "dataBrane.h"
#include "dataSummary.h"


using namespace std;

char TRANSMIT_QUEUE[] = "/usr/lib/cgi-bin/queue";

/*
void sendPageTop(const char *label) {
  printf(
    "Content-type: text/html\r\n\r\n<html>\r\n <head>\r\n  <title>%s</title>\r\n"
    "  <style>\r\n   .scrollable {\r\n    width:900;\r\n    height:400;\r\n"
    "    overflow: auto;\r\n   }\r\n  </style>\r\n </head>\r\n"
    " <body>\r\n  <table>\r\n   <tr><th>\r\n"
    "    <a href=/index.html><img src=/haleye.gif width=200 height=200></a>\r\n"
    "   </th><th valign=center>\r\n    <h1>%s</h1>\r\n   </th></tr>\r\n  </table>\r\n"
    , label, label);
}
*/
 
char *expandCamelCase(char *in, char *out) {
// THIS IS FOR ASCII ONLY (sorry multilinguals)
  char *ret = out;
  *(out++) = *(in++);
  while(*in) {
    if((*in>='A')&&(*in<='Z')) *(out++) = ' ';
/*
    if(!(*in & ' ')) {
     *(out++) = ' ';
    }
*/
    *(out++) = *(in++);
  }
  *out = 0;
  return ret;
}
  
char* timeString(char* space) {
time_t tt = time(NULL);
  struct tm *now = localtime(&tt);
  sprintf(space,"%04d%02d%02d%02d%02d%02d",
    now->tm_year+1900, now->tm_mon+1, now->tm_mday,
    now->tm_hour, now->tm_min, now->tm_sec
  );
  return(space);
}

void cat(char *section, int escapeFrom) {
char buffer[1024];

  sprintf(buffer,"data/%sForm.html",section);
  try {
    FILE* page=fopen(buffer,"r");
    if (page) {
      buffer[0]=0;
      for(int c=fgetc(page);c!=EOF;c=fgetc(page)) {
        if (c!='<') {
          putchar(c);
        } else if(escapeFrom) {
            printf("&#060");
        } else {
          putchar('<');
        }
      }
      fclose(page);
    } else {
      printf("FAILED TO OPEN %s<br>\r\n", buffer);
    }
  } catch(int e) {
    printf("\n(FAILED TO READ)\n\n");
  }
}


string userName, userRole, userTeam;

void back(string thisPage, char *baseline) {
  char buffer[512], time[256];
  DataBrane *db = new DataBrane();

  sprintf(buffer, "queue/dbdump_%s", timeString(time));
  db->dumpToFile(buffer);
  delete db;

  sprintf(buffer, "diff queue/dbdump_%s queue/dbdump_%s", baseline, time);
  int status = system(buffer);
  printf("%s ended in %d<br/>\n", buffer, status);
}

void pick(string thisPage) { 	//, char *seZuhMe, char *verb) {

  string readReport = "    <li><a href=" + thisPage + "+read+";

  printf( 
    "  <table cellspacing=2><tr><td valign=top>\n"
    "   Execution of this utility creates a backup on a data queue.<br/>\n"
    "   You may select one of these older backups as a reference version.<br/>\n"
    "   <ul>\n");

  FILE* list = popen("ls -l queue/dbdump_*","r");
  char buffer[512];
  while(NULL != fgets(buffer,512,list)) {
    printf("<!--%s into -->", buffer);
    char* b = strtok(buffer,"-"); // preceeding chaff
    strtok(NULL," "); // permissions
    strtok(NULL," "); 
    strtok(NULL," "); //owner
    strtok(NULL," "); // group
    char* size = strtok(NULL," ");
    b = strtok(NULL,"q"); strtok(NULL,"_");
    char* a = strtok(NULL," \n");
    printf("<a href=%s+back+%s>%s (%s bytes)</a><br/>\n", thisPage.c_str(), a,b, size);
  }
  pclose(list);
}

int main(int argc, char **argv) {
Session *sess = new Session(argc,argv);
   
  sess->spewTop("MDRS Habitat Activity Logger (prototype)<br>Database Synchronization");

  if((argc<=2) || !strcmp(argv[2],"pick")) { pick(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"back")) { back(sess->getRefreshLink(), argv[3]); }
/*
  
  userName = sess->getUserName();
  userRole = sess->getUserRole();
  userTeam = sess->getUserTeam();

*/
  printf(" </body>\n"
         "</html>\n");
}
