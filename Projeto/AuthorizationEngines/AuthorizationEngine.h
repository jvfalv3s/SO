/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef AUTHORIZATIONENGINE_H
#define AUTHORIZATIONENGINE_H

/**
 * Implements an Authorization Engine.
 */
void AuthEngine(struct shm_struct* shmPtr, int auth_proc_time);

/**
 * Creates a new authorization engine.
 */
void create_auth_eng(int auth_num);

/**
 * Checks the busyness of all authorizations engines and if they passsed the time they should, turns to not busy.
 */
void check_auth_busy();

/**
 * Checks if there is any authorization engine free and returns his number. If none is free return -1.
 */
int get_auth_eng_num()

#endif