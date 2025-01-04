#include <stdio.h>
#include "dataBrane.h"

const int LongestLine = 1024;
const int MaxDataSize = 8*1024;

      
void record(char* report, char* uid, char* label, char* content) {
  if(strchr(label,':')) *strrchr(label,':') = 0;
  if(strchr(content,0x0a)) *strrchr(content,0x0a) = 0;
  if(strchr(content,0x0d)) *strrchr(content,0x0d) = 0;
  printf("insert into reportContent (reportType, reportUid, label, content) values\n");
  printf("	('%s','%s','%s','%s');\n",report,uid,label,content);
}

void ingest(FILE* in, char* report, char* uid) {
char getBuffer[LongestLine];
char label[64];
char content[MaxDataSize];

  *content = 0;
  for(char* got=fgets(getBuffer,LongestLine,in);
      got==getBuffer; got=fgets(getBuffer,LongestLine,in)){

    int len = strlen(getBuffer);
    if(len <= 2) continue;
    if ( getBuffer[len-3] == ':') {
     // printf("header= %s",getBuffer);
      if(*content) record(report, uid, label, content);
      if(strlen(getBuffer)>=64) {
        fprintf(stderr,"FILE GARBLED. (way long label)\nAborting ingest of %s_%s\n",report,uid);
        return;
      }
      strcpy(label,getBuffer);
      *content = 0;
    } else {
    //  printf("(%d)%s",len,getBuffer);
      if(strlen(content)+strlen(getBuffer) < MaxDataSize) strcat(content,getBuffer);
      else fprintf(stderr,"Bufer overflow. truncating %s of %s_%s\n",label,report, uid);
    }
  }
  if(*content) record(report, uid, label, content);
}

int main(int argc, char** argv) {
  char buffer[512];
  if (argc < 2) {
    fprintf(stderr,"Must have a report file name as argument\n");
    return 0;
  }
  for(int argn=1;argn<argc;argn++) {
    FILE *in = fopen(argv[argn],"r");
    if (!in) {
      fprintf(stderr,"Failed to open %s\n", argv[1]);
    } else {
        char* fileName = (strchr(argv[argn],'/'))?strrchr(argv[1],'/')+1:argv[argn];
        char* report = strtok(strcpy(buffer,fileName),"_");
        char* dateTime = strtok(0,"_");
    	ingest(in, report, dateTime);
    	fclose(in);
    }
  }
}

