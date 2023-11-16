/**
    Justin Brunings and Riley Badnin
    Lab 4 (Live version): Determines robot location and knocks down target tower, given a map
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <globals.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define FREE_SPACE 0
#define TOWER 1
#define NUM_PARTICLES 100
#define SD_THRESHOLD 5
#define ADVANCE_TICKS 5        
#define ADVANCE_DEGREES 4.5   
#define TOWER_TICK_WIDTH 10.56 

void init_encoder()
{
    // enable encoder interrupts
    EIMSK = 0;
    EIMSK |= _BV(PCIE1) | _BV(PCIE0);

    PCMSK1 |= _BV(PCINT13); // PB5 - digital 5
    PCMSK0 |= _BV(PCINT6);  // PE6 - digital 4

    // enable pullups
    PORTE |= _BV(PE6);
    PORTB |= _BV(PB5);
}

volatile uint16_t left_encoder = 0;
volatile uint16_t right_encoder = 0;

ISR(PCINT0_vect)
{
    left_encoder++; // increment left encoder
}

ISR(PCINT1_vect)
{
    right_encoder++; // increment right encoder
}

struct map_info
{
    int num_towers;
    float tower_locations[4];
    int target_tower;
};

struct particle
{
    float location;
    int category;
    float weight;
    float summed_weight;
    int src_particle;
};

struct sensor_info
{
    int a, b, c, d;
};

// helper methods for proportional controller -------------------------

struct motor_command
{
    float left_speed;
    float right_speed;
};

void motor(int num, int speed)
{
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

int compare(const void *a, const void *b);
float generate_sensor_reading(struct sensor_info *tower_sensor, struct sensor_info *free_space_sensor, struct map_info *map, float curr_robot_position);
void get_user_inputs(struct map_info *map);
void advance_robot();
void generate_particles(struct particle *particles);
float calculate_particle_sd(struct particle *particles);
float get_motion_noise();
void categorize_particle(struct map_info *map, struct particle *particle);
void compute_weight(float sensor_reading, struct sensor_info *sensor, struct particle *particle);
void resample_particles(struct particle *particles, struct particle *new_particles);
void normalize_weights(float weight_sum, struct particle *particles);
float determine_location(float weight_sum, struct particle *particles);

int main(void)
{
    int i, degree_ct = 0;
    float weight_sum, sensor_reading, robot_location;
    struct sensor_info tower_sensor;
    struct sensor_info free_space_sensor;
    struct map_info map;
    struct particle particles[NUM_PARTICLES], new_particles[NUM_PARTICLES];

    init();
    init_encoder();
    motor(0, 0);
    motor(1, 0);

    get_user_inputs(&map);
    // map.num_towers = 3;
    // map.target_tower = 2;
    // map.tower_locations[0] = 0;
    // map.tower_locations[1] = 90;
    // map.tower_locations[2] = 225;

    // TODO: Replace with actual readings
    tower_sensor.a = 60;
    tower_sensor.b = 70;
    tower_sensor.c = 140;
    tower_sensor.d = 170;

    free_space_sensor.a = 0;
    free_space_sensor.b = 5;
    free_space_sensor.c = 30;
    free_space_sensor.d = 60;

    generate_particles(particles);

    while (!get_btn())
    {
    }


    while (calculate_particle_sd(particles) > SD_THRESHOLD || degree_ct < 360)
    {
        advance_robot();
        degree_ct += ADVANCE_DEGREES;

        sensor_reading = analog(4);

        weight_sum = 0.0;

        for (i = 0; i < NUM_PARTICLES; i++)
        {
            // advance the particle
            particles[i].location += ADVANCE_DEGREES;
            particles[i].location += get_motion_noise();
            if (particles[i].location >= 360)
                particles[i].location = particles[i].location - 360;

            // categorize
            categorize_particle(&map, &(particles[i]));

            // determine weight
            if (particles[i].category == TOWER)
                compute_weight(sensor_reading, &tower_sensor, &(particles[i]));
            else if (particles[i].category == FREE_SPACE)
                compute_weight(sensor_reading, &free_space_sensor, &(particles[i]));
            weight_sum += particles[i].weight;
        }

        normalize_weights(weight_sum, particles);

        resample_particles(particles, new_particles);
        for (i = 0; i < NUM_PARTICLES; i++)
            particles[i] = new_particles[i];
        qsort(particles, NUM_PARTICLES, sizeof(struct particle), compare);

        clear_screen();
        print_num(calculate_particle_sd(particles));
        lcd_cursor(0, 1);
        print_num(sensor_reading);
        _delay_ms(20);
    }

    robot_location = particles[NUM_PARTICLES / 2].location;
    clear_screen();
    print_num(robot_location);

    while (1)
    {
        if (abs(robot_location - map.tower_locations[map.target_tower]) < 4.1)
            break;

        advance_robot();
        robot_location += ADVANCE_DEGREES;
        if (robot_location >= 360)
            robot_location = robot_location - 360;
    }

    motor(0, 100);
    motor(1, 100);

    _delay_ms(850); // ow long a 90 degree turn is

    while (1)
    {
        motor(0, 50);
        motor(1, -50);
    }
}

void advance_robot()
{
    int stop = left_encoder + ADVANCE_TICKS;
    while (left_encoder <= stop)
    {
        struct motor_command command = compute_proportional(analog(0), analog(1));
        motor(0, command.left_speed / 2);
        motor(1, command.right_speed / 2);
        _delay_ms(15);
    }
}

int compare(const void *a, const void *b)
{

    struct particle *particleA = (struct particle *)a;
    struct particle *particleB = (struct particle *)b;

    return (particleA->location - particleB->location);
}

float generate_sensor_reading(struct sensor_info *tower_sensor, struct sensor_info *free_space_sensor, struct map_info *map, float location)
{
    int i = 0;
    float upper_lim, lower_lim;
    float category = FREE_SPACE;
    struct sensor_info *sensor;

    for (i = 0; i < map->num_towers; i++)
    {
        upper_lim = (map->tower_locations[i] + TOWER_TICK_WIDTH);
        lower_lim = (map->tower_locations[i] - TOWER_TICK_WIDTH);

        if ((upper_lim >= 360 && ((location > lower_lim && location < 360) || (location > 0 && location < (upper_lim - 360)))) ||
            (lower_lim < 0 && ((location < upper_lim && location > 0) || (location < 360 && location > (360.0 + lower_lim)))) ||
            (location < upper_lim && location > lower_lim))
            category = TOWER;
    }

    if (category == TOWER)
        sensor = tower_sensor;
    else if (category == FREE_SPACE)
        sensor = free_space_sensor;

    return (rand() % ((sensor->c) - (sensor->b))) + ((float)sensor->b) + get_motion_noise();
}

void get_user_inputs(struct map_info *map)
{
    int num_towers = 2;

    uint8_t yMove = 0;
    while (!get_btn())
    {
        clear_screen();
        print_string("Towers:");
        lcd_cursor(0, 1);
        print_num(num_towers);

        yMove = get_accel_y();
        if ((yMove < 240 && yMove > 128))
            num_towers = num_towers < 4 ? num_towers + 1 : 4;
        else if ((yMove > 15 && yMove < 128))
            num_towers = num_towers > 2 ? num_towers - 1 : 2;

        _delay_ms(500);
    }

    map->num_towers = num_towers;
    _delay_ms(500);

    int currLoc, i;
    for (i = 0; i < map->num_towers; i++)
    {
        clear_screen();
        currLoc = 0;

        while (!get_btn())
        {
            clear_screen();
            print_string("Tower ");
            print_num(i);
            lcd_cursor(0, 1);
            print_num(currLoc);

            yMove = get_accel_y();
            if ((yMove < 240 && yMove > 128))
                currLoc = currLoc < 360 ? currLoc + 45 : 0;
            else if ((yMove > 15 && yMove < 128))
                currLoc = currLoc > 0 ? currLoc - 45 : 360;

            _delay_ms(500);
        }

        map->tower_locations[i] = currLoc;
        _delay_ms(500);
    }

    int vader_loc = 1;
    while (!get_btn())
    {
        clear_screen();
        print_string("Vader:");
        lcd_cursor(0, 1);
        print_num(vader_loc);

        yMove = get_accel_y();
        if ((yMove < 240 && yMove > 128))
            vader_loc = vader_loc < num_towers ? vader_loc + 1 : num_towers;
        else if ((yMove > 15 && yMove < 128))
            vader_loc = vader_loc > 1 ? vader_loc - 1 : 1;

        _delay_ms(200);
    }

    map->target_tower = vader_loc - 1;
}

void generate_particles(struct particle *particles)
{
    float spacing_increment = 359 / (float)NUM_PARTICLES;
    float curr_loc_num = 0;
    int i;

    for (i = 0; i < NUM_PARTICLES; i++)
    {
        particles[i].location = curr_loc_num;
        particles[i].weight = 0.0;
        particles[i].summed_weight = 0.0;
        particles[i].category = 0;
        particles[i].src_particle = -1;
        curr_loc_num += spacing_increment;
    }
}

float calculate_particle_sd(struct particle *particles)
{
    float sum = 0.0;
    float mean;
    float SD = 0.0;
    int i;

    for (i = (.05 * NUM_PARTICLES); i < (NUM_PARTICLES - (.05 * NUM_PARTICLES)); i++)
        sum += particles[i].location;

    mean = sum / (NUM_PARTICLES - (.1 * NUM_PARTICLES));

    for (i = (.05 * NUM_PARTICLES); i < (NUM_PARTICLES - (.05 * NUM_PARTICLES)); i++)
        SD += pow(particles[i].location - mean, 2);

    return sqrt(SD / (NUM_PARTICLES - (.1 * NUM_PARTICLES)));
}

float get_motion_noise()
{
    float u1, u2;

    u1 = ((float)rand()) / ((float)RAND_MAX);
    u2 = ((float)rand()) / ((float)RAND_MAX);

    return sqrt((-2) * log(u1)) * cos(2 * M_PI * u2);
}

void categorize_particle(struct map_info *map, struct particle *particle)
{
    int i = 0;
    float location = particle->location;
    float upper_lim, lower_lim;

    particle->category = FREE_SPACE;

    for (i = 0; i < map->num_towers; i++)
    {
        upper_lim = (map->tower_locations[i] + TOWER_TICK_WIDTH);
        lower_lim = (map->tower_locations[i] - TOWER_TICK_WIDTH);

        if ((upper_lim >= 360 && ((location > lower_lim && location < 360) || (location > 0 && location < (upper_lim - 360)))) ||
            (lower_lim < 0 && ((location < upper_lim && location > 0) || (location < 360 && location > (360.0 + lower_lim)))) ||
            (location < upper_lim && location > lower_lim))
            particle->category = TOWER;
    }
}

void compute_weight(float sensor_reading, struct sensor_info *sensor, struct particle *particle)
{
    float a = (float)sensor->a;
    float b = (float)sensor->b;
    float c = (float)sensor->c;
    float d = (float)sensor->d;
    float x = sensor_reading;

    float u = 2.0 / (d + c - b - a);

    if (x >= b && x < c)
        particle->weight = u;
    else if (x >= a && x < b)
        particle->weight = u * ((x - a) / (b - a));
    else if (x >= c && x < d)
        particle->weight = u * ((d - x) / (d - c));
    else
        particle->weight = 0.0;
}

void normalize_weights(float weight_sum, struct particle *particles)
{
    int i;

    for (i = 0; i < NUM_PARTICLES; i++)
        particles[i].weight = particles[i].weight / weight_sum;
}

void resample_particles(struct particle *particles, struct particle *new_particles)
{
    int i, j, random_amt, index_to_replace, prev_index;
    float curr_index;
    float running_sum = 0, prev_summed_weight;

    // generate running sum
    for (i = 0; i < NUM_PARTICLES; i++)
    {
        running_sum += particles[i].weight;
        particles[i].summed_weight = running_sum;
    }

    // generate new particles
    for (i = 0; i < NUM_PARTICLES; i++)
    {
        curr_index = ((float)i + 1) / ((float)NUM_PARTICLES);
        prev_index = 0;
        prev_summed_weight = 0.0;

        for (j = 0; j < NUM_PARTICLES; j++)
        {
            if (particles[j].summed_weight >= curr_index)
            {
                new_particles[i] = particles[prev_index];
                new_particles[i].src_particle = prev_index;
                break;
            }

            if (particles[j].summed_weight > prev_summed_weight)
            {
                prev_index = j;
                prev_summed_weight = particles[j].summed_weight;
            }
        }
    }

    // make 5% of new particles random
    random_amt = .05 * NUM_PARTICLES;
    for (i = 0; i < random_amt; i++)
    {
        index_to_replace = rand() % 100;
        new_particles[index_to_replace].location = rand() % 360;
        new_particles[index_to_replace].src_particle = -1;
    }
}
