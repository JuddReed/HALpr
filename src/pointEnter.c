#include <iostream>
#include <math.h>
#include <string.h>
#include "session.h"
#include "dataBrane.h"

#define TOO_CLOSE 25.0

using namespace std;

void spewForm(string program, bool useUsngFormat) {
  cout <<
    "  <form name=\"snag a fix\" action=\"" << program << "\" method=\"post\">\n"
    "   <center>\n"
    "    <table>\n"
    "     <tr><th>ID</th><th>ZONE</th><th>EAST</th><th>NORTH</th>"
		"<th>DATUM</th><th>MAP LABEL</th></tr>\n"
    "      <td>xxx</td><td>"<<((useUsngFormat)?"12SWH":"12s")<<"</td>"
    "      <td align=right><input type=text name=easting  size=6 /></td>\n"
    "      <td align=right><input type=text name=northing size=7 /></td>\n"
    "      <td>NAD27</td>\n"
    "      <td align=right><input type=\"text\" name=\"label\" size=64 /></td>\n"
    "     </tr>\n"
    "    </table>\n"
    "    <input type=\"submit\" value=\"Record\" /><br>\n"
    "   </center>\n"
    "  </form>\n";
}

void parseLocation(char *buffer, bool useUsngFormat) {
long east, nort;

  if(NULL == fgets(buffer,1024,stdin)) {
      return;
  }

  char* w = strtok(buffer,"=");
  if (strcmp(w,"easting")) {
    printf("Sorry: bad format<br>");
    printf("Expected easting not %s<br>", w);
    return;
  }

  w=strtok(0,"&=");
  sscanf(w,"%ld", &east);

  w=strtok(0,"&=");
  if(strcmp(w,"northing")) {
    printf("Sorry: bad format<br>");
    printf("Expected northing not %s<br>", w);
    return;
  }

  w=strtok(0,"&=");
  sscanf(w,"%ld", &nort);

  w=strtok(0,"&=");
  if(strcmp(w,"label")) {
    printf("Sorry: bad format<br>");
    printf("Expected label not %s<br>", w);
    return;
  }

  char* label = w = strtok(0,"|\r\n");
  for(;*w;w++)if(*w=='+')*w=' ';

  if((nort<100)&&(east<100)) {
    printf("<b>Ignoring value out of bounds (%ld %ld: %s)</b><br>\n",east, nort, label);
    return;
  } else if((nort<1000)&&(east<1000)) {
    nort = 100 * nort;
    east = 100 * east;
  } else if((nort<10000)&&(east<10000)) {
    nort = 10 * nort;
    east = 10 * east;
  }
  if((nort<100000)&&(east<100000)) {
    nort += 4200000;
    east +=  500000;
  }
  else if((nort<4200000)||(east<500000) || (nort>4300000)||(east>600000)) {
    printf("<b>Ignoring value out of bounds<br/>(%ld %ld: %s)</b><br/>\n",east, nort, label);
    return;
  }

  DataBrane *db = new DataBrane();

  DataBrane::LOCATION *place = db->getPointSet();
  int size = db->pointCount();
  for(int i=0; i<size; i++) {
    if(TOO_CLOSE >= hypot( place[i].easting - east, place[i].northing - nort ) ) {
      printf("<b>Ignoring<br/>%s<br/>\nat 12S %ld %ld NAD27</b><br/>\n", label, east, nort);
      printf("<b>Its too close to<br/>%s<br/>at 12S %ld %ld NAD27</b><br/>\n",
          place[i].label.c_str(), place[i].easting, place[i].northing);
      delete db;
      return;
    }
  }

  db->insertLocation(east, nort, std::string(label));
  delete db;
  printf("<b>recorded (12s %ld %ld NAD27: %s)</b><br>\n",east, nort, label);
}

void editSelectedPoint(string program, bool usngFormat, int chosen) {
//todo(JR): LAZY BAD CODE HERE. Select just one not all and then linear walk!!!!
  DataBrane *db = new DataBrane();
  DataBrane::LOCATION *place = db->getPointSet((LOCATION_ORDER)0);
  int size = db->pointCount();

  for(int i=0;i<size;i++) if(place[i].uid == chosen) {
    cout <<
      "  Edit these values with caution! There is NO way to revert changes<br/>\n"
      "  <form name=\"fix a snag\" action=\"" << program << "\" method=\"post\">\n"
      "   <input type=HIDDEN name=uid value=\"" << chosen << "\" />\n"
      "   <center>\n"
      "    <table>\n"
      "     <tr><th>ZONE</th><th>EAST</th><th>NORTH</th><th>DATUM</th><th>MAP LABEL</th></tr>\n";
    
    if(usngFormat) {
      cout <<
      "      <td>12SWH</td>\n"
      "      <td align=right><input type=text name=easting  size=5 value=\""
      << place[i].easting%100000 << "\"/></td>\n"
      "      <td align=right><input type=text name=northing size=5 value=\""
      << place[i].northing%100000 << "\"/></td>\n";
    } else {
      cout <<
      "      <td>12s</td>\n"
      "      <td align=right><input type=text name=easting  size=6 value=\""
      << place[i].easting << "\"/></td>\n"
      "      <td align=right><input type=text name=northing size=7 value=\""
      << place[i].northing << "\"/></td>\n";
    }
    cout <<
      "      <td>NAD27</td>\n"
      "      <td align=right><input type=\"text\" name=\"label\" size=64 value=\""
      << place[i].label << "\"/></td>\n"
      "     </tr>\n"
      "    </table>\n"
      "    <input type=\"submit\" value=\"Update\" /><br>\n"
      "   </center>\n"
      "  </form>\n";
  }
}

int spewTable(bool usngFormat, int order) {
  DataBrane *db = new DataBrane();
  DataBrane::LOCATION *place = db->getPointSet((LOCATION_ORDER)order);
  int size = db->pointCount();

  cout <<
    "  <center>\n"
    "   <div class=\"scrollable\">\n"
    "    <table>\n"
    "     <tr><th>ID</th><th>ZONE</th><th>EAST</th><th>NORTH</th>"
    "<th>DATUM</th><th width=\"200\">LABEL PLACED ON MAP</th></tr>\n";

  for(int i=0;i<size;i++) {
    cout << "     <tr>";
    cout << "<td align=right>"<<place[i].uid<<"</td>";
    if(usngFormat) {
      cout << "<td>12SWH</td>";
      cout << "<td align=right>"<<place[i].easting%100000<<"</td>";
      cout << "<td align=right>"<<place[i].northing%100000<<"</td>";
    } else {
      cout << "<td>12s</td>";
      cout << "<td align=right>"<<place[i].easting<<"</td>";
      cout << "<td align=right>"<<place[i].northing<<"</td>";
    }
    cout << "<td>NAD27</td>";
    cout << "<td>"<<place[i].label<<"</td>";
    cout << "</tr>\n";
  }
  cout <<
    "    </table>\n"
    "   </div>\n"
    "  </center>\n";
}

bool parseArgs(int argc, char **argv, string refresh, int &order, int &id) {
bool useUsngFormat = false;
  if(argc>2) {
    if(0==strcasecmp(argv[2],"USNG")){
      useUsngFormat = true;
    } else if(0==strcasecmp(argv[2],"UTM")){
      useUsngFormat = false;
    } else printf("(unexpected argument in url)<br>");
  }
  if(argc>3) sscanf(argv[3],"%d",&order);
  if(argc>4) sscanf(argv[4],"%d",&id);
  
  cout << "<a href=" << refresh 
       << ((useUsngFormat)?"+UTM+":"+USNG+")
       << order << "+" << id
       << ">Switch to " 
       << ((useUsngFormat)?"UTM":"USNG")
       << "</a><br>\n";

  if(0 > id )return useUsngFormat;

  cout << "<br/><br/><ul>Sort by\n";
  cout  << "<li>\n";
  if(order!=0)cout << "<a href=" << refresh << ((useUsngFormat)?"+USNG":"+UTM")
    <<"+0+" << id <<">";
  cout << "order in DB";
  if(order!=0)cout << "</a>";
  cout  << "</li>\n";

  cout  << "<li>\n";
  if(order!=1)cout << "<a href=" << refresh << ((useUsngFormat)?"+USNG":"+UTM")
    <<"+1+" << id <<">";
  cout << "distance from MDRS";
  if(order!=1)cout << "</a>";
  cout  << "</li>\n";

  cout  << "<li>\n";
  if(order!=2)cout << "<a href=" << refresh << ((useUsngFormat)?"+USNG":"+UTM")
    <<"+2+" << id <<">";
  cout << "Easting";
  if(order!=2)cout << "</a>";
  cout  << "</li>\n";

  cout  << "<li>\n";
  if(order!=3)cout << "<a href=" << refresh << ((useUsngFormat)?"+USNG":"+UTM")
    <<"+3+" << id <<">";
  cout << "Northing";
  if(order!=3)cout << "</a>";
  cout  << "</li>\n";

  cout  << "</ul>\n";

  return useUsngFormat;
}

int main(int argc, char **argv) {
char buffer[1024];
int order = 0;
int chosen = -999;
Session* session = new Session(argc, argv);
  session->spewTop();
  printf("<table><tr><td valign=top>\n");
bool useUsngFormat = parseArgs(argc,argv, session->getRefreshLink(), order, chosen);
  if(0 <= chosen) {
    printf("</td><td>\n");
    editSelectedPoint(session->getRefreshLink(), useUsngFormat, chosen);
  } else {
    parseLocation(buffer,useUsngFormat);
    printf("</td><td>\n");
    spewTable(useUsngFormat, order);
    spewForm(session->getRefreshLink(), useUsngFormat);
  }
  printf("</td></tr></table>\n");
  session->spewEnd();
}
