#include <iostream>
#include <vector>  
#include <string>  
#include <string.h>  
#include <stdio.h>  
#include <stdlib.h> 

#include <cgicc/CgiDefs.h> 
#include <cgicc/Cgicc.h> 
#include <cgicc/HTTPHTMLHeader.h> 
#include <cgicc/HTMLClasses.h>

#include "session.h"
#include "dataBrane.h"

using namespace std;
using namespace cgicc;

#include <time.h>
char* timeString(char* space) {
time_t tt = time(NULL);
  struct tm *now = localtime(&tt);
  sprintf(space,"%04d%02d%02d%02d%02d%02d",
    now->tm_year+1900, now->tm_mon+1, now->tm_mday,
    now->tm_hour, now->tm_min, now->tm_sec
  );
  return(space);
}

#define ENUF 10	// the MAXIMUM number of photos
char* missing = (char*)"Gone_Missing";
void spewForm(string program, char *now) {
int i;
  
  char* caption[ENUF];
  char* picture[ENUF];
  for (i=0;i<ENUF;i++) {
    caption[i] = missing+5;
    picture[i] = missing;
  }

  if(now != NULL) {
    char buffer[1024];
    char* now = strdup(timeString(buffer));
    sprintf(buffer,"queue/%s_dailyPictureInfo", now);
        
 
    FILE* pictUp = fopen(buffer,"r");
    while(NULL != fgets(buffer, 1024, pictUp) ) {
      //printf("[%s]<br>\n", buffer);

      i = -1;
      sscanf(buffer+4,"%d",&i);
      strtok(buffer,":");
      if((i<0)||(i>=ENUF))continue;

      if(!strncmp(buffer,"CAPT",4)) {
        caption[i] = strdup(strtok(0,"\n"));
        if(NULL == caption[i]) caption[i] = missing+5;
        //printf("==>CAPT %d:%s<br>\n",i,caption[i]);
      } else
      if(!strncmp(buffer,"PICT",4)) {
        char* value = strtok(0,"\n");
        if(value == NULL) picture[i] = missing;
	else picture[i] = strdup(value);
        //printf("-->PICT %d:%s<br>\n",i,picture[i]);
      } else {
        printf("-->UNEXPECTED %d<br>\n",i);
      }
    }
    fclose ( pictUp);
  }

  cout <<
    "  <form name=\"snag a pix\" action=\"" << program << "+INGEST\""
                       " method=\"post\" enctype=\"multipart/form-data\">\n"
    "   <input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\"1024\"/>\n"
    "   <center>\n"
    "    <table>\n"
    "     <tr><th colspan=2>Photo contribution for today</th></tr>\n"
    "     <tr>\n";

  for(i = 0; i < ENUF; i+=2 ) {
    if(NULL == strchr(picture[i], '/'))
      printf("      <td><input type=\"file\" name=\"PICT%d\" value=\"%s\" /></td>",
        i, strchr(picture[i],'_')+1);
    else {
      printf("      <td><img src=\"/picture%s\" ></td>",strrchr(picture[i],'/'));
    }

    if(NULL == strchr(picture[i+1], '/'))
      printf(      "<td><input type=\"file\" name=\"PICT%d\" value=\"%s\" /></td>\n",
        i+1, strchr(picture[i+1],'_')+1);
    else
      printf("      <td><img src=\"/picture%s\" ></td>",strrchr(picture[i+1],'/'));

    printf("     </tr><tr>\n");

    printf("      <td><input type=\"text\" name=\"CAPT%d\" value=\"%s\" /></td>", 
        i, caption[i]);
    printf(      "<td><input type=\"text\" name=\"CAPT%d\" value=\"%s\" /></td>\n", 
        i+1, caption[i+1]);
    printf("     </tr><tr>\n");
  }

  cout <<
    "     <td colspan=2 align=center>"
    "<input type=\"submit\" value=\"DO UPLOADS\"/></td>\n"
    "     </tr>\n"
    "    </table>\n"
    "   </center>\n"
    "  </form>\n";
}

char* readMultipartUpload(char* folder) {

  Cgicc cgi;

  char buffer[1024];
  char* now = strdup(timeString(buffer));
  char* today = strdup(now);
  today[8] = 0;

  sprintf(buffer,"%s/%s_dailyPictureInfo", folder, now);
  FILE* pictUp = fopen(buffer,"w");

  for(const_form_iterator thing = cgi.getElements().begin();
      thing != cgi.getElements().end(); thing++) {
    fprintf(pictUp,"%s: %s\n",
      thing->getName().c_str(), thing->getValue().c_str());
  }

  for(const_file_iterator pic = cgi.getFiles().begin();
      pic != cgi.getFiles().end(); pic++) {
    sprintf(buffer,"queue/%s_%s", now, pic->getFilename().c_str());
    fprintf(pictUp,"%s:%s\n",
      pic->getName().c_str(), buffer);
    ofstream capture;
    capture.open(buffer);
    pic->writeToStream(capture);
    capture.close();
    sprintf(buffer,"/usr/lib/cgi-bin/queue/%s_%s", now, pic->getFilename().c_str());

    char dothis[512];
    sprintf(dothis, "cp %s /var/www/picture/%s_%s.jpg\n",buffer,today,pic->getName().c_str());
    sprintf(dothis, "cp %s /var/www/picture/\n",buffer);
    int ignore = system(dothis);
  }
  fclose(pictUp);
  return now;
}

char* procSecondArg(int argc, char **argv, string refresh) {
bool doUpLoad = false;
  if(argc>2) {
    if(0==strcasecmp(argv[2],"INGEST")){
      doUpLoad = true;
    } else printf("(unexpected argument in url)<br>");
  }
  if (doUpLoad)
    return readMultipartUpload((char*)"queue");
  else
     return NULL;
}

int main(int argc, char **argv) {
char buffer[1024];
Session* session = new Session(argc, argv);
  session->spewTop();
  char* now = procSecondArg(argc,argv, session->getRefreshLink());
  spewForm(session->getRefreshLink(), now);
  session->spewEnd();
}
