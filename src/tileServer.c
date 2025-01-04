#include <string.h>
#include <math.h>
#include <iostream>
#include "dataBrane.h"
#include "session.h"

//#define ROAD_GML_PATH "data/WayCntyNAD27.gml"
#define ROAD_GML_PATH "data/roadCoords.txt"

#define ROADBIT 1	// Show Roads
#define WAYPBIT 2	// show waypoints
#define ZDIRBIT 4	// Zoom in/out toggle
#define CLIKBIT 8	// MAP MODE: 0=browse 1=location picker
#define TYP0BIT 16	//   
#define TYP1BIT 32	//  > Base map type 0=photo+topo 1=topo 2=geology, 3=elevation
#define TYP2BIT 64	//  

//#define SHOW_FIDUCIALS

void sendPageTop(const char* program, const char *label, const char* sessionArg) {
  printf(
    "Content-type: text/html\n\n"
    "<html>\n"
    " <head>\n"
    "  <title>%s</title>\n", label);
  printf(
    "  <script language=\"JavaScript\">\n"
    "   function relMouseCoords(event, west,north,scale,extent){\n"
    "    var totalOffsetX = 0;\n"
    "    var totalOffsetY = 0;\n"
    "    var canvasX = 0;\n"
    "    var canvasY = 0;\n"
    "    var currentElement = document.getElementById(\"map_canvas\");\n"
    "    do{\n"
    "     totalOffsetX += currentElement.offsetLeft - currentElement.scrollLeft;\n"
    "     totalOffsetY += currentElement.offsetTop - currentElement.scrollTop;\n"
    "     currentElement = currentElement.offsetParent;\n"
    "    }\n"
    "    while(currentElement)\n"
    "    canvasX = event.pageX - 3 - totalOffsetX;\n"
    "    canvasY = event.pageY - 3 - totalOffsetY;\n"
    "    document.pointform.easting.value = (canvasX * scale)/extent + west;\n"
    "    document.pointform.northing.value = north - (canvasY * scale)/extent;\n"
    "    document.pointform.label.value = \"\";\n"
    "   }\n"
    "  </script>\n"
    " </head>\n"
    " <body>\n");
  printf(
    "  <form name=\"pointform\" method=\"post\" "
        "action=\"/cgi-bin/pointEnter?%s\">\n", sessionArg);
  printf(
    "   <table border=2><tr>\n"
    "    <td valign=TOP>\n"
    "     <table valign=top>\n"
    "      <tr><th>\n"
    "       <a href=%s?%s\"><img src=/haleye.gif width=200 height=200></a>\n",
    program, sessionArg);
  printf(
    "      </th></tr><tr><th valign=center>\n"
    "       <h1>%s</h1>\n"
    "      </th></tr>\n"
    "     </table>\n", label);
}

enum ZOOM_DIR { ZOOM_IN, ZOOM_OUT};
string zoomString(ZOOM_DIR zDir, char *buffer) {
  switch(zDir) {
  case ZOOM_IN:
    sprintf(buffer,"IN");
    break;
  case ZOOM_OUT:
    sprintf(buffer,"OUT");
    break;
  }
  return buffer;
}
string zoomString(int zDir) { return (zDir)?"IN":"OUT"; }

string THIS_PAGE;

const int kBaseChoiceCount = 4;
const string kBaseTypeChoice[] = {"Photo", "Quad", "Geol", "Elev"};
const char* kLinkFormat[kBaseChoiceCount] = {
  " xlink:href=\"/mapChip/orth_%d_%d.bmp\"",
  " xlink:href=\"/mapChip/topo_%d_%d.gif\"",
  " xlink:href=\"/mapChip/geol_%d_%d.bmp\"",
  " xlink:href=\"/mapChip/elev_%d_%d.bmp\""};

void spew_chip_link(int type, int e, int n) {
  printf(kLinkFormat[type], e,n);
}


const int kTokenSize = 250; // 1/2 of the size of the smallest chips
const int kWorldSize = 32000;

class ActionVector {
private:
  int inpEasting, inpNorthing;
  int south, west; // lowest coordinates shown
  int chipSize; // the width and height of each tile 62,125,250,...A
  int gridSize; //1,2,3,3,...3,2


  // only called by constructor. returns value that becomes chipSize
  int findZoomSize(int coord) {
    for(int s=kTokenSize/2;s<kWorldSize;s*=2)
      if(coord%s)return s;
    return kWorldSize;
  }

  // only called by constructor. returns value the becomes gridSize
  int setUpGrid(int e, int n) {
    gridSize = 3;
    if(chipSize < kTokenSize)gridSize =  1;
    if(chipSize <= kTokenSize)gridSize =  2;
    if(chipSize >= kWorldSize/2)gridSize = 2;
    
    if(chipSize < 2*kTokenSize) chipSize = 2*kTokenSize;

    if(gridSize == 2) {
      e = e + chipSize/2;
      n = n + chipSize/2;
    }
    west = e - (e%chipSize);
    south = n - (n%chipSize);
    if(gridSize != 1) {
      west -= chipSize;
      south -= chipSize;
    }
  }

public: 
  ActionVector(int e, int n) {
    // use resolution of coordinates to determine zoom level
    chipSize = findZoomSize(e);
    if(chipSize != findZoomSize(n))
      chipSize = kTokenSize;
    inpEasting = e;
    inpNorthing = n;
    setUpGrid(e,n);
  }

  int getGridSize() { return gridSize; }
  int imageSize() { return chipSize; }
  int getWest()  { return west; }
  int getSouth() { return south; }

  int extent(){ return chipSize * gridSize; }

  //c=0,r=0 is the NW chip not the center
  int pictCenterEast (int c) { return west + (2*c+1) * chipSize / 2; }
  int pictCenterNorth(int r) { return south+extent()-(2*r+1)*chipSize/2;}

  int panCenterEast(int column) { return inpEasting + imageSize() * column;}
  int panCenterNort(int row)    { return inpNorthing - imageSize() * row;}

  /**
   * Clicks in the center of the screen lead to zoom in or zoom out.
   * This is done by linking the 4 quadrents of the center image to
   * another image at either the next of previous zoom level.
   */
  int zoomCenterEast(int c, ZOOM_DIR direction) {
    int e = west + extent()/2; // center of screen
    if(direction == ZOOM_OUT) {
      e = e - e%(chipSize*2) + chipSize;
    } else {
      e -= chipSize/4; // center of next smaller chip on left
      e += c * chipSize/2; // adjusted for chip region;
    }
    return e;
  }
  int zoomCenterNort(int r, ZOOM_DIR direction) {
    int n = south + extent()/2; // center of screen
    if(direction == ZOOM_IN) {
      n += chipSize/4; // center of next smaller chip on left
      n -= r * chipSize/2; // adjusted for chip region;
    } else {
      n = n - n%(chipSize*2) + chipSize;
    }
    return n;
  }
}; // end of ActionVector class



void panZoomTable(ActionVector* situation, int baseType) {
  int size = situation->extent();
  int east = situation->getWest() + size/2;
  int nort = situation->getSouth() + size/2;
  int step = situation->imageSize();//size/3;
/*
  if(size>=24000)step = 0;
  else if(size<=1000)step = 500;
  else step = size/3;
*/
  cout << "    <br><center><table><tr>\n"
       << "     <td align=left><a href=/" << THIS_PAGE
       << "+" << east-step <<"+"<< nort+step << "+" << baseType << ">NW</a></td>\n"
       << "     <td align=center><a href=/" << THIS_PAGE
       << "+" << east        <<"+"<< nort+step << "+" << baseType << ">N</a></td>\n"
       << "     <td align=right><a href=/" << THIS_PAGE
       << "+" << east+step <<"+"<< nort+step << "+" << baseType << ">NE</a></td>\n"
       << "    </tr><tr>\n"
       << "     <td align=left rowSpan='2'><a href=/" << THIS_PAGE
       << "+" << east-step <<"+"<< nort << "+" << baseType << ">W</a></td>\n";

  if(size>=32000)cout << "<td/>\n";
  else if(size==24000)cout
       << "     <td align=center><a href=/" << THIS_PAGE + "+504000+4248000+"
       << baseType << ">OUT</a></td>\n";
  else if(size==12000)cout 
       << "     <td align=center><a href=/" << THIS_PAGE + "+516000+4252000+"
       << baseType << ">OUT</a></td>\n";
  else cout << "     <td align=center><a href=/" << THIS_PAGE
       << "+" << situation->zoomCenterEast(0,ZOOM_OUT) <<"+"<< situation->zoomCenterNort(0,ZOOM_OUT) <<"+" << baseType << ">OUT</a></td>\n";
  cout << "     <td align=right rowSpan='2'><a href=/" << THIS_PAGE
       << "+" << east+step <<"+"<< nort << "+" << baseType << ">E</a></td>\n"
       << "    </tr><tr>\n";
  if(size>500)cout << "     <td align=center><a href=/" << THIS_PAGE
       << "+" << situation->zoomCenterEast(0,ZOOM_IN) <<"+"<< situation->zoomCenterNort(0,ZOOM_IN) <<"+" << baseType << ">IN</a></td>\n";
  cout << "    </tr><tr>\n"
       << "     <td align=left><a href=/" << THIS_PAGE
       << "+" << east-step <<"+"<< nort-step << "+" << baseType << ">SW</a></td>\n"
       << "     <td align=center><a href=/" << THIS_PAGE
       << "+" << east        <<"+"<< nort-step << "+" << baseType << ">S</a></td>\n"
       << "     <td align=right><a href=/" << THIS_PAGE
       << "+" << east+step <<"+"<< nort-step << "+" << baseType << ">SE</a></td>\n"
       << "    </tr></table></center>\n";
}

void simpleLeftPanel(ActionVector* situation,  int baseType) {
  int size = situation->extent();
  int east = situation->getWest() + size/2;
  int nort = situation->getSouth()+ size/2;
  int onBase = baseType / TYP0BIT;
  int nextBase = (onBase+1)%kBaseChoiceCount;
      nextBase = baseType%16 + nextBase*16;
  char pageRoot[64];
  sprintf(pageRoot,"/%s+%d+%d",THIS_PAGE.c_str(), east, nort);

  cout << "     <br>\n";
  cout << "     <a href=/" << THIS_PAGE << "+" << east <<"+"<< nort <<"+"
       << nextBase <<">Base Map Type = " << kBaseTypeChoice[onBase] << "</a><br>\n";

  cout << "     <a href=/" << THIS_PAGE << "+" << east <<"+"<< nort << "+" << (baseType^WAYPBIT)
       << ">" << ((baseType&WAYPBIT)?"hide":"show") << " way points</a><br>\n";


  printf( "     <a href=\"%s+%d\">%s open roads</a><br>\n", pageRoot,baseType^ROADBIT,
    (baseType&ROADBIT)?(char*)"hide":(char*)"show");

  if (baseType&CLIKBIT) {
    printf("     <a href=\"%s+%d\">Click Map to Browse</a><br>\n", pageRoot,baseType^CLIKBIT);
    printf("     <a href=\"%s+%d\">Click map center to zoom %s</a><br>\n",pageRoot,baseType^ZDIRBIT,
      zoomString(baseType&ZDIRBIT).c_str() );
  } else {
    printf("     <a href=\"%s+%d\">Click Points on Map</a><br>\n", pageRoot,baseType^CLIKBIT);
    printf("     <a href=\"%s+%d\">Click map %s Destnations</a><br>\n",pageRoot,baseType^ZDIRBIT,
      (baseType&ZDIRBIT)?(char*)"CREATE":(char*)"SELECT");
    panZoomTable(situation,baseType);
    if(baseType&ZDIRBIT) {
      printf("UTM 12s <input type=\"text\" name=\"easting\" size=\"6\" />\n"
         "E, <input type=\"text\" name=\"northing\" size=\"7\" />N<br>\n"
         "Label:<input type=\"text\" name=\"label\" size=\"24\"/><br>\n"
         "<input type=\"submit\" value=\"Record\" /><br>\n");
    } else {
    }
  }
}

/***************** This paints a fiducial and coordinates at the center of the tile *****/
void paintFiducial(int x,int y,int size, int east, int north) {
   printf(" <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\""
          " style=\"stroke:black;stroke-width:2\"/>\n", x+size, y-5+size, x+size, y+5+size);
   printf(" <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\""
          " style=\"stroke:black;stroke-width:2\"/>\n", x-5+size, y+size, x+5+size, y+size);
   printf(" <text x=\"%d\" y=\"%d\" font-size=\"12\" fill=\"black\">"
          " %d</text>\n", x+11+size, y-2+size, east);
   printf(" <text x=\"%d\" y=\"%d\" font-size=\"12\" fill=\"black\">"
          "%d</text>\n", x+5+size, y+9+size, north);
}

void makeSvgTiles(ActionVector* situation, int baseType) {
  int tiles = situation->getGridSize();
  int paintSize = 600 / tiles;

  for(int r=0;r<tiles;r++)for(int c=0;c<tiles;c++) {
    int x = c*paintSize;
    int y = r*paintSize;
    cout << "      <image x=\"" << x << "\" y=\"" << y
         << "\" width=\"" << paintSize << "\" height=\"" << paintSize << "\"";
    spew_chip_link(baseType/TYP0BIT, situation->pictCenterEast(c), situation->pictCenterNorth(r) );
    cout << "/>\n";
    
#ifdef SHOW_FIDUCIALS
    // Paint a fiducial and coordinates at the center of the tile *****/
    paintFiducial(x,y,paintSize/2, situation->pictCenterEast(c), situation->pictCenterNorth(r));
#endif // SHOW_FIDUCIALS
  }
}


void svgOpenRoads(ActionVector* situation, int sizeSvg) {
int sizeMeters = situation->extent();
int boxW = situation->getWest();
int boxS = situation->getSouth();
int boxN = boxS + sizeMeters;
int boxE = boxW + sizeMeters;
int east, nort;

  if(boxS<4247000)boxS = 4247000;// added constraint to keep on the map

FILE* gml = fopen(ROAD_GML_PATH, "r");
if(gml<=0){fprintf(stderr,"NO ROAD DATA in %s\n", ROAD_GML_PATH);return;}

int wasIn, nowIn = 0;
  while(2==fscanf(gml,"%d,%d",&east,&nort)) {
    wasIn = nowIn;
    nowIn = ((boxW<east)&&(east<boxE)&&(boxS<nort)&&(nort<boxN))?1:0;
    if(nowIn && !wasIn) {
      cout <<
        "      <polyline style=\"fill:none;stroke:blue;stroke-width:5\"\n"
        "        points=\"";
    }
    if(nowIn)
      cout << " " << ((east-boxW)*sizeSvg)/sizeMeters << "," << (((boxN-nort)*sizeSvg)/sizeMeters);
    if(wasIn && !nowIn) cout << "\" />\n";
  }
  fclose(gml);
}

void shadedRing(int col, int row, int radius, char* color, float opacity) {
 
  printf("      <circle cx=\"%d\" cy=\"%d\" r=\"%d\""
         " style=\"fill:none;stroke:%s;stroke-width:2\" />\n",
    col,row, radius, color );

  printf("      <circle cx=\"%d\" cy=\"%d\" r=\"%d\" "
         "style=\"fill:yellow;stroke:%s;stroke-width:2;opacity:%f\"/>\n",
    col,row, radius, color, opacity );
}

void svgNoMotorZone(ActionVector* situation) {
int size = situation->extent();
int centEast = situation->getWest() + size/2;
int centNorth= situation->getSouth()+ size/2;
int PIXELS=600;
int ringE =  518250;
int ringN = 4250450;
int radius = 1200;
int airportE =  525000;
int airportN = 4252000;
int fiveMiles = (int)(5.0 * 1609.34) + 250;// 5 miles plus a small buffer
 
  int col = ((ringE-centEast)*PIXELS)/size + PIXELS/2;
  int row = ((centNorth-ringN)*PIXELS)/size + PIXELS/2;
  shadedRing(col,row, (radius*PIXELS)/size, (char*)"red", 0.1);
    
  col = ((airportE-centEast)*PIXELS)/size + PIXELS/2;
  row = ((centNorth-airportN)*PIXELS)/size + PIXELS/2;
  shadedRing(col,row, (fiveMiles*PIXELS)/size, (char*)"green", 0.00);
}

void svgWaypoints(ActionVector* situation, int sizeSvg) {
int sizeMeters = situation->extent();
int boxW = situation->getWest();
int boxS = situation->getSouth();
int boxN = boxS + sizeMeters;

DataBrane::LOCATION *place;
DataBrane *db = new DataBrane();
  place = db->getPointSet();
int size = db->pointCount();

  for(int i=0;i<size;i++) {
    int col = ((place[i].easting-boxW)*sizeSvg)/sizeMeters;
    int row = ((boxN-place[i].northing)*sizeSvg)/sizeMeters;

    if((row>=0)&&(row<600)&&(col>=0)&&(col<600)) {
      printf("      <circle cx=\"%d\" cy=\"%d\" r=3 stroke=\"red\"/>\n", col,row);
      printf("      <text x=\"%d\" y=\"%d\" font-size=\"12\" fill=\"red\">",
       col+8, row+4);
      printf("%d) %s", place[i].uid, place[i].label.c_str());
      printf("</text>\n");
    }
  }
  delete db;
}

/************************
#define WAYPOINT_PATH "data/newWayPoints"

void RETIRED_svgWaypoints(ActionVector* situation, int sizeSvg) {
char buffer[1025];
int sizeMeters = situation->extent();
int boxW = situation->west();
int boxS = situation->south();
int boxN = boxS + sizeMeters;
FILE* dots = fopen(WAYPOINT_PATH, "r");
 
  int east, nort, col,row;
  while(0<fgets(buffer,1024,dots)) {
    sscanf(buffer,"%d %d", &east, &nort);
    col = ((east-boxW)*sizeSvg)/sizeMeters;
    row = ((boxN-nort)*sizeSvg)/sizeMeters;

    if((row>=0)&&(row<600)&&(col>=0)&&(col<600)) {
      printf(" <circle cx=\"%d\" cy=\"%d\" r=3 stroke=\"red\"/>\n", col,row);
      printf(" <text x=\"%d\" y=\"%d\" font-size=\"12\" fill=\"red\" >\n",
       col+8,row+4);
      printf("%s",buffer);
      printf("</text>\n");
    }
  }
  fclose(dots);
}
/***********************/

void svgShowGoMap(ActionVector* situation, int flags) {
  for(int r=-1;r<=1;r++) for(int c=-1;c<=1;c++) if((r!=0)||(c!=0)) {
    printf(" <text x=\"%d\" y=\"%d\" font-size=\"12\" fill=\"black\" >%d</text>\n",
      (c+1)*200+60, (r+1)*200+90, situation->panCenterEast(c));
    printf(" <text x=\"%d\" y=\"%d\" font-size=\"12\" fill=\"black\" >%d</text>\n",
      (c+1)*200+60, (r+1)*200+110, situation->panCenterNort(r));
      
    printf("      <rect x=\"%03d\" y=\"%03d\" width=\"200\" height=\"200\""
           " style=\"fill:none\" />\n", (c+1)*200,(r+1)*200);
    printf("     </a>\n");
  }
}

void svgActionMap(ActionVector* situation, int flags) {
printf("<!--- Action Map pan -->\n");
  for(int r=-1;r<=1;r++) for(int c=-1;c<=1;c++) if((r!=0)||(c!=0)) {
    printf("      <a xlink:href=\"/%s+%d+%d+%d\">\n", THIS_PAGE.c_str(), 
     situation->panCenterEast(c), situation->panCenterNort(r), flags);
    printf("       <rect x=\"%03d\" y=\"%03d\" width=\"200\" height=\"200\""
           " style=\"opacity:0.0001\" />\n", (c+1)*200,(r+1)*200);
    printf("      </a>\n");
  }
printf("<!--- Action Map zoom -->\n");
  ZOOM_DIR d = (flags&ZDIRBIT)?ZOOM_IN:ZOOM_OUT;
  for(int r=0;r<=1;r++) for(int c=0;c<=1;c++) {
    printf("      <a xlink:href=\"/%s+%d+%d+%d\">\n", THIS_PAGE.c_str(), 
     situation->zoomCenterEast(c,d), situation->zoomCenterNort(r,d), flags);
    printf("       <rect x=\"%03d\" y=\"%03d\" width=\"100\" height=\"100\""
           " style=\"opacity:0.0001\" />\n", (c+2)*100,(r+2)*100);
    printf("      </a>\n");
  }
}
  
void svgLabelEdges(ActionVector* situation, int flags) {
int boxSize = situation->extent();
int boxWest = situation->getWest();
int boxSouth = situation->getSouth();

  int east = boxWest + 1000 - boxWest%1000;
  for(;east<boxWest+boxSize;east+=1000) {
    int x = (east-boxWest)*600/boxSize;
    printf("      <text x=%d y=%d>%d</text>\n",x-20,616,east);
  }

  int north = boxSouth + 1000 - boxSouth%1000;
  for(;north<boxSouth+boxSize;north+=1000) {
    int y = (north-boxSouth)*600/boxSize;
    printf("      <text x=%d y=%d>%d</text>\n",602,606-y,north);
  }
}
 
void makeBigMap(int type=0) {
int West =   514000;
int North = 4257000;
int East =   522000;
int South = 4246000;
int ringE =  518250;
int ringN = 4250450;
int radius = 1200;
int airportE =  525000;
int airportN = 4252000;
int fiveMiles = (int)(5.0 * 1609.34) + 250;// 5 miles plus a small buffer
int PixelPerChip = 50;
int MeterPerChip = 500;
 

// size in number of map chips
int wide = (East - West)/MeterPerChip;
int high = (North - South)/MeterPerChip;
int e,n;

  printf(
    "Content-type: text/html\n\n"
    "<html>\n"
    " <body>\n"
    "  <svg x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" >\n", 
    wide*PixelPerChip, high*PixelPerChip);

  for(int row=0;row<high;row++) {
    n = North - MeterPerChip/2 - MeterPerChip * row;
    for(int col=0;col<wide;col++) {
      e = West + MeterPerChip/2 + MeterPerChip * col;
      printf(
    "   <image x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" ",
        PixelPerChip*col, PixelPerChip*row, PixelPerChip, PixelPerChip);
      spew_chip_link(type, e,n);
      printf(" />\n");
  } }
  
  South = 4247000;
  FILE* gml = fopen(ROAD_GML_PATH, "r");
  if(gml<=0){fprintf(stderr,"NO ROAD DATA in %s\n", ROAD_GML_PATH);}

  int wasIn, nowIn = 0;
  while(2==fscanf(gml,"%d,%d",&e,&n)) {
    wasIn = nowIn;
    nowIn = ((West<e)&&(e<East)&&(South<n)&&(n<North))?1:0;
    if(nowIn && !wasIn) {
      cout <<
        "      <polyline style=\"fill:none;stroke:blue;stroke-width:5\"\n"
        "        points=\"";
    }
    if(nowIn)
      cout << " " << ((e-West)*PixelPerChip)/MeterPerChip << "," << (((North-n)*PixelPerChip)/MeterPerChip);
    if(wasIn && !nowIn) cout << "\" />\n";
  }
  fclose(gml);

  e = ringE;
  n = ringN;
  shadedRing( ((e-West)*PixelPerChip)/MeterPerChip, ((North-n)*PixelPerChip)/MeterPerChip,
      (radius*PixelPerChip)/MeterPerChip, (char*)"red", 0.1);

  e = airportE;
  n = airportN;
  shadedRing( ((e-West)*PixelPerChip)/MeterPerChip, ((North-n)*PixelPerChip)/MeterPerChip,
      (fiveMiles*PixelPerChip)/MeterPerChip, (char*)"green", 0.0);

  printf(
    "  </svg>\n"
    " </body>\n"
    "</html>\n");
}

int main(int argc, char **argv) {
int east, north, size;
int baseType = 0;

  Session* session = new Session(argc, argv);

  THIS_PAGE = "cgi-bin"+string(strrchr(argv[0],'/'))
     + "?" + session->getSessionArg();

  sscanf(argv[4],"%d",&baseType);

  if(0==strcasecmp(argv[2],"big")){makeBigMap(baseType/TYP0BIT);return 1;}

  sscanf(argv[2],"%d",&east);
  sscanf(argv[3],"%d",&north);


  sendPageTop(session->homeInSession().c_str(), (char*)"MDRS Map", session->getSessionArg().c_str());

  ActionVector state(east,north);

  printf("     <a href=/mapWrapper.html>metaData and background</a><br>\n");
  printf("     Center UTM: %d %d<br>\n",
    state.getWest() + state.extent()/2,
    state.getSouth()+ state.extent()/2);
  printf("     Extent: %d by %d meters<br>\n", state.extent(), state.extent());

  simpleLeftPanel(&state, baseType);

  printf("    </td><td id=\"map_cell\"><center>\n");
  printf("<div id=\"map_canvas\" onmousedown=\"relMouseCoords(event,%d,%d,%d,%d)\">\n",
                 state.getWest(),state.getSouth()+state.extent(), state.extent(),600);
  printf("     <svg x=\"0\" y=\"0\" width=\"660\" height=\"625\" >\n");

  makeSvgTiles(&state, baseType);
  svgNoMotorZone(&state);
  if(baseType & 2)svgWaypoints(&state, 600);
  if(baseType & 1)svgOpenRoads(&state, 600);

  //svgShowGoMap(&state, baseType);
  if(baseType & CLIKBIT)svgActionMap(&state, baseType);
  svgLabelEdges(&state, baseType);

  printf("     </svg>\n");
  printf("     </div>\n");
  printf("    </center></td>\n");
  printf("   </tr></table>\n");
  printf("  </form>\n");
  printf(" </body>\n");
  printf("</html>\n");
}
