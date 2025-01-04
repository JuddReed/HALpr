#ifndef HAL_SESSION_H_
#define HAL_SESSION_H_
#include <iostream>
#include <pqxx/pqxx>
#include "dataBrane.h"

using namespace std;
using namespace pqxx;

class Session {
  public:
    ~Session() { spewEnd(); }
    Session(int argc=0, char **argv=0);
    void spewTop(string label = "Habitat Activity Logger");
    void spewEnd();
    string makeLinkString(string destinationPage, char* space);
    string homeInSession(){return homeStr;};
    string getSessionArg(){return sessStr;};
    string getRefreshLink(){return whoAmI;};
    string getMembership(){return userMembership;};
    string getUserRole(){return userRole;};
    string getUserName(){return userName;};
    string getUserTeam(){return userTeam;};

  private:
    string homeStr;
    string sessStr;
    string whoAmI;
    string userName, userRole, userTeam, userMembership;
    int session_id;

    int startSession(int argc, char **argv);
    bool testSession(int session_id);
    void pickMember(char *arg0);
};
#endif // HAL_SESSION_H_
