/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef LOGFILEMANAGER_H
#define LOGFILEMANAGER_H

/**
 * Creates a new log file.
 */
void creatLogFile();

/**
 * Writes a new log in the log file and also prints the message in the console.
 */
void writeLog(char* newLog);

/**
 * Ends the log file writting a last log in it and in the console, it also destroys the mutex created.
 */
void endLogFile();

/**
 * Exists the program after an error printing the message in the screen and writting it in the log file.
 */
void error(char* error_message);

#endif