/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/
#include <sys/msg.h> 
#include <time.h>
#include "./MonitorEngine.h"
#include "../ShmData.h"
#include "../HelpData.h"
#include "../MessageQueue.h"
#include "../LogFileManager/LogFileManager.h"

static mq_message mq_msg;
static int mq_key;
static int mq_id;
static bool mqCreated = false;
static sem_t* mq_sem;

extern int MOBILE_USERS;
extern int QUEUE_POS;
extern int AUTH_SERVERS_MAX;
extern int AUTH_PROC_TIME;
extern int MAX_VIDEO_WAIT;
extern int MAX_OTHERS_WAIT;
extern bool AutReqManCreated, MonEngCreated;
extern struct shm_struct* shm_ptr;
extern int shm_size;

extern sem_t* shm_sem;
extern bool shmSemCreated;

extern pid_t SYS_PID, ARM_PID, ME_PID;


/**
 *  This function is responsible for the Monitor Engine
 */
void MonEng() {
    /* Creating the message queue key 
    mq_key = ftok(MQ_KEY_PATH, MQ_KEY_ID);
    if(mq_key == -1) error("Creating message queue key");
    */

    mq_key = 1234;

    /* Opening the message queue for reading */
    mq_id = msgget(mq_key, IPC_CREAT | 0200);  // 0200 --> write-only permissions
    if(mq_id == -1) error("Getting message queue id"); // Error if the message queue was not created
    mqCreated = true; // Flag to indicate that the message queue was created

    signal(SIGQUIT, endMonEng); // Signal to end the Monitor Engine
    signal(SIGUSR1, process_alerts); // Signal to process alerts

    while(true) {
        // Envie as estatísticas para o back user pela msg queue e para o log
        sendStatistics();

        sleep(30);
    }
    endMonEng(); //call for the end of the Monitor Engine function
}

/**
 *    Process alerts when certain signals are received
 */
void process_alerts() {
    char mq_sem_path[1024]; // Message queue semaphore path
    float plafond_percentage; 

    
    sem_wait(shm_sem);
    for(int i = 0; i < shm_ptr->n_users; i++) {
        plafond_percentage = (float)shm_ptr->users[i].current_plafond / shm_ptr->users[i].max_plafond; //calculate the percentage of the plafond used by the user
        mq_msg.mgg_type = shm_ptr->users[i].id;

        if(plafond_percentage >= 1 && (!shm_ptr->users[i].alert100sent)) { //100% usage of the plafond reached for this user
            
            strcpy(mq_msg.msg_text, "ALERT: YOUR PLAFOND REACHED 100%%."); // Copy the message to the message queue
            msgsnd(mq_id, &mq_msg, sizeof(mq_msg), 0); // Send the message to the message queue
            if(sprintf(mq_sem_path, "%s%d", MQ_NAMED_SEMAPHORE_GEN_PATH, shm_ptr->users[i].id) < 0) {
                MonEngError("CREATING MESSAGE QUEUE SEMAPHORE PATH"); // Error if the semaphore path was not created
            }
            mq_sem = sem_open(mq_sem_path, 0); 
            sem_post(mq_sem); 
            sem_close(mq_sem); 

            shm_ptr->users[i].alert100sent = true; // Flag to indicate that the alert was sent
        }
        else if (plafond_percentage >= 0.9 && (!shm_ptr->users[i].alert90sent)) { //90% or more plafond reached for this user
            strcpy(mq_msg.msg_text, "ALERT: YOUR PLAFOND REACHED 90%%."); // Copy the message to the message queue
            msgsnd(mq_id, &mq_msg, sizeof(mq_msg), 0); // Send the message to the message queue

            if(sprintf(mq_sem_path, "%s%d", MQ_NAMED_SEMAPHORE_GEN_PATH, shm_ptr->users[i].id) < 0) {
                MonEngError("CREATING MESSAGE QUEUE SEMAPHORE PATH");// Error if the semaphore path was not created
            }
            mq_sem = sem_open(mq_sem_path, 0); 
            sem_post(mq_sem); 
            sem_close(mq_sem); 

            shm_ptr->users[i].alert90sent = true; // Flag to indicate that the alert was sent
        }
        else if (plafond_percentage >= 0.8 && (!shm_ptr->users[i].alert80sent)) { //80% or more plafond reached for this user
            strcpy(mq_msg.msg_text, "ALERT: YOUR PLAFOND REACHED 80%%."); // Copy the message to the message queue
            msgsnd(mq_id, &mq_msg, sizeof(mq_msg), 0); // Send the message to the message queue

            if(sprintf(mq_sem_path, "%s%d", MQ_NAMED_SEMAPHORE_GEN_PATH, shm_ptr->users[i].id) < 0) {
                MonEngError("CREATING MESSAGE QUEUE SEMAPHORE PATH"); // Error if the semaphore path was not created
            }
            mq_sem = sem_open(mq_sem_path, 0); 
            sem_post(mq_sem);
            sem_close(mq_sem); 

            shm_ptr->users[i].alert80sent = true; // Flag to indicate that the alert was sent
        }
    }
    sem_post(shm_sem);
}


/**
 *   Sends statistics to the back user and to the log file
 */
void sendStatistics() {
    struct mq_message stats_message; //Trough this struct we will send the statistics to the back user
    char mq_sem_path[1024]; // Message queue semaphore path
    stats_message.mgg_type = 1; // Message type for the statistics

    if(sprintf(stats_message.msg_text, "STATS\nSERVICE / TOTAL DATA / AUTH REQS\nVIDEO:  %d  %d\nMUSIC:  %d  %d\nSOCIAL:  %d  %d", shm_ptr->total_VIDEO_data, shm_ptr->total_VIDEO_auths, // Copy the message to the message queue
               shm_ptr->total_MUSIC_data, shm_ptr->total_MUSIC_auths, shm_ptr->total_SOCIAL_data, shm_ptr->total_SOCIAL_auths) < 0) {
        MonEngError("CREATING STATS MESSAGE"); // Error if the message was not created
    }
    sem_post(shm_sem); 

    writeLog(stats_message.msg_text); // Write the message with the statistics to the log file

    msgsnd(mq_id, &stats_message, sizeof(stats_message), 0); // Send the message with the statistics to the message queue

    strcpy(mq_sem_path, MQ_NAMED_BACK_SEM_P); //This part of the code is responsible for sending the statistics to the back user
    mq_sem = sem_open(mq_sem_path, 0);
    sem_post(mq_sem);
    sem_close(mq_sem);
}

/**
 *   Handle errors in the Monitor Engine
 */
void MonEngError(char* error_message) {
    error(error_message); //call the error function
    signal(SIGQUIT, SIG_IGN); //Ignore the signal to end the Monitor Engine
    kill(SYS_PID, SIGQUIT); //Send the signal to end the system
    kill(0, SIGQUIT); //Send the signal to end the Monitor Engine
    endMonEng(); //call for the end of the Monitor Engine function
}

/**
 *   End the Monitor Engine
 */
void endMonEng() {
    sem_trywait(shm_sem);
    sem_post(shm_sem);
    int status; //obtain the status of the child process
    while(waitpid(0, &status, WNOHANG) > 0); //wait for the child process to end
    if(mqCreated) msgctl(mq_id, IPC_RMID, NULL); //remove the message queue
    exit(EXIT_SUCCESS); //exit the Monitor Engine
}
