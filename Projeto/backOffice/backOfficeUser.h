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