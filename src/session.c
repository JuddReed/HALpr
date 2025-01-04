#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "session.h"
#include "dataBrane.h"

using namespace std;

/**
 *
 * If called with no arguments or if only the program name is given,
 * Show a login menu with a list of valid users to choose among.
 *
 * If arg[0] is 'home' and role:# is specified, then create a new session
 * else if session:# is specified parse it
 * 
*/

Session::Session(int argc, char **argv) {
  homeStr = "/cgi-bin/home";
  sessStr = "sess:-1";
  userName = "you";
  userRole="guest";
  userTeam="MDRS crew";

  if (1 >= argc) {
    pickMember(argv[0]);
    return;
  }
 
  if ((0==strcmp(strrchr(argv[0],'/'),"/home"))
  &&  (0==strncmp(argv[1],"role:",5))) {
    startSession(argc, argv);
    return;
  }

  session_id = -1;
  if ( 0==strncmp(argv[1],"sess:",5) ) {
    session_id = atoi(strrchr(argv[1],':')+1);
  }

  if( !testSession(session_id) ) {
    spewTop();
    cout <<
      "   <br/>\n"
      "  <a href=/index.html>Exit to public Porch.</a><br/>\n"
      "   <br/>\n"
      "  <center>\n"
      "   Session interrupted.\n"
      "   <br/><a href=" << homeStr << ">Please sign in (again).</a><br/>\n"
      "  </center>\n";
    spewEnd();
  }
  
  ostringstream sis; sis << session_id;
  sessStr = "sess:" + sis.str();
  whoAmI = "/cgi-bin" + string(strrchr(argv[0],'/')) + "?" + sessStr;
}

void Session::pickMember(char *arg0) {
  DataBrane *db = new DataBrane();
  DataBrane::MEMBERS *member = db->getActiveMembers();
  int choices = db->memberCount();
  spewTop();
  cout <<
    "  <a href=/index.html>Exit to public Porch.</a><br/>\n"
    "  <center>\n"
    "    <h2>Sign in Form</h2>"
    "  <table>\n"
    "   <tr><th colspan=3>PICK YOUR NAME AND ROLE:</th></tr>\n";
  for(int r=0;r<choices;r++) {
    cout << "   <tr><td><a href=/cgi-bin/home?role:" << member[r].uid << ">\n";
    cout << member[r].name << " as the ";
    cout << member[r].role << " of the ";
    cout << member[r].team << " Team.</a></td></tr>\n";
  }
  cout << "  </table>\n";
  cout << "  </center>\n";
  spewEnd();
}

string Session::makeLinkString(string destinationPage, char* space) {
  sprintf(space, "/cgi-bin/%s?sess:%d", destinationPage.c_str(), session_id);
  return(  string(space) );
}

int Session::startSession(int argc, char **argv) {
  spewTop();
  int role_id = atoi(strrchr(argv[1],':')+1);
  DataBrane *db = new DataBrane();
  session_id = db->signIn(role_id);
  delete db;

  cout << "  <center>\n";

  if( testSession(session_id) ) {
    cout << "   Hello " << userName << ",<br>";
    cout << userRole << " of the " << userTeam << " Team.<br>";
  }

  cout << "   <h2>Welcome to Mars</h2>\n";
  cout << "   <br><a href=/cgi-bin"
    << string(strrchr(argv[0],'/'))
    << "?sess:" << session_id;
  for(int argn=2;argn<argc;argn++)
    cout << ":" + string(argv[argn]);

  cout << ">CONTINUE</a><br><br>\n";
  cout << "  </center>\n";
  spewEnd();
}

bool Session::testSession(int session_id) {
  DataBrane *db = new DataBrane();
  if ( db->testSession(session_id) ) {
    userName = db->getUserName(session_id);
    userRole = db->getUserRole(session_id);
    userTeam = db->getUserTeam(session_id);
    userMembership  = db->getMembership(session_id);
    delete db;
    return true;
  }
  return false;
}

void Session::spewTop(string label) {
  cout <<
    "Content-type: text/html\n\n"
    "<html lang=\"en-US\" xml:lang=\"en-US\" xmlns =\"http://www.w3.org/1999/xhtml\">\n"
    " <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\" />\n"
    " <head>\n"
    "  <title>" << label << "</title>\n"
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
    "     <a href=" << homeStr << "?" << sessStr << ">\n"
    "      <img src=/haleye.gif width=200 height=200>\n"
    "     </a>\n"
    "    </th>\n"
    "    <th valign=center>\n"
    "     <h1>" << label << "</h1>\n"
    "    </th>\n"
    "   </tr>\n"
    "  </table>\n"
  ;
}

void Session::spewEnd() {
  cout << 
    " </body>\n"
    "</html>\n";
  exit(1);
}
