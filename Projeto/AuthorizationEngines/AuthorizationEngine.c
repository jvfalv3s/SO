/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#include <sys/msg.h>
#include <time.h>
#include <stdbool.h>
#include "../LogFileManager/LogFileManager.h"
#include "./AuthorizationEngine.h"
#include "../IntQueues.h"
#include "../ShmData.h"
#include "../HelpData.h"
#include "../MessageQueue.h"

bool using_shm_sem = false;
int auth_eng_num;

extern mq_message mq_msg;
extern int mq_key;
extern int mq_id;
extern bool mqCreated;
extern sem_t* mq_sem;
extern struct shm_struct* shm_ptr;
extern int shm_size;
extern sem_t* shm_sem;
extern bool shmSemCreated;
extern int AUTH_PROC_TIME;

/**
 * Implements an Authorization Engine.
 */
void AuthEngine(int authEngNum) {
    auth_eng_num = authEngNum;

    char log_message[1024];
    if(sprintf(log_message, "AUTHORIZATION_ENGINE %d READY", auth_eng_num+1) < 0) {
        AuthEngError("CREATING AUTHORIZATON ENGINE READY LOG MESSSAGE");
    }
    writeLog(log_message);
    
    signal(SIGQUIT, SIG_IGN);
    signal(SIGINT, endAuthEng);

    struct message request;
    while(true) {
        sem_wait(shm_sem);
        using_shm_sem = true;

        read(shm_ptr->auth_engs[auth_eng_num].pipe_read_fd, &request, sizeof(request));
        sem_post(shm_sem);
        usleep(AUTH_PROC_TIME * 1000.0);
        sem_wait(shm_sem);
           
        if(request.id == 1) {
            process_user_req(auth_eng_num,request);
        } else {
            process_back_user_req(request);
        }
        sem_post(shm_sem);
        using_shm_sem = false;

        if(strcmp(request.command, "SOCIAL") == 0) {
            if(sprintf(log_message,"AUTHORIZATION_ENGINE %d: SOCIAL AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                AuthEngError("CREATING SOCIAL REQUEST COMPLETE LOG MESSAGE");
            }
            writeLog(log_message);
        }
        else if(strcmp(request.command, "MUSIC") == 0) {
            if(sprintf(log_message,"AUTHORIZATION_ENGINE %d: MUSIC AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                AuthEngError("CREATING MUSIC REQUEST COMPLETE LOG MESSAGE");
            }
            writeLog(log_message);
        }
        else if(strcmp(request.command, "VIDEO") == 0) {
            if(sprintf(log_message,"AUTHORIZATION_ENGINE %d: VIDEO AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                AuthEngError("CREATING VIDEO REQUEST COMPLETE LOG MESSAGE");
            }
            writeLog(log_message);
        }
    }
}

/**
 * Creates a new authorization engine.
 */
void create_auth_eng(int auth_num) {
    int tmp_pipe[2];
    pipe(tmp_pipe);
    shm_ptr->auth_engs[auth_num].pipe_read_fd = tmp_pipe[0];
    shm_ptr->auth_engs[auth_num].pipe_write_fd = tmp_pipe[1];
    shm_ptr->auth_engs[auth_num].busy = false;
    shm_ptr->auth_engs[auth_num].l_request_time = 0;
    if((shm_ptr->auth_engs[auth_num].pid = fork()) == 0) {
        shm_ptr->n_auth_engs++;
        AuthEngine(auth_num);
        exit(EXIT_SUCCESS);
    }
}

/**
 * Deletes an authorization engine if it's not busy.
 */
void remove_auth_eng(int auth_num) {
    if((shm_ptr->auth_engs[auth_num].pid != 0) && (!shm_ptr->auth_engs[auth_num].busy)) {
        kill(shm_ptr->auth_engs[auth_num].pid, SIGINT);
        shm_ptr->n_auth_engs--;
        shm_ptr->auth_engs[auth_num].pid = 0;
        shm_ptr->auth_engs[auth_num].busy = false;
        close(shm_ptr->auth_engs[auth_num].pipe_write_fd);
        close(shm_ptr->auth_engs[auth_num].pipe_read_fd);
    }
}

/**
 * Checks the busyness of all authorizations engines and if they passsed the time they should, turns to not busy.
 */
void check_auth_busy() {
    double dif;
    for(int i = 0; i < shm_ptr->n_auth_engs; i++) {
        if(shm_ptr->auth_engs[i].pid != 0 && shm_ptr->auth_engs[i].busy) {
            dif = difftime(shm_ptr->auth_engs[i].l_request_time, time(0));
            if(dif > (AUTH_PROC_TIME / 1000)) {
                shm_ptr->auth_engs[i].busy = false;
            }
        }
    }
}

/**
 * Checks if there is any authorization engine free and returns his number. If none is free return -1.
 */
int get_auth_eng_num() {
    for(int i = 0; i < shm_ptr->n_auth_engs; i++) {
        if((shm_ptr->auth_engs[i].pid != 0) && (!shm_ptr->auth_engs[i].busy)) {
            return i;
        }
    }
    return -1;
}

/**
 * Process a mobile user request.
 */
void process_user_req(int auth_eng_num, struct message request) {
    bool already_in = false;
    char log_message[1024];
    int i;
    for(i = 0; i < shm_ptr->n_users; i++) {
        if(request.id == shm_ptr->users[i].id)  {
            already_in = true;
            break;
        }
    }
    if(strcmp(request.command, "VIDEO") == 0 || strcmp(request.command, "MUSIC") == 0 || strcmp(request.command, "SOCIAL") == 0) {
        if(request.data_to_reserve <= shm_ptr->users[i].current_plafond) {
            shm_ptr->users[i].current_plafond = shm_ptr->users[i].current_plafond - request.data_to_reserve;
            
            if(strcmp(request.command, "VIDEO") == 0) {
                shm_ptr->total_VIDEO_auths++;
                shm_ptr->total_VIDEO_data = shm_ptr->total_VIDEO_data + request.data_to_reserve;
                if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: VIDEO AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                    AuthEngError("CREATING VIDEO REQUEST COMPLETE LOG MESSAGE");
                }
                writeLog(log_message);
            }
            else if(strcmp(request.command, "MUSIC") == 0) {
                shm_ptr->total_MUSIC_auths++;
                shm_ptr->total_MUSIC_data = shm_ptr->total_MUSIC_data + request.data_to_reserve;
                if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: MUSIC AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                    AuthEngError("CREATING MUSIC REQUEST COMPLETE LOG MESSAGE");
                }
                writeLog(log_message);
            }
            else {
                shm_ptr->total_SOCIAL_auths++;
                shm_ptr->total_SOCIAL_data = shm_ptr->total_SOCIAL_data + request.data_to_reserve;
                if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: SOCIAL AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                    AuthEngError("CREATING SOCIAL REQUEST COMPLETE LOG MESSAGE");
                }
                writeLog(log_message);
            }
        }
        if(shm_ptr->users[i].current_plafond/shm_ptr->users[i].max_plafond <= 80) {
            kill(shm_ptr->auth_engs[auth_eng_num].pid, SIGUSR1);
        }
    }
    else {
        if(already_in) {
            if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: USER %d ALREADY REGISTED", auth_eng_num+1, request.id) < 0) {
                AuthEngError("CREATING USER ALREADY REGISTED LOG MESSAGE");
            }
            writeLog(log_message);
        }
        else if(i == MAX_USERS_SHM-1) {
            if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: MAX NUMBER OF USERS REACHED", auth_eng_num+1) < 0) {
                AuthEngError("CREATING MAX NUMBER OF USERS LOG MESSAGE");
            }
            writeLog(log_message);
        }
        else {
            shm_ptr->users[i].id = request.id;
            shm_ptr->users[i].max_plafond = atoi(request.command);
            shm_ptr->users[i].current_plafond = 0;
        }
    }
}

/**
 * Process a back office user request.
 */
void process_back_user_req(struct message request) {
    if(strcmp(request.command, "data_stats") == 0) {
        struct mq_message stats_message;
        stats_message.mgg_type = 1;
        if(sprintf(stats_message.msg_text, "STATS\nSERVICE / TOTAL DATA / AUTH REQS\nVIDEO:  %d  %d\nMUSIC:  %d  %d\nSOCIAL:  %d  %d", shm_ptr->total_VIDEO_data, shm_ptr->total_VIDEO_auths,
                   shm_ptr->total_MUSIC_data, shm_ptr->total_MUSIC_auths, shm_ptr->total_SOCIAL_data, shm_ptr->total_SOCIAL_auths) < 0) {
            error("CREATING STATS MESSAGE");
        }
        msgsnd(mq_id, &stats_message, sizeof(stats_message), 0);
    }
    else {
        shm_ptr->total_MUSIC_auths = 0;
        shm_ptr->total_MUSIC_data = 0;
        shm_ptr->total_SOCIAL_auths = 0;
        shm_ptr->total_SOCIAL_data = 0;
        shm_ptr->total_VIDEO_auths = 0;
        shm_ptr->total_VIDEO_data = 0;
    }
}

/**
 * Handles an error in Authorization Engine.
 */
void AuthEngError(char* error_message) {
    error(error_message);
    endAuthEng();
}

/**
 * Ends Authotization Engine freeing all the resources.
 */
void endAuthEng() {
    if(using_shm_sem) sem_post(shm_sem);
    exit(EXIT_SUCCESS);
}
