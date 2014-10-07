#include <stdio.h>
#include <stdlib.h>

#include "CommunicationInterfaces.h"
#include "OperatorInformation.h"

#ifndef TRUE
#define TRUE kcg_true
#define FALSE kcg_false
#endif

operator_input_type ua_inputs;
operator_output_type ua_outputs;

void receiveMessage(SILANSAB_MESSAGE message) {
    LAUNCHER_INPUT_INTERFACE input;

    if (message.to == LAUNCHER) {
        input = message.input_interface.launcher_input_interface;
        switch (message.from) {
        case SATTELITE:
            printf("Received: Message from SATTELITE to LAUNCHER \n");
            ua_inputs.SignalFromSattelite = input.SignalFromSattelite;
            break;
        }
    }
}

void sendMessage(SILANSAB_MESSAGE *message) {
    message->from = LAUNCHER;
    switch (message->to) {
    case SATTELITE:
        printf("Sent: Message from LAUNCHER to SATTELITE \n");
        SATTELITE_INPUT_INTERFACE *output = &(message->input_interface.sattelite_input_interface);
        output->SignalFromLauncher = ua_outputs.SignalToSattelite;
    }
}

void clear_ua_inputs() {
    ua_receive_clear(&ua_inputs, NULL);

    ua_inputs.SignalFromSattelite = FALSE;
}

void clear_ua_outputs() {
    MyOperator_reset(&ua_outputs);
}
