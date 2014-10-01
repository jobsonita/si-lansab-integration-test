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

extern int gethostname(char *, size_t);
extern int usleep(__useconds_t __useconds);

int to_outside_socket
struct sockaddr_in to_outside_address;

inC_MyOperator ua_inputs;
outC_Myoperator ua_outputs;

int connectToDFServer(char *ip, int port) {
    int socket;

    struct hostent *server = NULL;
    struct sockaddr_in address;

    socket = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    server = gethostbyname(ip);
    if (server == NULL) {
        printf("ERROR: cannot resolve hostname (%s) !\n", ip);
        exit(1);
    }

    bzero((char *) &address, sizeof(address));
    bcopy((char *) server->h_addr_list[0], (char *) &address.sin_addr.s_addr, server->h_length);

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (connect(socket, (const struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("ERROR: cannot connect with host (%s/%d) !\n", ip, port);
        exit(1);
    }

    printf("Connected to DF Server (%s/%d) .\n", ip, port);

    return socket;
}

int connectToCommunicationServer() {
    int socket;
    int broadcast_option = 1;

    char *ip = "localhost" ;

    if ((!strcmp((char *) &ip, "localhost")) || (!strcmp((char *) &ip, "127.0.0.1")))
        gethostname((char *) &ip, sizeof(ip));

    if (gethostbyname(ip) == NULL) {
        printf("ERROR: cannot resolve hostname (%s) !\n", ip);
        exit(1);
    }

    socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket < 0) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &broadcast_option, sizeof(broadcast_option)) < 0) {
        printf("ERROR: cannot configure socket for SO_BROADCAST !\n");
        exit(1);
    }

    printf("Connected to Communication Server (%s) .\n", ip);

    return socket;
}

void * receiveMessagesFromPeers(void *ptr) {
    int from_outside_socket ;
    int reuseaddr_option = 1 ;

    SILANSAB_MESSAGE message;

    char buffer[MSG_SIZE];

    struct sockaddr_in address_sender ;
    struct sockaddr_in address_receiver ;

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
    int address_length = sizeof(struct sockaddr_in);

    while(1) {
        num_received_bytes = recvfrom(from_outside_socket, &buffer, MSG_SIZE, MSG_WAIT, (struct sockaddr *) &address_receiver, (socklen_t *) &address_length);
        if (num_received_bytes < 0) {
            printf("ERROR: could not receive messages from peers !\n");
            exit(1);
        }

        message = *(SILANSAB_MESSAGE *)buffer;

        switch (message.from) {
        case SATTELITE:
            ua_inputs.SignalFromSattelite = ((LAUNCHER_INPUT_INTERFACE) message.input_interface).SignalFromSattelite;
            break;
        default:
            printf("Received packet %d bytes long from unknown sender (%d) !\n", num_received_bytes, message.from);
            break ;
        }
    }

    close(from_outside_socket);
    return NULL;
}

void sendMessagesToPeers() {
    SILANSAB_MESSAGE message;

    message.to = SATTELITE;
    message.from = LAUNCHER;
    ((SATTELITE_INPUT_INTERFACE) message.input_interface).SignalFromLauncher = ua_outputs.SignalToSattelite;
    sendto(to_outside_socket, (char *) &message, sizeof(message), MSG_WAIT, (struct sockaddr *) &to_outside_address, sizeof(to_outside_address));

    /* Send messages to other peers */
}

void usage() {
    fprintf(stderr, "USAGE: Launch_ts01UA.c <server_ip> [<port_no>]\n");
    exit(1);
}

void parseParameters(int argc, char **argv, char **ip, int *port) {
    if (argc < 2)
        usage();
    if (argc >= 2)
        *ip = argv[1];
    if (argc >= 3)
        *port = atoi(argv[2]);
}

int main(int argc, char **argv) {
    char *DF_server_ip = "192.168.0.1";
    int DF_server_port = 1231;
    int DF_server_socket;

    parseParameters(argc, argv, &DF_server_ip, DF_server_port);

    printf("DF server IP...: [%s]\n", DF_server_ip);
    printf("DF server Port: [%d]\n", DF_server_port);
    printf("\n");

    DF_server_socket = connectToDFServer(DF_server_ip, DF_server_port);

    to_outside_socket = connectToCommunicationServer();

    to_outside_address.sin_family = AF_INET
    to_outside_address.sin_port = htons(DBUSPORT);
    to_outside_address.sin_addr.s_addr = INADDR_BROADCAST;
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

    char buffer[MSG_SIZE];
    int num_received_bytes;
    int num_sent_bytes;

    /* Start by resetting the outputs to their default values */
    MyOperator_reset(&ua_outputs);

    while (1) {
        /* Call the User Application generated by SCADE */
		MyOperator(&ua_inputs, &ua_outputs);

        /* Send all output signals to DF server */
        num_sent_bytes = ua_send(buffer, &ua_outputs, NULL);
        send(DF_server_socket, buffer, num_sent_bytes, MSG_WAIT);

        /* Send messages to each peer */
        sendMessagesToPeers();

        usleep(100000);

        /* Receive all input signals from DF server */
        num_received_bytes = recv(DF_server_socket, buffer, MSG_SIZE, 0);
        if (num_received_bytes > 0)
            ua_receive(buffer, num_received_bytes, &ua_inputs, NULL);
    }

    close(DF_server_socket);
    close(to_outside_socket);
    return 0;
}
