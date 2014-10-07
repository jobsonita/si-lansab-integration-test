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
    SATTELITE_INPUT_INTERFACE input;

    if (message.to == SATTELITE) {
        input = message.input_interface.sattelite_input_interface;
        switch (message.from) {
        case LAUNCHER:
            printf("Received: Message from LAUNCHER to SATTELITE \n");
            ua_inputs.SignalFromLauncher = input.SignalFromLauncher;
            break;
        }
    }
}

void sendMessage(SILANSAB_MESSAGE *message) {
    message->from = SATTELITE;
    switch (message->to) {
    case LAUNCHER:
        printf("Sent: Message from SATTELITE to LAUNCHER \n");
        LAUNCHER_INPUT_INTERFACE *output = &(message->input_interface.launcher_input_interface);
        output->SignalFromSattelite = ua_outputs.SignalToLauncher;
    }
}

void clear_ua_inputs() {
    ua_receive_clear(&ua_inputs, NULL);

    ua_inputs.SignalFromLauncher = FALSE;
}

void clear_ua_outputs() {
    MyOperator_reset(&ua_outputs);
}
