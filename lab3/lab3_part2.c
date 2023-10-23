#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

struct motor_command
{
    uint8_t left_speed;
    uint8_t right_speed;
}

struct motor_command
compute_proportional(uint8_t left, uint8_t right);

int main(void)
{
    struct motor_command motor_values;
    uint8_t leftSensor;
    uint8_t rightSensor;

    // immediately begin running the proportional controller
    clear_display();
    print_string("Proportional");

    // Drive in proportional controller mode until button is pressed
    while (!get_btn())
    {
        leftSensor = analog(0);
        rightSensor = analog(1);
        motor_values = compute_proportional(leftSensor, rightSensor);
        motor(0, motor_values.left_speed);
        motor(1, motor_values.right_speed);
    }

    // this will need to be in while loop
    clear_display();
    print_string("Data");

    // Set cursor to second line and print sensor readings before printing training

    while (!get_btn())
    {
        leftSensor = analog(0);
        rightSensor = analog(1);
        motor_values = compute_proportional(leftSensor, rightSensor);

        // add motor_values to some array of values to later be used for training
    }

    // During this mode, you will use the captured training data along with the proportional controller to train the neural network
    clear_display();
    print_string("Training");

    while (!get_btn())
    {
        // for data in dataSet

        // add motor_values to some array of values to later be used for training
    }

    // run neural network
    clear_display();
    print_string("AI MODE");

    while (!get_btn())
    {
        leftSensor = analog(0);
        rightSensor = analog(1);
        motor_values = compute_neural_network(leftSensor, rightSensor);
        motor(0, motor_values.left_speed);
        motor(1, motor_values.right_speed);
    }

    return 0;
}

motor_command compute_proportional(uint8_t left, uint8_t right)
{
    struct motor_command result;

    return result;
}

motor_command compute_neural_network(uint8_t left, uint8_t right)
{
    struct motor_command result;

    return result;
}

void train_neural_network(){
    // this will happen monday
}
