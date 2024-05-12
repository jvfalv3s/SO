/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h> 
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/select.h>
#include "../LogFileManager/LogFileManager.h"
#include "./AuthorizationEngine.h"
#include "ShmData.h"

struct shm_struct* shm_ptr;
int AUTH_PROC_TIME;

/**
 * Implements an Authorization Engine.
 */
void AuthEngine(struct shm_struct* shmPtr, int auth_proc_time) {
    shm_ptr = shmPtr;
    AUTH_PROC_TIME = auth_proc_time;
}

/**
 * Creates a new authorization engine.
 */
void create_auth_eng(int auth_num, pid_t monitor_engine_pid) {
    int tmp_pipe[2];
    pipe(tmp_pipe);
    shm_ptr->auth_engs[auth_num].pipe_read_fd = tmp_pipe[0];
    shm_ptr->auth_engs[auth_num].pipe_write_fd = tmp_pipe[1];
    shm_ptr->auth_engs[auth_num].busy = false;
    shm_ptr->auth_engs[auth_num].l_request_time = 0;
    if((shm_ptr->auth_engs[auth_num].pid = fork()) == 0) {
        shm_ptr->n_auth_engs++;
        close(shm_ptr->auth_engs[auth_num].pipe_write_fd);
        AuthEngine(shm_ptr, auth_num, monitor_engine_pid);
        exit(EXIT_SUCCESS);
    }
    close(shm_ptr->auth_engs[auth_num].pipe_read_fd);
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


