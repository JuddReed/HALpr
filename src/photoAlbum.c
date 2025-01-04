#include <stdlib.h>
#include <stdio.h>
#include <string.h>
  
const int COLUMNS = 8;

void spewHead(){
  printf(
    "Content-type: text/html\n\n"
    "<html lang=\"en-US\" xml:lang=\"en-US\" xmlns =\"http://www.w3.org/1999/xhtml\">\n"
    " <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\" />\n"
    " <head>\n"
    "  <title>Habitat Activity Photo Album</title>\n"
    "  <style>\n"
    "   .scrollable {\n"
    "     width:900;\n"
    "     height:360;\n"
    "     overflow: auto;\n"
    "   }\n"
    "  </style>\n"
    " </head>\n"
    " <body>\n"
    "  <table>\n"
    "   <tr>\n"
    "    <th>\n"
    "     <a href=/cgi-bin/home?sess:21>\n"
    "      <img src=/haleye.gif width=200 height=200>\n"
    "     </a>\n"
    "    </th>\n"
    "    <th valign=center>\n"
    "     <h1>MDRS Habitat Activity Logger (prototype)<br>Photo Album</h1>\n"
    "    </th>\n"
    "   </tr>\n"
    "  </table>\n"
    " <p>\n"
  );
}

const int MAX_PIC = 64;
void spewBody() {
char file[MAX_PIC][1024], capt[MAX_PIC][1024], buf[1024], day[1024];
  for(int i=0;i<16;i++) {
    *file[i] = 0;
    sprintf(capt[i], "(caption is missing)");
  }
  int did = 0;
  FILE *list = popen(
    "grep ':' /usr/lib/cgi-bin/queue/*_dailyPictureInfo |"
    " sed -e 's/_dailyPictureInfo//'|sort -r", "r");

  printf("  <center><div class=\"scrollable\">\n");
  printf("  <table>\n");
  *buf = 0;
  for(char* got = fgets(buf,1024,list); got ; got = fgets(buf,1024,list)) {
    if(strncmp(day,strchr(buf,'2'),8)) { // starting new day
      if(did) {
        printf("   <tr><td colspan=6>%d pictures from %s</td></tr>\n   <tr>\n",did,day);
        for(int i=0;i<MAX_PIC;i++)if(*file[i]) {
          printf("    <td><a href=\"/picture/%s\">"
                 "<img src=\"/picture/%s\" width=\"128\"><br/>%s</a></td>\n",
            file[i], file[i], capt[i]);
          *file[i] = 0;
          sprintf(capt[i], "(caption is missing)");
        }
        printf("   </tr>\n");
      }
      strncpy(day,strchr(buf,'2'),8); day[8]=(char)0;
      did = 0;
    }
    strtok(got,":");
    char* tag = strtok(000,":");
    char* val = strtok(000,":");
    int col; sscanf(&tag[4],"%d", &col);
    if(strlen(val)>3) {
      val[strlen(val)-1]=0;//trim off newline at end
      if(!strncmp("PICT",tag, 4))if(0 < sscanf(strrchr(val,'/')+1,"%s", file[col]))
      {
        //printf("cp /usr/lib/cgi-bin/%s /var/www/picture/",val);
        //sprintf(buf,"cp /usr/lib/cgi-bin/%s /var/www/picture/",val);
        //int i = system(buf);
        ++did;
      }
      if(!strncmp("CAPT",tag, 4)){
        //sscanf(val,"%s", capt[col]);
        if(strcmp(val," Missing"))strcpy(capt[col], val);
        //printf("%d:(%s) makes %s\n",col,val,capt[col]);
      }
    }
    *got = 0;
  } 
  pclose(list);
      
  if(did)
  {
        printf("   <tr><td colspan=6>%d pictures from %s</td></tr>\n   <tr>\n",did,day);
        for(int i=0;i<did;i++)if(*file[i]) {
          if((0!=i)&&(!(i%COLUMNS)))printf("</tr><tr>\n");
          printf("    <td><a href=\"/picture/%s\">"
                 "<img src=\"/picture/%s\" width=\"128\"><br/>%s</a></td>\n",
            file[i], file[i], capt[i]);
          *file[i] = 0;
          sprintf(capt[i], "(caption is missing)");
        }
    printf("   </tr>\n");
  }
  printf("  </table>\n");
  printf("  </div></center>\n");
}

void spewTail() {
  printf(" </body>\n");
  printf("</html>\n");
}

int main() {
  spewHead();
  spewBody();
  spewTail();
  return 0; // to avoid spurrious error report
}

