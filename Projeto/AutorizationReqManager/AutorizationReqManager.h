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
void AutReqMan(pid_t SYS_PID);

/**
 * Sender Thread.
 */
void* Sender();

/**
 * Receiver Thread.
 */
void* Receiver();

/**
 * Kills the Sender and Receiver threads.
 */
void killThreads();

/**
 * Unlink all created pipes.
 */
void unlinkPipes() {
    if(userPipeCreated == 1) unlink(USER_PIPE_PATH);
    if(backPipeCreated == 1) unlink(BACK_PIPE_PATH);
}

/**
 * Ends the Autorization Request Manager and his threads.
 * Not in final form.
 */
void endAutReqMan();

#endif