#include <iostream>
#include <string.h>
#include "session.h"
#include "dataBrane.h"

using namespace std;
char* thisHost() {
//FILE* c = popen("ifconfig wlan0 | grep 'inet add'| sed -e 's-.*addr:--' -e 's: .*::'","r");
FILE* c = popen("ifconfig eth0 | grep 'inet add' |sed -e 's: Bcast.*::' -e 's-.*:--'","r");
char buff[512];
  int got = fscanf(c,"%s",buff);
  pclose(c);

printf("ip=%s\n",buff);

//return "this host";
return strdup(buff);
}

struct Document {
string label, filename;
} manual[] = {
  {"Yahama Griz450 ATV", "lit-11626-21-46_griz450_4wd_1495.pdf"},
  {"MOLLE Pack Op Manual","10-8465-236-10.pdf"},
  {"Dual Flush Toilet","0GU031_DUAL_FLUSH_TOILET_IM.pdf"},
  {"e-trex GPS","eTrexOwnersManual.pdf"},//http://www8.garmin.com/manuals/eTrex_OwnersManual_SoftwareVersion3.00andabove_.pdf
  {"hp EliteBook 6930p","halLaptopHardware.pdf"},//http://h18000.www1.hp.com/products/quickspecs/13061_na/13061_na.pdf
  {"MODBUS Implementation Guide", "Modbus_Messaging_Implementation_Guide_V1_0b.pdf"},//http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf
  {"","theEnd"} },
  reference[] = {
  {"Reading UTM", "ReadingUTM.pdf"},
  {"Rodents of Utah", "rodents.pdf"}, // http://wildlife.utah.gov/education/newsletters/01fallwinter-gw.pdf
  {"Geology Study by Peterson and Roylance", "Geo-Stud-Vol-29-pt-2-Peterson-Roylance.pdf"},
  {"Salina Geologic Map", "SalinaGeolMap.pdf"},
  {"Geologic Map of the San Rafael Desert","ofr-404.pdf"},
  {"Water Pocket Fold Geology", "capitol.pdf"},
  {"Capitol Reef Geol Map", "CARE_glg.jpg"}, // http://www.nps.gov/gis/mapbook/mappingparks/CARE_glg.html
  {"San Rafael Swell cross section", "GeologySanRafael.jpg"},
  {"San Rafael Swell section", "images.jpg"},
  {"San Rafael Swell Western Flank", "SRS western flank.jpg"},
  {"Capitol Reef Strat Column", "Cap_Reed_Strat.pdf"},
  {"Foraminifera in Mancos Shale", "Geo-Stud-Vol-23-pt3-Maxfield.pdf"},
  {"Cretaceous Ferron Sandstone", "Geo-Stud-Vol-26-Part-2-Uresk.pdf"},
  {"Morrison Formation Stratigraphy", "Geo-Stud-Vol-29-pt-2-Peterson-Roylance.pdf"},
  {"Grand Junction Strat Column", "GrandJuncStrat.jpg"},
  {"GSENM Strat column", "GSENM strat with Sooner Rocks article.jpg"},
  {"Hanksville Strat Column", "HanksvilleStrat.pdf"},
  {"Ferron Facies", "Ferron_Facies.pdf"},//http://www.ariesgroup.net/images/Ferron_Facies.pdf
  {"Ferron Sandstone Stratigraphy", "Muddy_Canyon_North.pdf"},
  {"Old Geology Paper", "report.pdf"},
  {"another Old Geology Paper", "usgs-pp228-henry-mtns-geo-geography.pdf"},//http://ia700305.us.archive.org/0/items/UsgsPp228HenryMountainsReport/usgs-pp228-henry-mtns-geo-geography.pdf
  {"yet another Geology Paper", "AgeBrushyBasin_91.pdf"},//http://www.geology.byu.edu/wp-content/uploads/2010/06/AgeBrushyBasin_91.pdf

  {"","theEnd"} },

  marsScience[] = {
  {"Geologic Map and Strat Column of Mars", "mars_geol_marineris_s2_2x2sm.jpg"}, //http://blogs.agu.org/martianchronicles/2010/04/07/weve-come-a-long-way/
  {"","theEnd"} };

void showDocmentInventory(Session *sess) {
  cout <<
    " <h3>Here are some useful documents copied from the web and other sources.</h3>\n"
    " <table><tr><td valign=top><ul><h3>Operations Manuals</h3>\n";

  for(int i=0;manual[i].label.length()>0;i++) {
    cout << "  <li><a href=/cgi-bin/libraryView?" << sess->getSessionArg()
        << "+m+" << i << ">" << manual[i].label << "</a></li>\n";
  }

  cout << " </ul></td><td valign=TOP><ul><h3>Reference Documents Mirrored on site</h3>\n";

  for(int i=0;reference[i].label.length()>0;i++) {
    cout << "  <li><a href=/cgi-bin/libraryView?" << sess->getSessionArg()
        << "+r+" << i << ">" << reference[i].label << "</a></li>\n";
  }
  cout << " </ul></td></tr</table>\n";
}

void frameTheDocument(Session *sess, char** argv, char* host) {
  int index = -1;
  sscanf(argv[3],"%d",&index);

  cout << " <OBJECT width=\"95\%\" height=\"66\%\" data=\"http://" << host << "/importedDocuments/" 
    << ((strcmp(argv[2],"r"))?manual[index]:reference[index]).filename <<"\">\n"
    << " Document not found. please report this defect<br/>\n"
    << "</OBJECT><br/>\n";
  
  cout << "  <a href=/cgi-bin/libraryView?" << sess->getSessionArg()
     << ">List of on site documents</a>\n";
  cout << "  <a href=http://" << host << "/cgi-bin/home?" << sess->getSessionArg() 
      << ">HALp main first page</a>\n";
}

char *hostname(char **env) {
char buffer[1024];
  for ( char** e = env; *e; e++) {
    char* a = strcpy(buffer,*e);
    char* b = strtok(a,"=");
    if ( !strcmp(b,"HTTP_HOST") )
     return strdup(strtok(0,"="));
  }
  return NULL;
}

int main(int argc, char **argv, char **env) {
Session *sess = new Session(argc,argv);

  sess->spewTop("MDRS Habitat Activity Logger (prototype)");
  
  if(argc>3)
    frameTheDocument(sess, argv, hostname(env));
  else showDocmentInventory(sess);

  delete sess;
}
