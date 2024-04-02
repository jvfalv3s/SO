/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef BACKOFFICEUSER_H
#define BACKOFFICEUSER_H

/**
 * Handles the SIGINT signal.
 */
void handle_sigint(int sig);

/**
 * Frees all the resorces and prints error message.
 */
void error(char* str_to_print);

#endif