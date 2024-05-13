#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#define MAX_CHAR_MESSAGE_AMMOUNT 100
#define MQ_NAMED_SEMAPHORE_GEN_PATH "mq_sem_"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE that indicates about periodic stats */
#define MQ_NAMED_BACK_SEM_P "back_sem_P"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE that indicates about stats response */
#define MQ_NAMED_BACK_SEM_R "back_sem_R"

/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/tmp/message_queue"
#define MQ_KEY_ID 'a'

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
}mq_message;

#endif