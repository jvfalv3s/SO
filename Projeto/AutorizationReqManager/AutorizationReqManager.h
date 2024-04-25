/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef AUTORIZATIONREQMANAGER_H
#define AUTORIZATIONREQMANAGER_H

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan();

/**
 * Sender Thread.
 */
void* Sender();

/**
 * Receiver Thread.
 */
void* Receiver();

/**
 * Kills System all processes sending SIGQUIT to them.
 */
void killSys();

/**
 * Kills the Sender and Receiver threads.
 */
void killThreads();

/**
 * Unlink all created pipes and closes all file descriptors.
 */
void unlinkPipes();

/**
 * Reports error ocurred printing it in screen and into log file and ends all processes.
 */
void autReqError(char* error_message);

/**
 * Ends the Autorization Request Manager and his threads.
 */
void endAutReqMan();

#endif