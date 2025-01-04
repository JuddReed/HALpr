#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dataBrane.h"
#include "session.h"
#include "svgGraph.h"
#include "timeString.h"

/*
#include <time.h>
char* taggedTimeString(const char* prefix, char* space) {
time_t tt = time(NULL);
  struct tm *now = localtime(&tt);
  sprintf(space,"%s%04d%02d%02d%02d%02d%02d", prefix,
    now->tm_year+1900, now->tm_mon+1, now->tm_mday,
    now->tm_hour, now->tm_min, now->tm_sec
  );
  return(space);
}
*/

const char* keyKey[] = {"actionBy", "actionDate", "actionTime"};

void bannerSign(Session* session) {
  time_t tt = time(NULL);
  struct tm *now = localtime(&tt);

  printf(
    "     <table><tr>\n"
    "      <th align=right>Reported by:</th><td>%s</td>\n", session->getUserName().c_str() );
  printf(
    "      <th align=right>Report Date and Time:</th><td>%02d/%02d/%04d</td><td>%02d:%02d %s</td>\n"
    , now->tm_mon+1, now->tm_mday, now->tm_year+1900, ((1+now->tm_hour)%12)-1, now->tm_min,
    (12>=now->tm_hour)?"AM":"PM");
  printf(
    "     </tr><tr>\n"
    "      <th align=right>Activity by:</th>"
  );

  DataBrane *db = new DataBrane();
  DataBrane::MEMBERS* crew = db->getActiveMembers();
  int me, crewCount = db->memberCount();
  sscanf(session->getMembership().c_str(),"%d",&me);
  printf(
    "      <td><SELECT width=32 name=%s value=\"%d\">\n", keyKey[0], me);
  for(int i =0;i<crewCount;i++)printf(
    "       <OPTION%svalue=\"%d\">\"%s, %s of %s\" </OPTION>\n",
    (crew[i].uid == me)?" selected ":" ",
    crew[i].uid, crew[i].name.c_str(), crew[i].role.c_str(), crew[i].team.c_str());
  printf(
    "      </SELECT></td>\n");
  printf(
    "      <th align=right>Action Date and Time:</th>\n"
    "      <td><input type=date name=%s value=\"%04d-%02d-%02d\"/></td>\n"
    "      <td><input type=time name=%s value=\"%2d:%02d\"/></td>\n",
    keyKey[1], now->tm_year+1900, now->tm_mon+1, now->tm_mday,
    keyKey[2], now->tm_hour, now->tm_min);
  printf(
    "     </tr></table>\n");
}

enum CAT_MODE {EDIT,SHOW};
void cat(const char *section, CAT_MODE mode) {
int escapeFrom = 0;
char buffer[1024];

  try {
    FILE* page=fopen(section,"r");
    if (page) {
      buffer[0]=0;
      for(int c=fgetc(page);c!=EOF;c=fgetc(page)) {
        if (c!='<') {
          putchar(c);
        } else if(mode == EDIT) {
          printf("&#060");
        } else {
          putchar('<');
        }
      }
      fclose(page);
    } else {
      printf("FAILED TO OPEN %s<br>\r\n", section);
    }
  } catch(int e) {
    printf("\n(FAILED TO READ)\n\n");
  }
}

void menu(string program) {
  DataBrane *db = new DataBrane();
  DataBrane::SYSTEM *system = db->getSystemList();
  int size = db->systemCount();
    
  cout <<
      "  <center>\n"
      "   <table>\n"
      "    <tr><th align=left>Menu of Engineering Sus-systems</th></tr>\n";

  for(int i=0;i<size;i++) {
    cout <<
      "    <tr><td><a href=" << program << "+" << system[i].uid << "+tour>" 
      << system[i].label << "</a></td></tr>\n";
  }
  cout <<
      "   </table>\r\n";
}

void obsr(string program, int argc, char **argv) {
  char space[32];
  int parameterId, samples;
  int graphType = 0;

  sscanf(argv[2], "%d", &parameterId);
  sscanf(argv[4], "%d", &samples);
  if(argc>5)sscanf(argv[5], "%d", &graphType);

  DataBrane *db = new DataBrane();
  DataBrane::OBSERVATION *reading = db->getObservationsByParameter(parameterId);
  int size = db->observationCount();
  if(size<=0){printf("<H1>NO DATA </H1><br>");return;}
  int editObser = reading[size-1].observation_id;
  if(argc>6)sscanf(argv[6], "%d", &editObser);

  //printf("   <center><h2>%s</h2><br/>\n",parameterName);
  printf("   <a href=\"%s+menu\">Back to Parameters menu</a>\n", program.c_str());
  printf("   <a href=\"%s+%d+obsr+%s+%d\">Change graph Type</a>\n", program.c_str(),parameterId,
    argv[4], (graphType+1)%4);
  printf("   <a href=\"%s+%d+obsr+%d+%d\">Show All %d</a>\n", program.c_str(),parameterId,
    size, graphType, size);
  if(2*samples<size) printf(
    "   <a href=\"%s+%d+obsr+%d+%d\">Show Latest %d</a>\n", program.c_str(),parameterId,
    samples*2, graphType, samples*2);
  if(samples>1) printf(
    "   <a href=\"%s+%d+obsr+%d+%d\">Show Latest %d</a>\n", program.c_str(),parameterId,
    samples/2, graphType, samples/2);

  char format[128];
  sprintf(format,"%s+%d+%s+%s+%d+%%d",program.c_str(),parameterId,
    argv[3], argv[4], graphType);


  int a;
  printf("    <form name=\"Edit Observation\" method=\"post\" action=\"");
  printf("%s+%d+upda",program.c_str(),parameterId);
  for(a=4;a<argc;a++)printf("+%s",argv[a]);
  printf("\" >\n");
  for(a=size-1;a>0;a--)if(reading[a].observation_id==editObser)break;
  
    /*struct OBSERVATION {
      long observation_id;
      long activ_param_id;
      long membership_id;
      long datetime;
      double value;
      string units;
    };*/

    //"     <textarea name=\"content\" cols=\"100\" rows=\"26\">\r\n");
  printf("     <input type=\"hidden\" name=\"observation_id\" value=\"%ld\"/>\n",
    reading[a].observation_id);
  printf("     <input type=\"hidden\" name=\"activ_param_id\" value=\"%ld\"/>\n",
    reading[a].activ_param_id);
  printf("     <input type=\"hidden\" name=\"membership_id\" value=\"%ld\"/>\n",
    reading[a].membership_id);
  printf("     <input type=\"datetime\" name=\"datetime\" value=\"%s\"/>\n", 
    dbTimeString(reading[a].datetime, space));
  printf("     <input type=\"textbox\" name=\"value\" value=\"%lf\"/>\n", reading[a].value);
  printf("     <input type=\"textbox\" name=\"units\" value=\"%s\"/>\n", reading[a].units.c_str());
  printf("     <input type=\"submit\" value=\"Save\"/><br/>\n");
  printf("    </form>\n");

  SvgGraph svgGraph(1024, 256);
  svgGraph.graphType = (SvgGraph::GraphType)graphType;
  int start = size - samples;
  if(start<0) start = 0;
  svgGraph.load(&reading[start], size - start);
  svgGraph.head();
  svgGraph.spew();
  svgGraph.link(format);
  svgGraph.tail();
}

void upda(string program, int argc, char **argv) {
char message[1024];
int len=fread(message,1,1024,stdin);
  message[len]=0;
  printf("%d: %s<br>\n",len, message);
  obsr(program, argc, argv);
}

void tour(string program, char *sysStr) {
  int sysId;
  sscanf(sysStr, "%d", &sysId);

  DataBrane *db = new DataBrane();
  DataBrane::SYSTEM *system = db->getSystemList();
  int size = db->systemCount();
  int i; for(i=0; i<size; i++)if(system[i].uid == sysId)break;
  if(i>=size) {
    printf("  <h1>LOGIC ERROR: GET HELP</h1>\n");
    return;
  }
  printf("  Return to <a href=%s+menu>Systems menu.</a>\n", program.c_str());
  cout <<
      "  <center><table border=1>\n"
      "   <tr><th colspan=2>Welcome to the "+system[i].label+" Systems Page</th></tr>\n"
      "   <tr><td colspan=2>Here you can enter and review data in two ways.<br/>\n"
      "    All of the parameters related to this system can be viewed and edited individually.<br/>\n"
      "    Parameters associated with specific engineering activities can also be entered by\n"
      "    filling out an engineering activity form."
      "   </td></tr>\n"
      "   <tr>\n";

  DataBrane::ACTIVITY *action = db->getSystemActions(system[i].uid);
  int actionCount = db->activityCount();
  cout << "    <th>select from " << actionCount << " " << system[i].label << " Actions</th>\n";

  DataBrane::PARAMETER *parameter = db->getSystemParameters(system[i].uid);
  int paramCount = db->parameterCount();
  cout << "    <th>select from " << paramCount << " " << system[i].label << " Parameters</th>\n";
  cout <<
      "   </tr>\n"
      "   <tr><td width=256px height=256px>\n"
      "    <div style=\"width:200px; height:200px; overflow:auto\" >\n";
  for(int j=0;j<actionCount;j++) printf(
      "     <a href=%s+%ld+doit>%s<a><br/>\n", program.c_str(),
      action[j].uid, action[j].label.c_str());
  cout <<
      "    </div>\n"
      "   </td><td>\n"
      "    <div style=\"width:256px; height:256px; overflow:auto\" >\n";
  for(int j=0;j<paramCount;j++) printf(
      "     <a href=%s+%ld+obsr+32>%s<a><br/>\n", program.c_str(),
      parameter[j].uid, parameter[j].label.c_str());
  cout <<
      "    </div>\n"
      "   </td></tr>\n"
      "   <tr>\n"
      "    <td><a href="+program+"+"+sysStr+"+nope>Add new Activity (not working yet)</a></td>\n"
      "    <td><a href="+program+"+"+sysStr+"+nope>Add new Parameter (not working yet)</a></td>\n"
      "   </tr>\n"
      "  </table>\r\n";
}

void edit(string program, char *actionId) {
  DataBrane *db = new DataBrane();
  DataBrane::DATA_ASSET* asset = db->getActivityForm(actionId);
                          
  int actId = sscanf(actionId, "%d", &actId);
  DataBrane::PARAMETER* actPar = db->getActivityParameters(actId);
  int numPar = db->parameterCount();

  printf(
    "   <center>\n"
    "    Note: When making forms, the input element names MUST match the numeric values in the\n"
    "    database. That means all names must be in the first column of this table:<br/>\n");
  printf(
    "    as in \"%s %s:&#060input type=\"text\" name=\"%ld\" />&#060br/>\"\n",
    actPar[0].label.c_str(), actPar[0].context.c_str(), actPar[0].activityParameterId);
  printf(
    "    <table>\n"
    "     <tr><th>Unique Id</th><th>Units</th><th>Parameter Name</th><th>Circumstance</th></tr>\n");
  for(int p=0;p<numPar;++p)printf("<tr><th>%ld</th><td>%s</td><td>%s</td><td>%s</td></tr>\n",
    actPar[p].activityParameterId,
    actPar[p].units.c_str(),
    actPar[p].label.c_str(),
    actPar[p].context.c_str());
/*      long uid;
      long systemId;
      long stepNumber;
      string label, units, description, howToRead, context;
      int alarmMask;
      double lowAlarm, highAlarm;*/
  printf(
    "    </table>\n");
    
  printf(
    "    <form name=\"Action Form Editor\" method=\"post\" action=\"");
  cout << program << "+" << string(actionId) << "+view\">\n";
  printf("    <input type=\"submit\" value=\"Save\"/><br>\r\n"
    "     <textarea name=\"content\" cols=\"100\" rows=\"26\">\r\n");
  cat(asset->path.c_str(), EDIT);
  printf("     </textarea><br>\r\n   </form>\r\n   </center>\r\n");
}

class ReadingList {
private:
  string tag, val;
  ReadingList* link;

public:
  ReadingList() { link=NULL; }
  ReadingList(char* t, char* v) { set(t,v); link=NULL; }

  void set(char* t, char* v) {
    tag = string(t);
    val = string(v);
  }

  ReadingList* addReading (char* t, char* v) {
    return link = new ReadingList(t, v);
  }
  
  ReadingList* getLink(){ return link; }
  const char* getTag(){ return tag.c_str(); }
  const char* getValue(){ return val.c_str(); }
  char* copyValue(){ return strdup(val.c_str()); }
};

void loadActionData(ReadingList* firstObs) {
  int member, year,month,day, hour,minute, activParam;
  double value;
  ReadingList* lastObs=firstObs;
  sscanf(lastObs->getValue(),"%d", &member);
  lastObs=lastObs->getLink();
  sscanf(lastObs->getValue(),"%4d-%2d-%2d", &year, &month, &day);
  lastObs=lastObs->getLink();
  sscanf(lastObs->getValue(),"%2d%%3A%2d", &hour, &minute);

  DataBrane::OBSERVATION fact;
  fact.membership_id = member;
  fact.datetime = joinTimeParts(year,month,day, hour,minute, 00);

  DataBrane *db = new DataBrane();
  for(lastObs=lastObs->getLink();lastObs!=NULL;lastObs=lastObs->getLink()) {
    sscanf(lastObs->getTag(),"%d", &activParam);
    fact.activ_param_id = activParam;
    sscanf(lastObs->getValue(),"%lf",&value);
    fact.value = value;
    fact.units = "unspecified";
    db->putObservations(&fact, 1);
  }
}

void save(string program, char *actionId) {
  ReadingList firstObs, *lastObs;
  bool redo = false;

  char buffer[1024];
  int step = 0;
  for(char* got=fgets(buffer, 1024,stdin);got!=NULL;got=fgets(buffer, 1024, stdin)) {
    for(char* tag = strtok(buffer,"=");tag;tag = strtok(000,"=")) {
      char* val = strtok(000,"&");
      if(val==NULL) {
        printf("    <b><a href=%s+%s+doit> ", program.c_str(), actionId);
        printf("%s IS MISSING AND REQUIRED</a></b><br/>\n", tag);
        redo = true;
        break;
      }
      char* isAt = strchr(val,'=');
      if(0!=isAt) {
        printf("    <b><a href=%s+%s+doit> ", program.c_str(), actionId);
        printf("%s IS MISSING AND REQUIRED</a></b><br/>\n", tag);
        redo = true;
        *isAt++ = 0;
        if(*isAt==0) {
          printf("    <b><a href=%s+%s+doit> ", program.c_str(), actionId);
          printf("%s IS MISSING AND REQUIRED</a></b><br/>\n", val);
          redo = true;
        } else {
          if(step==0) {
            firstObs.set(val,isAt);
            lastObs = &firstObs;
          } else lastObs = lastObs->addReading(val,isAt);
          ++step;
        }
      } else {
       if(step==0) {
          firstObs.set(tag,val);
          lastObs = &firstObs;
        } else {
          lastObs = lastObs->addReading(tag,val);
        }
        ++step;
      }
    }
  }
  if(!redo) {
    loadActionData(&firstObs);
    printf("%d observations recorded<br/>\n", step);
  }
}

void view(string program, char *actionId, char* uid) {
  DataBrane *db = new DataBrane();
  DataBrane::DATA_ASSET* asset = db->getActivityForm(actionId);
  char buffer[512];
  sprintf(buffer, "%s.%s", asset->path.c_str(), uid);

  cout <<
    "  <center>\n"
    "  <table border=2 cellspacing=2>\n"
    "   <tr><th>\n"
    "    <a href=" << program << "+" << string(actionId) << "+keep+"<< string(uid) << ">"
                                                 "CONFIRM<a> your edits,\n"
    "    <a href=" << program << "+" << string(actionId) << "+edit>"
                                                 "RESUME<a> editing without changes,\n"
    "    <a href=" << program << "+" << string(actionId) << "+doit>"
                                                 "ABANDON<a> edits to original form.<br>\n"
    "   </th></tr><tr><td>\n"
    "    <div style=\"width:1000; height:300; overflow:auto\">\n";

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
    "   </td></tr><table>\n";
}

void doit(string program, char *uid, Session* session) {
  DataBrane *db = new DataBrane();
  DataBrane::DATA_ASSET* asset = db->getActivityForm(uid);
  string page = asset->label;
  cout <<
    "  <center>\n"
    "   <form name=\"" << page << "\" method=\"post\" action=\"";
  cout << program << "+" << string(uid) << "+save\">\n";
  cout <<
    "    <input type=\"submit\" value=\"Submit\" />\n"
    "    <a href=" << program << "+menu>Abort to systems menu.</a>\n"
    "    <table border=\"2\" cellspacing=\"2\"><tr><td colspan=2>\n"
    "     <div style=\"width:1000; height:300; overflow:auto\">\n";

  bannerSign(session);
  cat(asset->path.c_str(), SHOW);

  cout <<"\n"
    "     </div>\n"
    "    </td></tr><tr><td align=LEFT>\n"
    "     <input type=\"submit\" value=\"Submit for Earth Transmission\"/><br>\n"
    "    </td><td align=RIGHT>\n"
    "     <a href=" << program << "+"<<uid<<"+edit >Edit</a>\n"
    "    </td></tr></table>\n"
    "   </form>\n"
    "  </center>\n";
}

void keep(string program, char *actionId, char* timeStamp) {
  DataBrane *db = new DataBrane();
  DataBrane::DATA_ASSET* asset = db->getActivityForm(actionId);
  const int NUF_SPACE = 1024;
  char buffer[NUF_SPACE];
  sprintf(buffer, "cp %s %s.before_%s", asset->path.c_str(), asset->path.c_str(), timeStamp);
  int status = system(buffer);
  if(status != 0) {
    printf("<center>\n"
      "<H1>OOPS: Failed to back up old version of form</H1><br/>\n"
      "<B>Changes not saved</B><br/><br/>\n");
    printf("<i>%s (%d)</i><br/>\n",buffer, status);
    return;
  }
  sprintf(buffer, "mv %s.%s %s", asset->path.c_str(), timeStamp, asset->path.c_str());
  status = system(buffer);
  if(status != 0) {
    printf("<center>\n"
      "<H1>OOPS: Failed save changes.</H1><br/><br/>\n");
    printf("<i>%s (%d)</i><br/>\n",buffer, status);
    return;
  }
  // TODO record an instance of authorship
}


int main(int argc, char **argv) {
Session *session;

  session = new Session(argc,argv);
  session->spewTop("MDRS Engineering Subsystems ");

  string program = session->getRefreshLink();

  cout <<
    "    Go to <a href="
      << session->homeInSession() << "?" << session->getSessionArg()
      << ">H.A.L. root</a><br/>\n";
         if(!strcmp(argv[2],"menu")) { menu(program); // pick a system
  } else if(!strcmp(argv[3],"obsr")) { obsr(program, argc, argv); // observe, graph, edit a param
  } else if(!strcmp(argv[3],"upda")) { upda(program, argc, argv); // Update an observation
  } else if(!strcmp(argv[3],"tour")) { tour(program, argv[2]); // pick an activity
  } else if(!strcmp(argv[3],"doit")) { doit(program, argv[2], session); // fill out the form
  } else if(!strcmp(argv[3],"save")) { save(program, argv[2]); // save the data entered in the form
  } else if(!strcmp(argv[3],"edit")) { edit(program, argv[2]); // change the form
  } else if(!strcmp(argv[3],"view")) {
    char uid[32];
    taggedTimeString(session->getMembership().c_str(), uid);
                                       view(program, argv[2], uid); // review changes to the form
  } else if(!strcmp(argv[3],"keep")) { keep(program, argv[2], argv[4]); // keep changes to the form
               doit(program, argv[2], session); // and then go fill out the form
  } else  cout << 
    "   Return to <a href=" << session->homeInSession() << ">index</a>.<br>\n";
  cout <<
    "  </center>\n"
    " </body>\n"
    "</html>\n";
}
