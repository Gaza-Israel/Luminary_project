#include "Controller.h"

void calibrate_all(){
    
    //get addresses received from others in the buffer
    while (/* condition */)
    {
        if (Serial.available()) {
        char line[128];
        size_t lineLength = Serial.readBytesUntil('\n', line, 127);
        line[lineLength] = '\0';

        char response[MyCommandParser::MAX_RESPONSE_SIZE];
        myparser._parser.processCommand(line, response);
        Serial.println(response);
        }

        if (I2C::buffer_not_empty()){
        I2C_message_protocol::parse_message(I2C::pop_message_from_buffer(),&L1);
        }
    }
    


    //first one is going to calibrate; others do cross calibration


}