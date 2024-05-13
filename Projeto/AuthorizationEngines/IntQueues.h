#ifndef INTQUEUES_H
#define INTQUEUES_H

#define MAX_CHAR_COMMAND 30

/* Message from message queue struct */
typedef struct message {
    int id;
    char command[MAX_CHAR_COMMAND];
    int data_to_reserve;
    time_t request_time;
}message;

typedef struct queue {
    struct message* messages;
    int read_pos;
    int write_pos;
    int max_queue_pos;
    int n_empty;
}queue;

#endif