#include <stdio.h>
#include <stdlib.h>	// for free of string mad by strdup
#include <string.h>
#include "dataBrane.h"

char* polishedText(char* space, const int maxlen, FILE* in) {
  char* got = fgets(space,maxlen,in);
  if(got == space) {
    int from,into;
    for(from=into=0;space[from];++from){
      switch ((unsigned char)space[from]) {
        case 0x0a:
        case 0x0d:
          // ignore newline and return
        break;
        case '+' :
          space[into++] = ' ';
        break;
        case '\'' :
          // just skip over single quotes (to avoid mistakening then as end of string tokens)
        break;
        case 0xc3:
          space[into++] = 'a'; // drop umplat over a
        break;
        default:
          space[into++] = space[from];
        break;
      }
    }
    space[into]=0;
    //for(from=into=0;space[from];++from)if(space[from]<=0)into++;
    //if(into) {
      //fprintf(stderr,"%s\n",space);
      //for(from=0;space[from];++from)fprintf(stderr,"%c",(space[from]<=0)?'^':'_');
      //fprintf(stderr,"\n");
    //}
    for(from=0;space[from];++from)if(space[from]<=0)space[from]='_';
  }
  return got;
}

void process(char* queueFile) {
  DataBrane *db = new DataBrane();
  char buff[8*1024];
  char tag[1024];
  char value[8*1024];

FILE* in = fopen(queueFile,"r");
  if(in == NULL) {
    fprintf(stderr,"Failed to open %s\n", queueFile);
    return;
  }
bool first = true;
char* reporTyp = strdup(strtok(strcpy(buff,strrchr(queueFile,'/')+1),"_"));
char* dateTime = strdup(strtok(0,"_"));
int itemUId;
long reportId = 0;
int line = 0;
int kMaxLen = 255;

  sscanf(dateTime, "%d", &itemUId);
  itemUId *= 1000;

  if(buff!=polishedText(buff,kMaxLen,in)) {
    fprintf(stderr,"Read failure on %s\n", queueFile);
    return;
  }
  if(strncmp("ReportText=",buff,11)) {
    fprintf(stderr,"BAD REPORT FORMAT in %s (%s)\n", queueFile, buff);
    return;
  }
  char* label = strdup(&buff[11]);
  char dateString[512], authorName[512];

  char* got = polishedText(dateString,kMaxLen,in);
  if(dateString != got) {
    fprintf(stderr,"Read failure on %s\n", queueFile);
    return;
  }
  got = polishedText(authorName,kMaxLen,in);
  if(authorName != got) {
    fprintf(stderr,"Read failure on %s\n", queueFile);
    return;
  }
  if(':' != dateString[strlen(dateString)-1]) {
    reportId = db->putReport(reporTyp, dateString, label, authorName);
  } else {
    fprintf(stderr,"BAD REPORT FORMAT in %s (%s)\n", queueFile, buff);
    return;
  }
    
  got = polishedText(buff,kMaxLen,in);
  if(buff != got) {
    fprintf(stderr,"Read failure on %s\n", queueFile);
    return;
  }

  if(':' != buff[strlen(buff)-1]) {
    first = false;
    strcpy(tag, "ReportText");
    *value = 0;
    line = 0;
  }

  for(;got==buff; got=polishedText(buff,kMaxLen,in)) {
    for(int index = strlen(buff)-1;(index>0) && ((buff[index]==0x0a)||(buff[index]==0x0d));
    buff[index--]=0);	// 
    if(':' == buff[strlen(buff)-1]) { // tag line
      buff[strlen(buff)-1] = 0; // remove token colon
      if(!first) {
        if(strlen(value)!=0)db->putReportItem(reportId, tag, line++, value);
      }

      first = false;
      strcpy(tag, buff);
      *value = 0;
      line = 0;
    } else {
      if(strlen(value)!=0)db->putReportItem(reportId, tag, line++, value);
      strcpy(value,buff);
    }
  }
      
  if(strlen(value)!=0)db->putReportItem(reportId, tag, line++, value);

  fclose(in);
  free(label);
}

int main(int argc, char **argv) {
 for(int a=1;a<argc;a++)process(argv[a]);
}

