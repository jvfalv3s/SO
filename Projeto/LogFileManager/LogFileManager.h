#ifndef LOGFILEMANAGER.H
#define LOGFILEMANAGER.H

const char* creatLogFile();

void writeLog(const char* logFileName, char* newLog);

void endLogFile(char* logFileName);

void error(char* error_message);

#endif