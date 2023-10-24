#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct motor_command
{
    uint8_t left_speed;
    uint8_t right_speed;
};

struct motor_training_value
{
    uint8_t left;
    uint8_t right;
    uint8_t left_speed;
    uint8_t right_speed;
};

struct hidden_node
{
    float in1;
    float in2;
    float w1;
    float w2;
    float b;
};

struct output_node
{
    float in1;
    float in2;
    float in3;
    float w1;
    float w2;
    float w3;
    float b;
};

struct motor_command
compute_proportional(uint8_t left, uint8_t right);
struct motor_command
compute_neural_network(uint8_t left, uint8_t right);
void initialize_neural_network();
void motor(uint8_t num, int8_t speed);
int calculateErr(u08 actual);

struct hidden_node h1;
struct hidden_node h2 = {0, 0, 0, 0, 0};
struct hidden_node h3 = {0, 0, 0, 0, 0};

struct output_node o1 = {0, 0, 0, 0, 0, 0, 0};
struct output_node o2 = {0, 0, 0, 0, 0, 0, 0};

int main(void)
{
    init();

    struct motor_command curr_motor_command;
    struct motor_training_value curr_training_value;

    // This may need casting, not sure if it will work correctly

    uint8_t leftSensor;
    uint8_t rightSensor;
    uint8_t training_iteration_count = 0;
    uint16_t data_count = 0;

    struct motor_training_value *training_data = malloc(sizeof(struct motor_training_value) * 1000);

    initialize_neural_network();

    // set analog ports to input
    digital_dir(0, 0);
    digital_dir(1, 0);

    // set motors to 0 speed
    motor(0, 0);
    motor(1, 0);

    // immediately begin running the proportional controller
    clear_screen();
    print_string("Proportional");

    // Drive in proportional controller mode until button is pressed
    while (!get_btn())
    {
        leftSensor = analog(0);
        rightSensor = analog(1);
        curr_motor_command = compute_proportional(leftSensor, rightSensor);
        motor(0, curr_motor_command.left_speed);
        motor(1, curr_motor_command.right_speed);
    }

    while (get_btn())
        ;

    // this will need to be in while loop
    clear_screen();
    print_string("Data");

    motor(0, 0);
    motor(1, 0);

    _delay_ms(3000);

    // Set cursor to second line and print sensor readings before printing training

    while (!get_btn())
    {
        clear_screen();
        print_string("Data ");
        print_num(data_count);

                _delay_ms(1000);


        leftSensor = analog(0);
        rightSensor = analog(1);

        lcd_cursor(0, 1);
        print_num(leftSensor);
        print_string(" ");
        print_num(rightSensor);

        curr_motor_command = compute_proportional(leftSensor, rightSensor);

        curr_training_value.left = leftSensor;
        curr_training_value.right = rightSensor;
        curr_training_value.left_speed = curr_motor_command.left_speed;
        curr_training_value.left_speed = curr_motor_command.right_speed;

        training_data[data_count] = curr_training_value;

        data_count++;

        _delay_ms(1000);
    }

    while (get_btn())
        ;

    // print_string(" ");
    // print_num(training_data[198].right);
    // print_string(" ");
    // lcd_cursor(0, 1);
    // print_num(training_data[198].left_speed);
    // print_string(" ");
    // print_num(training_data[198].right_speed);
    // print_string(" ");

    // During this mode, you will use the captured training data along with the proportional controller to train the neural network
    clear_screen();
    print_string("Training");

    _delay_ms(1000);

    uint8_t yMove = 0;
    while (!get_btn())
    {
        clear_screen();
        print_string("Iterations:");
        lcd_cursor(0, 1);
        print_num(training_iteration_count);

        yMove = get_accel_y();
        if ((yMove < 240 && yMove > 128))
            training_iteration_count < 255 ? training_iteration_count + 1 : 0;
        else if ((yMove > 15 && yMove < 128))
            training_iteration_count > 0 ? training_iteration_count - 1 : 0;

        _delay_ms(50);
    }

    while (!get_btn())
    {
        // for data in dataSet
        int i;
        for (i = 0; i < data_count; i++)
        {
            clear_screen();
            float r = training_data[i].left_speed;
            print_num(r); // DEBUG THIS, it freaks out the second I try to use values from the training data
            _delay_ms(200);
        }
        // add motor_values to some array of values to later be used for training
    }

    // run neural network
    clear_screen();
    print_string("AI MODE");

    while (!get_btn())
    {
        leftSensor = analog(0);
        rightSensor = analog(1);
        curr_motor_command = compute_neural_network(leftSensor, rightSensor);
        motor(0, curr_motor_command.left_speed);
        motor(1, curr_motor_command.right_speed);
    }

    free(training_data);
    return 0;
}

void initialize_neural_network()
{
    srand(time(NULL));

    // print_num(rand() % 100);
    h1.w1 = (float)rand() / (float)RAND_MAX;
    h1.w2 = (float)rand() / (float)RAND_MAX;
    h1.b = (float)rand() / (float)RAND_MAX;

    h2.w1 = (float)rand() / (float)RAND_MAX;
    h2.w2 = (float)rand() / (float)RAND_MAX;
    h2.b = (float)rand() / (float)RAND_MAX;

    h3.w1 = (float)rand() / (float)RAND_MAX;
    h3.w2 = (float)rand() / (float)RAND_MAX;
    h3.b = (float)rand() / (float)RAND_MAX;

    o1.w1 = (float)rand() / (float)RAND_MAX;
    o1.w2 = (float)rand() / (float)RAND_MAX;
    o1.w3 = (float)rand() / (float)RAND_MAX;
    o1.b = (float)rand() / (float)RAND_MAX;

    o2.w1 = (float)rand() / (float)RAND_MAX;
    o2.w2 = (float)rand() / (float)RAND_MAX;
    o2.w3 = (float)rand() / (float)RAND_MAX;
    o2.b = (float)rand() / (float)RAND_MAX;
}

struct motor_command compute_proportional(uint8_t left, uint8_t right)
{
    struct motor_command result;

    u08 maxSpeed = 20;
    u08 rightWheelBuffer = 2;

    if (left < 100 && right < 100)
    {
        result.left_speed = maxSpeed;
        result.right_speed = (maxSpeed + rightWheelBuffer) * -1;
    }
    else
    {
        result.left_speed = calculateErr(right) * 0.4;
        result.right_speed = ((calculateErr(left) * 0.4) + rightWheelBuffer) * -1;
    }

    return result;
}

struct motor_command compute_neural_network(uint8_t left, uint8_t right)
{
    struct motor_command result;

    h1.in1 = left;
    h1.in2 = right;

    h2.in1 = left;
    h2.in2 = right;

    h3.in1 = left;
    h3.in2 = right;

    o1.in1 = (h1.in1 * h1.w1) + (h1.in2 * h1.w2) + h1.b;
    o1.in2 = (h2.in1 * h2.w1) + (h2.in2 * h2.w2) + h2.b;
    o1.in3 = (h3.in1 * h3.w1) + (h3.in2 * h3.w2) + h3.b;

    o2.in1 = (h1.in1 * h1.w1) + (h1.in2 * h1.w2) + h1.b;
    o2.in2 = (h2.in1 * h2.w1) + (h2.in2 * h2.w2) + h2.b;
    o2.in3 = (h3.in1 * h3.w1) + (h3.in2 * h3.w2) + h3.b;

    result.left_speed = (o1.in1 * o1.w1) + (o1.in2 * o1.w2) + (o1.in3 * o1.w3) + o1.b;
    result.left_speed = (o1.in1 * o1.w1) + (o1.in2 * o1.w2) + (o1.in3 * o1.w3) + o1.b;
    result.left_speed = (o1.in1 * o1.w1) + (o1.in2 * o1.w2) + (o1.in3 * o1.w3) + o1.b;

    result.right_speed = (o2.in1 * o2.w1) + (o2.in2 * o2.w2) + (o2.in3 * o2.w3) + o2.b;
    result.right_speed = (o2.in1 * o2.w1) + (o2.in2 * o2.w2) + (o2.in3 * o2.w3) + o2.b;
    result.right_speed = (o2.in1 * o2.w1) + (o2.in2 * o2.w2) + (o2.in3 * o2.w3) + o2.b;

    return result;
}

void train_neural_network(struct motor_training_value currVal)
{
    // this will happen monday
    print_num(23);
    return;
    // should complete 1 "epoch": compute all new weights and biases then update
}

// helper methods for neural network computations ---------------------

// void compute_output_layer_slope(float curr_node_out, float curr_node_target, float inp_node_out)
// {
//     return (curr_node_out - curr_node_target) * (curr_node_out * (1 - curr_node_out)) * inp_node_out;
// }

// float compute_new_output_layer_weight(float old_w)
// {
//     return old_w - (.025 * compute_output_layer_slope());
// }

// void compute_hidden_layer_slope()
// {

// }

// float compute_new_hidden_layer_slope(struct hidden_node)
// {
//     return old_w - (.025 * compute_hidden_layer_slope());
// }

// helper methods for proportional controller -------------------------

void motor(uint8_t num, int8_t speed)
{
    set_servo(num, 127 + (speed * 0.3));
}

int calculateErr(u08 actual)
{
    u08 error = abs(40 - actual);
    if (error > 100)
        return 100;
    if (error < 0)
        return 20;
    return error;
}