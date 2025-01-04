#ifndef HABACTLOG_TIMESTRING_H_
#define HABACTLOG_TIMESTRING_H_
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Parses local time from a packed time string and returns seconds since 1970.
// Input char array is interpreted as "%m/%d/%Y+%I:%M%p" as in 09/11/2001+9:15AM
// The use of a '+' instead of ' ' between day and hour is for http form parsing.
int parseDateTime(char* timestr);

// Parses local time from a packed time string and returns seconds since 1970.
// Input char array is interpreted as "%Y%m%d%H%M%S" as in 200109110915.
int parsePackedDateTime(char* timestr);

// This is the complement of parsePackedDateTime(). it produces a packed string
// encoding the local time as "%Y%m%d%H%M%S". You must supply the char buffer
// to be loaded and returned.
char* denseTimeString(char* space);

// like the above but formatted with "%Y-%m-%d %H:%M:%S" as used in sql
char* dbTimeString(long ticks, char* space);

// generic form used by the two above formatters
char* timeString(char* space, const char* format);

// exactly like denseTimeString() except it add a prefix for making UNIQUE IDs.
char* taggedTimeString(const char* prefix, char* space);

// Parses local time from date and time components and returns seconds since 1970.
int joinTimeParts(int year, int month, int day, int hour, int minute, int second);

#endif // HABACTLOG_TIMESTRING_H_
