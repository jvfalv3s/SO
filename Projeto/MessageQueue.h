#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#define MQ_SEM_PATH "mq_sem"
#define MAX_CHAR_MESSAGE_AMMOUNT 100

/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/message_queue"
#define MQ_KEY_ID 'a'

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
}mq_message;

struct mq_message message;
int mq_key;
int mq_id;
sem_t *sem;

#endif