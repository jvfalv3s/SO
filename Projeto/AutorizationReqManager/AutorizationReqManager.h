/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef AUTORIZATIONREQMANAGER_H
#define AUTORIZATIONREQMANAGER_H

#define BUF_SIZE 100
/* Paths to user and back pipe */
#define USER_PIPE_PATH "/tmp/user_pipe"
#define BACK_PIPE_PATH "/tmp/back_pipe"
#include "../IntQueues.h"

#include <stdbool.h>

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan();


/***********************************************************************
 *                                                                     *
 *                               THREADS                               *
 *                                                                     *
 ***********************************************************************/

/**
 * Sender Thread.
 */
void* Sender(void* arg);

/**
 * Receiver Thread.
 */
void* Receiver(void* arg);


/***********************************************************************
 *                                                                     *
 *                           HELP FUNCTIONS                            *
 *                                                                     *
 ***********************************************************************/

/**
 * Creates the 2 queues, vid_queue and other_queue.
 */
void create_queues();

/**
 * Divides buffer info from pipes to the given char pointers.
*/
void div_buf_info(char* buf, char** arg1, char** arg2, char** arg3);

/**
 * Writes in a queue the new message (data, can be null).
 */
void write_in_queue(struct queue* queue_ptr, char* ID, char* command, char* data);

/**
 * Gets a message from the queue indicated by the pointer and return the data in the struct indicated by the pointer.
 */
void get_from_queue(struct queue* queue_ptr, struct message* request);

/**
 * Updates the queues eliminating expired requests.
 */
void update_queues();

/**
 * Check if any of the two internal queues are at 'percentage' of their total size.
 */
bool any_queue_is(double percentage);

/**
 * Sends a request to an authorization engine.
 */
void send_req_to(int auth_eng_num, struct message request);


/***********************************************************************
 *                                                                     *
 *                   RESOURCES MANAGEMENT FUNCTIONS                    *
 *                                                                     *
 ***********************************************************************/

/**
 * Logs all the requests in VIDEO and OTHER queues that were not executed.
 */
void logQueuesReqs();

/**
 * Sends SIGQUIT to all group processes and wait Monitor Engine to end to end.
 */
void killProcesses();

/**
 * Kills the Sender and Receiver threads.
 */
void killThreads();

/**
 * Deletes all Authorizations Engines and free all their resources from shared memory.
 */
void deleteAllAuthEngs();

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