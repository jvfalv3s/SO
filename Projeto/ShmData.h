/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef SHMDATA_H
#define SHMDATA_H

#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

/* Comment this line to don't show debug messages */
#define DEBUG

/* Shared memory important definitions */
#define MAX_USERS_SHM 20      // Max number users (defines shm size)
#define SHM_PATH "shm"        // Path to shm file
#define SHM_SEM_PATH "shm_sem"

/* Sharerd memory structur */
typedef struct user {
    int id;
    int current_plafond;
    int max_plafond;
} user;

typedef struct auth_eng{
    pid_t pid;
    int pipe_read_fd;
    int pipe_write_fd;
    bool busy;
    time_t l_request_time;
}auth_eng;

typedef struct shm_struct {
    struct user users[MAX_USERS_SHM];
    struct auth_eng* auth_engs;
    int total_VIDEO_data;
    int total_VIDEO_auths;
    int total_MUSIC_data;
    int total_MUSIC_auths;
    int total_SOCIAL_data;
    int total_SOCIAL_auths;
    int n_users;
    int n_auth_engs;
}shm_struct;

struct shm_struct* shm_ptr;
int shm_size = sizeof(struct shm_struct);

sem_t* shm_sem;
bool shmSemCreated = false;

#endif