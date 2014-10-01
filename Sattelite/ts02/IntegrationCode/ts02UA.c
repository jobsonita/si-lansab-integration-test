#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


#include "ua.h"
#include "MyOperator.h"

#include "CommunicationInterfaces.h"

/* the port users will be connecting to */
#define DBUSPORT 4950

#define MSG_WAIT_OPTION 0

extern int gethostname(char *, size_t);

char *DF_server_ip = "127.0.0.1";
int DF_server_port = 1231;
int DF_server_socket;

char *to_outside_ip = "127.0.0.1";
int to_outside_socket;
struct sockaddr_in to_outside_address;

int from_outside_socket;


inC_MyOperator ua_inputs;
outC_MyOperator ua_outputs;


void connectToDFServer() {
    printf("DF server IP...: [%s]\n", DF_server_ip);
    printf("DF server Port: [%d]\n", DF_server_port);
    printf("\n");

    struct hostent *server = NULL;
    struct sockaddr_in address;

    DF_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (DF_server_socket < 0) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    server = gethostbyname(DF_server_ip);
    if (server == NULL) {
        printf("ERROR: cannot resolve hostname (%s) !\n", DF_server_ip);
        exit(1);
    }

    memset(&address, 0, sizeof(address));
    memcpy(&address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    address.sin_family = AF_INET;
    address.sin_port = htons(DF_server_port);

    if (connect(DF_server_socket, (const struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("ERROR: cannot connect with host (%s/%d) !\n", DF_server_ip, DF_server_port);
        exit(1);
    }

    printf("Connected to DF Server (%s/%d) .\n", DF_server_ip, DF_server_port);
}

void receiveMessage(SILANSAB_MESSAGE message) {
    SATTELITE_INPUT_INTERFACE input;

    if (message.to == SATTELITE) {
        input = message.input_interface.sattelite_input_interface;
        switch (message.from) {
        case LAUNCHER:
            printf("Message from LAUNCHER to SATTELITE : %d\n", input.SignalFromLauncher);
            ua_inputs.SignalFromLauncher = input.SignalFromLauncher;
            break;
        default:
            printf("Received packet from unknown sender (%d) !\n", message.from);
            break ;
        }
    }
}

void * receiveMessagesFromPeers(void *ptr) {
    struct sockaddr_in address_sender;
    int reuseaddr_option = 1;

    from_outside_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (from_outside_socket < 0 ) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    /* This call is what allows broadcast packets to be received by many processes on same host */
    if (setsockopt(from_outside_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_option, sizeof(reuseaddr_option)) < 0 ) {
        printf("ERROR: cannot configure socket for SO_REUSEADDR !\n");
        exit(1);
    }

    memset(&address_sender, 0, sizeof(address_sender));

    address_sender.sin_family = AF_INET;
    address_sender.sin_port = htons(DBUSPORT);
    address_sender.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(from_outside_socket, (struct sockaddr *) &address_sender, sizeof(address_sender)) < 0 ) {
        printf("ERROR: cannot bind socket !\n");
        exit(1);
    }

    int num_received_bytes;
    struct sockaddr_in address_receiver;
    int address_length = sizeof(struct sockaddr_in);

    char buffer[MSG_SIZE];

    while(1) {
        num_received_bytes = recvfrom(from_outside_socket, buffer, MSG_SIZE, MSG_WAIT_OPTION, (struct sockaddr *) &address_receiver, (socklen_t *) &address_length);
        if (num_received_bytes < 0) {
            printf("ERROR: could not receive messages from peers !\n");
            exit(1);
        }

        receiveMessage(*(SILANSAB_MESSAGE*)buffer);
    }

    return NULL;
}

void connectToPeers() {
    int broadcast_option = 1;

    if ((!strcmp((char *) &to_outside_ip, "localhost")) || (!strcmp((char *) &to_outside_ip, "127.0.0.1")))
        gethostname((char *) &to_outside_ip, sizeof(to_outside_ip));

    if (gethostbyname(to_outside_ip) == NULL) {
        printf("ERROR: cannot resolve hostname (%s) !\n", to_outside_ip);
        exit(1);
    }

    to_outside_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (to_outside_socket < 0) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    if (setsockopt(to_outside_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_option, sizeof(broadcast_option)) < 0) {
        printf("ERROR: cannot configure socket for SO_BROADCAST !\n");
        exit(1);
    }

    to_outside_address.sin_family = AF_INET;
    to_outside_address.sin_port = htons(DBUSPORT);
    to_outside_address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    memset(to_outside_address.sin_zero, 0, sizeof(to_outside_address.sin_zero));

    pthread_t thread_receiver;
    pthread_attr_t attri;
    struct sched_param params;

    pthread_attr_init(&attri);
    pthread_attr_setinheritsched(&attri, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_getschedparam(&attri, &params);

    params.sched_priority = 20;
    pthread_attr_setschedparam(&attri, &params);

    pthread_create(&thread_receiver, &attri, receiveMessagesFromPeers, NULL);

    printf("Connected to Peers (%s) .\n", to_outside_ip);
}

void sendMessagesToPeers() {
    SILANSAB_MESSAGE message;

    message.to = LAUNCHER;
    message.from = SATTELITE;
    LAUNCHER_INPUT_INTERFACE *output = &(message.input_interface.launcher_input_interface);
    
    output->SignalFromSattelite = ua_outputs.SignalToLauncher;
    
    sendto(to_outside_socket, (char *) &message, sizeof(message), MSG_WAIT_OPTION, (struct sockaddr *) &to_outside_address, sizeof(to_outside_address));

    /* Send messages to other peers */
}

void usage() {
    fprintf(stderr, "USAGE: Launch_ts02UA.c <server_ip> [<port_no>]\n");
    exit(1);
}

void parseParameters(int argc, char **argv) {
    if (argc < 2)
        usage();
    if (argc >= 2)
        DF_server_ip = argv[1];
    if (argc >= 3)
        DF_server_port = atoi(argv[2]);
}

int main(int argc, char **argv) {
    parseParameters(argc, argv);

    connectToDFServer();

    connectToPeers();

    char buffer[MSG_SIZE];
    int num_received_bytes;
    int num_sent_bytes;

    /* Start by resetting the outputs to their default values */
    MyOperator_reset(&ua_outputs);

    while (1) {
        /* Call the User Application generated by SCADE */
		MyOperator(&ua_inputs, &ua_outputs);

        ua_receive_clear(&ua_inputs, NULL);
        ua_inputs.SignalFromLauncher = 0;

        /* Send all output signals to DF server */
        num_sent_bytes = ua_send((buffer_el *) buffer, &ua_outputs, NULL);
        send(DF_server_socket, buffer, num_sent_bytes, MSG_DONTWAIT);

        /* Send messages to each peer */
        sendMessagesToPeers();

        usleep(100000);

        /* Receive all input signals from DF server */
        num_received_bytes = recv(DF_server_socket, buffer, MSG_SIZE, MSG_DONTWAIT);
        if (num_received_bytes > 0)
            ua_receive((buffer_el *) buffer, num_received_bytes, &ua_inputs, NULL);
    }

    close(DF_server_socket);
    close(to_outside_socket);
    close(from_outside_socket);
    return 0;
}
