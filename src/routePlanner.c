#include <iostream>
#include <math.h>
#include <string.h>
#include "session.h"
#include "dataBrane.h"
#include <iostream>
#include <algorithm> // for next_permutation

char TRANSMIT_QUEUE[] = "/usr/lib/cgi-bin/queue";
using namespace std;

static const char* dir[16] = {
  (char*)"N", (char*)"NNE", (char*)"NE", (char*)"ENE",
  (char*)"E", (char*)"ESE", (char*)"SE", (char*)"SSE",
  (char*)"S", (char*)"SSW", (char*)"SW", (char*)"WSW",
  (char*)"W", (char*)"WNW", (char*)"NW", (char*)"NNW"
};

#define WEST_EDGE 511000
#define EAST_EDGE 522000
#define NORTH_EDGE 4261250
#define SOUTH_EDGE 4247000
#define CENTER_EAST 518250
#define CENTER_NORTH 4250450
#define CENTER_RADIUS 1200

class Point {
private:
 int east, north;
public:
  Point() { east= -1; north = -1;}
  Point(int e, int n) { set(e,n);}
  void set(Point p) { set(p.easting(), p.northing()); }
  void set(int e, int n) { east=e; north=n;}
  int easting(){return east;}
  int northing(){return north;}
  bool isValid() { return ((WEST_EDGE <= east)&&(east <= EAST_EDGE)
      && (SOUTH_EDGE <= north)&&(north <= NORTH_EDGE));
  }

  double farFrom(Point that) { return farFrom(that.easting(), that.northing()); } 
  double farFrom(int e, int n) { return hypot((double)(east - e), (double)(north - n)); }

  bool tooCloseToHome() { return (CENTER_RADIUS >= farFrom(CENTER_EAST, CENTER_NORTH)); }

  const char* dirName(Point p) { return dir[(int)(((11.25+heading(p))*16.0)/360.0)%16]; }
  double heading(Point p) { return heading(p.east, p.north); }
  double heading(int x, int y) { return
      ((M_PI - atan2((double)(x-east), (double)(north-y)))*180.0/M_PI); }

  char* intoString(char* buffer) { sprintf(buffer,"%d,%d", east, north); return buffer; }
  char* toString() { char buffer[64]; intoString(buffer); return strdup(buffer); } 
  bool fromString(char* buffer) { return(2==sscanf(buffer,"%d,%d", &east, &north)); }
};

/* A CrossRoad is a kind of Point.
  CrossRoads have additional properties that map the distance and route to 'home'
  An array of CrossRoads can be traversed using the way home index as a tree link
*/
class CrossRoad: public Point {
public:
  double fromHome;
  int wayHome; // index of the next crossroad on the way home.
  void setWayHome(int trackId, double distance) {wayHome = trackId; fromHome = distance;}

  char* intoString(char* buffer) {
    sprintf(buffer,"%d,%d, (%lf from %d)", easting(), northing(), fromHome, wayHome);
    return buffer;
  }

  Point dupPoint() { return Point(easting(),northing()); }
}; // end of class CrossRoad


class Track {
public:
char id[32];
int fn;         // first node (index into CrossRoads list)
int fp;         // first point (index into the coordinates buffer)
int ln,lp;// likewise for the last locations and indexes
double length; // length of track in meters
int np; // length of track in number of points

  /* this utility function finds or adds a node in the table for the start of the track */
  bool setEndNode(bool firstNotLast, CrossRoad* node, int &numNode, Point point) {
    int nodeIndex;
    bool isNew = false;
    for(nodeIndex=0;nodeIndex<numNode;nodeIndex++) {
      if(1.0 > node[nodeIndex].farFrom(point))break;
    }
    if(nodeIndex==numNode) { 
      node[nodeIndex].set(point);
      numNode++;
      isNew = true;
    }
    if(firstNotLast)
         fn = nodeIndex;
    else ln = nodeIndex;
  }// end of setEndNode

  void clone(Track* into) {
    into->fn = fn;
    into->ln = ln;
    into->fp = fp;
    into->lp = lp;
    into->np = np;
    into->length = length;
  }
};

class Destination {
private:
public:
  Point poi;
  int accessNode;
  char* label;
  void setLabel(const char* charStr) {label = strdup(charStr);}
//TODO (LOTS)
};

#define WANDERING 2048
struct Segment {
public:
  int headNode, tailNode, nodeList[WANDERING], trackList[WANDERING];
  double length;
  void navigate(int oneNode, int twoNode, Track *track, int numTrack, Point *point, CrossRoad *node) {
    int nodeTable[WANDERING], edgeTable[WANDERING], breadCrum[WANDERING];
    double distance[WANDERING];
    headNode = oneNode;
    tailNode = twoNode;
    breadCrum[0]=-1;
    distance[0]=0;
    nodeTable[0] = headNode;
    edgeTable[0] = -1;
    int i, j = 1;
    double shortestLength = 20000000.0;
    int shortend = -1;

    if(1.0 > node[headNode].farFrom(node[tailNode].easting(),node[tailNode].northing()))return;
/*
    printf("find path from node %d to node %d<br>\n", headNode, tailNode);
    printf("It is %lf meters as the crow flies<br>\n", 
     node[headNode].farFrom(node[tailNode].easting(),node[tailNode].northing()));
*/

    for(i=0;i<j;i++) {
      int thisNode = nodeTable[i];
      for(int t=0;t<numTrack;t++)if(t!=edgeTable[i]) {
        bool looping = false;
        for(int n=i;n>0;n=breadCrum[n]) {
          if(edgeTable[n] == t)looping = true;
          if(looping)break;
        }
        if(looping)continue;
        if(shortestLength > distance[i] + track[t].length) {
          bool follow = false;
          if(track[t].fn == thisNode) {
            nodeTable[j] = track[t].ln;
            follow = true;
          } else if(track[t].ln == thisNode) {
            nodeTable[j] = track[t].fn;
            follow = true;
          }
          if(follow) {
            distance[j] = distance[i] + track[t].length;
            edgeTable[j] = t;
            breadCrum[j] = i;
            if(nodeTable[j] == tailNode) {
              shortestLength = distance[j];
              shortend = j;
            }
            if(j>=WANDERING) {
              printf("<B>WANDERING (this is not a good thing. Tell Engineering.)</B><br/>\n");
            } else {
              j++;
            }
    } } } }
    if(shortend>0) {
      length = distance[shortend];
      int i,l = shortend;
      for(i=0;;i++) {
        nodeList[i] = nodeTable[l];
        trackList[i] = edgeTable[l];
        if(l<=0)break;
        l = breadCrum[l];
      }
      //nodeList[i] = trackList[i] = -1;
      nodeList[i+1] = trackList[i+1] = -1;
    }
  }// end navigate()
};

#define ROAD_FILE_PATH (char*)"/usr/lib/cgi-bin/data/roadCoords.txt"
#define MAXTRACK 512

class RouteMap {
private:
  CrossRoad node[512]; int numNode;
  Point point[10000]; int numPoint;
  Track track[MAXTRACK]; int numTrack;
  Segment segment[128]; int numSegment;
  int leastLength, *bestSegmentVisitOrder;
  int panoNodeCount;


/***
  void searchForUnmarkedCrossroad(
      CrossRoad* node, int &numNode, Point* point, int &numPoint, Track* track, int &numTrack) {
    for(int t=0;t<numTrack;t++) {
      for(int p=track[t].fp+1;p<track[t].lp;p++) {
        for(int n=0;n<numNode;n++) {
          if(1.0>point[p].farFrom(node[n].easting(), node[n].northing())) {
            printf("UNMARKED NEAR INTERSECTION AT %d,%d is near %d,%d (%d)<br/>\n",
              node[n].easting(), node[n].northing(), 
              point[p].easting(), point[p].northing(), p);
          }
        }
      }
    }
  }
***/

  int loadTracksAndStepsAndNodes(
      CrossRoad* node, int &numNode, Point* point, int &numPoint, Track* track, int &numTrack) {
    int ENUFBUF = 64;
    char space[32];
    char buffer[ENUFBUF];
    FILE* dataFile = fopen(ROAD_FILE_PATH, "r");
    int e,n;
    numNode = numPoint = 0;
    for(numTrack=0;numTrack<MAXTRACK;numTrack++) {
      if(2 != fscanf(dataFile,"%d,%d",&e,&n))break;
      if((e==-1)||(n==-1)) {
        numTrack--;
        continue;
      }
      point[numPoint].set(e,n); numPoint++;

      track[numTrack].fp = numPoint - 1;
      track[numTrack].setEndNode(true, node,numNode, point[numPoint-1]);
      
      track[numTrack].length = 0;
      for(track[numTrack].np=1;;track[numTrack].np++) {
        if(2 != fscanf(dataFile,"%d,%d",&e,&n))break;
        if((e==-1)||(n==-1))break;
        point[numPoint].set(e,n); numPoint++;
        track[numTrack].length += point[numPoint-2].farFrom(point[numPoint-1]);
      }

      track[numTrack].lp = numPoint - 1;
      track[numTrack].setEndNode(false, node,numNode, point[numPoint-1]);
    }
    fclose(dataFile);
    return numPoint;
  }

  int closestRoadNode(int nearTrack, int stepOnTrack, double intoTrack,
      CrossRoad* node, int &numNode, Point* point, int &numPoint, Track* track, int &numTrack) {
    int newTrack = nearTrack;
    if(track[nearTrack].fp != stepOnTrack) {
      //clone track 
      newTrack = numTrack; numTrack++;
      track[nearTrack].clone( &track[newTrack]);
      track[nearTrack].length = intoTrack;
      track[newTrack].length -= intoTrack;
      track[nearTrack].lp = track[newTrack].fp = stepOnTrack;
      track[nearTrack].setEndNode(false, node,numNode, point[stepOnTrack]);
      track[newTrack].setEndNode(true, node,numNode, point[stepOnTrack]);
/*
    } else {
      printf("set node number %d as an access point<br/>\n", track[nearTrack].fn);
*/
    }
    return track[newTrack].fn;
  }//end of closestRoadNode()

  int segmentIndexLookup(int row, int col, int count) {
    if(col==row)return -1;
    int tmp, retval;
    if(col<row) { tmp=col; col=row; row=tmp; }

    for(retval = tmp = 0; tmp < row; tmp++)retval += count - tmp - 1;
    retval += col - row - 1;
    return retval;
  }

public:
  RouteMap() {
    loadTracksAndStepsAndNodes(node, numNode, point, numPoint, track, numTrack);
    panoNodeCount = numNode;
  }

  int makeRouteMatrix(Destination wayPoint[], int wayPointCount, bool verbose) {
    int i,j,k;
    int numTrails = (wayPointCount * ( wayPointCount - 1 )) / 2;
    for(i = 0, j = 0;j<wayPointCount-1;j++) for(k= j+1;k<wayPointCount;i++,k++) {
      segment[i].navigate(wayPoint[j].accessNode, wayPoint[k].accessNode,
        track,numTrack, point, node);
    }
    if(verbose) {
      printf(
        " <table border=2>\n"
        "  <tr><th colspan='%d'>access point distances matrix</th></tr>\n"
        "  <tr><th/>\n", 1+wayPointCount);
      for(i=0;i<wayPointCount;i++) printf("<th>%dE %dN</th>",
          node[wayPoint[i].accessNode].easting(), node[wayPoint[i].accessNode].northing());
      printf("</tr>\n  ");
      for(i=0,j=0;j<wayPointCount;j++) {
        printf("<tr><th>%dE %dN</th>",
          node[wayPoint[j].accessNode].easting(), node[wayPoint[j].accessNode].northing());
        for(k=0;k<wayPointCount;k++) printf("<td>%5.0lf</td>",
          (k==j)?0.0:segment[segmentIndexLookup(j, k, wayPointCount)].length);
        printf("</tr>\n  ");
      }
      printf("</table>\n  ");
    }
  
    return numTrails;
  }

  int shortOrder(int wayPointCount) {
    bestSegmentVisitOrder = new int[wayPointCount+1];
    leastLength = 20000000;
    int i, permutation[wayPointCount];
    for(i=0;i<wayPointCount;i++)permutation[i]=i;
    printf("<!---\n<table>\n");
    for(;;) {
      double length = 00;
      if(permutation[0] != 0) break;
      for(i=1;i<wayPointCount;i++) {
        length += segment[segmentIndexLookup(permutation[i-1],permutation[i],wayPointCount)].length;
      }
      length += segment[segmentIndexLookup(permutation[i-1],permutation[0],wayPointCount)].length;
      if(length < leastLength) {
        printf(" <tr>");
        leastLength = length;
        printf("<td>%d</td>",leastLength);
        for(i=0;i<wayPointCount;i++)bestSegmentVisitOrder[i] = permutation[i];
        for(i=0;i<wayPointCount;i++)printf("<td>%d</td>",permutation[i]);
        printf("</tr>\n");
      }
      if(!next_permutation(permutation, permutation+wayPointCount)) break;
    }
    printf("</table>\n--->\n");
    return leastLength;
  }

  void findClosestPoints( Destination wayPoint[], int wayPointCount ) {
    for(int wp=0;wp<wayPointCount;wp++) {
      double shortest = 20000000.0;
      int nearTrack = 0;
      int stepOnTrack = 0;
      double intoTrack = 0.0;
      for(int tr=0;tr<numTrack;tr++) {
        double along = 0.0;
        for(int pt=track[tr].fp;pt<track[tr].lp;pt++) {
          double from = point[pt].farFrom(wayPoint[wp].poi);
          if(shortest > from) {
            shortest = from; 
    	    nearTrack = tr;
    	    stepOnTrack = pt;
            intoTrack = along;
          }
          along += point[pt].farFrom(point[pt+1]);
      } }

      wayPoint[wp].accessNode = closestRoadNode(
          nearTrack,stepOnTrack,intoTrack, node, numNode, point, numPoint, track, numTrack);     

/*
      printf("%d,%d is %lf meters from road at %d,%d<br/>\n",
        wayPoint[wp].poi.easting(), wayPoint[wp].poi.northing(),
        wayPoint[wp].poi.farFrom(
          node[wayPoint[wp].accessNode].easting(),
          node[wayPoint[wp].accessNode].northing()),
        node[wayPoint[wp].accessNode].easting(),
        node[wayPoint[wp].accessNode].northing());
*/
    }
  }//end  findClosestPoints(wayPoint, wayPointCount)

  void mapTripLeg(string panoViewLink, int legNum, Segment seg, int w,int n, int scale) {
    if(1.0 > seg.length) return;
      
    for(int i=0;seg.nodeList[i] >=0; i++) {
/**
      if(i>0) {
        printf("  <polyline style=\"fill:none;stroke:blue;stroke-width:2\"\n    points=\"");
        printf(" %d,%d %d,%d",
          (node[seg.nodeList[i-1]].easting() - w) / scale,
          (n - node[seg.nodeList[i-1]].northing()) / scale,
          (node[seg.nodeList[i]].easting() - w) / scale,
          (n - node[seg.nodeList[i]].northing()) / scale);
        printf(" \"/>\n");
      }
/*/
      printf("  <polyline style=\"fill:none;stroke:blue;stroke-width:2\"\n    points=\"");
      for(int p = track[seg.trackList[i]].fp;p <= track[seg.trackList[i]].lp; p++)printf(" %d,%d",
        (point[p].easting() - w) / scale, (n - point[p].northing()) / scale);
      printf(" \"/>\n");
/**/

      if(seg.nodeList[i]<panoNodeCount) {
        printf("  <a xlink:href=\"%s+%d+%d",
          panoViewLink.c_str(),
          node[seg.nodeList[i]].easting(), node[seg.nodeList[i]].northing());
        if(i>0)
          printf("+%d+%d",node[seg.nodeList[i-1]].easting(), node[seg.nodeList[i-1]].northing());
        if(seg.nodeList[i+1] >=0 )
          printf("+%d+%d",node[seg.nodeList[i+1]].easting(), node[seg.nodeList[i+1]].northing());
        printf("\">\n");
        printf("   <circle cx=\"%d\" cy=\"%d\" r=\"3\""
             " style=\"fill:blue;stroke:blue;stroke-width:2\"/>\n",
            (node[seg.nodeList[i]].easting() - w) / scale,
            (n - node[seg.nodeList[i]].northing()) / scale);
        printf("  </a>\n");
      } else {
        printf(" <circle cx=\"%d\" cy=\"%d\" r=\"3\""
             " style=\"fill:white;stroke:blue;stroke-width:1\"/>\n",
            (node[seg.nodeList[i]].easting() - w) / scale,
            (n - node[seg.nodeList[i]].northing()) / scale);
      }
    }
// trackList[i] = -1;
  }
    
  // draws the whole road net in thin red lines.
  void sketchWholeRoadNet(int n,int e,int w,int s,int scale) {
    for(int t=0;t<numTrack; ++t) {
      printf("  <polyline style=\"fill:none;stroke:red;stroke-width:1\"\n    points=\"");
      for(int p = track[t].fp;p <= track[t].lp; p++)
        if (
          (n>=point[p].northing())
        &&(s<=point[p].northing())
        &&(e>=point[p].easting())
        &&(w<=point[p].easting())
        ) printf(" %d,%d",
            (point[p].easting() - w) / scale, (n - point[p].northing()) / scale);
      printf(" \"/>\n");
    }
  }

  void simpleMap(string sessionArg, Destination *wayPoint, int wayPointCount ) {
    //<div id="map_canvas" onmousedown="relMouseCoords(event,517500,4251000,2)">
    int n,e,w,s, i,j, seg;
    w = e = wayPoint[bestSegmentVisitOrder[0]].poi.easting();
    n = s = wayPoint[bestSegmentVisitOrder[0]].poi.northing();

    for(i=0;i<wayPointCount;i++) {
      if(n < wayPoint[bestSegmentVisitOrder[i]].poi.northing())
        n = wayPoint[bestSegmentVisitOrder[i]].poi.northing();
      if(s > wayPoint[bestSegmentVisitOrder[i]].poi.northing())
        s = wayPoint[bestSegmentVisitOrder[i]].poi.northing();
      if(e < wayPoint[bestSegmentVisitOrder[i]].poi.easting())
        e = wayPoint[bestSegmentVisitOrder[i]].poi.easting();
      if(w > wayPoint[bestSegmentVisitOrder[i]].poi.easting())
        w = wayPoint[bestSegmentVisitOrder[i]].poi.easting();

      if(n < node[wayPoint[bestSegmentVisitOrder[i]].accessNode].northing())
        n = node[wayPoint[bestSegmentVisitOrder[i]].accessNode].northing();
      if(s > node[wayPoint[bestSegmentVisitOrder[i]].accessNode].northing())
        s = node[wayPoint[bestSegmentVisitOrder[i]].accessNode].northing();
      if(e < node[wayPoint[bestSegmentVisitOrder[i]].accessNode].easting())
        e = node[wayPoint[bestSegmentVisitOrder[i]].accessNode].easting();
      if(w > node[wayPoint[bestSegmentVisitOrder[i]].accessNode].easting())
        w = node[wayPoint[bestSegmentVisitOrder[i]].accessNode].easting();
      if(i==0)continue;
      seg = segmentIndexLookup(bestSegmentVisitOrder[i-1],bestSegmentVisitOrder[i],wayPointCount);
//
      for(int p = track[segment[seg].trackList[i]].fp;p <= track[segment[seg].trackList[i]].lp; p++) {
        if(n<point[p].northing())n = point[p].northing();
        if(s>point[p].northing())s = point[p].northing();
        if(e<point[p].easting())e = point[p].easting();
        if(w>point[p].easting())w = point[p].easting();
      }
/*
      for(j=0;segment[seg].nodeList[j] >=0; j++) {
        if(n < node[segment[seg].nodeList[j]].northing())
          n = node[segment[seg].nodeList[j]].northing();
        if(s > node[segment[seg].nodeList[j]].northing())
          s = node[segment[seg].nodeList[j]].northing();
        if(e < node[segment[seg].nodeList[j]].easting())
          e = node[segment[seg].nodeList[j]].easting();
        if(w > node[segment[seg].nodeList[j]].easting())
          w = node[segment[seg].nodeList[j]].easting();
      }
*/
    }

#define IMAGE_SIZE 1000
    int metersPerPixel = IMAGE_SIZE / 200;
    int halfPictMeters = IMAGE_SIZE / 2;

    n += (2*halfPictMeters) - (n%(2*halfPictMeters));
    e += (2*halfPictMeters) - (e%(2*halfPictMeters));
    w -= w % (2*halfPictMeters);
    s -= s % (2*halfPictMeters);
    int wide = (e - w) / metersPerPixel;
    int high = (n - s) / metersPerPixel;

    printf(" <center><svg x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" >\n", wide+60,high+10);
    for(j=0;j<high/200;j++)for(i=0;i<wide/200;i++) {
      printf("  <image x=\"%d\" y=\"%d\" width=\"200\" height=\"200\" "
          "xlink:href=\"/mapChip/topo_%d_%d.gif\"/>\n",
        //"xlink:href=\"/mapChip/orth_%d_%d.bmp\"/>\n",
          200*i,200*j,
        w + halfPictMeters + (2*halfPictMeters)*i,
        n - halfPictMeters - (2*halfPictMeters)*j
      );
    }
    for(j=1;j<high/200;j++){
      printf("  <text x=\"%d\" y=\"%d\" fill=\"black\">%d</text>\n",
        wide+2,200*j+6, n - (2*halfPictMeters)*j);
    }
    for(i=1;i<wide/200;i++) {
      printf("  <text x=\"%d\" y=\"%d\" fill=\"black\">%d</text>\n",
        200*i-24, high+12, w + (2*halfPictMeters)*i);
    }

    sketchWholeRoadNet(n,e,w,s,metersPerPixel);

    for(i=0;i<wayPointCount;i++) {
      printf("  <circle cx=\"%d\" cy=\"%d\" r=\"2\""
        " style=\"fill:green;stroke:green;stroke-width:2\"/>\n",
        (node[wayPoint[bestSegmentVisitOrder[i]].accessNode].easting() - w) / metersPerPixel,
        (n - node[wayPoint[bestSegmentVisitOrder[i]].accessNode].northing()) / metersPerPixel);

      printf("  <circle cx=\"%d\" cy=\"%d\" r=\"5\""
        " style=\"fill:yellow;stroke:green;stroke-width:3;opacity:0.5\"/>\n",
        (wayPoint[bestSegmentVisitOrder[i]].poi.easting() - w) / metersPerPixel,
        (n - wayPoint[bestSegmentVisitOrder[i]].poi.northing()) / metersPerPixel);
           
      printf("  <polyline points=\" %d,%d %d,%d\""
        " style=\"fill:none;stroke:green;stroke-width:3\"/>\n",
        (wayPoint[bestSegmentVisitOrder[i]].poi.easting() - w) / metersPerPixel,
        (n - wayPoint[bestSegmentVisitOrder[i]].poi.northing()) / metersPerPixel,
        (node[wayPoint[bestSegmentVisitOrder[i]].accessNode].easting() - w) / metersPerPixel,
        (n - node[wayPoint[bestSegmentVisitOrder[i]].accessNode].northing()) / metersPerPixel);
    }

    for(i=1;i<wayPointCount;i++) {
      seg = segmentIndexLookup(bestSegmentVisitOrder[i-1],bestSegmentVisitOrder[i],wayPointCount);
      mapTripLeg(sessionArg, i, segment[seg], w,n, metersPerPixel);
    }
    seg = segmentIndexLookup(bestSegmentVisitOrder[i-1],bestSegmentVisitOrder[0],wayPointCount);
    mapTripLeg(sessionArg, i, segment[seg], w,n, metersPerPixel);
    printf(" </svg>\n");
  }

  char* formatHref(char* space, string panoViewLink, int thisE, int thisN, int lastE=0, int lastN=0, int nextE=0, int nextN=0) {
    sprintf(space, "href=\"%s+%d+%d", panoViewLink.c_str(), thisE, thisN);
    if(lastE>0)sprintf(space+strlen(space), "+%d+%d", lastE, lastN);
    if(nextE>0)sprintf(space+strlen(space), "+%d+%d", nextE, nextN);
    sprintf(space+strlen(space), "\"");
    return space;
  }

  void oneTripLeg(string linkString, int legNum, Segment seg, int firstNode) {
    char space[1024];
    printf("    <tr><th>%d</th>", legNum);
    if(1.0 > seg.length) printf("<td colspan=2>Stay there and visit another point of interest</td></tr>\n");
    else {
      printf("<td>Drive </td><td>%5.0lf meters ", seg.length);
      if(firstNode == seg.tailNode ) {
        printf("from 12S %dE %dN NAD27 to 12S %dE %dN NAD27</td></tr>\n",
          node[seg.tailNode].easting(), node[seg.tailNode].northing(),
          node[seg.headNode].easting(), node[seg.headNode].northing());
        for(int i=1;seg.nodeList[i] >=0; i++) {
          printf("<tr><td/><td/><td>");
          if(seg.nodeList[i-1]<panoNodeCount)printf("<a %s>",
            formatHref(space, linkString,
              node[seg.nodeList[i-1]].easting(), node[seg.nodeList[i-1]].northing(),
              node[seg.nodeList[i-2]].easting(), node[seg.nodeList[i-2]].northing(),
              node[seg.nodeList[i-0]].easting(), node[seg.nodeList[i-0]].northing()));
          printf("12S %dE %dN NAD27",
            node[seg.nodeList[i-1]].easting(), node[seg.nodeList[i-1]].northing());
          if(seg.nodeList[i-1]<panoNodeCount)printf("</a>");
          printf(" to ");
          if(seg.nodeList[i]<panoNodeCount)printf("<a %s>",
            formatHref(space, linkString,
              node[seg.nodeList[i-0]].easting(), node[seg.nodeList[i-0]].northing(),
              node[seg.nodeList[i-1]].easting(), node[seg.nodeList[i-1]].northing(),
              node[seg.nodeList[i+1]].easting(), node[seg.nodeList[i+1]].northing()));
          printf("12S %dE %dN NAD27",
            node[seg.nodeList[i]].easting(), node[seg.nodeList[i]].northing());
          if(seg.nodeList[i]<panoNodeCount)printf("</a>");
          printf("</td></tr>\n");
        }
// trackList[i] = -1;
      } else {
        printf("from 12S %dE %dN NAD27 to 12S %dE %dN NAD27</td></tr>\n",
          node[seg.headNode].easting(), node[seg.headNode].northing(),
          node[seg.tailNode].easting(), node[seg.tailNode].northing());
        int i;
        for(i=1;seg.nodeList[i] >=0; i++);
        for(i-=2;i>=0;i--) {
          printf("<tr><td/><td/><td>");
          if(seg.nodeList[i+1]<panoNodeCount)printf("<a %s>",
            formatHref(space, linkString,
              node[seg.nodeList[i+1]].easting(), node[seg.nodeList[i+1]].northing(),
              node[seg.nodeList[i-0]].easting(), node[seg.nodeList[i-0]].northing(),
              node[seg.nodeList[i+2]].easting(), node[seg.nodeList[i+2]].northing()));
          printf("12S %dE %dN NAD27",
            node[seg.nodeList[i+1]].easting(), node[seg.nodeList[i+1]].northing());
          if(seg.nodeList[i+1]<panoNodeCount)printf("</a>");
          printf(" to ");
          if(seg.nodeList[i]<panoNodeCount)printf("<a %s>",
            formatHref(space, linkString,
              node[seg.nodeList[i+0]].easting(), node[seg.nodeList[i+0]].northing(),
              node[seg.nodeList[i-1]].easting(), node[seg.nodeList[i-1]].northing(),
              node[seg.nodeList[i+1]].easting(), node[seg.nodeList[i+1]].northing()));
          printf("12S %dE %dN NAD27",
            node[seg.nodeList[i]].easting(), node[seg.nodeList[i]].northing());
          if(seg.nodeList[i]<panoNodeCount)printf("</a>");
          printf("</td></tr>\n");
        }
      }
    }
  }

  void shortestLoopDirections(string linkString, Destination *wayPoint, int wayPointCount ) {
    printf("  <center><table border=2>\n"
           "    <tr><td colspan=7 align=center>(browse BACK to edit waypoint selection)</td></tr>"
           "    <tr><th rowspan=2>#</th>"
      "<th colspan=3>Interesting Point</th>"
      "<th rowspan=2>Vector</th>"
      "<th colspan=2>Access Point</th></tr>\n"
      "    <tr><th>Reason</th>"
      "<th>Easting</th><th>Northing</th><th>Easting</th><th>Northing</th></tr>\n");

    int i;for(i=0;i<wayPointCount;i++) {
      printf("   <tr><th>%d</th><td>%s</td><td>%d</td><td>%d</td>\n", i,
          wayPoint[bestSegmentVisitOrder[i]].label,
          wayPoint[bestSegmentVisitOrder[i]].poi.easting(),
          wayPoint[bestSegmentVisitOrder[i]].poi.northing());
      printf("<td>is %4.0f meters %s of </td>",
          wayPoint[bestSegmentVisitOrder[i]].poi.farFrom(node[wayPoint[bestSegmentVisitOrder[i]].accessNode]),
          node[wayPoint[bestSegmentVisitOrder[i]].accessNode].dirName(wayPoint[bestSegmentVisitOrder[i]].poi));
      printf("<td>%d</td><td>%d</td>",
      node[wayPoint[bestSegmentVisitOrder[i]].accessNode].easting(),
      node[wayPoint[bestSegmentVisitOrder[i]].accessNode].northing());
      printf("</tr>\n");
    }
    printf("   <tr><th>%d</th> <td>%s</td> <td>%d</td><td>%d</td>"
           " <td>is %4.0f meters %s of </td> <td>%d</td><td>%d</td></tr>\n",
        0, wayPoint[bestSegmentVisitOrder[0]].label,
           wayPoint[bestSegmentVisitOrder[0]].poi.easting(),
           wayPoint[bestSegmentVisitOrder[0]].poi.northing(),
           wayPoint[bestSegmentVisitOrder[0]].poi.farFrom(
      node[wayPoint[bestSegmentVisitOrder[0]].accessNode]),
      node[wayPoint[bestSegmentVisitOrder[0]].accessNode].dirName(
           wayPoint[bestSegmentVisitOrder[0]].poi),
      node[wayPoint[bestSegmentVisitOrder[0]].accessNode].easting(),
      node[wayPoint[bestSegmentVisitOrder[0]].accessNode].northing());
    printf("  </table></center>\n");

    printf("  <center><table>\n    <tr><th colspan=3>"
           "The shortest round trip is a %6.3f kilometer ATV drive.:</th></tr>\n",
      (float)leastLength*0.001);

    int s;
    for(i=1;i<wayPointCount;i++) {
      s = segmentIndexLookup(bestSegmentVisitOrder[i-1],bestSegmentVisitOrder[i],wayPointCount);
      oneTripLeg(linkString, i, segment[s], wayPoint[bestSegmentVisitOrder[i-1]].accessNode);
      printf("    <tr><td/><td/><td>Walk %4.0f meters %s to POI %d at %d %d</td></tr>\n",
          wayPoint[bestSegmentVisitOrder[i]].poi.farFrom(
            node[wayPoint[bestSegmentVisitOrder[i]].accessNode]),
          node[wayPoint[bestSegmentVisitOrder[i]].accessNode].dirName(
            wayPoint[bestSegmentVisitOrder[i]].poi),
          bestSegmentVisitOrder[i],
          wayPoint[bestSegmentVisitOrder[i]].poi.easting(),
          wayPoint[bestSegmentVisitOrder[i]].poi.northing()
      );
    }
    s = segmentIndexLookup(bestSegmentVisitOrder[i-1],bestSegmentVisitOrder[0],wayPointCount);
    oneTripLeg(linkString, i, segment[s], wayPoint[bestSegmentVisitOrder[i-1]].accessNode);
    printf("  </table></center>\n");
  }
}; //END OF CLASS RouteMap

void selectWayPoints(string sessionArg, string program) {
  DataBrane *db = new DataBrane();
  DataBrane::LOCATION *place = db->getPointSet();
  int size = db->pointCount();

  cout <<
    "  <center>\n"
    "   <table>\n"
    "    <tr><th>Select a few of these "<<size<<" waypoints</th></tr>\n"
    "    <tr><td><a href=/cgi-bin/pointEnter?" << sessionArg << ">\n"
    "     Or jump to here to add new ones.\n"
    "    </a></td></tr>\n"
    "   </table>\n"
    "   <form name=\"snagPois\" action=\"" << program << "\" method=\"post\">\n"
    "   <div class=\"scrollable\">\n"
    "   <table>\n";

  cout << "    <INPUT TYPE=\"hidden\" NAME=\"waypoint\" VALUE=\"" <<
    place[0].easting<<"_"<<place[0].northing<<"_"<<place[0].label <<
    "\" />\n";

  for(int i=1;i<size;i++)  cout <<
    "    <tr><td><INPUT TYPE=\"checkbox\" NAME=\"waypoint\" VALUE=\"" <<
    place[i].easting<<"_"<<place[i].northing<<"_"<<place[i].label <<
    "\">" << place[i].uid<<" 12s " <<
    place[i].easting<<","<<place[i].northing<<","<<place[i].label <<
    "   </td></tr>\n";
  cout <<
    "   </table>\n"
    "   </div>\n";
  cout <<
    "   <table><tr><td><input type=\"submit\" value=\"Plan Route\" /></td></tr></table>\n"
    "   </form>\n"
    "  </center>\n";

}

void backToFront(Point point, double along, int cross, CrossRoad* crossRoad) {
char buf[64];

  if (crossRoad[cross].fromHome>0.0) backToFront( crossRoad[cross].dupPoint(),
    crossRoad[cross].fromHome, crossRoad[cross].wayHome, crossRoad);
  else printf("<tr><th>Start at %s</th></tr>\n", crossRoad[cross].intoString(buf) );

  printf("<tr><td>Go %5.3lf km to %s</td></tr>\n",
      (along - crossRoad[cross].fromHome)/1000.0, point.intoString(buf));
}

void findRoute(Point destination,
    CrossRoad* crossRoad, int numNode, Point* point, int numPoint, Track* track, int numTrack) {
  char b1[64], b2[64];
  int t,p;
  int nearTrack, nearPoint;
  double nearFrom = 20000000.0;

  for (t = 0;t<numTrack; t++) {
    for(p = track[t].fp; p<=track[t].lp; p++) {
      double from = destination.farFrom(point[p]);
      if (from < nearFrom) {
        nearFrom = from;
        nearPoint = p;
        nearTrack = t;
  } } }

  double along = 0.0;
  for(p = track[nearTrack].fp; p!=nearPoint; p++) {
    along += point[p].farFrom(point[p+1]);
  }

  double head = destination.heading(point[nearPoint]);
  int wrong = track[nearTrack].ln;
  int cross = track[nearTrack].fn;
  if(crossRoad[cross].fromHome > crossRoad[wrong].fromHome) {
    cross = wrong;
    along = track[nearTrack].length - along;
  }
  along += crossRoad[cross].fromHome;

  backToFront(point[nearPoint], along, cross, crossRoad);

  printf("<tr><td>Then park along the road at %s and walk %d meters bearing %d to %s</td></tr>\n",
      point[nearPoint].intoString(b2), (int)nearFrom,
      (180 + (int)destination.heading(point[nearPoint]))%360,
      destination.intoString(b1) );

  printf("<tr><td>To go home, Walk %d meters bearing %d from %s back to the road at %s</td></tr>\n",
      (int)nearFrom, (int)destination.heading(point[nearPoint]),
      destination.intoString(b1), point[nearPoint].intoString(b2) );

  printf("<tr><th>Then drive %lf meters via:</th></tr>\n", along);
  Point way = point[nearPoint];
  while(crossRoad[cross].fromHome > 0.0) {
    printf("<tr><td>  Go %5.3lf km %s to %s</td></tr>\n",
        (along - crossRoad[cross].fromHome) / 1000.0,
        way.dirName(crossRoad[cross]), crossRoad[cross].intoString(b1));

    way = crossRoad[cross].dupPoint();
    along = crossRoad[cross].fromHome;
    cross = crossRoad[cross].wayHome;
  }
  printf("<tr><td>  and finally go %5.3lf km %s to the MDRS Habitat.</td></tr>\n",
      along/1000.0, way.dirName(crossRoad[cross]) );
}

    
char* getClean(char* buffer, int size, FILE* stream) {
  char* got = fgets(buffer, size, stream);
  if(got!=NULL)for(int i=0;i<size;i++){
    if(got[i] == '+')got[i] = ' ';
    if(got[i] == '\n')got[i] = 0;
    if(got[i] == 0)break;
  }
  return got;
}

int getEvaPlanForm(Destination wayPoint[],char **argv) {
const int ENUFBUF = 8192;
char *got, buffer[ENUFBUF];
long east, nort;
  int count = 0;

  sprintf(buffer,"%s/%s_%s",TRANSMIT_QUEUE,argv[3],argv[4]);
  FILE* repo = fopen(buffer,"r");
  if(NULL == repo) {
    printf("failed to get a plan<br/>\n");
  } else {
    got = fgets(buffer,ENUFBUF,repo);// read and ignore title
    printf("  <table>\n");
    printf("   <tr><td colspacing=2 valign=top align=center><table>\n");
    got = fgets(buffer,ENUFBUF,repo);// read and display the report date
    printf("    <tr><th>Date:</th><td>%s</td>\n",buffer);
    got = getClean(buffer,ENUFBUF,repo);// read and display the report Author
    printf("    <th>Author:</th><td>%s</td></tr>",buffer);
    printf("   </table></td></tr>\n");
    printf("   <tr><td><table border=1 cellspacing=1>\n");
/***********************
* 20140303 This all has to be replaced since I merged planning route and planning report.
* report now begins with a table of waypoints

    printf("   <tr><th>Role</th><th>Crew Member</th><th>Suit</th><th>ATV</th></tr>");
    for(int p=0;p<5;p++) {
      printf("   <tr>");
      got = fgets(buffer,ENUFBUF,repo);// read role
      printf("<th>%s</th>",strtok(buffer," +	"));//show first word as role
      got = getClean(buffer,ENUFBUF,repo);// read person name
      printf("<td align=center>%s</td>",buffer);
      got = getClean(buffer,ENUFBUF,repo);// read and ignore SUIT title
      got = getClean(buffer,ENUFBUF,repo);// read SUIT number
      printf("<td align=center>%s</td>",buffer);
      got = fgets(buffer,ENUFBUF,repo);// read and ignore ATV title
      got = fgets(buffer,ENUFBUF,repo);// read ATV number
      printf("<td>%s</td>",buffer);
      printf("   </tr>");
    }
    printf("  </table></td>\n");
    printf("  <td><table border=1 cellpadding=1>\n");
    printf("   <tr><th>Destination</th><th>Access Point</th><th>Purpose</th></tr>");
    DataBrane *db = new DataBrane();
    DataBrane::LOCATION *place = db->getPointSet();
    east = place[0].easting,place[0]; nort = place[0].northing;
    printf("   <tr><td>12s %ldE %ldN NAD27</td>", east,nort);
    printf("   <td/><td>%s</td></tr>\n", place[0].label.c_str());
    wayPoint[count].setLabel(place[0].label.c_str());
    wayPoint[count].poi.set(east,nort);
    count++;
    for(int p=0;p<5;p++) {
      int did = 0;
      printf("   <tr>");
      east = nort = 0;
      got = fgets(buffer,ENUFBUF,repo);// read and ignore easting tag
      got = fgets(buffer,ENUFBUF,repo);// read easting number
      sscanf(buffer,"%ld",&east);
      got = fgets(buffer,ENUFBUF,repo);// read and ignore northing tag
      got = fgets(buffer,ENUFBUF,repo);// read northing number
      sscanf(buffer,"%ld",&nort);
      if((east!=0)&&(nort!=0)) {
        printf("<td>12s %ldE %ldN NAD27</td>", east,nort);
        wayPoint[count].poi.set(east,nort);
        did=1;
      } else printf("<td/>");
      east = nort = 0;
      got = fgets(buffer,ENUFBUF,repo);// read and ignore easting tag
      got = fgets(buffer,ENUFBUF,repo);// read easting number
      sscanf(buffer,"%ld",&east);
      got = fgets(buffer,ENUFBUF,repo);// read and ignore northing tag
      got = fgets(buffer,ENUFBUF,repo);// read northing number
      sscanf(buffer,"%ld",&nort);
      if((east!=0)&&(nort!=0)) {
        printf("<td>12s %ldE %ldN NAD27</td>", east,nort);
        if(did==0)wayPoint[count].poi.set(east,nort);
        did=1;
      } else printf("<td/>");
      got = fgets(buffer,ENUFBUF,repo);// read and ignore purpose tag
      got = getClean(buffer,ENUFBUF,repo);// read purpose string
      printf("<td>%s</td>",buffer);
      printf("</tr>\n");
      for(char* c=buffer;*c;c++)if(*c=='+')*c=' ';
      wayPoint[count].setLabel(buffer);
      count+=did;
****************************************/
    printf("    <tr><th>#</th><th>UTM Coordinates</th><th>Purpose of visit</th></tr>\n");
    for(int p=0;p<15;p++) {
      int did = 0;
      got = fgets(buffer,ENUFBUF,repo);// read and ignore distance tag
      if(!strstr(buffer,"way"))break;
      got = fgets(buffer,ENUFBUF,repo);// read and ignore distance
      got = fgets(buffer,ENUFBUF,repo);// read and easting tag
      if(!strstr(buffer,"East:"))break;
      got = fgets(buffer,ENUFBUF,repo);// read easting number
      sscanf(buffer,"%ld",&east);
      got = fgets(buffer,ENUFBUF,repo);// read and ignore northing tag
      got = fgets(buffer,ENUFBUF,repo);// read northing number
      sscanf(buffer,"%ld",&nort);
      if((east!=0)&&(nort!=0)) {
        printf("<th>%d</th><td>12s %ldE %ldN NAD27</td>", count+1, east,nort);
        if(did==0)wayPoint[count].poi.set(east,nort);
        did=1;
      } else printf("<td/>");
      got = fgets(buffer,ENUFBUF,repo);// read and ignore purpose tag
      got = getClean(buffer,ENUFBUF,repo);// read purpose string
      printf("<td>%s</td>",buffer);
      printf("</tr>\n");
      for(char* c=buffer;*c;c++)if(*c=='+')*c=' ';
      wayPoint[count].setLabel(buffer);
      count+=did;
    }
    printf("   </table></td></tr>\n");

    printf("%s",buffer);	// spew ignored eva number tag
    got = fgets(buffer,ENUFBUF,repo);// read and ignore eva number
    printf("%s<br/>\n",buffer);

    got = fgets(buffer,ENUFBUF,repo);// read rogue flag tag
    printf("%s",buffer);	// spew tag
    got = fgets(buffer,ENUFBUF,repo);// read rogue flag
    printf("%s<br/>\n",buffer);

    got = fgets(buffer,ENUFBUF,repo);// read route change tag
    printf("%s",buffer);	// spew tag
    got = getClean(buffer,ENUFBUF,repo);// read route change narrative
    printf("%s<br/>\n",buffer);

    printf("   <tr>\n"
           "    <td><table border=1 cellpadding=1>\n");
    printf("     <tr><th>Role</th><th>Crew Member</th><th>Suit</th><th>ATV</th></tr>");
    for(int p=0;p<5;p++) {
      printf("   <tr>");
      got = fgets(buffer,ENUFBUF,repo);// read role
      printf("<th>%s</th>",strtok(buffer," +	"));//show first word as role
      got = getClean(buffer,ENUFBUF,repo);// read person name
      printf("<td align=center>%s</td>",buffer);
      got = getClean(buffer,ENUFBUF,repo);// read and ignore SUIT title
      got = getClean(buffer,ENUFBUF,repo);// read SUIT number
      printf("<td align=center>%s</td>",buffer);
      got = fgets(buffer,ENUFBUF,repo);// read and ignore ATV title
      got = fgets(buffer,ENUFBUF,repo);// read ATV number
      printf("<td>%s</td>",buffer);
      printf("   </tr>");
    }
    printf("    </table></td>\n"
           "    <td><table border=1 cellpadding=1>\n");
    printf("    </table></td>\n"
           "   </tr>\n"
           "  </table>\n");
/****************************************/
    while(fgets(buffer,ENUFBUF,repo)) {
      printf("%s",buffer);
      if(!fgets(buffer,ENUFBUF,repo)) break;
      printf("%s<br/>\n",buffer);
    }
    fclose(repo);
  }
  return count;
}

int parseLocations(Destination wayPoint[]) {
const int ENUFBUF = 8192;
char buffer[ENUFBUF], label[512];
long east, nort;

  if(NULL == fgets(buffer,ENUFBUF,stdin)) {
printf("   <font color=red>(You must enter a destination point)</font>\n");
    return 0;
  }
  int count = 0;;
  for(char* w = strtok(buffer,"&");NULL!=w; w = strtok(NULL,"&") ) {
    int e,n;
    sscanf(w,"waypoint=%d_%d_%s",&e,&n,label);
    for(char* c=label;*c;c++)if(*c=='+')*c=' ';
    wayPoint[count].setLabel(label);
    wayPoint[count].poi.set(e,n);
    count++;
  }
  return count;
}

void spewHelp() {
  cout << "  <center><table>\n";
  cout << "   <tr><th>Welcome to the EVA route planner</th></tr>\n";
  cout << "   <tr><td>EVA routes are suggested by computing the shortest legal path\n"
          "    to an entered destination point.<br>Additional waypoints can also be entered\n"
          "    but this version does not compute multiple-destinitaion routes.<br>Another\n"
          "    form is also presented to record what equipment is to be used.<br>Please\n"
          "    submit a completed EVA plan the night before each EVA."
          "   </th></td>\n";
  cout << "  </table></center>\n";
  return;
}

void promptPlanning() {
  cout << "  <table>\n";
  cout << "   <tr><td>Now edit the route as needed and</td></tr>\n";
  cout << "   <tr><td>add any other points where you plan</td></tr>\n";
  cout << "   <tr><td>to stop and collect samples or data.</td></tr>\n";
  cout << "  </table>\n";
  return;
}

bool checkPedestrianOnly(Destination wayPoint[], int wayPointCount) {
  for(int j=0;j<wayPointCount;j++)  {
    if( !wayPoint[j].poi.tooCloseToHome() ) {
      return false;
    }
  }
  return true;
}

/*TODO 
  Modify route planner to receive a list of waypoints and plan as follows:
(Note: this is a sub-optimal solution to the traveling salesman problem)
  Guess distance from hab for each point.
  Select the farthest point (A) and plan a route.
  Note any waypoints nearest to a track NOT along the route from the farthest.
  Pick the farthest of these (B). and plan route H->B B->A A->H.
  Note any waypoints which are nearer to a track NOT along these routes.
  Pick the farthest of these (C). 
  Plan routes H->C->B->A->H H->B->C->A->H H->B->A->C->H and pick the shortest.
  Note any waypoints which are nearer to a track NOT along these routes.
  Pick the farthest of these (D). and find the best insert place as above.
  Repeat until all points are near a traversed track.
  Generate itinery with waypoint visits in last traversal of nearest track.
*/
void planRoute(string panoViewLink, Destination wayPoint[], int wayPointCount) {
  DataBrane *db = new DataBrane();
  DataBrane::LOCATION *place = db->getPointSet();
  int size = db->pointCount();
  
  cout << "  <center><table>\n";

  if( checkPedestrianOnly(wayPoint, wayPointCount) ) {
    cout << "<tr><th>Only Pedestrian EVA's are permitted this close to the hab.</th></tr>\n";
    cout << "  </table></center>\n";
    return;
  }

  RouteMap routes;
  routes.findClosestPoints(wayPoint, wayPointCount);
  routes.makeRouteMatrix(wayPoint, wayPointCount, false);
  int roundTripDriveLenght = routes.shortOrder(wayPointCount);
  routes.shortestLoopDirections(panoViewLink, wayPoint, wayPointCount);
  routes.simpleMap(panoViewLink, wayPoint, wayPointCount );
}

void pointEntryPanel(Point destination, string program) {
  if(!destination.isValid()) {
    cout<<"  <table>\n";
    cout<<"   <tr><th colspan=2>Enter destination</th><tr>\n";
    cout<<"   <tr><td colspan=2>511000 < Easting < 522000</td></tr>\n";
    cout<<"   <tr><td colspan=2>4247000 < Northing < 4261000</td></tr>\n";
  }
  cout <<
    "   <form name=\"snag a fix\" action=\"" << program << "\" method=\"post\">\n"
    "   <tr><th>ZONE</th><td>12S</td></tr>\n"
    "   <tr><th>EASTING</th>"
    "    <td align=right><input type=text name=easting size=6";
  if((destination.easting()>511000)&&(destination.easting()<522000))
    cout << " value=" <<destination.easting();
  cout << "/></td></tr>\n"
    "   <tr><th>NORTHING</th>"
    "    <td align=right><input type=text name=northing size=7";
  if((destination.northing()>4247000)&&(destination.northing()<4261000))
    cout << " value=" <<destination.northing();
  cout << "/></td></tr>\n"
    "   <tr><th>DATUM</th><td>NAD27</td></tr>\n"
    "   <tr><td colspan=2><input type=\"submit\" value=\"Plan Route\" /></td></tr>\n"
    "  </form>\n"
    "  </table>\n";
}

char linkSpace[64];

int main(int argc, char **argv) {
Destination wayPoint[100];
int wayPointCount = 0;

Session* session = new Session(argc, argv);

  session->spewTop();
  if((argc>2) && (!strcmp(argv[2],"play"))) {
    wayPointCount = getEvaPlanForm(wayPoint, argv);
    planRoute(session->makeLinkString("panoViewer", linkSpace), wayPoint, wayPointCount);
  } else if((argc>2) && (!strcmp(argv[2],"plan"))) {
    char *root = strtok(strdup(session->getRefreshLink().c_str()),"?");
    *strrchr(root,'/') = 0;
    char buff[512];
    sprintf(buff,"%s/reportGenWrapper?%s+fill+EvaPlan",root,strtok(0,"+"));
    selectWayPoints(session->getSessionArg(), string(buff));
  } else {
    wayPointCount = parseLocations(wayPoint);
    if(wayPointCount <= 0) {
      selectWayPoints(session->getSessionArg(), session->getRefreshLink());
    } else {
      //cout << " <table><tr><td>\n"; // divide the screen into columns
      //promptPlanning();
      //cout << " </td><td>\n"; // end left column and start right
      planRoute(session->makeLinkString("panoViewer", linkSpace), wayPoint, wayPointCount);
      //cout << " </td></tr></table>\n";// end wrapping table made in pointEntryPanel()
    }
  }
  session->spewEnd();
}
