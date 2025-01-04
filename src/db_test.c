#include <iostream>
#include "dataBrane.h"

using namespace std;

int test_asset_list(DataBrane *db) {
  DataBrane::DATA_ASSET *guides = db->getQuickGuides();
  int size = db->quickGuideCount();
    
  ostringstream s; s << size;
  cout << "\n\nThere are " << s.str() << " Quick Guides as of today:\n";

  for(int i=0;i<size;i++) {
    cout << guides[i].uid << " , " << guides[i].order << " | "
      << guides[i].label << " is stored in " << guides[i].path << ".\n";
  }
  return 0;
}

int test_membership(DataBrane *db) {
  DataBrane::MEMBERS *member = db->getActiveMembers();
  int size = db->memberCount();
    
  ostringstream s; s << size;
  cout << "\n\nThere are " << s.str() << " ACTIVE team members as of today:\n";

  for(int i=0;i<size;i++) {
    cout << member[i].uid << " | " << member[i].name
      << " is a " << member[i].role
       << " of the " << member[i].team << " team.\n";
  }

  return 0;
}

int main() {
  DataBrane *db = new DataBrane();
  DataBrane::LOCATION *place = db->getPointSet();
  int size = db->pointCount();

  fprintf(stderr,"Found %d waypoints:\n", size );
  long mi=0,me=0,mn=0;
  for(int i=0;i<size;i++) {
    ostringstream u; u << place[i].uid;
    ostringstream e; e << place[i].easting;
    ostringstream n; n << place[i].northing;
    cout << " | " << u.str() << " | " << e.str() << " | " << n.str()
         << " | " << place[i].label << " |" << endl;

    mi = (mi > place[i].uid     ) ? mi : place[i].uid;
    me = (me > place[i].easting ) ? me : place[i].easting;
    mn = (mn > place[i].northing) ? mn : place[i].northing;
  }

  cout << "Add a new point 141 meters NE of others..\n";

  db->insertLocation(me+50, mn+50, "dot dot dot", mi+1);

  //also insert without setting the index

  db->insertLocation(me+100, mn+100, "dash dash dash");

  db->adhoc("UPDATE LOCATION SET label='test' WHERE location_id=2");

  test_membership(db);
  test_membership(db);
  test_membership(db);
  test_membership(db);
  test_asset_list(db);
  test_asset_list(db);
  test_asset_list(db);
}
