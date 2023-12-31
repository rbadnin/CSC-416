#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

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

// struct hidden_node
// {
//     float in1;
//     float in2;
//     float w1;
//     float w2;
//     float b;
// };

// struct output_node
// {
//     float in1;
//     float in2;
//     float in3;
//     float w1;
//     float w2;
//     float w3;
//     float b;
// };

struct motor_command compute_proportional(int left, int right);
struct motor_command compute_neural_network(float left, float right);
void train_neural_network(struct motor_training_value currVal);
void initialize_neural_network();
void motor(int num, int speed);
int calculateErr(int actual);

// helper methods for neural network computations ---------------------

// float compute_new_weight(float old_weight, float slope)
// {
//     return old_weight - (0.025f * slope);
// }

// float compute_outer_layer_slope(float out_h, float out_o, float target)
// {
//     return (out_o - target) * (out_o * (1.f - out_o)) * out_h;
// }

// float compute_hidden_layer_slope(struct motor_command outputs, struct motor_training_value currVal,
//                                  float weight_o1, float weight_o2, float out_h, float inp)
// {
//     return (((outputs.left_speed - currVal.left_speed) * (outputs.left_speed * (1 - outputs.left_speed)) * weight_o1) + ((outputs.right_speed - currVal.right_speed) * (outputs.right_speed * (1 - outputs.right_speed)) * weight_o2)) * (out_h * (1 - out_h)) * inp;
// }

// helper methods for proportional controller -------------------------

void motor(int num, int speed)
{
    printf("Speed running: %d\n", speed);
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

// struct hidden_node h1 = {0, 0, 0, 0, 0};
// struct hidden_node h2 = {0, 0, 0, 0, 0};
// struct hidden_node h3 = {0, 0, 0, 0, 0};

// struct output_node o1 = {0, 0, 0, 0, 0, 0, 0};
// struct output_node o2 = {0, 0, 0, 0, 0, 0, 0};

// Define neural network structure
#define INPUT_NODES 2
#define HIDDEN_NODES 3
#define OUTPUT_NODES 2
#define LEARNING_RATE 0.1

typedef struct {
    float weights[INPUT_NODES][HIDDEN_NODES];
    float bias_hidden[HIDDEN_NODES];
    float hidden_outputs[HIDDEN_NODES];
    float hidden_errors[HIDDEN_NODES];
    float weights_hidden_output[HIDDEN_NODES][OUTPUT_NODES];
    float bias_output[OUTPUT_NODES];
    float output[OUTPUT_NODES];
    float output_errors[OUTPUT_NODES];
} NeuralNetwork;

float sigmoid(float x) {
    return 1.0 / (1.0 + exp(-x));
}

float sigmoid_derivative(float x) {
    return x * (1.0 - x);
}

void initialize_network(NeuralNetwork *network) {
    for (int i = 0; i < INPUT_NODES; ++i) {
        for (int j = 0; j < HIDDEN_NODES; ++j) {
            network->weights[i][j] = ((float)rand() / RAND_MAX); // Scale to 0-1
        }
    }

    for (int i = 0; i < HIDDEN_NODES; ++i) {
        network->bias_hidden[i] = ((float)rand() / RAND_MAX);
        for (int j = 0; j < OUTPUT_NODES; ++j) {
            network->weights_hidden_output[i][j] = ((float)rand() / RAND_MAX);
        }
    }

    for (int i = 0; i < OUTPUT_NODES; ++i) {
        network->bias_output[i] = ((float)rand() / RAND_MAX);
    }
}

void forward_propagation(NeuralNetwork *network, float input[INPUT_NODES]) {
    for (int i = 0; i < HIDDEN_NODES; ++i) {
        float sum = 0;
        for (int j = 0; j < INPUT_NODES; ++j) {
            sum += input[j] * network->weights[j][i];
        }
        network->hidden_outputs[i] = sigmoid(sum + network->bias_hidden[i]);
    }

    for (int i = 0; i < OUTPUT_NODES; ++i) {
        float sum = 0;
        for (int j = 0; j < HIDDEN_NODES; ++j) {
            sum += network->hidden_outputs[j] * network->weights_hidden_output[j][i];
        }
        network->output[i] = sigmoid(sum + network->bias_output[i]);
    }
}

void backpropagation(NeuralNetwork *network, float input[INPUT_NODES], float target[OUTPUT_NODES]) {
    for (int i = 0; i < OUTPUT_NODES; ++i) {
        network->output_errors[i] = target[i] - network->output[i];
        float delta_output = network->output_errors[i] * sigmoid_derivative(network->output[i]);
        for (int j = 0; j < HIDDEN_NODES; ++j) {
            network->weights_hidden_output[j][i] += LEARNING_RATE * delta_output * network->hidden_outputs[j];
        }
        network->bias_output[i] += LEARNING_RATE * delta_output;
    }

    for (int i = 0; i < HIDDEN_NODES; ++i) {
        float error = 0;
        for (int j = 0; j < OUTPUT_NODES; ++j) {
            error += network->output_errors[j] * network->weights_hidden_output[i][j];
        }
        network->hidden_errors[i] = error * sigmoid_derivative(network->hidden_outputs[i]);
        for (int j = 0; j < INPUT_NODES; ++j) {
            network->weights[j][i] += LEARNING_RATE * network->hidden_errors[i] * input[j];
        }
        network->bias_hidden[i] += LEARNING_RATE * network->hidden_errors[i];
    }
}

int main(void)
{
    struct motor_command curr_motor_command;
    // struct motor_training_value curr_training_value;
    NeuralNetwork network;
    initialize_network(&network);

    // This may need casting, not sure if it will work correctly

    int leftSensor;
    int rightSensor;
    int training_iteration_count = 0;
    int data_count = 0;
    char answer[2];
    char answer2[2];

    struct motor_training_value *training_data = malloc(sizeof(struct motor_training_value) * 200);

    //initialize_neural_network();

    // immediately begin running the proportional controller
    printf("\nPROPORTIONAL MODE\n");

    // Drive in proportional controller mode until button is pressed

    while (strcmp(answer, "n"))
    {
        printf("Enter a left sensor reading: ");
        scanf("%d", &leftSensor);
        printf("Enter a right sensor reading: ");
        scanf("%d", &rightSensor);

        curr_motor_command = compute_proportional(leftSensor, rightSensor);
        motor(0, curr_motor_command.left_speed);
        motor(1, curr_motor_command.right_speed);

        printf("Continue in proportional mode? (y/n): ");
        scanf("%s", answer);
    }

    // this will need to be in while loop
    printf("\nDATA MODE\n");

    // Set cursor to second line and print sensor readings before printing training

    while (strcmp(answer2, "n"))
    {
        printf("Enter a left sensor reading: ");
        scanf("%d", &leftSensor);
        printf("Enter a right sensor reading: ");
        scanf("%d", &rightSensor);

        struct motor_training_value curr_training_value;
        curr_motor_command = compute_proportional(leftSensor, rightSensor);

        // convert everything into 0-1 range before training
        curr_training_value.left = ((float)leftSensor) / (float)255;
        curr_training_value.right = ((float)rightSensor) / (float)255;
        curr_training_value.left_speed = (float)curr_motor_command.left_speed / 100.f;
        curr_training_value.right_speed = ((float)curr_motor_command.right_speed * -1.f) / 100.f;

        training_data[data_count] = curr_training_value;

        data_count++;

        printf("Continue in data mode? (y/n): ");
        scanf("%s", answer2);
    }

    // During this mode, you will use the captured training data along with the proportional controller to train the neural network
    printf("\n----------------------\nData collected(%d):\n", data_count);
    int i;
    for (i = 0; i < data_count; i++)
    {
        printf("\t(%d) Left: %.1f, Right: %.1f, LeftSpeed: %.1f, RightSpeed: %.1f\n", i, training_data[i].left * 255, training_data[i].right * 255, training_data[i].left_speed * 100, training_data[i].right_speed * 100);
    }
    printf("----------------------\n");

    printf("\nTRAINING MODE\n");
    printf("Enter number of iterations: ");
    scanf("%d", &training_iteration_count);
    printf("Running %d iterations.\n\n", training_iteration_count);

    int curr_training_count = 0;
    while (curr_training_count < training_iteration_count)
    {
        // Cycle back to start of dataset if there are more training iterations than data
        if (curr_training_count >= data_count) // UPDATE: Added an =
        {
            curr_training_count = 0;
            training_iteration_count -= data_count - 1;
        }

        printf("Training with data %d -> Left: %.1f, Right: %.1f, LeftSpeed: %.1f, RightSpeed: %.1f\n", curr_training_count,
               training_data[curr_training_count].left * 255, training_data[curr_training_count].right * 255, training_data[curr_training_count].left_speed * 100, training_data[curr_training_count].right_speed * 100);
        
        float inputs[] = {training_data[curr_training_count].left, training_data[curr_training_count].right};
        float outputs[] = {training_data[curr_training_count].left_speed, training_data[curr_training_count].right_speed};

        forward_propagation(&network, inputs);
        backpropagation(&network, inputs, outputs);

        // train_neural_network(training_data[curr_training_count]);
        curr_training_count++;
    }

    printf("Training complete\n");

    // run neural network
    printf("\nAI MODE\n");

    while (1)
    {
        printf("Enter a left sensor reading: ");
        scanf("%d", &leftSensor);
        printf("Enter a right sensor reading: ");
        scanf("%d", &rightSensor);
        float inputs[] = {leftSensor / 255.f, rightSensor / 255.f};
        forward_propagation(&network, inputs);
        //curr_motor_command = compute_neural_network(((float)leftSensor) / 255.f, ((float)rightSensor) / 255.f); // needs 0-1 range for input
        motor(0, network.output[0] * 100);
        motor(1, network.output[1] * -100);
    }

    free(training_data);
    return 0;
}

// void initialize_neural_network()
// {
//     srand(time(NULL));

//     // print_num(rand() % 100);
//     h1.w1 = (float)rand() / (float)RAND_MAX;
//     h1.w2 = (float)rand() / (float)RAND_MAX;
//     h1.b = (float)rand() / (float)RAND_MAX;

//     h2.w1 = (float)rand() / (float)RAND_MAX;
//     h2.w2 = (float)rand() / (float)RAND_MAX;
//     h2.b = (float)rand() / (float)RAND_MAX;

//     h3.w1 = (float)rand() / (float)RAND_MAX;
//     h3.w2 = (float)rand() / (float)RAND_MAX;
//     h3.b = (float)rand() / (float)RAND_MAX;

//     o1.w1 = (float)rand() / (float)RAND_MAX;
//     o1.w2 = (float)rand() / (float)RAND_MAX;
//     o1.w3 = (float)rand() / (float)RAND_MAX;
//     o1.b = (float)rand() / (float)RAND_MAX;

//     o2.w1 = (float)rand() / (float)RAND_MAX;
//     o2.w2 = (float)rand() / (float)RAND_MAX;
//     o2.w3 = (float)rand() / (float)RAND_MAX;
//     o2.b = (float)rand() / (float)RAND_MAX;
// }

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

    // printf("\nWith inputs left = %d and right = %d, compute_proprtional() is returning left speed = %f, right speed = %f\n\n", left, right, result.left_speed, result.right_speed);
    return result;
}

// float activation(float val)
// {
//     return 1.f / (1.f + exp(-1.f * val));
// }

// struct motor_command compute_neural_network(float left, float right)
// {
//     struct motor_command result;

//     h1.in1 = left;
//     h1.in2 = right;

//     h2.in1 = left;
//     h2.in2 = right;

//     h3.in1 = left;
//     h3.in2 = right;

//     o1.in1 = activation((h1.in1 * h1.w1) + (h1.in2 * h1.w2) + h1.b);
//     o1.in2 = activation((h2.in1 * h2.w1) + (h2.in2 * h2.w2) + h2.b);
//     o1.in3 = activation((h3.in1 * h3.w1) + (h3.in2 * h3.w2) + h3.b);

//     o2.in1 = activation((h1.in1 * h1.w1) + (h1.in2 * h1.w2) + h1.b);
//     o2.in2 = activation((h2.in1 * h2.w1) + (h2.in2 * h2.w2) + h2.b);
//     o2.in3 = activation((h3.in1 * h3.w1) + (h3.in2 * h3.w2) + h3.b);

//     result.left_speed = activation((o1.in1 * o1.w1) + (o1.in2 * o1.w2) + (o1.in3 * o1.w3) + o1.b);

//     result.right_speed = activation((o2.in1 * o2.w1) + (o2.in2 * o2.w2) + (o2.in3 * o2.w3) + o2.b);

//     return result;
// }

// void print_curr_weights()
// {
//     printf("------------------------------------\n");
//     printf("Weight    o1    o2    h1    h2    h3\n");
//     printf("w1    %6.1f%6.1f%6.1f%6.1f%6.1f\n", o1.w1, o2.w1, h1.w1, h2.w1, h3.w1);
//     printf("w2    %6.1f%6.1f%6.1f%6.1f%6.1f\n", o1.w2, o2.w2, h1.w2, h2.w2, h3.w2);
//     printf("w3    %6.1f%6.1f     -     -     -\n", o1.w3, o2.w3);
//     printf("b     %6.1f%6.1f%6.1f%6.1f%6.1f\n", o1.b, o2.b, h1.b, h2.b, h3.b);
//     printf("------------------------------------\n");
// }

// void train_neural_network(struct motor_training_value currVal)
// {
//     struct hidden_node temp_h1 = {0, 0, 0, 0, 0};
//     struct hidden_node temp_h2 = {0, 0, 0, 0, 0};
//     struct hidden_node temp_h3 = {0, 0, 0, 0, 0};

//     struct output_node temp_o1 = {0, 0, 0, 0, 0, 0, 0};
//     struct output_node temp_o2 = {0, 0, 0, 0, 0, 0, 0};

//     struct motor_command outputs = compute_neural_network(currVal.left, currVal.right);

//     // compute new outer weight and biases (don't update yet)
//     temp_o1.w1 = compute_new_weight(o1.w1, compute_outer_layer_slope(o1.in1, outputs.left_speed, currVal.left_speed));
//     temp_o1.w2 = compute_new_weight(o1.w2, compute_outer_layer_slope(o1.in2, outputs.left_speed, currVal.left_speed));
//     temp_o1.w3 = compute_new_weight(o1.w3, compute_outer_layer_slope(o1.in3, outputs.left_speed, currVal.left_speed));
//     temp_o1.b = compute_new_weight(o1.b, compute_outer_layer_slope(-1, outputs.left_speed, currVal.left_speed));

//     temp_o2.w1 = compute_new_weight(o2.w1, compute_outer_layer_slope(o2.in1, outputs.right_speed, currVal.right_speed));
//     temp_o2.w2 = compute_new_weight(o2.w2, compute_outer_layer_slope(o2.in2, outputs.right_speed, currVal.right_speed));
//     temp_o2.w3 = compute_new_weight(o2.w3, compute_outer_layer_slope(o2.in3, outputs.right_speed, currVal.right_speed));
//     temp_o2.b = compute_new_weight(o2.b, compute_outer_layer_slope(-1, outputs.right_speed, currVal.right_speed));

//     // compute new hidden weight and biases (don't update yet)

//     printf("new h1.w1: %f\n", compute_hidden_layer_slope(outputs, currVal, o1.w1, o2.w1, o1.in1, currVal.left));
//     printf("old h1.w1: %f\n", h1.w1);

//     temp_h1.w1 = compute_new_weight(h1.w1, compute_hidden_layer_slope(outputs, currVal, o1.w1, o2.w1, o1.in1, currVal.left));
//     temp_h1.w2 = compute_new_weight(h1.w2, compute_hidden_layer_slope(outputs, currVal, o1.w1, o2.w1, o1.in1, currVal.right));
//     temp_h1.b = compute_new_weight(h1.b, compute_hidden_layer_slope(outputs, currVal, o1.w1, o2.w1, o1.in1, -1));

//     temp_h2.w1 = compute_new_weight(h2.w1, compute_hidden_layer_slope(outputs, currVal, o1.w2, o2.w2, o1.in2, currVal.left));
//     temp_h2.w2 = compute_new_weight(h2.w2, compute_hidden_layer_slope(outputs, currVal, o1.w2, o2.w2, o1.in2, currVal.right));
//     temp_h2.b = compute_new_weight(h2.b, compute_hidden_layer_slope(outputs, currVal, o1.w2, o2.w2, o1.in2, -1));

//     temp_h3.w1 = compute_new_weight(h3.w1, compute_hidden_layer_slope(outputs, currVal, o1.w3, o2.w3, o1.in3, currVal.left));
//     temp_h3.w2 = compute_new_weight(h3.w2, compute_hidden_layer_slope(outputs, currVal, o1.w3, o2.w3, o1.in3, currVal.right));
//     temp_h3.b = compute_new_weight(h3.b, compute_hidden_layer_slope(outputs, currVal, o1.w3, o2.w3, o1.in3, -1));

//     // update all weights and biases
//     print_curr_weights();

//     o1.w1 = temp_o1.w1;
//     o1.w2 = temp_o1.w2;
//     o1.w3 = temp_o1.w3;
//     o1.b = temp_o1.b;

//     o2.w1 = temp_o2.w1;
//     o2.w2 = temp_o2.w2;
//     o2.w3 = temp_o2.w3;
//     o2.b = temp_o2.b;

//     h1.w1 = temp_h1.w1;
//     h1.w2 = temp_h1.w2;
//     h1.b = temp_h1.b;

//     h2.w1 = temp_h2.w1;
//     h2.w2 = temp_h2.w2;
//     h2.b = temp_h2.b;

//     h3.w1 = temp_h3.w1;
//     h3.w2 = temp_h3.w2;
//     h3.b = temp_h3.b;

//     return;
// }
