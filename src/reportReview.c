#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "session.h"
#include "dataBrane.h"
#include "dataSummary.h"


using namespace std;

char TRANSMIT_QUEUE[] = "/usr/lib/cgi-bin/queue";
char EMAIL_ADDRESS[] = "juddereed@gmail.com";	// someone on earth

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

/****************8
void sideMenu(string thisPage) {
  printf("    <a href=%s+menu>Reports Menu</a>\n", thisPage.c_str());
  printf("    <a href=%s+days>Old Reports</a>\n", thisPage.c_str());
}

void heat(string thisPage) {
  sideMenu(thisPage);
  int numReading = loadHvacLogs();
  spewHeatSvg(stdout, numReading);
}

void propane(string thisPage) {
  sideMenu(thisPage);
  int numReading = loadPropaneLog();
  spewLpSvg(stdout, numReading);
}

void diesel(string thisPage) {
  sideMenu(thisPage);
  int numReading = loadDieselLog();
  spewDieSvg(stdout, numReading);
}

void network(string thisPage) {
  sideMenu(thisPage);
  int numReading = loadDownloadLog();
  spewDownSvg(stdout, numReading);
}

void satSignal(string thisPage) {
  sideMenu(thisPage);
  int numReading = loadSignalLog();
  spewSignalSvg(stdout, numReading);
}

void water(string thisPage, int period) {
  sideMenu(thisPage);
  int numReading = loadWaterLogs();
  spewSvg(stdout, numReading, period);
}
**************************/

void days(string thisPage) {
#define NUF 1024
  char buf[NUF], ruf[NUF];
  char *what, *when;
  char evaPlanner[1024];
  char habMonitor[1024];
  char photoAlbum[1024];
  strcpy(evaPlanner, thisPage.c_str());
  *strrchr(evaPlanner,'/') = 0;
  strcat(evaPlanner,"/routePlanner");
  strcat(evaPlanner,strchr(thisPage.c_str(),'?'));
  
  strcpy(habMonitor, thisPage.c_str());
  *strrchr(habMonitor,'/') = 0;
  strcat(habMonitor,"/temperatureGraph");
  strcat(habMonitor,strchr(thisPage.c_str(),'?'));

  strcpy(photoAlbum, thisPage.c_str());
  *strrchr(photoAlbum,'/') = 0;
  strcat(photoAlbum,"/photoAlbum");
  strcat(photoAlbum,strchr(thisPage.c_str(),'?'));
  
  sprintf(buf,"ls %s | grep '_2'| sort -r -k 2 -t _", TRANSMIT_QUEUE);
  FILE* stream = popen(buf,"r");
  int before=0, today = 0;

  printf("   <table><tr><td valign=top>\n");
  printf("    <a href=reportGenWrapper%s+menu>Reports menu Page</a>\n",
      strchr(thisPage.c_str(),'?'));
/*********************************
  printf("    <br><br>\n"
         "    <a href=%s+water>Water Summary</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+lp>Propane Summary</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+netw>Download Summary</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+satsig>Satellite Signal</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+fuel>Generator Fuel Summary</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+heat>Heat Summary</a><br>\n", thisPage.c_str());
  printf("    <br><br>\n"
         "    <a href=%s>photoAlbum</a><br>\n", photoAlbum);
************************************/
  printf("   </td><td>\n");
  printf("    <center><div class=\"scrollable\">\n");
  printf("    <table><tr><th>DATE</th><th colspan=5>Reports</th>\n");

/*********************
  for(char* got=fgets(buf,NUF,stream);NULL!=got;got=fgets(buf,NUF,stream)) {
    //fputs(buf,stdout);
    what = strtok(buf,"_");
    when = strtok(000,"\n");
    //if(0==strcmp(what,"CommanderCheckIn"))continue;
    if(0==strcmp(what,"fulldump"))continue;
    if(0==strcmp(what,"DbDump"))continue;
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
    char* theTool = NULL;
    if(!strcmp(what,"EvaPlan")) {
      theTool = evaPlanner;
    } else if(!strcmp(what,"Monitor")) {
      theTool = habMonitor;
    }
    printf("      <td><a href=%s+play+%s+%s>%s</a></td>\n",
        (theTool)?theTool:thisPage.c_str(),   what,when,  what);
  }
************************/
  DataBrane *db = new DataBrane();
  DataBrane::REPORT *report = db->getReportSet();
  int numReport = db->reportCount();
  char todayString[32];
  for(int r=0; r<numReport;++r) {
    if(strcmp(todayString, report[r].report_date.c_str())) {
      strcpy(todayString, report[r].report_date.c_str());
      char justday[32];
      strcpy(justday,todayString);
      justday[4]='/';
      justday[7]='/';
      justday[10]=0;
      printf("      </tr><tr><th>%s</th>\n",justday);
    }
    printf("      <td><a href=%s+play+%ld>%s</a></td>\n",
        thisPage.c_str(),   report[r].report_id, 
        report[r].report_type.c_str()); 

  }

/***********************/
  printf("    </tr></table></div></center>\n");
  pclose(stream);
  printf("   </td></tr></table>\n");
}

/***************************
void plan(char* inText) {
printf("THE REST IS ALL EVA PLAN<br/>\n");
      for(;*inText;inText++) {
        if(*inText=='+')putchar(' ');
        else putchar((int)(*inText));
      }
}
****************************/

void play(string thisPage, char* uid) {
  printf("  <table><tr><td valign=top>\n");
  printf("    <a href=reportGenWrapper%s+menu>Return to Reports Page</a><br>\n", thisPage.c_str());
  printf("    <a href=%s+days>Return to list of old reports</a><br>\n", thisPage.c_str());
  printf("  </td><td>\n"
         "   <form name=\"Report Text\" method=\"post\" "
         "action=\"mailto:%s\" ENCTYPE=\"text/plain\">\n", EMAIL_ADDRESS);

  cout <<
    "   <table><tr><td align=center>" 
    "     This is just a display of the submitted report.<br>\n"
    "     Edits will not be saved. <b>Editing here is pointless.</b><br>\n"
  //"     You may <input type=\"submit\" value=\"email it to earth\"/><br>\n"
    "    </td></tr><tr><td>"
    "     <textarea name=\"ReportText\" cols=\"100\" rows=\"25\">\n";

  DataBrane *db = new DataBrane();
  DataBrane::REPORT *report = db->getReportSet(uid);
  printf("%s\n",report[0].label.c_str());
  printf("%s\n",report[0].report_date.c_str());
  printf("%s\n",report[0].author.c_str());
  DataBrane::REPORT_ITEM *reportItem = db->getReportItemSet(uid);
  int numReportItem = db->reportItemCount();
  for(int i=0;i<numReportItem;++i) {
    if(0==reportItem[i].line)printf("%s:\n",reportItem[i].key.c_str());
    printf("%s\n",reportItem[i].value.c_str());
  }

  cout <<
    "     </textarea>\n"
    "    </td></tr></table>\n"
    "   </form>\n"
    "  </td></tr></table>\n"
    " </body>\n"
    "</html>\n";
}

string userName, userRole, userTeam;
/****************************************************
void tell(string thisPage, char* report) {
  char filePath[1024], space[2048], time[1024];
  sprintf(filePath,"%s/%s_%s", TRANSMIT_QUEUE, report, timeString(time));

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
  cout << "   <table><tr><td>Go To<ul>\n";
  if(strstr(report,"vaPlan")) { // its an eva plan so show us the map
     strcpy(space,thisPage.c_str()); 
     char *rest = strrchr(space,'?');
     *strrchr(space,'/') = 0;
     printf("    <li><a href=%s/routePlanner%s+play+%s+%s>Review Posted Report</a></li>\n",
      space, rest, report, time);
  } else printf(
    "    <li><a href=%s+play+%s>Review Posted Report</a></li>\n",
      thisPage.c_str(), report, time);
  cout <<
    "    <li><a href=" <<thisPage<< "+menu>Reports Menu.</a></li>\n"
    "    <li><a href=/index.html>Home Page</a></li>\n"
    "   </ul></td></tr></table>\n"
    "  </center>\n"
    " </body>\n"
    "</html>\n";
}
*****************************/

void send(string thisPage, char* report) {
  cout <<
    "  <center>\n"
    "   <form name=\"Report Text\" method=\"post\" action=\""
/**
    << thisPage << "+tell+" << string(report) << "\">\n";
/*/
    << thisPage << "+tell+" << string(report) << "\">\n";
/**/

  cout <<
    "   <table><tr><td>" 
    "     The purpose of this page is to provide formatting as required.<br>\n"
    "     <b>Editing on this page is not recommended.</b><br>\n"
    "     Instead, you should go back and edit data on the form.\n"
    "<br><b>Copy the content of the box below and paste it into the body of an email.</b>\n"
    "    </td></tr><tr><td>"
    "     <input type=\"submit\" value=\"RECORD\"/><br>\n"
    "     <textarea name=\"ReportText\" cols=\"100\" rows=\"30\">\n";

  char wholeForm[8192], buffer[512];
  int len = fread(wholeForm,1,8192,stdin);

  for(char *token=strtok(wholeForm,"&");token;token=strtok(0,"&")) {
    char *value = strchr(token,'=')+1;
    *(value-1)=0;
    if( *token > 'Z') *token ^= ' ';
    if(0==strcmp(token,"Body"))printf("\n");
    else if(0!=strcmp(token,"ReportType")*strcmp(token,"DateTime")*strcmp(token,"WrittenBy"))
      printf("%s:\n", expandCamelCase(token,buffer));
    for(;*value;value++) {
      if(*value=='%') {
        ++value;
        int letter;
        sscanf(value,"%02x",&letter);
        ++value;
        putchar(letter);
      } else if(*value=='+')putchar(' ');
      else putchar((int)(*value));
    }
    putchar('\n');
  }
 
  cout <<
    "     </textarea>\n"
    "    </td></tr></table>\n"
    "   </form>\n"
    "  </center>\n"
    " </body>\n"
    "</html>\n";
}

/********************************************
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
						 ">RESTART<a> editing without change,\n"
    "    <a href=" <<thisPage<< "+fill+" <<string(report)<< 
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
*************************************/

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

void parseWaypointList() {
printf("<h2>Points of Interest</h2>\n"
       "<table>\n"
       " <tr><th>Point</th>\n"
       "  <th>Distance</th>\n"
       "  <th>Easting</th>\n"
       "  <th>Northing</th>\n"
       "  <th>Point of Interest</th>\n"
       "  <th>Purpose of Visit</th>\n"
       " </tr>");
const int ENUF = 1024*8;
char buff[ENUF];
  int pn = 0;
  while( char* got = fgets(buff,ENUF,stdin)) {
    char* wp = strtok(buff,"&");
    int he,hn; char label[512];
    sscanf(wp,"waypoint=%d_%d_%s",&he,&hn,label);
    for(got=label;*got;++got)if(*got=='+')*got=' ';
    printf(" <tr>\n"
           "  <th>%d</th>\n"
           "  <td><input type=\"hidden\" width=5 name=\"point%dAway\" value=\"%d\" />%d</td>\n"
           "  <td><input type=\"hidden\" width=5 name=\"point%dEast\" value=\"%d\" />%d</td>\n"
           "  <td><input type=\"hidden\" width=5 name=\"point%dNorth\" value=\"%d\" />%d</td>\n"
           "  <td>%s</td>\n"
           "  <td><input type=\"hidden\" width=100 name=\"purpose%d\" value=\"home\" /></td>\n"
            " </tr>\n",
        pn,  pn,0,0, pn,he,he, pn,hn,hn, label, pn);
      ++pn;
    for(wp=strtok(0,"&");wp;wp=strtok(0,"&")) {
      int e,n;
      sscanf(wp,"waypoint=%d_%d_%s",&e,&n,label);
      for(got=label;*got;++got)if(*got=='+')*got=' ';
      int away = (int)hypot((double)(e-he), (double)(n-hn));
      printf(" <tr>\n"
             "  <th>%d</th>\n"
             "  <td><input type=\"hidden\" width=5 name=\"point%dAway\" value=\"%d\" />%d</td>\n"
             "  <td><input type=\"hidden\" width=5 name=\"point%dEast\" value=\"%d\" />%d</td>\n"
             "  <td><input type=\"hidden\" width=5 name=\"point%dNorth\" value=\"%d\" />%d</td>\n"
             "  <td>%s</td>\n"
             "  <td><input type=\"text\" width=100 name=\"purpose%d\" /></td>\n"
             " </tr>\n",
          pn,  pn,away,away, pn,e,e, pn,n,n, label, pn);
      ++pn;
    }
  }
  printf("<table>\n");
}

/*******************************
void fill(string thisPage, char* page) {	// here is where you see a form to be filled out
  cout <<
    "  <center>\n"
    "   <form name=\"" << string(page) << "\" method=\"post\" action=\"";
  cout << thisPage << "+send+" << string(page) << "\">\n";
  cout <<"    <input type=\"submit\" value=\"Submit\" />\n";

  cout <<"    <a href=" << thisPage << "+menu>Abort to reports menu.</a>\n";
  cout <<"    <table border=\"2\" cellspacing=\"2\"><tr><td colspan=2><div class=\"scrollable\">\n";
  cout <<"     <input type=hidden name=ReportType value=\""+string(page)+" Report\"/>\n";

  bannerSign();
  if(strstr(page,"EvaPlan"))parseWaypointList();
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
  fill(thisPage, section);
}
******************************************/

void readReportLinks(const char* format) {
  FILE* list = popen(
    "ls /usr/lib/cgi-bin/data/*Form.html | sed -e 's:.*/::' -e 's:Form.html::'|grep -v EngSup | grep -v Eva",
    "r");
  char reportName[512], expanded[512];
  while(reportName == fgets(reportName,511,list)) {
    reportName[strlen(reportName)-1]=0;//strip off newline
    expandCamelCase(reportName, expanded);
    printf(format,reportName, expanded);
  }
  pclose(list);
}

/**********************************
void menu(string thisPage) { 	//, char *seZuhMe, char *verb) {

  string readReport = "    <li><a href=" + thisPage + "+fill+";

  cout << 
    "  <table cellspacing=2><tr><td valign=top>\n"
    "   Reports are to be sent daily before 8:00 PM local time.<br/>\n"
    "   If they are sent after 9:00 PM they risk not being accepted.<br/>\n"
    "   See the <a href=/cgi-bin/quickGuideWrapper"
    + string(strchr(thisPage.c_str(),'?')) + "+14+fill>Reports Quick Guide\n"
    "   </a> for more information.\n"
    "   <ul>\n";

  char formatString[1024];
  sprintf(formatString,"%s%%s>%%s Report</a></li>\n",readReport.c_str());
  readReportLinks(formatString);
  printf(
    "   </ul>\n"
    "  </td><td valign=top>\n"
    "   Additional engineering reports are most easily recorded<br/>\n"
    "   during the engineering activity to which they pertain.<br/>\n"
    "   Crew Engineer, please record these supplimental reports<br/>\n"
    "   whenever the related settings or conditions change.<br/>\n"
    "  <ul>\n");
  cout << readReport << "EngSupPower+fill>Power Interruption</a></li>\n";
  cout << readReport << "EngSupWater+fill>Potable Water</a></li>\n";
  cout << readReport << "EngSupHVAC+fill>Heating and Cooling</a></li>\n";
  cout << "   </ul>\n";
  printf(
    "   EVA Planning and reporting use a combined report with<br>"
    "   mapping and GPS tracking components<br>\n"
    "  <ul>\n");


  cout << "   <li><a href=/cgi-bin/routePlanner"
    + string(strchr(thisPage.c_str(),'?')) + ">Explore possible EVA routes</a></li>\n";
  cout << "   <li><a href=/cgi-bin/routePlanner"
    + string(strchr(thisPage.c_str(),'?')) + "+plan>Record an EVA Plan.</a></li>\n";
  cout << readReport << "Eva+fill>Report EVA outcome.</a></li>\n";
  cout << "   </ul>\n";
  cout << "  </td></tr></table>\n";

  printf("  <a href=photoUpload%s>Attach Photos</a><br>\n",strchr(thisPage.c_str(),'?'));
  printf("  <a href=%s+days>Review Reports in Queue</a>\n",thisPage.c_str());
  printf("  <br/>\n"
         "  <a href=reportReview%s+days>Review Old Reports</a>\n",strchr(thisPage.c_str(),'?'));
}
***************************************/

int main(int argc, char **argv) {
Session *sess = new Session(argc,argv);
   
/*********************
  if((!strcmp(argv[2],"water")) 
  &&(!strcmp(argv[2],"lp"))) 
    sess->spewTop("MDRS Habitat Activity Logger (prototype)<br>Water Usage Graphs");
  else
******************/
    sess->spewTop("MDRS Habitat Activity Logger (prototype)<br>Old Reports Review");
  
  userName = sess->getUserName();
  userRole = sess->getUserRole();
  userTeam = sess->getUserTeam();

  cout << " <p>\n";

/***********************8
       if(!strcmp(argv[2],"menu")) { menu(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"fill")) { fill(sess->getRefreshLink(), argv[3]); } // fill out form
  else if(!strcmp(argv[2],"save")) { save(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"doit")) { doit(sess->getRefreshLink(), argv[3], argv[4]); }
  else if(!strcmp(argv[2],"edit")) { edit(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"send")) { send(sess->getRefreshLink(), argv[3]); }
  else if(!strcmp(argv[2],"tell")) { tell(sess->getRefreshLink(), argv[3]); }
  else
************************/
       if(!strcmp(argv[2],"days")) { days(sess->getRefreshLink()); }	// menu of reports by day
  else if(!strcmp(argv[2],"play")) { play(sess->getRefreshLink(), argv[3]); }// show previously submitted report
/***************************
  else if(!strcmp(argv[2],"heat")) { heat(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"lp")) { propane(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"netw")) { network(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"fuel")) { diesel(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"satsig")) { satSignal(sess->getRefreshLink()); }
  else if(!strcmp(argv[2],"water")) { 
    int period = 14;
    if(argc>3)sscanf(argv[3],"%d", &period);
    water(sess->getRefreshLink(), period);
  }
******************************/
  else
    printf("  Return to <a href=/reportGenHome.html>index</a>.<br>\r\n");
  printf(" </body>\r\n");
  printf("</html>\r\n");
  return 0;
}
