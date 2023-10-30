/**
    Name:  Riley Badnin and Justin Brunings
    Lab 3 part 2
    Description: Implementation of a line-following program for your robot with addition of neural network
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
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

struct motor_command compute_proportional(int left, int right);
struct motor_command compute_neural_network(float left, float right);
void train_neural_network(struct motor_training_value currVal);
void initialize_neural_network();
void motor(int num, int speed);
int calculateErr(int actual);

// helper methods for proportional controller -------------------------

void motor(int num, int speed)
{
    // printf("Speed running: %d\n", speed);
    set_servo(num, 127 + (speed * 0.3));
}

int calculateErr(int actual)
{
    int error = abs(40 - actual);
    if (error > 100)
        return 100;
    if (error < 0)
        return 20;
    return error;
}

// Define neural network structure
#define INPUT_NODES 2
#define HIDDEN_NODES 3
#define OUTPUT_NODES 2
#define LEARNING_RATE 0.1

typedef struct
{
    float weights[INPUT_NODES][HIDDEN_NODES];
    float bias_hidden[HIDDEN_NODES];
    float hidden_outputs[HIDDEN_NODES];
    float hidden_errors[HIDDEN_NODES];
    float weights_hidden_output[HIDDEN_NODES][OUTPUT_NODES];
    float bias_output[OUTPUT_NODES];
    float output[OUTPUT_NODES];
    float output_errors[OUTPUT_NODES];
} NeuralNetwork;

float sigmoid(float x)
{
    return 1.0 / (1.0 + exp(-x));
}

float sigmoid_derivative(float x)
{
    return x * (1.0 - x);
}

// Fill in weights and biases with random values
void initialize_network(NeuralNetwork *network)
{
    srand(time(NULL));
    for (int i = 0; i < INPUT_NODES; ++i)
    {
        for (int j = 0; j < HIDDEN_NODES; ++j)
        {
            network->weights[i][j] = ((float)rand() / RAND_MAX); // Scale to 0-1
        }
    }

    for (int i = 0; i < HIDDEN_NODES; ++i)
    {
        network->bias_hidden[i] = ((float)rand() / RAND_MAX);
        for (int j = 0; j < OUTPUT_NODES; ++j)
        {
            network->weights_hidden_output[i][j] = ((float)rand() / RAND_MAX);
        }
    }

    for (int i = 0; i < OUTPUT_NODES; ++i)
    {
        network->bias_output[i] = ((float)rand() / RAND_MAX);
    }
}

// Used to progress through the neural network in a forward manner -> produce outputs
void forward_propagation(NeuralNetwork *network, float input[INPUT_NODES])
{
    for (int i = 0; i < HIDDEN_NODES; ++i)
    {
        float sum = 0;
        for (int j = 0; j < INPUT_NODES; ++j)
        {
            sum += input[j] * network->weights[j][i];
        }
        network->hidden_outputs[i] = sigmoid(sum + network->bias_hidden[i]);
    }

    for (int i = 0; i < OUTPUT_NODES; ++i)
    {
        float sum = 0;
        for (int j = 0; j < HIDDEN_NODES; ++j)
        {
            sum += network->hidden_outputs[j] * network->weights_hidden_output[j][i];
        }
        network->output[i] = sigmoid(sum + network->bias_output[i]);
    }
}

// Used to update weights
void backpropagation(NeuralNetwork *network, float input[INPUT_NODES], float target[OUTPUT_NODES])
{
    // updates output weights
    for (int i = 0; i < OUTPUT_NODES; ++i)
    {
        network->output_errors[i] = target[i] - network->output[i];
        float delta_output = network->output_errors[i] * sigmoid_derivative(network->output[i]);
        for (int j = 0; j < HIDDEN_NODES; ++j)
        {
            network->weights_hidden_output[j][i] += LEARNING_RATE * delta_output * network->hidden_outputs[j];
        }
        network->bias_output[i] += LEARNING_RATE * delta_output;
    }

    // updates hidden weights
    for (int i = 0; i < HIDDEN_NODES; ++i)
    {
        float error = 0;
        for (int j = 0; j < OUTPUT_NODES; ++j)
        {
            error += network->output_errors[j] * network->weights_hidden_output[i][j];
        }
        network->hidden_errors[i] = error * sigmoid_derivative(network->hidden_outputs[i]);
        for (int j = 0; j < INPUT_NODES; ++j)
        {
            network->weights[j][i] += LEARNING_RATE * network->hidden_errors[i] * input[j];
        }
        network->bias_hidden[i] += LEARNING_RATE * network->hidden_errors[i];
    }
}

int main(void)
{
    init();
    struct motor_command curr_motor_command;
    NeuralNetwork network;
    initialize_network(&network);

    int leftSensor;
    int rightSensor;
    int training_iteration_count = 0;
    int data_count = 0;
    char answer[2];
    char answer2[2];

    struct motor_training_value *training_data = malloc(sizeof(struct motor_training_value) * 200);

    // set motors to 0 speed
    motor(0, 0);
    motor(1, 0);

    // immediately begin running the proportional controller
    clear_screen();
    print_string("Proportional");

    // Drive in proportional controller mode until button is pressed
    while (!get_btn())
    {
        clear_screen();
        leftSensor = analog(0);
        rightSensor = analog(1);
        curr_motor_command = compute_proportional(leftSensor, rightSensor);
        motor(0, curr_motor_command.left_speed);
        motor(1, curr_motor_command.right_speed);
        print_num(curr_motor_command.left_speed);
        print_string(" ");
        print_num(curr_motor_command.right_speed * -1);
        _delay_ms(20);
    }

    while (get_btn())
        ;

    clear_screen();
    print_string("Data");

    motor(0, 0);
    motor(1, 0);

    _delay_ms(3000);

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

        struct motor_training_value curr_training_value;
        curr_motor_command = compute_proportional(leftSensor, rightSensor);

        // convert everything into 0-1 range before training
        curr_training_value.left = ((float)leftSensor) / (float)255;
        curr_training_value.right = ((float)rightSensor) / (float)255;
        curr_training_value.left_speed = (float)curr_motor_command.left_speed / 100.f;
        curr_training_value.right_speed = ((float)curr_motor_command.right_speed * -1.f) / 100.f;

        training_data[data_count] = curr_training_value;

        data_count++;

        _delay_ms(120);
    }

    while (get_btn())
        ;

    while (1)
    {
        motor(0, 0);
        motor(1, 0);
        training_iteration_count = 0;
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

        training_iteration_count = training_iteration_count * 100;

        int curr_training_count = 0;
        while (curr_training_count < training_iteration_count)
        {
            // Cycle back to start of dataset if there are more training iterations than data
            if (curr_training_count >= data_count) // UPDATE: Added an =
            {
                curr_training_count = 0;
                training_iteration_count -= data_count - 1;
            }

            float inputs[] = {training_data[curr_training_count].left, training_data[curr_training_count].right};
            float outputs[] = {training_data[curr_training_count].left_speed, training_data[curr_training_count].right_speed};

            forward_propagation(&network, inputs);
            backpropagation(&network, inputs, outputs);

            curr_training_count++;
        }

        clear_screen();
        print_string("Training");
        lcd_cursor(0, 1);
        print_string("Complete");

        _delay_ms(2000);

        clear_screen();
        print_string("AI MODE");

        while (!get_btn())
        {
            clear_screen();

            lcd_cursor(0, 1);
            leftSensor = analog(0);
            rightSensor = analog(1);

            float inputs[] = {leftSensor / 255.f, rightSensor / 255.f};
            forward_propagation(&network, inputs);
            motor(0, network.output[0] * 100);
            motor(1, network.output[1] * -100);

            print_num(network.output[0] * 100);
            print_string(" ");
            print_num(network.output[1] * 100);

            _delay_ms(20);
        }
    }

    free(training_data);
    return 0;
}

struct motor_command compute_proportional(int left, int right)
{
    struct motor_command result;

    int maxSpeed = 20;
    int rightWheelBuffer = 2;

    if (left < 100 && right < 100)
    {
        result.left_speed = maxSpeed;
        result.right_speed = -1 * (maxSpeed + rightWheelBuffer);
    }
    else
    {
        result.left_speed = calculateErr(right) * 0.4;
        result.right_speed = ((calculateErr(left) * 0.4) + rightWheelBuffer) * -1;
    }

    return result;
}
