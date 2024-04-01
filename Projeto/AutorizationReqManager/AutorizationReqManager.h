/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef AUTORIZATIONREQMANAGER_H
#define AUTORIZATIONREQMANAGER_H

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan();

/**
 * Sender Thread.
 */
void* Sender();

/**
 * Receiver Thread.
 */
void* Receiver();

/**
 * Ends the Autorization Request Manager and his threads.
 * Not in final form.
 */
void endAutReqMan();

#endif