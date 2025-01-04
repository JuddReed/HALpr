#ifndef HAB_ACTIVITY_LOGGER_DATASUMMARY_H_
#define HAB_ACTIVITY_LOGGER_DATASUMMARY_H_

/***********************************************************
*
* This file will be SHORT LIVED. AS SOON AS the data migrate into the database
* where they belong, this body of code will be obsolete.
*
************************************************************/

/*
#define BIG_GRAPH 1024
struct Sample {
  long tick;
  float reading;
  float loftFrom, meterFrom, tankFrom, trailerFrom;
  float loftInto, meterInto, tankInto, trailerInto;
  float tankFinal;
} reading[BIG_GRAPH];

char* expectedtags[] = {
  (char*)"Water+Logging+Date+And+Time:",
  (char*)"Static+Tank+Level+Before:",
  (char*)"Water+Meter+Reading+Before:",
  (char*)"Loft+Tank+Level+Before:",
  (char*)"Water+Used+During+Fill:",
  (char*)"Water+Meter+Reading+After:",
  (char*)"Loft+Tank+Level+After:",
  (char*)"Static+Tank+Level+After:",
  (char*)"Static+Tank+Refilled:",
  (char*)"Trailer+Tank+Level+Before:",
  (char*)"Trailer+Tank+Level+After:",
  (char*)"Final+Static+Tank+Level:" };
*/

/*
int parseDateTime(char* timestr) {
long parseRestOfFile(FILE* in, long dateTime, int numReading) {
void sketchCan(FILE* out, int centerX, int bottomY, int w, int h, char* styleText) {
void sketchTube(FILE* out, int centerX, int bottomY, int w, int h, float turn,
 float level, char* fluidStyle, char* tubeStyle) {
void sketchFractionTank(FILE* out, int centerX, int centerY, int size, float fillFract) {
void sketchTanks(FILE* out, Sample* reading, int BlockSize) {
*/
void spewSvg(FILE* out, int numReading, int period=14);
void spewSql(FILE* out, int numReading);
int loadWaterLogs();

void spewLpSvg(FILE* out, int numReading);
void spewDieSvg(FILE* out, int numReading);
void spewDownSvg(FILE* out, int numReading);
void spewSignalSvg(FILE* out, int numReading);

int loadPropaneLog();
int loadDieselLog();
int loadDownloadLog();
int loadSignalLog();

void spewHeatSvg(FILE* out, int numReading);
int loadHvacLogs();

#endif // HAB_ACTIVITY_LOGGER_DATASUMMARY_H_
