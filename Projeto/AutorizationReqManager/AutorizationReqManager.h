#ifndef AUTORIZATIONREQMANAGER.H
#define AUTORIZATIONREQMANAGER.H

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan(const char* logFileName);

/**
 * Sender Thread.
 */
void* Sender();

/**
 * Receiver Thread.
 */
void* Receiver();

/**
 * Ends the Autorization Request Manager and his threads.
 * Not in final form.
 */
void endAutReqMan();

#endif