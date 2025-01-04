#ifndef HABACTLOG_DATABRANE_H_
#define HABACTLOG_DATABRANE_H_
#include <iostream>
#include <string.h>
#include <pqxx/pqxx>
#include "timeString.h"

using namespace std;
using namespace pqxx;
    
enum TYPE_OF_CONTACT { CONTACT_PERSON, CONTACT_GROUP };
enum LOCATION_ORDER {DB_ENTRY, FROMHAB, EASTING, NORTHING};

class DataBrane {
  public:
    struct ACTIVITY {
      long uid;
      long systemId;
      long dataAssetId;
      string label;
      string description;
    };

    struct DATA_ASSET {
      long uid;
      long order;
      string label;
      string path;
    };

    struct LOCATION {
      int uid;
      long easting, northing;
      float precision;
      string label;
    };

    struct MEMBERS {
      int uid;
      string name, role, team;
     // int person_id, team_id, role_id;
      string enterDate, leaveDate;
    };

    struct OBSERVATION {
      long observation_id;
      long activ_param_id;
      long membership_id;
      long datetime;
      double value;
      string units;
    };

    struct PARAMETER {
      long uid;
      long systemId;
      long activityParameterId, stepNumber;
      string label, units, description, howToRead, context;
      int alarmMask;
      double lowAlarm, highAlarm;
    };

    struct PERSON_FACT {
      int uid;
      int person;
      string key;
      string value;
    };
  
    struct PERSON {
      int person_id;
      string name;
      string login;
      string password;
      PERSON_FACT *fact[];
    };

    struct REPORT {
      long report_id;
      string report_type;
      string report_date;
      string label;
      string author;
    };

    struct REPORT_ITEM {
      long report_item_id;
      long report_id;
      string key;
      int line;
      string value;
    };

    struct ROLE {
      long role_id;
      string title;
      string description;
    };

    struct SYSTEM {
      long uid;
      string label;
      string description;
    };

    struct TEAM {
      long team_id;
      string team_name;
      string email_list;
    };

    enum CAT_MODE {EDIT,SHOW};
    //enum TYPE_OF_CONTACT { CONTACT_PERSON, CONTACT_GROUP };

    DataBrane(string connString);
    DataBrane();

    void cat(const char *section, CAT_MODE mode);

    void dumpCreatesToFile(const char *filePath);
    void dumpToFile(const char *filePath);

    ACTIVITY* getSystemActions(int systemId);
    int activityCount();

    DATA_ASSET* getDataAssetById(int id);
    DATA_ASSET* getActivityForm(string actionId);
    DATA_ASSET* getQuickGuides();
    int quickGuideCount();

    LOCATION* getPointSet(LOCATION_ORDER orderBy=DB_ENTRY);
    int pointCount();
    void insertLocation(long easting,long northing, string label, int uid=-1, float precision=10.0);
    bool updateLocation(int uid, long easting,long northing, float precision, string label);

    MEMBERS* getActiveMembers();
    MEMBERS* getActiveMembers(bool current);
    int putMembership(int personId, int roleId, int teamId, char* dayOne, char* dayEnd);
    int memberCount();

    OBSERVATION* getListedParameterslastObservation(long parameter[], int count);
    OBSERVATION* getObservationsByParameter(long parameter);
    int observationCount();
    int putObservations(OBSERVATION facts[], int count);
    int updateObservations(OBSERVATION facts[], int count);

    PERSON* getPersonSet();
    int putPerson(char* name, char* login, char* password);
    int personCount();

    REPORT* getReportSet(char* report_id=NULL);
    int reportCount();
    int putReport(char* report_type, char* report_date,
        char* label, char* author);
    // author should be a person

    REPORT_ITEM* getReportItemSet(char* report_id=0);
    int reportItemCount();
    int putReportItem(long report_id, char* key, int line, char* value);

    ROLE* getRoleSet();
    int roleCount();

    PARAMETER* getSystemParameters(int systemId);
    PARAMETER* getActivityParameters(int activityId);
    int parameterCount();

    SYSTEM* getSystemList();
    int systemCount();

    TEAM* getTeamSet();
    int putTeam(char* team_name, char* email_list);
    int teamCount();

    string getEmailUrl(TYPE_OF_CONTACT contactType, string who);

    int signIn(int member_uid);
    bool testSession(int session_id);
    string getUserName(int session_id);
    string getUserRole(int session_id);
    string getUserTeam(int session_id);
    string getMembership(int session_id);

    void adhoc(string sql);
  private:
    void hookup();
    string timeOut() { return "59";}
    result doQuery(string sql);

    pqxx::connection* C;
    string connectYarn;

    ACTIVITY *activitySet;
    DATA_ASSET *quickGuideSet;
    LOCATION *pointSet;
    MEMBERS *memberSet;
    OBSERVATION *observationSet;
    PARAMETER *parameterSet;
    PERSON *personSet;
    REPORT *reportSet;
    REPORT_ITEM *reportItemSet;
    ROLE *roleSet;
    SYSTEM *systemSet;
    TEAM *teamSet;
    int numActivity, numMember, numObservation, numParameter, numPerson,
        numPoint, numQuickGuide, numReport, numReportItem, numRole, numSystem,
        numTeam;
};
#endif // HABACTLOG_DATABRANE_H_
