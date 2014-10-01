#ifndef COMMUNICATIONINTERFACES_H
#define COMMUNICATIONINTERFACES_H

#define LAUNCHER 1
#define SATTELITE 2

typedef struct {
    int SignalFromSattelite;
} LAUNCHER_INPUT_INTERFACE;

typedef struct {
    int SignalFromLauncher;
} SATTELITE_INPUT_INTERFACE;

typedef union {
	LAUNCHER_INPUT_INTERFACE launcher_input_interface;
	SATTELITE_INPUT_INTERFACE sattelite_input_interface;
} INPUT_INTERFACE;

typedef struct {
	int to, from;
    INPUT_INTERFACE input_interface;
} SILANSAB_MESSAGE;

typedef struct {
	mqd_t from_launcher, to_launcher;
	mqd_t from_sattelite, to_sattelite;
} SILANSAB_MESSAGE_QUEUES;

#define MSG_SIZE 2048

#endif
