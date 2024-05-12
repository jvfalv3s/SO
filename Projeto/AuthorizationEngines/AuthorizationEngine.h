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
void create_auth_eng();

/**
 * Checks the busyness of all authorizations engines and if they passsed the time they should, turns to not busy.
 */
void check_auth_busy();

#endif