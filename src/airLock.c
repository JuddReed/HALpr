#include<stdio.h>
#include<string.h>

char *sensorName[12] = {
	(char*)"CPU",	(char*)"North Bridge",   (char*)"ExpressCardBay",
	(char*)"GPU",	(char*)"9-Cell Battery", (char*)"Ultrabay",
	(char*)"Primary Battery", (char*)"Ultrabay primary", (char*)"RAM",
	(char*)"Under Touch Pad",      (char*)"Mini-PCI Area",
	(char*)"twelve"
};
 
#define PROC_FORMAT "/sys/devices/platform/thinkpad_hwmon/temp%d_input"

void sendPageTop(const char *label, int refreshRate) {
  printf(
    "Content-type: text/html\r\n\r\n"
    "<html>\r\n"
    " <head>\r\n");
  if(refreshRate>0)printf(
    "  <META HTTP-EQUIV=\"refresh\" CONTENT=\"%d\">\r\n", refreshRate);
  printf(
    "  <title>%s</title>\r\n"
    " </head>\r\n"
    " <body>\r\n"
    "  <table><tr><td><a href=/index.html><img src=/haleye.gif></a></td>"
    "    <th><h1>%s</h1></th>"
    "  </tr></table>\t\n",
    label,label);
}

void readtemp(){
char buff[128];
FILE* p;
  int i,t;
    	
  printf(
    "   <table>\r\n"
    "    <tr><th>location</th><th>Fahrenheit</th><th>Celsus</th><tr>\r\n");
  for(i=0;i<12;i++) {
    sprintf(buff,PROC_FORMAT,i+1);
    p = fopen(buff,"r");
    if(p) {
      if( (1==fscanf(p,"%d",&t)) && !((t==50000)&&(i==4)) )
    	printf("    <tr><th>%s</th><td align=right>%d F</td><td align=right>%d C</td></tr>\r\n",
		sensorName[i],
		32+t*9/5000, t/1000);
      fclose(p);
    }
    printf("    <tr><th>XXs</th></tr>\r\n");
  }
  printf("   </table>\r\n");
}

char* CLOCK_PICT = (char*)"data/clockFace.svg";
void clockFace(){
FILE* c = fopen(CLOCK_PICT,"w");
  fprintf(c,
    "<?xml version=\"1.0\" standalone=\"no\"?>\n"
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
    "<svg width=\"%s\" height=\"%s\" viewBox=\"0 0 1000 1000\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n",
	"100%", "100%");
  fprintf(c,"<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
	"stroke=\"green\" stroke-width=\"8\" />\n",	1,1,999,999);
 
  fprintf(c,"<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
	"stroke=\"green\" stroke-width=\"8\" />\n",	1,999,999,1);

  fprintf(c,"</svg>\n");
  fclose(c);

  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", "/clockFace.svg" );
  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", "cgi-bin/clockFace.svg" );
  printf("<br>\n");
  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", "data/clockFace.svg" );
  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", CLOCK_PICT);
  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", "/cgi-bin/data/clockFace.svg" );
  printf("<br>\n");
  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", "clockFace.svg" );
  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", "/data/clockFace.svg" );
  printf("   <embed width=\"%s\" height=\"%s\" src=\"%s\" type=image/svg+xml>\n", "150", "150", "cgi-bin/data/clockFace.svg" );
}

struct AIR_LOCK_STATE {
  char* name;
  int nextIndex;
} stateTable[] = {
  {(char*)"DRAINING",	1},
  {(char*)"EXIT_OKAY",	2},
  {(char*)"EVA_AWAY",	3},
  {(char*)"EVA_BACK",	4},
  {(char*)"FILLING",	5},
  {(char*)"COME_ON_IN",	6},
  {(char*)"INNER_OPEN",	7},
  {(char*)"INNER_SHUT",	0}
};

float parseParam(AIR_LOCK_STATE *state, int argc, char **argv) {
}

int main(int argc, char **argv) {
AIR_LOCK_STATE state;
float pressure;

  sendPageTop("Marking Time",10);

  pressure =  parseParam(&state, argc, argv);
  
  printf("  <center><table><tr><td>\r\n");
  readtemp();
  printf("  </td><td>\r\n");
  clockFace();
  printf("  </td></tr></table></center>\r\n"
         " </body>\r\n"
         "</html>\r\n");
};
