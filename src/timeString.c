#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* timeString(char* space, char* format) {
time_t tt = time(NULL);
  struct tm *now = localtime(&tt);
  sprintf(space,"%04d%02d%02d%02d%02d%02d",
    now->tm_year+1900, now->tm_mon+1, now->tm_mday,
    now->tm_hour, now->tm_min, now->tm_sec
  );
  return(space);
}

char* dbTimeString(long ticks, char* space) {
  time_t tt = (time_t)ticks;
  struct tm *when =  localtime(&tt);
  strftime(space,32,"%Y-%m-%d %H:%M:%S", when);
  return space;
}

char* denseTimeString(char* space) {
  return timeString(space, (char*)"%04d%02d%02d%02d%02d%02d");
}

char* taggedTimeString(const char* prefix, char* space) {
time_t tt = time(NULL);
  struct tm *now = localtime(&tt);
  sprintf(space,"%s%04d%02d%02d%02d%02d%02d", prefix,
    now->tm_year+1900, now->tm_mon+1, now->tm_mday,
    now->tm_hour, now->tm_min, now->tm_sec
  );
  return(space);
}

int parseDateTime(char* timestr) {
tm unixTime;
  char *leftOver = strptime(timestr, "%m/%d/%Y+%I:%M%p", &unixTime);
  unixTime.tm_sec = 0;
  time_t tick = mktime(&unixTime);
if(tick<0)printf("<!--- bad time %ld at %s -->\n",tick, timestr);
  return tick;
}

int parsePackedDateTime(char* timestr) {
tm unixTime;
  char *leftOver = strptime(timestr, "%Y%m%d%H%M%S", &unixTime);
  unixTime.tm_sec = 0;
  time_t tick = mktime(&unixTime);
  return tick;
}

int joinTimeParts(int year, int month, int day, int hour, int minute, int second) {
time_t tt = time(NULL);
  struct tm *now = localtime(&tt);
  now->tm_year = year-1900;
  now->tm_mon = month-1;
  now->tm_mday = day;
  now->tm_hour = hour;
  now->tm_min = minute;
  now->tm_sec = second;
  time_t tick = mktime(now);
  return tick;
}

