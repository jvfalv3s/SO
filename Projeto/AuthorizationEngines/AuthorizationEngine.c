/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#include <sys/msg.h>
#include <time.h>
#include "../LogFileManager/LogFileManager.h"
#include "./AuthorizationEngine.h"
#include "../IntQueues.h"
#include "../ShmData.h"
#include "../HelpData.h"

/**
 * Implements an Authorization Engine.
 */
void AuthEngine(int auth_eng_num) {
    char* log_message;
    if(sprintf(log_message, "AUTHORIZATION_ENGINE %d READY", auth_eng_num+1) < 0) {
        error("CREATING AUTHORIZATON ENGINE READY LOG MESSSAGE");
    }
    writeLog(log_message);
    
    struct message request;
    while(true) {
        read(shm_ptr->auth_engs[auth_eng_num].pipe_read_fd, &request)
        usleep(AUTH_PROC_TIME * 1000.0);

        sem_wait(shm_sem);
        if(request.id == 1) {
            process_user_req(request);
        } else {
            process_back_user_req(request);
        }
        sem_post(shm_sem);

        if(strcmp(request.command, "SOCIAL") == 0) {
            writelog("AUTHORIZATION_ENGINE %d: SOCIAL AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id);
        }
        else if(strcmp(request.command, "MUSIC") == 0) {
            writelog("AUTHORIZATION_ENGINE %d: MUSIC AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id);
        }
        else if(strcmp(request.command, "VIDEO") == 0) {
            writelog("AUTHORIZATION_ENGINE %d: VIDEO AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id);
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
        close(shm_ptr->auth_engs[auth_num].pipe_write_fd);
        AuthEngine(auth_num);
        exit(EXIT_SUCCESS);
    }
    close(shm_ptr->auth_engs[auth_num].pipe_read_fd);
}

/**
 * Deletes an authorization engine if it's not busy.
 */
void remove_auth_eng(int auth_num) {
    if((shm_ptr->auth_engs[auth_num].pid != 0) && (!shm_ptr->auth_engs[auth_num].busy)) {
        kill(shm_ptr->auth_engs[auth_num].pid, SIGQUIT);
        shm_ptr->auth_engs[auth_num].pid = 0;
        shm_ptr->auth_engs[auth_num].busy = false;
        close(shm_ptr->auth_engs[auth_num].pipe_write_fd);
    }
}

/**
 * Checks the busyness of all authorizations engines and if they passsed the time they should, turns to not busy.
 */
void check_auth_busy() {
    double dif;
    for(int i = 0; i < shm_ptr->n_auth_engs; i++) {
        if(shm_ptr->auth_engs[i].pid != NULL && shm_ptr->auth_engs[i].busy) {
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
        if((shm_ptr->auth_engs[i].pid != NULL) && (!shm_ptr->auth_engs[i].busy)) {
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
    char* log_message;
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
                    error("CREATING VIDEO REQUEST COMPLETE LOG MESSAGE");
                }
                writelog(log_message);
            }
            else if(strcmp(request.command, "MUSIC") == 0) {
                shm_ptr->total_MUSIC_auths++;
                shm_ptr->total_MUSIC_data = shm_ptr->total_MUSIC_data + request.data_to_reserve;
                if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: MUSIC AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                    error("CREATING MUSIC REQUEST COMPLETE LOG MESSAGE");
                }
                writelog(log_message);
            }
            else {
                shm_ptr->total_SOCIAL_auths++;
                shm_ptr->total_SOCIAL_data = shm_ptr->total_SOCIAL_data + request.data_to_reserve;
                if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: SOCIAL AUTHORIZATION REQUEST (ID = %d) PROCESSING COMPLETED", auth_eng_num+1, request.id) < 0) {
                    error("CREATING SOCIAL REQUEST COMPLETE LOG MESSAGE");
                }
                writelog(log_message);
            }
        }
        if(shm_ptr->users[i].current_plafond/shm_ptr->users[i].max_plafond <= 80) {
            kill(shm_ptr->auth_engs[auth_eng_num].pid, SIGUSR1);
        }
    }
    else {
        if(already_in) {
            if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: USER %d ALREADY REGISTED", auth_eng_num+1, request.id) < 0) {
                error("CREATING USER ALREADY REGISTED LOG MESSAGE");
            }
            writelog(log_message);
        }
        else if(i == MAX_USERS_SHM-1) {
            if(sprintf(log_message, "AUTHORIZATION_ENGINE %d: MAX NUMBER OF USERS REACHED", auth_eng_num+1) < 0) {
                error("CREATING MAX NUMBER OF USERS LOG MESSAGE");
            }
            writelog(log_message);
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