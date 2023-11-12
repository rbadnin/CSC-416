#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define FREE_SPACE 0
#define TOWER 1
#define NUM_PARTICLES 100 // FIXME: adjust as needed
#define SD_THRESHOLD 1 
#define ADVANCE_TICKS 5 // FIXME: choose proper ticks
#define ADVANCE_DEGREES 3 // FIXME: when I move the robot x ticks forward, how many degrees is that on the circle?
#define TOWER_TICK_WIDTH 15 // FIXME: choose a proper value for this

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

int compare(const void *a, const void *b);
float generate_sensor_reading(struct sensor_info* tower_sensor, struct sensor_info* free_space_sensor, struct map_info* map, float curr_robot_position);
void get_user_inputs(struct map_info* map, float* robot_position);
void generate_particles(struct particle* particles);
float calculate_particle_sd(struct particle* particles);
float get_motion_noise();
void categorize_particle(struct map_info* map, struct particle* particle);
void compute_weight(float sensor_reading, struct sensor_info* sensor, struct particle* particle);
void resample_particles(struct particle* particles, struct particle* new_particles);
void normalize_weights(float weight_sum, struct particle* particles);
float determine_location(float weight_sum, struct particle* particles);



int main(void)
{
    int i, degree_ct = 0;
    char c; //TODO: delete for robot version
    float weight, weight_sum, robot_position, sensor_reading, final_location_sum, final_location;
    struct sensor_info tower_sensor;
    struct sensor_info free_space_sensor;
    struct map_info map;
    struct particle particles[NUM_PARTICLES], new_particles[NUM_PARTICLES];
    struct particle currParticle;

    get_user_inputs(&map, &robot_position);

    // TODO: Replace with actual readings
    tower_sensor.a = 0;
    tower_sensor.b = 8;
    tower_sensor.c = 10;
    tower_sensor.d = 50;

    free_space_sensor.a = 30;
    free_space_sensor.b = 200;
    free_space_sensor.c = 220;
    free_space_sensor.d = 240;

    generate_particles(particles);

    printf("DETERMINING ROBOT LOCATION\n\n");

    while (calculate_particle_sd(particles) > SD_THRESHOLD && degree_ct < 360) 
    {
        // TODO: delete this scan for robot program
        printf("Click enter to continue (Hold enter to complete program).");
        scanf("%c", &c);

        robot_position += ADVANCE_DEGREES;
        degree_ct += ADVANCE_DEGREES;
        if (robot_position >= 360)
            robot_position = robot_position - 360;

        printf("\n\nMoving robot %d degrees forward, and is now at position %.1f\n", ADVANCE_DEGREES, robot_position); // TODO: replace with moving the actual robot ADVANCE_TICKS forward

        sensor_reading = generate_sensor_reading(&tower_sensor, &free_space_sensor, &map, robot_position);
        printf("Generated sensor reading: %.1f\n", sensor_reading);

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

        printf("Current SD: %.3f\n\n", calculate_particle_sd(particles));
    }

    final_location = determine_location(weight_sum, particles);
    printf("\n\nFINAL LOCATION: %.2f\n\n", final_location);
}


// FIXME: Make more accurate, and make it work across the 359 -> 0 transition
float determine_location(float weight_sum, struct particle* particles)
{
    qsort(particles, NUM_PARTICLES, sizeof(struct particle), compare);
    return particles[NUM_PARTICLES/2].location;
}


int compare(const void *a, const void *b) 
{
  
    struct particle *particleA = (struct particle *) a;
    struct particle *particleB = (struct particle *) b;
  
    return (particleB->location - particleA->location);
}


float generate_sensor_reading(struct sensor_info* tower_sensor, struct sensor_info* free_space_sensor, struct map_info* map, float location)
{
    int i = 0;
    float upper_lim, lower_lim;
    float category = FREE_SPACE;
    struct sensor_info* sensor;

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
        
    return (rand() % ((sensor->c) - (sensor->b))) + ((float) sensor->b) + get_motion_noise();
}


void get_user_inputs(struct map_info* map, float* robot_position)
{
    int i;

    printf("\n---------------------------------\nLAB 4\n---------------------------------\n\n");

    printf("INITIALIZATION\n\n");

    printf("input the number of towers on the map (between 2 and 4): ");
    scanf("%d", &(map->num_towers));

    for (i = 0; i < map->num_towers; i++)
    {
        printf("input the location of tower #%d: ", (i + 1));
        scanf("%f", &(map->tower_locations[i]));
    }

    printf("input the target tower: ");
    scanf("%d", &(map->target_tower));
    map->target_tower--;

    printf("\n\t→ map values received: %d towers at locations ", map->num_towers);

    for (i = 0; i < map->num_towers; i++)
    {
        printf("%.1f, ", map->tower_locations[i]);
    }

    printf("and target tower is tower #%d\n\n", (map->target_tower + 1));

    printf("input the robot starting position: ");
    scanf("%f", robot_position);

    printf("\n\t→ robot is starting at position %.1f\n\n", *robot_position);
}


void generate_particles(struct particle* particles)
{
    float spacing_increment = 359 / (float) NUM_PARTICLES;
    float curr_loc_num = 0;
    int i;

    printf("\nGENERATING PARTICLES\n\n");

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


float calculate_particle_sd(struct particle* particles)
{
    float sum = 0.0;
    float mean;
    float SD = 0.0;
    int i;

    for (i = 0; i < NUM_PARTICLES; ++i) 
        sum += particles[i].location;

    mean = sum / NUM_PARTICLES;

    for (i = 0; i < NUM_PARTICLES; ++i)
        SD += pow(particles[i].location - mean, 2);

    return sqrt(SD / NUM_PARTICLES);
}


float get_motion_noise()
{
    float u1, u2;

    u1 = ((float) rand()) / ((float) RAND_MAX);
    u2 = ((float) rand()) / ((float) RAND_MAX);

    return sqrt((-2) * log(u1)) * cos(2 * M_PI * u2);
}


void categorize_particle(struct map_info* map, struct particle* particle)
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


void compute_weight(float sensor_reading, struct sensor_info* sensor, struct particle* particle)
{
    float a = (float) sensor->a;
    float b = (float) sensor->b;
    float c = (float) sensor->c;
    float d = (float) sensor->d;
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


void normalize_weights(float weight_sum, struct particle* particles)
{
    int i;

    for (i = 0; i < NUM_PARTICLES; i++)
        particles[i].weight = particles[i].weight / weight_sum;
}


void resample_particles(struct particle* particles, struct particle* new_particles)
{
    int i, j, random_amt, index_to_replace, prev_index;
    float curr_index;
    struct particle prev;
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
        curr_index = ((float) i + 1)/((float) NUM_PARTICLES);
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

// TODO: delete
void print_particles(struct particle* particles)
{
    int i;
    printf("\nParticle #     Location       Weight         Summed Weight      Category     Src Part\n");

    for (i = 0; i < NUM_PARTICLES; i++)
    {
        printf("%10d%13.3f%13.3f%22.3f%14d%12d\n", i, particles[i].location, particles[i].weight, particles[i].summed_weight, particles[i].category, particles[i].src_particle);
    }
    
}