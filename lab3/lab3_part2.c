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
    float left_speed;
    float right_speed;
};

struct motor_training_value
{
    float left;
    float right;
    float left_speed;
    float right_speed;
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

struct motor_command compute_proportional(uint8_t left, uint8_t right);
struct motor_command compute_neural_network(float left, float right);
void train_neural_network(struct motor_training_value currVal);
void initialize_neural_network();
void motor(uint8_t num, int8_t speed);
int calculateErr(u08 actual);

// helper methods for neural network computations ---------------------

float compute_new_weight(float old_weight, float slope)
{
    return old_weight - (0.025f * slope);
}

float compute_outer_layer_slope(float out_h, float out_o, float target)
{
    return (out_o - target) * (out_o * (1.f - out_o)) * out_h;
}

float compute_hidden_layer_slope(struct motor_command outputs, struct motor_training_value currVal,
                                 float weight_o1, float weight_o2, float out_h, float inp)
{
    return (((outputs.left_speed - currVal.left_speed) * (outputs.left_speed * (1 - outputs.left_speed)) * weight_o1) + ((outputs.right_speed - currVal.right_speed) * (outputs.right_speed * (1 - outputs.right_speed)) * weight_o2)) * (out_h * (1 - out_h)) * inp;
}

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

struct hidden_node h1 = {0, 0, 0, 0, 0};
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

    struct motor_training_value *training_data = malloc(sizeof(struct motor_training_value) * 200);

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
        _delay_ms(200);
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

        leftSensor = analog(0);
        rightSensor = analog(1);

        lcd_cursor(0, 1);
        print_num(leftSensor);
        print_string(" ");
        print_num(rightSensor);

        curr_motor_command = compute_proportional(leftSensor, rightSensor);

        // convert everything into 0-1 range before training
        curr_training_value.left = ((float)leftSensor) / (float) 255;
        curr_training_value.right = ((float)rightSensor) / (float) 255;
        curr_training_value.left_speed = ((float) curr_motor_command.left_speed) / 100.f;
        curr_training_value.right_speed = ((float)curr_motor_command.right_speed * -1.f) / 100.f;

        training_data[data_count] = curr_training_value;

        data_count++;

        _delay_ms(200);
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
            training_iteration_count = training_iteration_count < 255 ? training_iteration_count + 1 : 0;
        else if ((yMove > 15 && yMove < 128))
            training_iteration_count = training_iteration_count > 0 ? training_iteration_count - 1 : 0;

        _delay_ms(50);
    }

    uint8_t curr_training_count = 0;
    while (curr_training_count < training_iteration_count)
    {
        // Cycle back to start of dataset if there are more training iterations than data
        if (curr_training_count >= data_count)
        {
            curr_training_count = 0;
            training_iteration_count -= data_count - 1;
        }

        int i = 0; 
        for (i = 0; i < 10; i++)
            train_neural_network(training_data[curr_training_count]);
        curr_training_count++;
    }

    clear_screen();
    print_string("Training");
    lcd_cursor(0, 1);
    print_string("Complete");

    _delay_ms(2000);

    // run neural network
    clear_screen();
    print_string("AI MODE");

    while (!get_btn())
    {
        leftSensor = analog(0);
        rightSensor = analog(1);
        curr_motor_command = compute_neural_network(((float)leftSensor) / 255.f, ((float)rightSensor) / 255.f); // needs 0-1 range for input
        motor(0, curr_motor_command.left_speed * 100);
        motor(1, curr_motor_command.right_speed * -100);
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
        result.right_speed = -1 * (maxSpeed + rightWheelBuffer);
        // motor(0, maxSpeed);
        // motor(1, -1 * (maxSpeed + rightWheelBuffer));
    }
    else
    {
        result.left_speed = calculateErr(right) * 0.4;
        result.right_speed = ((calculateErr(left) * 0.4) + rightWheelBuffer) * -1;
        // motor(0, calculateErr(right) * 0.4);
        // motor(1, ((calculateErr(left) * 0.4) + rightWheelBuffer) * -1);
    }

    return result;
}

float activation(float val)
{
    return 1.f / (1.f + exp(-1.f * val));
}

struct motor_command compute_neural_network(float left, float right)
{
    struct motor_command result;

    h1.in1 = left;
    h1.in2 = right;

    h2.in1 = left;
    h2.in2 = right;

    h3.in1 = left;
    h3.in2 = right;

    o1.in1 = activation((h1.in1 * h1.w1) + (h1.in2 * h1.w2) + h1.b);
    o1.in2 = activation((h2.in1 * h2.w1) + (h2.in2 * h2.w2) + h2.b);
    o1.in3 = activation((h3.in1 * h3.w1) + (h3.in2 * h3.w2) + h3.b);

    o2.in1 = activation((h1.in1 * h1.w1) + (h1.in2 * h1.w2) + h1.b);
    o2.in2 = activation((h2.in1 * h2.w1) + (h2.in2 * h2.w2) + h2.b);
    o2.in3 = activation((h3.in1 * h3.w1) + (h3.in2 * h3.w2) + h3.b);

    result.left_speed = activation((o1.in1 * o1.w1) + (o1.in2 * o1.w2) + (o1.in3 * o1.w3) + o1.b);

    result.right_speed = activation((o2.in1 * o2.w1) + (o2.in2 * o2.w2) + (o2.in3 * o2.w3) + o2.b);

    return result;
}

/**
    float compute_new_weight(float old_weight, float slope)
    {
        return old_weight - (.025 * slope);
    }

    float compute_outer_layer_slope(float out_h, float out_o, float target)
    {
        return (out_o - target) * (out_o * (1 - out_o)) * out_h;
    }

    float compute_hidden_layer_slope(struct motor_command outputs, struct motor_training_value currVal,
                                    float weight_o1, float weight_o2, float out_h, float inp)
    {
        return (((outputs.left_speed - currVal.left_speed) * (outputs.left_speed * (1 - outputs.left_speed)) * weight_o1)
         + ((outputs.right_speed - currVal.right_speed) * (outputs.right_speed * (1 - outputs.right_speed)) * weight_o2))
          * (out_h * (1 - out_h)) * inp;
    }

*/
void train_neural_network(struct motor_training_value currVal)
{
    struct hidden_node temp_h1 = {0, 0, 0, 0, 0};
    struct hidden_node temp_h2 = {0, 0, 0, 0, 0};
    struct hidden_node temp_h3 = {0, 0, 0, 0, 0};

    struct output_node temp_o1 = {0, 0, 0, 0, 0, 0, 0};
    struct output_node temp_o2 = {0, 0, 0, 0, 0, 0, 0};

    struct motor_command outputs = compute_neural_network(currVal.left, currVal.right);

    clear_screen();
    print_num(o2.w1 * 100.f);
    // lcd_cursor(0, 1);
    // print_num(currVal.right_speed * 100.f);


    // compute new outer weight and biases (don't update yet)
    temp_o1.w1 = compute_new_weight(o1.w1, compute_outer_layer_slope(o1.in1, outputs.left_speed, currVal.left_speed));
    temp_o1.w2 = compute_new_weight(o1.w2, compute_outer_layer_slope(o1.in2, outputs.left_speed, currVal.left_speed));
    temp_o1.w3 = compute_new_weight(o1.w3, compute_outer_layer_slope(o1.in3, outputs.left_speed, currVal.left_speed));
    temp_o1.b = compute_new_weight(o1.b, compute_outer_layer_slope(-1, outputs.left_speed, currVal.left_speed));

    temp_o2.w1 = compute_new_weight(o2.w1, compute_outer_layer_slope(o2.in1, outputs.right_speed, currVal.right_speed));
    temp_o2.w2 = compute_new_weight(o2.w2, compute_outer_layer_slope(o2.in2, outputs.right_speed, currVal.right_speed));
    temp_o2.w3 = compute_new_weight(o2.w3, compute_outer_layer_slope(o2.in3, outputs.right_speed, currVal.right_speed));
    temp_o2.b = compute_new_weight(o2.b, compute_outer_layer_slope(-1, outputs.right_speed, currVal.right_speed));

    // compute new hidden weight and biases (don't update yet)
    temp_h1.w1 = compute_new_weight(h1.w1, compute_hidden_layer_slope(outputs, currVal, o1.w1, o2.w1, o1.in1, currVal.left));
    temp_h1.w2 = compute_new_weight(h1.w2, compute_hidden_layer_slope(outputs, currVal, o1.w1, o2.w1, o1.in1, currVal.right));
    temp_h1.b = compute_new_weight(h1.b, compute_hidden_layer_slope(outputs, currVal, o1.w1, o2.w1, o1.in1, -1));

    temp_h2.w1 = compute_new_weight(h2.w1, compute_hidden_layer_slope(outputs, currVal, o1.w2, o2.w2, o1.in2, currVal.left));
    temp_h2.w2 = compute_new_weight(h2.w2, compute_hidden_layer_slope(outputs, currVal, o1.w2, o2.w2, o1.in2, currVal.right));
    temp_h2.b = compute_new_weight(h2.b, compute_hidden_layer_slope(outputs, currVal, o1.w2, o2.w2, o1.in2, -1));

    temp_h3.w1 = compute_new_weight(h3.w1, compute_hidden_layer_slope(outputs, currVal, o1.w3, o2.w3, o1.in3, currVal.left));
    temp_h3.w2 = compute_new_weight(h3.w2, compute_hidden_layer_slope(outputs, currVal, o1.w3, o2.w3, o1.in3, currVal.right));
    temp_h3.b = compute_new_weight(h3.b, compute_hidden_layer_slope(outputs, currVal, o1.w3, o2.w3, o1.in3, -1));

    // update all weights and biases
    o1.w1 = temp_o1.w1;
    o1.w2 = temp_o1.w2;
    o1.w3 = temp_o1.w3;
    o1.b = temp_o1.b;

    o2.w1 = temp_o2.w1;
    o2.w2 = temp_o2.w2;
    o2.w3 = temp_o2.w3;
    o2.b = temp_o2.b;

    h1.w1 = temp_h1.w1;
    h1.w2 = temp_h1.w2;
    h1.b = temp_h1.b;

    h2.w1 = temp_h2.w1;
    h2.w2 = temp_h2.w2;
    h2.b = temp_h2.b;

    h3.w1 = temp_h3.w1;
    h3.w2 = temp_h3.w2;
    h3.b = temp_h3.b;

    _delay_ms(20);

    return;
}
