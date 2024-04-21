/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef MOBILEUSER_H
#define MOBILEUSER_H

/**
 * Sends the registation message with the mobile user ID and initial plafond.
 */
void send_reg_message(int initial_plafond);

/**
 * Sends a social request to user pipe.
 */
void send_social_req(int data_to_reserve);

/**
 * Sends a music request to user pipe.
 */
void send_music_req(int data_to_reserve);

/**
 * Sends a video request to user pipe.
 */
void send_video_req(int data_to_reserve);

/**
 * Gets the milliseconds.
 */
long long get_millis();

/**
 * Frees resorces (mutex and pipe file descriptor)
 */
void free_resorces();

/**
 * Handles the SIGINT signal.
 */
void handle_sigint(int sig);

/**
 * Frees all the resorces and prints error message.
 */
void error(char* str_to_print);

#endif
