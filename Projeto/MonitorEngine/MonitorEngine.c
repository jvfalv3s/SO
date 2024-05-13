/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/
#include <sys/msg.h> 
#include <sys/types.h> // Add this line
#include <fcntl.h> // Add this line
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "./MonitorEngine.h"
#include "../ShmData.h"
#include "../HelpData.h"
#include "../MessageQueue.h"
#include "../LogFileManager/LogFileManager.h"

void MonEng() {
    /* Creating the message queue key */
    mq_key = ftok(MQ_KEY_PATH, MQ_KEY_ID);
    if(mq_key == -1) error("Creating message queue key");

    /* Opening the message queue for reading */
    mq_id = msgget(mq_key, IPC_CREAT | 0200);  // 0200 --> write-only permissions
    if(mq_id == -1) error("Getting message queue id");
    mqCreated = true;

    signal(SIGQUIT, endMonEng);
    signal(SIGUSR1, process_alerts);

    while(true) {
        // Envie as estatísticas para o back user pela msg queue e para o log
        sendStatistics();

        sleep(30);
    }
    endMonEng();
}

//funcao para processar os avisos
void process_alerts() {
    char* mq_sem_path;
    float plafond_percentage;

    sem_wait(shm_sem);
    for(int i = 0; i < shm_ptr->n_users; i++) {
        plafond_percentage = (float)shm_ptr->users[i].current_plafond / shm_ptr->users[i].max_plafond;
        message.mgg_type = shm_ptr->users[i].id;

        if(plafond_percentage >= 1 && (!shm_ptr->users[i].alert100sent)) {
            // Plafond de 100% atingido para este usuário
            strcpy(message.msg_text, "ALERT: YOUR PLAFOND REACHED 100%%.");
            msgsnd(mq_id, &message, sizeof(message), 0);

            if(sprintf(mq_sem_path, "%s%d", MQ_NAMED_SEMAPHORE_GEN_PATH, shm_ptr->users[i].id) < 0) {
                MonEngError("CREATING MESSAGE QUEUE SEMAPHORE PATH");
            }
            mq_sem = sem_open(mq_sem_path, 0);
            sem_post(mq_sem);
            sem_close(mq_sem);

            shm_ptr->users[i].alert100sent = true;
        }
        else if (plafond_percentage >= 0.9 && (!shm_ptr->users[i].alert90sent)) {
            // Plafond de 90% ou mais atingido para este usuário
            strcpy(message.msg_text, "ALERT: YOUR PLAFOND REACHED 90%%.");
            msgsnd(mq_id, &message, sizeof(message), 0);

            if(sprintf(mq_sem_path, "%s%d", MQ_NAMED_SEMAPHORE_GEN_PATH, shm_ptr->users[i].id) < 0) {
                MonEngError("CREATING MESSAGE QUEUE SEMAPHORE PATH");
            }
            mq_sem = sem_open(mq_sem_path, 0);
            sem_post(mq_sem);
            sem_close(mq_sem);

            shm_ptr->users[i].alert90sent = true;
        }
        else if (plafond_percentage >= 0.8 && (!shm_ptr->users[i].alert80sent)) {
            // Plafond de 80% ou mais atingido para este usuário
            strcpy(message.msg_text, "ALERT: YOUR PLAFOND REACHED 80%%.");
            msgsnd(mq_id, &message, sizeof(message), 0);

            if(sprintf(mq_sem_path, "%s%d", MQ_NAMED_SEMAPHORE_GEN_PATH, shm_ptr->users[i].id) < 0) {
                MonEngError("CREATING MESSAGE QUEUE SEMAPHORE PATH");
            }
            mq_sem = sem_open(mq_sem_path, 0);
            sem_post(mq_sem);
            sem_close(mq_sem);

            shm_ptr->users[i].alert80sent = true;
        }
    }
    sem_post(shm_sem);
}

void sendStatistics() {
    struct mq_message stats_message;
    char* mq_sem_path;
    stats_message.mgg_type = 1;

    sem_wait(shm_sem);
    if(sprintf(stats_message.msg_text, "STATS\nSERVICE / TOTAL DATA / AUTH REQS\nVIDEO:  %d  %d\nMUSIC:  %d  %d\nSOCIAL:  %d  %d", shm_ptr->total_VIDEO_data, shm_ptr->total_VIDEO_auths,
               shm_ptr->total_MUSIC_data, shm_ptr->total_MUSIC_auths, shm_ptr->total_SOCIAL_data, shm_ptr->total_SOCIAL_auths) < 0) {
        MonEngError("CREATING STATS MESSAGE");
    }
    sem_post(shm_sem);

    writelog(stats_message.msg_text);

    msgsnd(mq_id, &stats_message, sizeof(stats_message), 0);

    stcpy(mq_sem_path, MQ_NAMED_BACK_SEM_P);
    mq_sem = sem_open(mq_sem_path, 0);
    sem_post(mq_sem);
    sem_close(mq_sem);
}

void MonEngError(char* error_message) {
    kill(SYS_PID, SIGQUIT);
    endMonEng();
}

void endMonEng() {
    if(mqSemCreated) {
        sem_close(sem);
        sem_unlink(MQ_SEM_PATH);
    }
    if(mqCreated) msgctl(mq_id, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}
