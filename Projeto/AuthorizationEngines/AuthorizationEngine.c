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

int AuthEngine(struct shm_struct shm_ptr) {
    
}