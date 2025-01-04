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

long parseTimeInput() {
  int year,month,day,hour,minute;
  const int kEnoughLetters = 512;
  char buf[kEnoughLetters];
  char* b=fgets(buf,kEnoughLetters,stdin);
  if(b==NULL)return -1;
  strtok(buf,"=");
  if(3 != sscanf(strtok(0,"="), "%d-%d-%d",&year,&month,&day))return -1;
  if(2 != sscanf(strtok(0,"="), "%d%%3A%d",&hour,&minute))return -1;

  time_t tock;
  time( &tock );
  tm* tick = localtime( &tock );
  tick->tm_year = year - 1900;
  tick->tm_mon = month - 1;
  tick->tm_mday = day;
  tick->tm_hour = hour;
  tick->tm_min = minute;
  tick->tm_sec = 0;
  tock = mktime(tick);
  return (long)tock;
}


struct CHECK_LIST_LIST { int startStep; char* label; char* banner;
} check_list_list[] = {
  { 1, (char *)"Prepare For EVA",	(char*)"Do this before each EVA"},
  { 2, (char *)"Move Potable Water",	(char*)"This is the way we load our water"},
  { 3, (char*)"Heating and Ventilation Status",(char*)
	"Please complete this form whenever a thermostat is adjusted."
	" In addition, it would be helpful to have this form completed"
	" 3 times per day during periods of extreme weather to document"
	" HVAC system effectiveness." },
  { 4, (char *)"Prepare ATVs for an EVA",(char*)"Do this before each EVA that uses the ATV rovers"},
  { -1, (char *)"End of the List.",	(char*)"LOGIC ERROR: GET HELP"}
};

CHECK_LIST_LIST* getListByStart(int start){
  for(int j=0;check_list_list[j].startStep>=0;++j) 
    if(check_list_list[j].startStep==start)
      return &check_list_list[j];
  return NULL;
}

void doNextStep(const char* program, int argc, char **argv) {
  if(argc<2) {
    printf("<h1>LOGIC ERROR: Get HELP</h1>\n");
    return;
  }
  int sequence_Id=0, timeStamp=0, stepNumber=0;
  if(argc>2)sscanf(argv[2],"%d",&sequence_Id);
  if(argc>3)sscanf(argv[3],"%d",&timeStamp);

  CHECK_LIST_LIST* checkList = getListByStart(sequence_Id);

  printf("<li><STRONG>%s</STRONG><ul>\n", checkList->label);
  if(timeStamp<=0) {
    if(argc<=4) {
      printf("  %s<br/>\n", checkList->banner);
      printf("<form name=\"Data logging Sequence Step\""
        "  method=\"post\" action=\"%s+%d+0+0\">\n", program,sequence_Id);
      printf(
        " <li>Please enter the Date and Time of these data:\n"
        "  <input type=\"date\" name=\"dataGetDate\"/>\n"
        "  <input type=\"time\" name=\"dataGetTime\"/>\n"
	"  <input type=\"submit\" value=\"continue\" />\n"
        " </li>\n"
        "</form>\n");
      return;
    } else {
      timeStamp = parseTimeInput();
      if(timeStamp<1000) {
        printf(" Date and Time incorrect. Please <a href=%s+%d+0>try again</a>.<br/>\n",
          program,sequence_Id);
        return;
      }
      stepNumber = -1;
    }
  }
  else if(argc>4)sscanf(argv[4],"%d",&stepNumber);

  /* TODO
    show date time
    get all data in this sequence with this timestamp ordered by step number
    for elements with < stepnumber
      show label:value
    rof
    get actionStep(sequenceID, stepnumber)
    ( {... parameterID ... gotoStep ... enum{always, valid, inRange}test ...gotoOnTrue gotoOnFalse }
    if got sample with == timeStamp
      set value
    fi
    create form with goto link
    switch on recipe data type
    action: show the action directons
    number: prompt for number with bounds
    picklist: show radio buttons
    hctiws
    show 'continue' button
    end of form
  */

  printf("  <br/><br/>Or abort back to <a href=%s>the menu of recipes</a><br/>",program);
}

void menu(const char* program) {
  printf("<ul>\n");
  for(int recipe = 0; check_list_list[recipe].startStep >=0; recipe++) {
    printf("<li><a href=\"%s+%d\">%s</a></li>\n",
      program,check_list_list[recipe].startStep,check_list_list[recipe].label);
  }
  printf("</ul>\n");
}

int main(int argc, char **argv) {
Session *session;

  session = new Session(argc,argv);
  session->spewTop("MDRS Engineering Subsystems ");

  string program = session->getRefreshLink();
  string userName = session->getUserName();

  if(argc<=2) {
    menu(program.c_str());
  } else {
    doNextStep(program.c_str(), argc,argv);
  }
  printf(" </body>\n</html>\n");
}
