#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dataBrane.h"
#include "session.h"

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

string getAssestProperty(char *section, int which) {
  if(0==strcmp(section,"menu"))return "Choose your";
int assetId = 0;
  sscanf(section,"%d",&assetId);
  DataBrane *db = new DataBrane();
  DataBrane::DATA_ASSET *guide = db->getDataAssetById(assetId);
  if(which == 0)
    return guide->label;
  else
    return guide->path;
}

char* getEmailLink(char* tag, char* val) {
TYPE_OF_CONTACT contactType = CONTACT_PERSON;
       if(!strcasecmp(tag, "person")) { contactType = CONTACT_PERSON; }
  else if(!strcasecmp(tag, "group"  )) { contactType = CONTACT_GROUP; }
  else {
    return strdup("href=http://mdrs.marssociety.org/");
  }

  DataBrane *db = new DataBrane();
  string url = db->getEmailUrl(contactType, string(val));
  char* link = strdup(url.c_str());
  return link;
}

enum CAT_MODE {EDIT,SHOW};
void cat(char *section, CAT_MODE mode) {

  try {
    FILE* page=fopen(getAssestProperty(section, 1).c_str(), "r");
    if (page) {
      char buffer[1024];
      buffer[0]=0;
      for(int c=fgetc(page);c!=EOF;c=fgetc(page)) {
        if (c!='<') {
          putchar(c);
        } else if(mode==EDIT) {
          printf("&#060");
        } else {
          int n = fscanf(page,"%s",buffer);
          if(0!=strncasecmp(buffer,"HAL",3)) {
            printf("<%s",buffer);
          } else {
            char* cp = buffer;
            for(c=fgetc(page);c!=EOF;c=fgetc(page))
              if((c==' ')||(c=='	')||(c=='\n')||(c=='\r'))break;
            for(*cp++=c;c!=EOF;c=fgetc(page)) {
              if((c==' ')||(c=='	'))continue;
              if((c=='\n')||(c=='\r'))continue;
              if((c==',')||(c=='>'))break;
              *cp++=c;
	      *cp=0;
	    }
            char* tag = strtok(buffer,"= ,	\n\r");
            char* val = strtok(0,"= ,	\n\r");
            printf("<a %s", getEmailLink(tag,val));
            putchar(c);
          }
        }
      }
      fclose(page);
    } else {
      printf("FAILED TO OPEN %s<br>\r\n", section);
    }
  } catch(int e) {
    printf("(FAILED TO READ)");
  }
}

//TODO:For now verb is a no-op with implied meaning of display. 
void menu(string program, char *verb) {
  DataBrane *db = new DataBrane();

  DataBrane::DATA_ASSET *guides = db->getQuickGuides();
  int size = db->quickGuideCount();
    
  cout
    <<"  <center>\n"
      "   Menu of Quick Guides"
      "   <table border=2 cellspacing=2>\n"
      "    <tr><td>\n"
      "     <div class=\"scrollable\">\r\n";

  for(int i=0;i<size;i++) {
    cout <<
      "   <li><a href=" << program << "+" << guides[i].uid << "+read>" 
      << guides[i].label << "</a></li>\n";
  }

  cout
    <<"     </div>\n"
      "    </td></tr>\n"
      "    <tr><td align=right>\n"
    //"     <a href="+program+"+"+page+"+edit>(edit this page)</a>"
      "    </td></tr>\n"
      "   </table>\r\n";
}

string userName;

void view(string program, char *page) {
  cout
    <<"  <center>\n"
      "   <a href=" << program << "+menu+read>Return to Quick Guide menu.</a>\n"
      "   <table border=2 cellspacing=2>\n"
      "    <tr><td>\n"
      "     <div class=\"scrollable\">\r\n";
  cat(page, SHOW);
  cout
    <<"     </div>\n"
      "    </td></tr>\n"
      "    <tr><td align=right>\n"
      "     <a href="+program+"+"+page+"+edit>(edit this page)</a>";
      "    </td></tr>\n"
      "   </table>\r\n";
}

void edit(string program, char *page) {
  printf(
    "   <center>\n"
    "    <form name=\"Quick Guide Editor\" method=\"post\" action=\"");
  cout << program;
  cout << "+" << string(page);
  cout << "+save\">\n";
// + "+" + \"/cgi-bin%s?%s+save\">\r\n", strrchr(thisPage,'/'), page);
  printf("    <input type=\"submit\" value=\"Save\"/><br>\r\n"
    "     <textarea name=\"QuickGuide\" cols=\"100\" rows=\"30\">\r\n");
  cat(page, EDIT);
  printf("     </textarea><br>\r\n   </form>\r\n   </center>\r\n");
}

void save(string program, char *section) {
  char uid[32];
  timeString(uid);

  char buffer[512];
  sprintf(buffer,"%s.%s", getAssestProperty(section, 1).c_str(), uid);

  cout << 
    "  <center>\n"
    "  <table border=2 cellspacing=2>\n"
    "   <tr><th>\n"
    "    <a href=" << program << "+"
      << section << "+doit+" << string(uid) <<">CONFIRM</a> your edits,\n"
      << "    <a href=" << program << "+" << section
      << "+edit>RESUME</a> editing without change,\n"
    "    <a href=" << program << "+" << section << "+read>ABANDON</a>"
    " changes and return to quide<br>\n"
    "   </th></tr><tr><td>\r\n"
    "    <div class=\"scrollable\">\r\n";

  for(int c=fgetc(stdin);c!=EOF;c=fgetc(stdin)) {if(c=='=')break;}
  
  try {
    FILE* page=fopen(buffer,"w");
    for(int c=fgetc(stdin);c!=EOF;c=fgetc(stdin)) {
      if(c=='+')c=' ';
      if(c=='%')int x=scanf("%02x",&c);
      putchar(c);
      fputc(c,page);
    }
    fclose(page);
  } catch(int e) {
    printf("<br><br><center><H1>(FAILED TO SAVE)</h1>");
  }
  printf(
    "    </div>\r\n"
    "   </td></tr>\r\n"
    "  </table>\r\n");
}

void doit(string program, char *section, char *uid) {
  char buffer[1024];
  string filePath = getAssestProperty(section, 1);
  sprintf(buffer, "mv %s %s.before_%s", filePath.c_str(), filePath.c_str(), uid);
  int status = system(buffer);
  sprintf(buffer, "mv %s.%s %s", filePath.c_str(), uid, filePath.c_str());
  status = system(buffer);
  view(program, section);
}

int main(int argc, char **argv) {
Session *session;
  if(argc<4) return 0;

  session = new Session(argc,argv);
  session->spewTop("MDRS - " + getAssestProperty(argv[2], 0) + " Quick Guide");

  string program = session->getRefreshLink();
  userName = session->getUserName();

  if(!strcmp(argv[2],"menu")) {
    cout
      << "    Go to <a href="
      << session->homeInSession() << "?" << session->getSessionArg()
      << ">H.A.L. root</a><br/>\n";
    menu(program, argv[3]);
  } else if(!strcmp(argv[3],"read")) {
    view(program, argv[2]);
  } else if(!strcmp(argv[3],"edit")) {
    edit(program, argv[2]);
  } else if(!strcmp(argv[3],"save")) {
    save(program, argv[2]);
  } else if(!strcmp(argv[3],"doit")) {
    doit(program, argv[2],argv[4]);
  } else
    cout << 
      "  Return to <a href=" << session->homeInSession() << ">index</a>.<br>\n"
      "  </center>\r\n";
}
