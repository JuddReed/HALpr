#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "session.h"
#include "dataBrane.h"
#include "dataSummary.h"


using namespace std;

char* mailToAddress = (char*)"c2h2_o2@yahoo.com";
//string mailToAddress = "mdrs-mission-support@marssociety.org";

char TRANSMIT_QUEUE[] = "/usr/lib/cgi-bin/queue";

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

void days(string thisPage) {
#define NUF 1024
  char buf[NUF], ruf[NUF];
  char *what, *when;
  
  sprintf(buf,"ls %s | grep '_2'| sort -r -k 2 -t _", TRANSMIT_QUEUE);
  FILE* stream = popen(buf,"r");
  int before=0, today = 0;

  printf("   <table><tr><td valign=top>\n");
  printf("    <a href=%s+menu>Reports Menu Page</a><br>\n", thisPage.c_str());
  printf("    <br><br>\n"
         "    <a href=%s+water>Water Summary</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+lp>Propane Summary</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+heat>Heat Summary</a><br>\n", thisPage.c_str());
  printf("   </td><td>\n");
  printf("    <center><div class=\"scrollable\">\n");
  printf("    <table><tr><th>DATE</th><th colspan=5>Reports</th>\n");
  for(char* got=fgets(buf,NUF,stream);NULL!=got;got=fgets(buf,NUF,stream)) {
    //fputs(buf,stdout);
    what = strtok(buf,"_");
    when = strtok(000,"\n");
    //if(0==strcmp(what,"CommanderCheckIn"))continue;
    //if(0==strcmp(what,"Engineering"))continue;
    //if(0==strcmp(what,"HealthSafety"))continue;
    if(0==strcmp(what,"EngSupHVAC"))continue;
    if(0==strcmp(what,"EngSupWater"))continue;
    strcpy(ruf,when);
    ruf[8] = 0;
    sscanf(ruf,"%d",&today);
    if(today != before) {
      before = today;
      printf("      </tr><tr><th>%d/%d/%d</th>\n",
          today/10000, (today/100)%100, today%100);
    }
    printf("      <td><a href=%s+play+%s+%s>%s</a></td>\n",
                               thisPage.c_str(),   what,when,  what);
  }
  printf("    </tr></table></div></center>\n");
  pclose(stream);
  printf("   </td></tr></table>\n");
}

void play(string thisPage, char* report, char* uid) {
  printf("  <table><tr><td valign=top>\n");
  printf("    <a href=%s+menu>Return to Reports Page</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+days>Return to list of old reports</a><br>\n", thisPage.c_str());
  printf("  </td><td>\n");
  cout <<"   <form name=\"Report Text\" method=\"post\" action=\""
       << thisPage << "+yell+" << string(report) << "\">\n";

  cout <<
    "   <table><tr><td align=center>" 
    "     This is just a display of the submitted report.<br>\n"
    "     Edits will not be saved. <b>Editing here is pointless.</b><br>\n"
    "    </td></tr><tr><td>"
    "     <textarea name=\"ReportText\" cols=\"100\" rows=\"25\">\n";

  char wholeForm[8192], buffer[512];
  sprintf(buffer,"%s/%s_%s",TRANSMIT_QUEUE,report,uid);
  FILE *oldReport = fopen(buffer,"r");
  if(oldReport == NULL) 
     printf("failed to open %s \n",buffer);
  else {
    int len = fread(wholeForm,1,8192,oldReport);
    fclose ( oldReport);
  }

/*
  for(char *token=strtok(wholeForm,"&");token;token=strtok(0,"&")) {
    printf("token = %x [%c]\n", (int)token,*token);
    char *value = strchr(token,'=')+1;
    if(value!=NULL) {
      *(value-1)=0;
    }
    if(( *token >= 'a')&&( *token <= 'z')) *token ^= ' ';
      printf("%s:\n", expandCamelCase(token,buffer));
*/
      char *value;

      if(!strncasecmp(wholeForm,"ReportText",10))
        value = strchr(wholeForm,'=')+1;	// strip off leading "ReportText=
      else
        value = wholeForm;

      for(;*value;value++) {
        if(*value=='+')putchar(' ');
/*
        else if(*value=='%') {
          ++value;
          int letter;
          sscanf(value,"%02x",&letter);
          ++value;
          putchar(letter);
        }
*/
        else putchar((int)(*value));
      }
/*
    putchar('\n');
  }
*/
 
  cout <<
    "     </textarea>\n"
    "    </td></tr></table>\n"
    "   </form>\n"
    "  </td></tr></table>\n"
    " </body>\n"
    "</html>\n";
}

string userName, userRole, userTeam;

void tell(string thisPage, char* report) {
  char filePath[1024], space[2048];
  sprintf(filePath,"%s/%s_%s", TRANSMIT_QUEUE, report, timeString(space));

  cout <<
    "  <center>\n"
    "   <br><br><br>\n";
  try {
    FILE* queue = fopen(filePath,"w");
    if(queue) {
      for(int c=fgetc(stdin);c!=EOF;c=fgetc(stdin)) {
        if(c=='%')int x=scanf("%02x",&c);
        fputc(c,queue);
      }
      fclose(queue);
      sprintf(space,"stat --format=\"%s\" %s", "%s", filePath);
      queue = popen(space, "r");
      int size;
      if(1==fscanf(queue,"%d",&size)){
        pclose(queue);
        printf("<br><br>%d bytes queued to send to earth.<br>\r\n", size);
      } else cout << "   <H1>(FAILED TO SEND REPORT TO EARTH)</h1>";
    } else cout << "   <H1>(FAILED TO SEND REPORT TO EARTH)</h1>";
  } catch(int e) {
    cout << "   <H1>(FAILED TO SEND REPORT TO EARTH)</h1>";
  }
  cout <<
    "   <table><tr><td>Go To<ul>\n"
    "    <li><a href=" <<thisPage<< "+menu>Reports Menu.</a></li>\n"
    "    <li><a href=/index.html>Home Page</a></li>\n"
    "   </ul></td></tr></table>\n"
    "  </center>\n"
    " </body>\n"
    "</html>\n";
}

void send(string thisPage, char* report) {
  char filePath[1024], space[2048];
  FILE* queue;
  sprintf(filePath,"%s/%s_%s", TRANSMIT_QUEUE, report, timeString(space));
  try {
    queue = fopen(filePath,"w");
  } catch(int e) {
    printf( "   <H1>(FAILED TO SAVE REPORT)</H1>\n");
  }
  char wholeForm[8192], buffer[512];
  int size = fread(wholeForm,1,8192,stdin);
  printf("%d bytes reported.<br>\n", size);
  printf(
    "  <center><table><tr><td>" 
    "     The following report has been recorded as displayed below.<br/>\n"
    "     You may forward it on to mission support if it is correct.<br/>\n"
    "     Edits applied on this page will not be saved.<br>\n"
    "     Instead, you should go back and edit data on the form.\n"
    "  </td></tr><tr><td>\n");
  printf(
    "    <a href=%s+menu>Return to Reports Menu</a>\n", thisPage.c_str());
  printf(
    "  </td><td align=right>\n"
    "  <form name=\"Report Text\" "
    "method=\"post\" action=\"mailto:%s?Subject=%s\" ENCTYPE=\"text/plain\"> \n",
                                 mailToAddress, report);
  printf(
    "     <input type=\"submit\" value=\"SEND TO MISSION SUPPORT\"/><br>\n"
    "  </td></tr><tr><td colspan=2>\n"
    "     <textarea name=report cols=\"100\" rows=\"30\">\n");


  for(char *token=strtok(wholeForm,"&");token;token=strtok(0,"&")) {
    char *value = strchr(token,'=')+1;
    *(value-1)=0;
    if( *token > 'Z') *token ^= ' ';
    if(0==strcmp(token,"Body")) {
      fprintf(queue,"\n");
      printf("\n");
    } else if(0!=strcmp(token,"ReportType")*strcmp(token,"DateTime")*strcmp(token,"WrittenBy")) {
      fprintf(queue,"%s:\n", expandCamelCase(token,buffer));
      printf("%s:\n", expandCamelCase(token,buffer));
    }
    int letter;
    for(;*value;value++) {
      letter = (int)(*value);
      if(*value=='%') {
        ++value;
        sscanf(value,"%02x",&letter);
        ++value;
      } else if(*value=='+') {
        letter = 0x020;
      }
      putc(letter,queue);
      putc(letter,stdout);
    }
    putc('\n',queue);
    putc('\n',stdout);
  }
 
  cout <<
    "     </textarea>\n"
    "    </td></tr></table>\n"
    "   </form>\n"
    "  </center>\n"
    " </body>\n"
    "</html>\n";
}

void edit(string thisPage, char* report) {
  cout <<
    "  <center>\n"
    "   <form name=\"Report Form Editor\" method=\"post\" action=\"";
  cout << thisPage << "+save+" << string(report) << "\">\n";
  cout <<
    "    <input type=\"submit\" value=\"Save\"/><br>\n"
    "    <textarea name=\"ReportForm\" cols=\"100\" rows=\"30\">\n";
  cat(report,1);
  cout <<
    "    </textarea><br>\n"
    "   </form>\n"
    "  </center>\n";
}

void save(string thisPage, char* report) {
  char buffer[512];
  char uid[32];
  timeString(uid);
  sprintf(buffer,"data/%sForm.html.%s",report, uid);

  cout <<
    "  <center>\n"
    "  <table border=2 cellspacing=2>\n"
    "   <tr><th>\n"
    "    <a href=" <<thisPage<< "+doit+" <<string(report)<<"+"<<string(uid)<<
						 ">CONFIRM<a> your edits,\n"
    "    <a href=" <<thisPage<< "+edit+" <<string(report)<<
						 ">RESUME<a> editing without change,\n"
    "    <a href=" <<thisPage<< "+read+" <<string(report)<< 
						">ABANDON<a> edits to original form.<br>\n"
    "   </th></tr><tr><td>\n"
    "    <div class=\"scrollable\">\n";

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
    cout << "<br><br><center><H1>(FAILED TO SAVE)</h1>";
  }
  cout << 
    "    </div>\n"
    "   </td></tr>\n"
    "  </table>\n";
}

void bannerSign() {
  cout <<
    "     <table><tr>\n"
    "      <th>Date"/*" and Time"*/":</th><td><textarea name=\"dateTime\" "
    "rows=\"1\" cols=\"16\" >";

  time_t tt = time(NULL);
  struct tm *now = localtime(&tt);
  //printf("%02d:%02d ", now->tm_hour, now->tm_min);
  printf("%02d/%02d/%04d", now->tm_mon+1, now->tm_mday, now->tm_year+1900);

  cout << "</textarea></td>\n"
    "      <th>Author"/*" (name, role, team)"*/"</th><td><textarea name=\"writtenBy\" "
    "rows=\"1\" cols=\"50\">" << userName
//  << ", " << userRole << " of " << userTeam << " team."
    << "</textarea></td></tr>\n"
    "    </table>\n";
}

void view(string thisPage, char* page) {
  cout <<
    "  <center>\n"
    "   <form name=\"" << string(page) << "\" method=\"post\" action=\"";
  cout << thisPage << "+send+" << string(page) << "\">\n";
  cout <<"    <input type=\"submit\" value=\"Submit\" />\n";

  cout <<"    <a href=" << thisPage << "+menu>Abort to reports menu.</a>\n";
  cout <<"    <table border=\"2\" cellspacing=\"2\"><tr><td colspan=2><div class=\"scrollable\">\n";
  cout <<"     <input type=hidden name=ReportType value=\""+string(page)+" Report\"/>\n";

  bannerSign();
  cat(page,0);
  cout <<"\n"
         "   </td></tr><tr><td align=LEFT>\n";
  cout <<"    <input type=\"submit\" value=\"Submit for Earth Transmission\"/><br>\n";
  cout <<"   </td><td align=RIGHT>\n";
  cout <<"    <a href=" << thisPage << "+edit+" << string(page) <<">Edit</a>\n";
  cout <<"   </td></tr></table>\n";
  cout <<"  </center>\n";
}

void doit(string thisPage, char *section, char *uid) {
  char buffer[512];
  sprintf(buffer, "mv data/%sForm.html data/%sForm.before_%s", section, section, uid);
  int status = system(buffer);
  sprintf(buffer, "mv data/%sForm.html.%s data/%sForm.html", section, uid, section);
  status = system(buffer);
  view(thisPage, section);
}

void menu(string thisPage) { 	//, char *seZuhMe, char *verb) {

  string readReport = "    <li><a href=" + thisPage + "+read+";

  cout << 
    "  <table cellspacing=2><tr><td valign=top>\n"
    "   Reports are to be sent daily before 8:00 PM local time.<br/>\n"
    "   If they have not been sent before 9:00 PM they risk not being accepted.<br/>\n"
    "   See the <a href=/cgi-bin/quickGuideWrapper"
    + string(strchr(thisPage.c_str(),'?')) + "+14+read>Reports Quick Guide\n"
    "   </a> for more information.\n"
    "   <ul>\n";

    cout << readReport << "CommanderCheckIn>Commander Check-In Report</a></li>\n";
    cout << readReport << "Commander>Commander Report</a></li>\n";
    cout << readReport << "Engineering+read>Engineering Report</a></li>\n";
    cout << readReport << "Eva>EVA Report</a></li>\n";
    cout << readReport << "EvaPlan>Future EVA Plan</a></li>\n";
    cout << readReport << "GreenHab>Green Hab Report</a></li>\n";
    cout << readReport << "HealthSafety>Health & Safety Officer Report</a></li>\n";
    cout << readReport << "Journalist>Journalist Report</a></li>\n";
    cout <<
    "   </ul>\n";
    cout << "  </td></tr></table>\n";

    //printf("  <a href=photoUpload%s>Attach Photos</a><br>\n",strchr(thisPage.c_str(),'?'));
    printf("  <a href=%s+days>Review Some old Reports</a>\n",thisPage.c_str());
}

int main(int argc, char **argv) {
Session *sess = new Session(argc,argv);
   
  sess->spewTop("MDRS Habitat Activity Logger (prototype)<br>Daily Report Entry");
  
  userName = sess->getUserName();
  userRole = sess->getUserRole();
  userTeam = sess->getUserTeam();

  cout << " <p>\n";

       if(!strcmp(argv[2],"menu")) { menu(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"read")) { view(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"save")) { save(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"doit")) { doit(sess->getRefreshLink(), argv[3], argv[4]); }
  else if(!strcmp(argv[2],"edit")) { edit(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"send")) { send(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"tell")) { tell(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"days")) { days(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"play")) { play(sess->getRefreshLink(), argv[3], argv[4]); }
  else
    printf("  Return to <a href=/reportGenHome.html>index</a>.<br>\r\n");
  printf(" </body>\r\n");
  printf("</html>\r\n");
}
