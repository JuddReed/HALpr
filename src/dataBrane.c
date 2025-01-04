#include <iostream>
#include <unistd.h>
#include <pqxx/pqxx>
#include "dataBrane.h"

using namespace std;
using namespace pqxx;

DataBrane::DataBrane(string connString) {
  connectYarn = connString;
  hookup();
}
  
DataBrane::DataBrane() {
  connectYarn = "user=postgres password=ez24get dbname=hal";
  hookup();
}

void DataBrane::cat(const char *section, CAT_MODE mode) {
int escapeFrom = 1;
char buffer[1024];

  try {
    FILE* page=fopen(section,"r");
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
      printf("FAILED TO OPEN %s<br>\r\n", section);
    }
  } catch(int e) {
    printf("\n(FAILED TO READ)\n\n");
  }
}

char* allCaps(const char *asciiCharacterArray, char* space){
  char* s = space;
  for ( const char* l = asciiCharacterArray; *l; ++l)*s++=('a'>*l)?*l:*l&~' ';*s=0;
  return space;
}

void DataBrane::dumpCreatesToFile(const char *filePath) {
  char space[1024];
  char morespace[1024];
  FILE* dump;
  try {
    dump=fopen(filePath,"w");
    work W(*C);
    result R = W.exec("SELECT * FROM INFORMATION_SCHEMA.COLUMNS "
         "where table_schema = 'public' "
         "order by table_name,ordinal_position");
    for(int c=0;c<R.size();c++) {
      if ((c==0) || (R[c]["table_name"] != R[c-1]["table_name"]) ) {
        if(c)printf("\n<br/>);\n<br/>\n<br/>");
        printf("CREATE TABLE %s ( <br/>\n", allCaps(R[c]["table_name"].c_str(), space));
      } else {
        printf(",<br/>\n");
      }
      printf("  %s %s", allCaps(R[c]["column_name"].c_str(), space),
             (strcmp(R[c]["data_type"].c_str(), (char*)"character varying"))
              ?allCaps(R[c]["data_type"].c_str(),morespace)
              :(char*)"VARCHAR"
            );
      if(strlen(R[c]["character_maximum_length"].c_str()))
        printf("(%s)",R[c]["character_maximum_length"].c_str());

      if(0==strcmp(R[c]["ordinal_position"].c_str(),"1")) printf(" UNIQUE PRIMARY KEY");
    } 
  } catch (const exception &e) {
    printf("Database archive failed: %s\n", e.what());
    fprintf(stderr,"Database archive failed: %s\n", e.what());
  }
  if(dump) fclose(dump);
}



void DataBrane::dumpToFile(const char *filePath) {
  char selectCommand[4096];
  char insertCommand[4096];
  char space[4096];
  FILE* dump;
  try {
    printf("dumping database build into %s<br/>\n",filePath);
    dump=fopen(filePath,"w");
    work W(*C);
    result R = W.exec("SELECT * FROM INFORMATION_SCHEMA.COLUMNS "
         "where table_schema = 'public' "
         "order by table_name,ordinal_position");
    sprintf(insertCommand,"INSERT INTO %s\n\t(%s",
        R[0]["table_name"].c_str(), R[0]["column_name"].c_str());
    sprintf(selectCommand,"SELECT %s",
                                    R[0]["column_name"].c_str());
    int first=0;
    for(int c=1;c<=R.size();c++) {
      if ((c==R.size())||(R[c]["table_name"] != R[c-1]["table_name"] )) {
        strcat(insertCommand,(char*)")\n VALUES");
        strcat(selectCommand,(char*)" FROM ");
        strcat(selectCommand,R[c-1]["table_name"].c_str());

printf("%s<br/>\n",selectCommand);
        allCaps(R[c-1]["table_name"].c_str(), space);
        fprintf(dump,"\\echo\n"
               "\\echo Reload %s\n"
               "delete from %s;\n",  space, space);
printf("<br/>load %s<br/>\nfrom %s<br/>\n",  space, space);
printf("%s<br/>\n", allCaps(insertCommand, space));
        fprintf(dump,"%s", allCaps(insertCommand, space));

        result RR = W.exec(selectCommand);
        for(int rc=0;rc<RR.size();rc++) {
          if(!strcmp(R[first+0]["data_type"].c_str(), (char*)"character varying")) {
            //printf("('%s'", RR[rc][0].c_str());
            fprintf(dump,"\t('%s'", RR[rc][0].c_str());
          } else {
            //printf("(%s", RR[rc][0].c_str());
            fprintf(dump,"\t(%s", RR[rc][0].c_str());
          }
          for(int cc=1;cc<c-first;cc++) {
            if(!strcmp(R[first+cc]["data_type"].c_str(), (char*)"character varying")) {
              //printf(", '%s'", RR[rc][cc].c_str());
              fprintf(dump,", '%s'", RR[rc][cc].c_str());
            } else if(!strcmp(R[first+cc]["data_type"].c_str(),
                (char*)"timestamp without time zone")) {
              strcpy(space, RR[rc][cc].c_str());
              if (strstr(space, (char*)"00:00:00")) {
                fprintf(dump,", DATE '%s'", strtok(space,(const char*)" "));
              }
              else  fprintf(dump,", (date and time) '%s'", RR[rc][cc].c_str());
            }else{
              //printf(", %s", RR[rc][cc].c_str());
              fprintf(dump,", %s", RR[rc][cc].c_str());
            }
          }
          //printf(")%c<br/>\n",(rc<RR.size()-1)?',':';');
          fprintf(dump,")%c\n",(rc<RR.size()-1)?',':';');
        }

        sprintf(insertCommand,"INSERT INTO %s\n\t(%s",
            R[c]["table_name"].c_str(), R[c]["column_name"].c_str());
        sprintf(selectCommand,"SELECT %s",
                                        R[c]["column_name"].c_str());
        first = c;
        continue;
      }
      if(c==R.size())break;
      strcat(insertCommand,(char*)", ");
      strcat(insertCommand, R[c]["column_name"].c_str());
      strcat(selectCommand,(char*)", ");
      strcat(selectCommand, R[c]["column_name"].c_str());
    }
  } catch (const exception &e) {
    printf("Database archive failed: %s\n", e.what());
    fprintf(stderr,"Database archive failed: %s\n", e.what());
  }
  printf("<br/>done<br/>\n");
  if(dump) fclose(dump);
  fsync((int)stdout);
}



DataBrane::DATA_ASSET* DataBrane::getActivityForm(string actionId) {
  DataBrane::DATA_ASSET* resource = NULL;
  try {
    work W(*C);
    result R = W.exec(
      "SELECT "
      "  ACTIVITY.activity_id,"
      "  ACTIVITY.data_asset_id,"
      "  DATA_ASSET.data_asset_id,"
      "  DATA_ASSET.sortable,"
      "  DATA_ASSET.label,"
      "  DATA_ASSET.path "
      " FROM ACTIVITY,DATA_ASSET "
      " where DATA_ASSET.data_asset_id=ACTIVITY.data_asset_id"
      " and ACTIVITY.activity_id="+actionId );
    if(1 == R.size()) {
      resource = new DataBrane::DATA_ASSET;
      R[0]["data_asset_id"] >> resource->uid;
      R[0]["sortable"] >> resource->order;
      R[0]["label"] >> resource->label;
      R[0]["path"] >> resource->path;
    }
  } catch (const exception &e) {
    printf("Data Asset Fetch error: %s\n", e.what());
    fprintf(stderr,"Data Asset Fetch error: %s\n", e.what());
  }
  return resource;
}

DataBrane::ACTIVITY* DataBrane::getSystemActions(int systemId) {
  try {
    work W(*C);
    result R = W.exec(
      "SELECT "
      "  ACTIVITY.activity_id,"
      "  ACTIVITY.system_id,"
      "  ACTIVITY.data_asset_id,"
      "  ACTIVITY.label,"
      "  ACTIVITY.description "
      " FROM ACTIVITY where ACTIVITY.system_id=" + to_string(systemId) );
    numActivity = R.size();
    activitySet = new ACTIVITY[numActivity];

    for(int i=0;i<numActivity; i++) {
      R[i]["activity_id"] >> activitySet[i].uid;
      R[i]["system_id"] >> activitySet[i].systemId;
      R[i]["data_asset_id"] >> activitySet[i].dataAssetId;
      R[i]["label"] >> activitySet[i].label;
      R[i]["description"] >> activitySet[i].description;
    }
  } catch (const exception &e) {
    printf("System Activities poll error: %s\n", e.what() );
    fprintf(stderr, "System Activities poll error: %s\n", e.what() );
  }
  return activitySet;
}
int DataBrane::activityCount(){return numActivity;}

/***
    int updateObservations(OBSERVATION facts[], int count);
    OBSERVATION* getListedParameterslastObservation(long parameter[], int count);
/**/
DataBrane::OBSERVATION* DataBrane::getObservationsByParameter(long parameter){
  char query[1024], timeBuffer[32];
  try {
    sprintf(query,
      "SELECT "
      "  ACTIVITY_PARAMETER.parameter_id,"
      "  ACTIVITY_PARAMETER.activity_parameter_id,"
      "  OBSERVATION.observation_id,"
      "  OBSERVATION.activ_param_id,"
      "  OBSERVATION.membership_id,"
      "  extract(epoch from OBSERVATION.datetime) as ticks,"
      "  OBSERVATION.value,"
      "  OBSERVATION.units "
      " FROM ACTIVITY_PARAMETER,OBSERVATION"
      " WHERE ACTIVITY_PARAMETER.activity_parameter_id=OBSERVATION.activ_param_id"
      "  AND ACTIVITY_PARAMETER.parameter_id=%ld"
      " ORDER BY ticks,value",  parameter);

    work W(*C);
    result R = W.exec(query);
    numObservation = R.size();
    observationSet = new OBSERVATION[numObservation];

    for(int i=0;i<numObservation; i++) {
      R[i]["observation_id"] >> observationSet[i].observation_id;
      R[i]["activ_param_id"] >> observationSet[i].activ_param_id;
      R[i]["membership_id"] >> observationSet[i].membership_id;
      R[i]["ticks"] >> observationSet[i].datetime;
      R[i]["value"] >> observationSet[i].value;
      R[i]["units"] >> observationSet[i].units;
    }
  } catch (const exception &e) {
    printf("System Observations poll error: %s\n", e.what() );
    fprintf(stderr, "System Observations poll error: %s\n", e.what() );
  }
  return observationSet;
}

int DataBrane::observationCount(){return numObservation;}

int DataBrane::putObservations(OBSERVATION facts[], int count) {
  char query[512], timeBuffer[32];
  long uid=0;
  result R = doQuery("select max(observation_id) from OBSERVATION;");
  if(1 == R.size()) { R[0]["max"] >> uid; }
  uid+=1;

  for(int tuple=0;tuple<count;tuple++) {
    if(uid < facts[tuple].observation_id) uid = facts[tuple].observation_id;
    sprintf(query,
      "INSERT INTO OBSERVATION (OBSERVATION_ID, ACTIV_PARAM_ID, MEMBERSHIP_ID,"
      " DATETIME, VALUE, UNITS) VALUES (%ld,%ld,%ld,TIMESTAMP '%s',%lf,'%s')", uid,
      facts[tuple].activ_param_id, facts[tuple].membership_id,
      dbTimeString(facts[tuple].datetime,timeBuffer),
      facts[tuple].value, facts[tuple].units.c_str());
    adhoc(query);
    ++uid;
  }
  return uid;
}

DataBrane::PARAMETER* DataBrane::getActivityParameters(int activityId) {
  char query[512];
  sprintf(query,
      "SELECT "
      "  PARAMETER.parameter_id,"
      "  PARAMETER.system_id,"
      "  ACTIVITY_PARAMETER.activity_parameter_id,"
      "  ACTIVITY_PARAMETER.step,"
      "  PARAMETER.title,"
      "  PARAMETER.description,"
      "  PARAMETER.units,"
      "  PARAMETER.how_to_read,"
      "  ACTIVITY_PARAMETER.context AS context,"
      "  PARAMETER.alarms_active_mask,"
      "  PARAMETER.low_alarm,"
      "  PARAMETER.high_alarm"
      " FROM ACTIVITY_PARAMETER,PARAMETER"
      " WHERE ACTIVITY_PARAMETER.activity_id=%d"
      "   AND ACTIVITY_PARAMETER.parameter_id=PARAMETER.parameter_id "
      " ORDER BY activity_parameter_id", activityId);
  try {
    work W(*C);
    result R = W.exec(string(query));
    numParameter = R.size();
    parameterSet = new PARAMETER[numParameter];

    for(int i=0;i<numParameter; i++) {
      int got;
      //sscanf(R[i]["activity_parameter_id"].c_str(),"%d",&got); parameterSet[i].uid = got;
      R[i]["parameter_id"] >> parameterSet[i].uid;
      R[i]["system_id"] >> parameterSet[i].systemId;
      R[i]["activity_parameter_id"] >> parameterSet[i].activityParameterId;
      R[i]["step"] >> parameterSet[i].stepNumber;
      R[i]["title"] >> parameterSet[i].label;
      R[i]["units"] >> parameterSet[i].units;
      R[i]["description"] >> parameterSet[i].description;
      R[i]["how_to_read"] >> parameterSet[i].howToRead;
      R[i]["context"] >> parameterSet[i].context;
      R[i]["alarms_active_mask"] >> parameterSet[i].alarmMask;
      R[i]["low_alarm"] >> parameterSet[i].lowAlarm;
      R[i]["high_alarm"] >> parameterSet[i].highAlarm;
    }
  } catch (const exception &e) {
    fprintf(stderr,"System Parameters poll error: %s\n", e.what());
  }
  return parameterSet;
}

DataBrane::PARAMETER* DataBrane::getSystemParameters(int systemId) {
  try {
    work W(*C);
    result R = W.exec(
      "SELECT "
      "  PARAMETER.parameter_id,"
      "  PARAMETER.system_id,"
      "  '-999' as stepNumber,"
      "  PARAMETER.title,"
      "  PARAMETER.description,"
      "  PARAMETER.units,"
      "  PARAMETER.how_to_read,"
      "  'Not reported in query' AS context,"
      "  PARAMETER.alarms_active_mask,"
      "  PARAMETER.low_alarm,"
      "  PARAMETER.high_alarm"
      " FROM PARAMETER where PARAMETER.system_id=" + to_string(systemId) );
    numParameter = R.size();
    parameterSet = new PARAMETER[numParameter];

    for(int i=0;i<numParameter; i++) {
      R[i]["parameter_id"] >> parameterSet[i].uid;
      R[i]["system_id"] >> parameterSet[i].systemId;
      R[i]["stepNumber"] >> parameterSet[i].stepNumber;
      R[i]["title"] >> parameterSet[i].label;
      R[i]["units"] >> parameterSet[i].units;
      R[i]["description"] >> parameterSet[i].description;
      R[i]["how_to_read"] >> parameterSet[i].howToRead;
      R[i]["context"] >> parameterSet[i].context;
      R[i]["alarms_active_mask"] >> parameterSet[i].alarmMask;
      R[i]["low_alarm"] >> parameterSet[i].lowAlarm;
      R[i]["high_alarm"] >> parameterSet[i].highAlarm;
    }
  } catch (const exception &e) {
    fprintf(stderr,"System Parameters poll error: %s\n", e.what());
  }
  return parameterSet;
}
int DataBrane::parameterCount(){return numParameter;}

DataBrane::DATA_ASSET* DataBrane::getQuickGuides() {
  try {
    work W(*C);
    result R = W.exec(
      "SELECT"
      "  DATA_ASSET.data_asset_id,"
      "  DATA_ASSET.sortable,"
      "  DATA_ASSET.label,"
      "  DATA_ASSET.path "
      "FROM DATA_ASSET, DATA_TYPE "
      " where DATA_ASSET.data_type_id=DATA_TYPE.data_type_id"
      " and DATA_TYPE.data_type_name='Quick Guide'"
      " order by DATA_ASSET.sortable"
    );
    numQuickGuide = R.size();
    // if(quickGuideSet>0)delete(quickGuideSet);
    quickGuideSet = new DATA_ASSET[numQuickGuide];

    for(int i=0;i<numQuickGuide; i++) {
      R[i]["data_asset_id"] >> quickGuideSet[i].uid;
      R[i]["sortable"] >> quickGuideSet[i].order;
      R[i]["label"] >> quickGuideSet[i].label;
      R[i]["path"] >> quickGuideSet[i].path;
    }
  } catch (const exception &e) {
    fprintf(stderr,"Quick Guide Menu poll error: %s\n", e.what());
  }
  return quickGuideSet;
}
int DataBrane::quickGuideCount(){return numQuickGuide;}

DataBrane::DATA_ASSET* DataBrane::getDataAssetById(int id) {
  DataBrane::DATA_ASSET* resource = NULL;
  try {
    ostringstream daId; daId << id;
    work W(*C);
    result R = W.exec(
      "SELECT"
      "  DATA_ASSET.data_asset_id,"
      "  DATA_ASSET.sortable,"
      "  DATA_ASSET.label,"
      "  DATA_ASSET.path "
      "FROM DATA_ASSET"
      " where DATA_ASSET.data_asset_id=" + daId.str()
    );
    if(1 == R.size()) {
      resource = new DataBrane::DATA_ASSET;
      R[0]["data_asset_id"] >> resource->uid;
      R[0]["sortable"] >> resource->order;
      R[0]["label"] >> resource->label;
      R[0]["path"] >> resource->path;
    }
  } catch (const exception &e) {
    fprintf(stderr,"Data Asset Fetch error: %s\n", e.what());
  }
  return resource;
}

char* locationOrderChoice[] =
    {(char*)"location_id",(char*)"rsq",(char*)"easting",(char*)"northing"};
DataBrane::LOCATION* DataBrane::getPointSet(LOCATION_ORDER orderBy) {
  char query[512];
  sprintf(query,
      "SELECT location_id, easting, northing, precision, label,"
      " (easting-518230)*(easting-518230)+(northing-4250725)*(northing-4250725) as rsq"
      " FROM LOCATION ORDER BY %s", locationOrderChoice[(int)orderBy]);
  try {
    work W(*C);
    result R = W.exec(string(query));   //("SELECT * FROM LOCATION");
    numPoint = R.size();
    if(pointSet>0)delete(pointSet);
    pointSet = new LOCATION[numPoint];

    for(int i=0;i<R.size(); i++) {
      R[i]["location_id"] >> pointSet[i].uid;
      R[i]["easting"] >> pointSet[i].easting;
      R[i]["northing"] >> pointSet[i].northing;
      R[i]["precision"] >> pointSet[i].precision;
      R[i]["label"] >> pointSet[i].label;
    }
  } catch (const exception &e) {
    fprintf(stderr,"position poll error:%s\n", e.what());
  }
  return pointSet;
}

int DataBrane::pointCount(){return numPoint;}

void DataBrane::insertLocation(long easting, long northing, string label, int uid, float precision) {
  if(uid<=0) {
    uid = 0;
    result R = doQuery("select max(location_id) from LOCATION;");
    if(1 == R.size()) {
      R[0]["max"] >> uid;
    }
    uid+=1;
  }
  ostringstream i; i << uid;
  ostringstream e; e << easting;
  ostringstream n; n << northing;
  ostringstream p; p << precision;
  adhoc("INSERT INTO LOCATION (location_id,easting,northing,precision,label) VALUES ("
      + i.str() + "," + e.str() + "," + n.str() + "," + p.str() + ",'" + label + "')");
}
  
bool DataBrane::updateLocation(int uid, long easting, long northing, float precision, string label) {
  char query[512];
  sprintf(query,"UPDATE LOCATION SET easting=%ld, northing=%ld, precision=%f, label=%s"
      " WHERE location_id=%d", easting, northing, precision, label.c_str(), uid);
  result R = doQuery(query);
  printf("<B>%ld location updated</B>\n",R.size());
  return (1 == R.size());
}
  
DataBrane::MEMBERS* DataBrane::getActiveMembers() {
 return getActiveMembers(true);
}

DataBrane::MEMBERS* DataBrane::getActiveMembers(bool current) {
  char query[1024];
  sprintf(query,
      "select"
      " MEMBERSHIP.membership_id,"
      " PERSON.name,"
      " ROLE.title,"
      " TEAM.team_name as team,"
      " MEMBERSHIP.enter_date, MEMBERSHIP.leave_date"
      " from PERSON, MEMBERSHIP, TEAM, ROLE"
      " where PERSON.person_id = MEMBERSHIP.person_id"
      " and MEMBERSHIP.team_id = TEAM.team_id and ROLE.role_id = MEMBERSHIP.role_id"
  );
  if(current) {
    strcat(query, 
      " and enter_date <= current_date and current_date <= leave_date");
  }

  try {
    work W(*C);
    result R = W.exec(query);

    numMember = R.size();
    if(memberSet>0)delete(memberSet);
    memberSet = new MEMBERS[numMember];

    for(int i=0;i<R.size(); i++) {
      R[i]["membership_id"] >> memberSet[i].uid;
      R[i]["name"] >> memberSet[i].name;
      R[i]["title"] >> memberSet[i].role;
      R[i]["team"] >> memberSet[i].team;
      R[i]["enter_date"] >> memberSet[i].enterDate;
      R[i]["leave_date"] >> memberSet[i].leaveDate;
    }
  } catch (const exception &e) {
    fprintf(stderr,"membership poll error:%s\n", e.what());
  }
  return memberSet;
}

int DataBrane::putMembership(int personId, int roleId, int teamId, char* dayOne, char* dayEnd) {
  char query[512];
  long uid=0;
  sprintf(query,
      "SELECT * from MEMBERSHIP WHERE PERSON_ID=%d AND ROLE_ID=%d AND TEAM_ID=%d;",
      personId, roleId, teamId);
  result R = doQuery(query);
  if(0 != R.size()) { 
    if(1 != R.size())printf("NOTE POSSIBLE DB LOGIC ERROR: Non-unique membership<br>\n");
    printf("INFO: Database already has this membership: updating active dates:<br>\n");
    R[0]["membership_id"] >> uid;
    sprintf(query,
        "UPDATE MEMBERSHIP SET ENTER_DATE=DATE '%s', LEAVE_DATE=DATE '%s' WHERE MEMBERSHIP_ID=%ld",
        dayOne, dayEnd, uid);
    printf("doing SQL command: %s<br>\n",query);
    adhoc(query);
    return uid;
  }

  R = doQuery("select max(membership_id) from membership;");
  if(1 == R.size()) { R[0]["max"] >> uid; }
  uid+=1;
    
  sprintf(query,
      "INSERT INTO MEMBERSHIP (MEMBERSHIP_ID, PERSON_ID, ROLE_ID, TEAM_ID, ENTER_DATE, LEAVE_DATE)"
      " values (%ld, %d,%d,%d, DATE '%s', DATE '%s')",
      uid, personId, roleId, teamId, dayOne,dayEnd);
  printf("doing SQL command: %s<br>\n",query);
  adhoc(query);
  return uid;
}

int DataBrane::memberCount(){return numMember;}

DataBrane::PERSON* DataBrane::getPersonSet() {
  try {
    work W(*C);
    result R = W.exec(
      "select"
      " PERSON.person_id,"
      " PERSON.name,"
      " PERSON.login,"
      " PERSON.password"
      " from PERSON"
      " order by PERSON.name"
    );

    numPerson = R.size();
    if(personSet>0)delete(personSet);
    personSet = new PERSON[numPerson];

    for(int i=0;i<R.size(); i++) {
      R[i]["person_id"] >> personSet[i].person_id;
      R[i]["name"] >> personSet[i].name;
      R[i]["login"] >> personSet[i].login;
      R[i]["password"] >> personSet[i].password;
    }
  } catch (const exception &e) {
    fprintf(stderr,"people poll error:%s\n", e.what());
  }
  return personSet;
}

int DataBrane::putPerson(char* name, char* login, char* password) {
  char query[512];
  sprintf(query,
      "SELECT * from PERSON WHERE NAME='%s' OR LOGIN='%s';", name, login);
  result R = doQuery(query);
  if(0 != R.size()) { 
    printf("ERROR: Database already has %ld persons with that name or login<br>\n",R.size());
    return -R.size();
  }

  long uid=0;
  R = doQuery("select max(person_id) from PERSON;");
  if(1 == R.size()) { R[0]["max"] >> uid; }
  uid+=1;
    
  sprintf(query,
      "INSERT INTO PERSON (PERSON_ID, NAME, LOGIN, PASSWORD)"
      " values (%ld,'%s','%s','%s')", uid, name, login, password);
printf("doing SQL command: %s<br>\n",query);
  adhoc(query);
  return uid;
}

int DataBrane::personCount(){return numPerson;}

DataBrane::REPORT* DataBrane::getReportSet(char* uid) {
  numReport = 0;
  char query[1024];
  if(uid) sprintf(query,
      "SELECT "
      " REPORT.report_id,"
      " REPORT.report_type,"
      " REPORT.report_date,"
      " REPORT.label,"
      " REPORT.author"
      " from REPORT where report_id = %s", uid);
  else sprintf(query,
      "SELECT "
      " REPORT.report_id,"
      " REPORT.report_type,"
      " REPORT.report_date,"
      " REPORT.label,"
      " REPORT.author"
      " from REPORT order by report_date desc");
  try {
    work W(*C);
    result R = W.exec(query);

    numReport = R.size();
    //if(reportSet>0)delete(reportSet);
    reportSet = new REPORT[numReport];

    for(int i=0;i<R.size(); i++) {
      R[i]["report_id"] >> reportSet[i].report_id;
      R[i]["report_type"] >> reportSet[i].report_type;
      R[i]["report_date"] >> reportSet[i].report_date;
      R[i]["label"] >> reportSet[i].label;
      R[i]["author"] >> reportSet[i].author;
    }
  } catch (const exception &e) {
    fprintf(stdout,"report poll error:%s<br>\n", e.what());
    fprintf(stderr,"report poll error:%s\n", e.what());
  }
  return reportSet;
}
int DataBrane::reportCount(){return numReport;}

int DataBrane::putReport(char* reportType, char* reportDate,
    char* label, char* author) {
  char query[512];
  long uid=0;
  result R = doQuery("select max(report_id) from REPORT;");
  if(1 == R.size()) { R[0]["max"] >> uid; }
  uid+=1;
    
  sprintf(query,
      "INSERT INTO REPORT "
      "(REPORT_ID, REPORT_TYPE, REPORT_DATE, LABEL, AUTHOR)"
      " values (%ld,'%s', DATE '%s', '%s', '%s')", uid, 
      reportType, reportDate, label, author);
//fprintf(stderr, "doing SQL command: %s\n",query);
  adhoc(query);
  return uid;
}

DataBrane::REPORT_ITEM* DataBrane::getReportItemSet(char* report) {
  numReportItem = 0;
  try {
    work W(*C);
    char query[2048];
    if(report>0) sprintf(query,
      "select"
      " REPORT_ITEM.report_item_id,"
      " REPORT_ITEM.report_id,"
      " REPORT_ITEM.item_name,"
      " REPORT_ITEM.item_line,"
      " REPORT_ITEM.item_text"
      " from REPORT_ITEM"
      " where report_id = %s"
      " order by report_item_id", report
    );
    else sprintf(query,
      "select"
      " REPORT_ITEM.report_item_id,"
      " REPORT_ITEM.report_id,"
      " REPORT_ITEM.item_name,"
      " REPORT_ITEM.item_line,"
      " REPORT_ITEM.item_text"
      " from REPORT_ITEM"
    );

    result R = W.exec(query);
    numReportItem = R.size();
    //if(reportSet>0)delete(reportSet);
    reportItemSet = new REPORT_ITEM[numReportItem];

    for(int i=0;i<R.size(); i++) {
      R[i]["report_item_id"] >> reportItemSet[i].report_item_id;
      R[i]["report_id"] >> reportItemSet[i].report_id;
      R[i]["item_name"] >> reportItemSet[i].key;
      R[i]["item_line"] >> reportItemSet[i].line;
      R[i]["item_text"] >> reportItemSet[i].value;
    }
  } catch (const exception &e) {
    fprintf(stdout,"reportItem poll error:%s<br>\n", e.what());
    fprintf(stderr,"reportItem poll error:%s\n", e.what());
  }
  return reportItemSet;
}
int DataBrane::reportItemCount(){return numReportItem;}

int DataBrane::putReportItem(long reportId, char* tag, int line, char* value) {
  char query[512];
  long uid=0;
  result R = doQuery("select max(report_item_id) from REPORT_ITEM;");
  if(1 == R.size()) { R[0]["max"] >> uid; }
  uid+=1;
    
  sprintf(query,
      "INSERT INTO REPORT_ITEM "
      "(REPORT_ITEM_ID, REPORT_ID, ITEM_NAME, ITEM_LINE, ITEM_TEXT)"
      " values (%ld, %ld, '%s', %d, '%s')", uid, 
      reportId, tag, line, value);
//fprintf(stderr, "doing SQL command: %s\n",query);
  adhoc(query);
  return uid;
}

DataBrane::ROLE* DataBrane::getRoleSet() {
  numRole = 0;
  try {
    work W(*C);
    result R = W.exec(
      "select"
      " ROLE.role_id,"
      " ROLE.title,"
      " ROLE.description"
      " from ROLE"
    );

    numRole = R.size();
    //if(roleSet>0)delete(roleSet);
    roleSet = new ROLE[numRole];

    for(int i=0;i<R.size(); i++) {
      R[i]["role_id"] >> roleSet[i].role_id;
      R[i]["title"] >> roleSet[i].title;
      R[i]["description"] >> roleSet[i].description;
    }
  } catch (const exception &e) {
    fprintf(stdout,"role poll error:%s<br>\n", e.what());
    fprintf(stderr,"role poll error:%s\n", e.what());
  }
  return roleSet;
}

int DataBrane::roleCount(){return numRole;}

DataBrane::TEAM* DataBrane::getTeamSet() {
  try {
    work W(*C);
    result R = W.exec(
      "select"
      " TEAM.team_id,"
      " TEAM.team_name,"
      " TEAM.email_list"
      " from TEAM"
    );

    numTeam = R.size();
    //if(teamSet>0)delete(teamSet);
    teamSet = new TEAM[numTeam];

    for(int i=0;i<R.size(); i++) {
      R[i]["team_id"] >> teamSet[i].team_id;
      R[i]["team_name"] >> teamSet[i].team_name;
      R[i]["email_list"] >> teamSet[i].email_list;
    }
  } catch (const exception &e) {
    fprintf(stderr,"team poll error:%s\n", e.what());
  }
  return teamSet;
}

int DataBrane::putTeam(char* team_name, char* email_list) {
  char query[512];
  sprintf(query,
      "SELECT * from TEAM where TEAM_NAME='%s';", team_name);
  result R = doQuery(query);
  if(0 != R.size()) { 
    printf("ERROR: Database already has a team named %s<br>\n", team_name);
    return -R.size();
  }

  long uid=0;
  R = doQuery("select max(team_id) from TEAM;");
  if(1 == R.size()) { R[0]["max"] >> uid; }
  uid+=1;
    
  sprintf(query,
      "INSERT INTO TEAM (TEAM_ID, TEAM_NAME, EMAIL_LIST)"
      " values (%ld,'%s','%s')", uid, team_name, email_list);
printf("doing SQL command: %s<br>\n",query);
  adhoc(query);
  return uid;
}

int DataBrane::teamCount(){return numTeam;}

DataBrane::SYSTEM* DataBrane::getSystemList() {
  try {
    work W(*C);
    result R = W.exec(
      "select"
      " SYSTEM.system_id,"
      " SYSTEM.title,"
      " SYSTEM.description"
      " from SYSTEM"
    );

    numSystem = R.size();
    //if(systemSet>0)delete(systemSet);
    systemSet = new SYSTEM[numSystem];

    for(int i=0;i<R.size(); i++) {
      R[i]["system_id"] >> systemSet[i].uid;
      R[i]["title"] >> systemSet[i].label;
      R[i]["description"] >> systemSet[i].description;
    }
  } catch (const exception &e) {
    fprintf(stderr,"subsystem poll error:e.what()\n");
  }
  return systemSet;
}

int DataBrane::systemCount(){return numSystem;}

int DataBrane::signIn(int membership) {
  int sid = 0;
  result R = doQuery("select max(session_id) from SESSION;");
  if(1 == R.size()) {
    R[0]["max"] >> sid;
  }
  
  ostringstream sess; sess << sid+1;
  ostringstream memb; memb << membership;

  adhoc("INSERT INTO SESSION (session_id,membership_id,enter_time,leave_time) VALUES ("
      + sess.str() + "," + memb.str() + ", now(), now()+interval '" + timeOut() + " minutes' )" );

  return sid + 1;
}


string DataBrane::getEmailUrl(TYPE_OF_CONTACT contactType, string name) {
  string query;
  switch (contactType) {
  case CONTACT_GROUP:
    query = "select email_list as EMAIL from team where team_name='" + name + "'";
    break;
  case CONTACT_PERSON:
   query = "select value as EMAIL from person,person_Fact"
	" where person.person_id=person_Fact.person_id"
	" and person_Fact.tag='email'"
	" and person.login='" + name + "'";
    break;
  default:
    return "href=http://mdrs.marssociety.org/";
  }
  result R = doQuery(query);

  if(1 == R.size()) {
    string link;
    R[0]["EMAIL"] >> link;
    return "href=mailto:" + link;
  }

  ostringstream s; s << R.size();
  return "href=http://mdrs.marssociety.org?s="+s.str()+"+q="+query;
}


bool DataBrane::testSession(int session_id) {
  try {
    work W(*C);
    ostringstream sess; sess << session_id;
    result R = W.exec(
      "select * from SESSION where "
      "session_id=" + sess.str() +
      " and now() < leave_time"
    );

    if ( 1 == R.size() ) {
      result R = W.exec(
        "update session"
	" set leave_time=now()+interval '" + timeOut() + " minutes'"
	" where session_id=" + sess.str()
      );
      W.commit();
      return true;
    } else {
      return false;
    }
  } catch (const exception &e) {
    fprintf(stderr, "session poll error: %s\n", e.what());
    return false;
  }
}

string DataBrane::getUserName(int session_id) {
  ostringstream sess; sess << session_id;
  result R = doQuery(
    "select person.name as val from session,membership,person"
    " where membership.person_id=person.person_id"
    " and session.membership_id=membership.membership_id"
    " and session.session_id=" + sess.str());

  if(1 == R.size()) {
    string value;
    R[0]["val"] >> value;
    return value;
  } else {
    return "annonymous";
  }
}

string DataBrane::getUserTeam(int session_id) {
  ostringstream sess; sess << session_id;
  result R = doQuery(
    "select team.team_name as value from session,membership,team"
    " where team.team_id=membership.team_id"
    " and session.membership_id=membership.membership_id"
    " and session.session_id=" + sess.str());

  if(1 == R.size()) {
    string value;
    R[0]["value"] >> value;
    return value;
  } else {
    return "MDRS crew";
  }
}

string DataBrane::getUserRole(int session_id) {
  ostringstream sess; sess << session_id;
  result R = doQuery(
    "select role.title as value from session,membership,role"
    " where role.role_id=membership.role_id"
    " and session.membership_id=membership.membership_id"
    " and session.session_id=" + sess.str());

  if(1 == R.size()) {
    string value;
    R[0]["value"] >> value;
    return value;
  } else {
    return "guest";
  }
}

string DataBrane::getMembership(int session_id) {
  ostringstream sess; sess << session_id;
  result R = doQuery(
    "select session.membership_id as value from session"
    " where session.session_id=" + sess.str());

  if(1 == R.size()) {
    string value;
    R[0]["value"] >> value;
    return value;
  } else {
    return "0";
  }
}

void DataBrane::adhoc(string sql) {
  doQuery(sql);
}

result DataBrane::doQuery(string sql) {
  result r;
  try {
    work W(*C);
    result r = W.exec(sql);
    W.commit();
    return r;
  } catch (const exception &e) {
    fprintf(stderr,"postgresql command error\n");
    fprintf(stderr,"error=%s\n", e.what());
  }
  return r;
}

//private members
void DataBrane::hookup() {
  memberSet = 0;
  personSet = 0;
  pointSet = 0;
  quickGuideSet = 0;
  roleSet = 0;
  teamSet = 0;

  try {
    C = new pqxx::connection(connectYarn);
  } catch (const exception &e) {
    fprintf(stderr,"%s\n", e.what());
  }
}
