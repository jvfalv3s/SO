/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef SHM_DATA_H
#define SHM_DATA_H

/* Comment this line to don't show debug messages */
#define DEBUG

/* Shared memory important definitions */
#define MAX_USERS_SHM 20      // Max number users (defines shm size)
#define SHM_PATH "./tmp/shm"  // Path to shm file

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
    struct user[MAX_USERS_SHM];
    struct auth_eng* auth_engs;
    int n_users;
    int n_auth_engs;
}shm_struct;

#endif