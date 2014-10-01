#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <pthread.h>
#include <errno.h>
#include "CommunicationInterfaces.h"

void * launcher_message_handler(void *data) {
    SILANSAB_MESSAGE_QUEUES message_queues = *((SILANSAB_MESSAGE_QUEUES*) data);
    
    SILANSAB_MESSAGE *message;
    char buffer[MSG_SIZE];
    int num_received_bytes;

    printf("Thread LAUNCHER inicializado! \n");

    /* EXECUTANDO */
    while (1) {
        if (message_queues.from_launcher != (mqd_t) -1) {
            /* Recebendo dados da porta nao bloqueante! */
            num_received_bytes = (int) mq_receive(message_queues.from_launcher, buffer, MSG_SIZE, NULL);
            
            if(num_received_bytes >= 0) {
                printf("Received %d bytes\n", num_received_bytes);
                
                buffer[MSG_SIZE - 1] = '\0';
                
                message = (SILANSAB_MESSAGE*) buffer;
                
                switch(message->to) {
                case SATTELITE:
                    if (message_queues.to_sattelite != (mqd_t) -1) {
                        if (mq_send(message_queues.to_sattelite, (char *) message, sizeof(SILANSAB_MESSAGE) + 1, 0) == -1)
                            printf("LAUNCHER send to SATTELITE error\n");
                        else
                            printf("LAUNCHER sending to SATTELITE... \n");
                    }
                    break;
                }
            }
        }
    }
}

void * sattelite_message_handler(void *data) {
    SILANSAB_MESSAGE_QUEUES message_queues = *((SILANSAB_MESSAGE_QUEUES*) data);
    
    SILANSAB_MESSAGE *message;
    char buffer[MSG_SIZE];
    int num_received_bytes;

    printf("Thread SATTELITE inicializado! \n");

    /* EXECUTANDO */
    while (1) {
        if (message_queues.from_sattelite != (mqd_t) -1) {
            /* Recebendo dados da porta nao bloqueante! */
            num_received_bytes = (int) mq_receive(message_queues.from_sattelite, buffer, MSG_SIZE, NULL);
            
            if(num_received_bytes >= 0) {
                printf("Received %d bytes\n", num_received_bytes);
                
                buffer[MSG_SIZE - 1] = '\0';
                
                message = (SILANSAB_MESSAGE*) buffer;
                
                switch(message->to) {
                case LAUNCHER:
                    if (message_queues.to_launcher != (mqd_t) -1) {
                        if (mq_send(message_queues.to_launcher, (char *) message, sizeof(SILANSAB_MESSAGE) + 1, 0) == -1)
                            printf("SATTELITE send to LAUNCHER error\n");
                        else
                            printf("SATTELITE sending to LAUNCHER... \n");
                    }
                    break;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Threads */
    pthread_t thread_launcher;
    pthread_t thread_sattelite;
    
    struct sched_param params;
    
    /* Queue ports variables */
    SILANSAB_MESSAGE_QUEUES queues;
    
    int create_flags = O_CREAT | O_RDWR | O_NONBLOCK;
    mode_t perms = S_IRUSR | S_IWUSR;
    struct mq_attr attr;
    attr.mq_maxmsg = 20;
    attr.mq_msgsize = MSG_SIZE;

    /* Abrindo as Queue ports */
    /* LAUNCHER Queues */
    queues.from_launcher = mq_open("/FROM_LAUNCHER", create_flags, perms, &attr);
    if (queues.from_launcher != (mqd_t) -1)
        printf("Started queue port FROM_LAUNCHER...\n");
    else {
        printf("ERROR opening queue port FROM_LAUNCHER\n");
        exit(1);
    }
    queues.to_launcher = mq_open("/TO_LAUNCHER", create_flags, perms, &attr);
    if (queues.to_launcher != (mqd_t) -1)
        printf("Started queue port TO_LAUNCHER...\n");
    else {
        printf("ERROR opening queue port TO_LAUNCHER\n");
        exit(1);
    }
    /* SATTELITE Queues */
    queues.from_sattelite = mq_open("/FROM_SATTELITE", create_flags, perms, &attr);
    if (queues.from_sattelite != (mqd_t) -1)
        printf("Started queue port FROM_SATTELITE...\n");
    else {
        printf("ERROR opening queue port FROM_SATTELITE\n");
        exit(1);
    }
    queues.to_sattelite = mq_open("/TO_SATTELITE", create_flags, perms, &attr);
    if (queues.to_sattelite != (mqd_t) -1)
        printf("Started queue port TO_SATTELITE...\n");
    else {
        printf("ERROR opening queue port TO_SATTELITE\n");
        exit(1);
    }

    pthread_attr_t attri;
    pthread_attr_init(&attri);
    pthread_attr_setinheritsched(&attri, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_getschedparam(&attri, &params);
    params.sched_priority = 20;
    pthread_attr_setschedparam(&attri, &params);

    pthread_create(&thread_launcher, &attri, &launcher_message_handler, &queues);
    pthread_create(&thread_sattelite, &attri, &sattelite_message_handler, &queues);


    while(!getchar()){}


    /* Fechando todas as Queue ports */
    /* LAUNCHER Queues */
    printf("Closing queue port FROM_LAUNCHER: %d\n", mq_close(queues.from_launcher));
    printf("Closing queue port TO_LAUNCHER: %d\n", mq_close(queues.to_launcher));

    /* LAUNCHER Queues */
    printf("Closing queue port FROM_SATTELITE: %d\n", mq_close(queues.from_sattelite));
    printf("Closing queue port FROM_SATTELITE: %d\n", mq_close(queues.to_sattelite));

    exit(EXIT_SUCCESS);
}
