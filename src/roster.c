#include <string.h>
#include <stdio.h>
#include "session.h"
#include "dataBrane.h"

void parseRosterForm(char *buffer) {
char *word, *dayOne, *dayEnd, token[16];
int personId, teamId, roleId;
  //printf("    %s<br>\n",buffer);
  for(int row=0;row<10;++row) {
    word = strtok((row==0)?buffer:0,"=");
    sprintf(token,"personId_%d",row);
    if(strcmp(token,word)) {
      printf("OOPS: expected %s but got %s<br>\n", token, word);
      return;
    }
    word = strtok(0,"&");
    sscanf(word,"%d",&personId);

    word = strtok(0,"=");
    sprintf(token,"roleId_%d",row);
    if(strcmp(token,word)) {
      printf("OOPS: expected %s but got %s<br>\n", token, word);
      return;
    }
    word = strtok(0,"&");
    sscanf(word,"%d",&roleId);

    word = strtok(0,"=");
    sprintf(token,"teamId_%d",row);
    if(strcmp(token,word)) {
      printf("OOPS: expected %s but got %s<br>\n", token, word);
      return;
    }
    word = strtok(0,"&");
    sscanf(word,"%d",&teamId);

    word = strtok(0,"=");
    sprintf(token,"enterDate_%d",row);
    if(strcmp(token,word)) {
      printf("OOPS: expected %s but got %s<br>\n", token, word);
      return;
    }
    dayOne = strtok(0,"&");

    word = strtok(0,"=");
    sprintf(token,"leaveDate_%d",row);
    if(strcmp(token,word)) {
      printf("OOPS: expected %s but got %s<br>\n", token, word);
      return;
    }
    dayEnd = strtok(0,"&");
    if(personId>=0) {
      printf("%d: person=%d role=%d team=%d active %s until %s<br>\n", 
          row, personId, roleId, teamId, dayOne,dayEnd);
      DataBrane *db = new DataBrane();
      db->putMembership(personId, roleId, teamId, dayOne, dayEnd);
    }
  }
}

void parsePersonForm(char *buffer) {
  char *name, *login, *password;
  name = strtok(buffer,"=");
  if(strcmp(name,"NAME")) {
    printf("OOPS: expected %s but got %s<br>\n", "NAME", name);
    return;
  }
  name = strtok(0,"&=");
  if(!strcmp(name,"LOGIN")) {
    printf("OOPS: A valid person name is required!<br>\n");
    return;
  }
  login = strtok(0,"=");
  if(strcmp(login,"LOGIN")) {
    printf("OOPS: expected %s but got %s<br>\n", "LOGIN", login);
    return;
  }
  login = strtok(0,"&=");
  if(!strcmp(login,"PASSWORD")) {
    printf("OOPS: A valid login name is required!<br>\n");
    return;
  }
  // for now password is NOT parsed or recorded
  for(char* l=name;*l;++l)if(*l=='+')*l=' ';
  DataBrane *db = new DataBrane();
  printf("add person named %s as %s with password %s<br>\n", name, login, (char*)"none");
  db->putPerson(name, login, (char*)"none");
}

void parseTeamForm(char *buffer) {
  char *teamName, *emailList;
  teamName = strtok(buffer,"=");
  if(strcmp(teamName,"TEAM_NAME")) {
    printf("OOPS: expected %s but got %s<br>\n", "TEAM_NAME", teamName);
    return;
  }
  teamName = strtok(0,"&=");
  if(!strcmp(teamName,"EMAIL_LIST")) {
    printf("OOPS: A valid team name is required!<br>\n");
    return;
  }
  emailList = strtok(0,"=");
  if(strcmp(emailList,"EMAIL_LIST")) {
    printf("OOPS: expected %s but got %s<br>\n", "EMAIL_LIST", emailList);
    return;
  }
  emailList = strtok(0,"&");

  char* l;
  for(l=teamName;*l;++l)if(*l=='+')*l=' ';
  // TODO: more needed here to handle the '@' and other odd characters
  //for(l=emailList;*l;++l)if(*l=='+')*l='_'; // render spaces as '_'

  DataBrane *db = new DataBrane();
  printf("add team named %s with NO EMAIL ALIAS<br>\n", teamName, emailList);
  db->putTeam(teamName, (char*)"");
}

void newTeamForm(string refresh) {
  printf("    <form name=\"add Team\" action=\"%s+T\" method=\"post\">\n", refresh.c_str());
  printf("     Create a new Team Named:<input type=\"text\" name=\"TEAM_NAME\" size=16 /><br>\n"
         "     with (optional) email List:<input type=\"text\" name=\"EMAIL_LIST\" size=16 value=\"none\"/><br>\n"
         "     <input type=\"submit\" value=\"(click HERE to register the new team)\" />\n"
         "    </form>\n");
}

void newPersonForm(string refresh) {
  printf("    <form name=\"add Person\" action=\"%s+P\" method=\"post\">\n", refresh.c_str());
  printf("     Register a new Person Named:<input type=\"text\" name=\"NAME\" size=16 /><br>\n"
         "     with user login on identity:<input type=\"text\" name=\"LOGIN\" size=16 /><br>\n"
         "     and (optional) new password:<input type=\"text\" name=\"PASSWORD\" size=16 value=\"none\" /><br>\n"
         "     <input type=\"submit\" value=\"(click HERE to register the new person)\" />\n"
         "    </form>\n");
}

void teamRosterForm(string refresh) {
  DataBrane *db = new DataBrane();
  DataBrane::ROLE *role = db->getRoleSet();
  int numRole = db->roleCount();

  DataBrane::TEAM *team = db->getTeamSet();
  int numTeam = db->teamCount();

  DataBrane::PERSON *person = db->getPersonSet();
  int numPerson = db->personCount();

  int i,j;

  printf("    This form supports registering known people to specific roles in known teams<br>\n"
         "    <form name=\"add Roster\" action=\"%s+R\" method=\"post\"><table>\n", refresh.c_str());
  for(i=0;i<10;++i) {
    printf("     <tr><td><SELECT name=personId_%d value=\"999\">\n", i);
    printf("      <OPTION VALUE=\"-1\" SELECTED=TRUE>  </OPTION>\n");
    for(j=0;j<numPerson;++j) printf(
           "      <OPTION VALUE=\"%d\" >%s</OPTION>\n", person[j].person_id, person[j].name.c_str());
    printf("     </SELECT></td>\n"
           "     <td><SELECT name=roleId_%d value=\"999\">\n", i);
    for(j=0;j<numRole;++j) printf(
           "      <OPTION VALUE=\"%ld\" >%s</OPTION>\n", role[j].role_id, role[j].title.c_str());
    printf("     </SELECT></td>\n"
           "     <td><SELECT name=teamId_%d value=\"999\">\n", i);
    for(j=0;j<numTeam;++j) printf(
           "      <OPTION VALUE=\"%ld\" >%s</OPTION>\n", team[j].team_id, team[j].team_name.c_str());
    printf("     </SELECT></td>\n"
           "     <td><input type=date name=enterDate_%d value=\"2000-01-01\"/></td>\n"
           "     <td><input type=date name=leaveDate_%d value=\"1999-12-30\"/></td></tr>\n",i,i);
  }
  printf("     <tr><td colspan=4 align=center>"
         "      WARNING: Once registered, there is no tool for editing the above roster! <br>\n"
         "      <input type=\"submit\" value=\"(click HERE to register the above roster)\" />\n"
         "     </td></tr>\n"
         "    </table></form>\n");
}

void parseArgs(int argc, char **argv, string refresh) {
  if(argc<=2) {
    printf("  <table border=\"1\" cellspacing=\"2\">\n"
           "   <tr><td>\n");
    newTeamForm(refresh);
    printf("   </td><td>\n");
    newPersonForm(refresh);
    printf("   </td></tr><tr><td colspan=2>\n");
    teamRosterForm(refresh);
    printf("   </td></tr>\n"
           "  </table>\n");
  } else {
    char buffer[4096];
    int got = fread(buffer,1,4096,stdin);
    printf("got %d bytes<br>\n", got);
    switch (argv[2][0]) {
    case 'T':
      parseTeamForm(buffer);
      break;
    case 'P':
      parsePersonForm(buffer);
      break;
    case 'R':
      parseRosterForm(buffer);
      break;
    default:
      printf("    %s<br>\n",argv[2]);
      printf("    %s<br>\n",buffer);
    }
    printf("  <a href=\"%s\">Go back to entry forms</a>\n", refresh.c_str());
  }
}

int main(int argc, char **argv) {
Session* session = new Session(argc, argv);
  session->spewTop();
  parseArgs(argc,argv, session->getRefreshLink());
  session->spewEnd();
}
