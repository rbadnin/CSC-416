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
#define SD_THRESHOLD 10 // FIXME: choose a better value for this
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
};

struct sensor_info
{
    int a, b, c, d;
};

float generate_sensor_reading(struct sensor_info* tower_sensor, struct sensor_info* free_space_sensor, struct map_info* map, float curr_robot_position);
void get_user_inputs(struct map_info* map, float* robot_position);
struct particle* generate_particles(int numParticles);
float calculate_particle_sd(struct particle* particles, int num_particles);
float get_motion_noise();
void categorize_particle(struct map_info* map, struct particle* particle);
void compute_weight(float sensor_reading, struct sensor_info* sensor, struct particle* particle);
void resample_particles(struct particle* particles, struct particle* new_particles, int num_particles);
void normalize_weights(float weight_sum, struct particle* particles, int num_particles);
void print_particles(struct particle* particles); // TODO: delete

int main(void)
{
    int i;
    char c; //TODO: delete for robot version
    float weight, weight_sum, robot_position, sensor_reading, final_location_sum, final_location;
    struct sensor_info tower_sensor;
    struct sensor_info free_space_sensor;
    struct map_info map;
    struct particle* particles;
    struct particle* new_particles = malloc(NUM_PARTICLES * sizeof(struct particle));
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

    particles = generate_particles(NUM_PARTICLES);

    printf("DETERMINING ROBOT LOCATION\n\n");

    while (calculate_particle_sd(particles, NUM_PARTICLES) > SD_THRESHOLD)
    {
        // TODO: delete this scan for robot program
        printf("Click enter to continue.\n");
        scanf("%c", &c);

        sensor_reading = generate_sensor_reading(&tower_sensor, &free_space_sensor, &map, robot_position);
        printf("Current robot location: %.1f, sensor reading: %.1f\n\n", robot_position, sensor_reading);

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

        normalize_weights(weight_sum, particles, NUM_PARTICLES);

        resample_particles(particles, new_particles, NUM_PARTICLES);

        robot_position += ADVANCE_TICKS;
        if (robot_position >= 360)
            robot_position = robot_position - 360;

        printf("\n\nMoving robot %d ticks forward, and is now at position %.1f\n", ADVANCE_TICKS, robot_position); // TODO: replace with moving the actual robot ADVANCE_TICKS forward
        printf("SD: %.3f\n", calculate_particle_sd(particles, NUM_PARTICLES));
    }

    // TODO: check this works
    final_location_sum = 0;
    for (i = 0; i < NUM_PARTICLES; i++)
        final_location_sum += particles[i].location;
    final_location = final_location_sum / NUM_PARTICLES;

    printf("\n\nFINAL LOCATION: %.2f\n\n", final_location);

    free(particles);
    free(new_particles);
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


struct particle* generate_particles(int num_particles)
{
    struct particle* particles = malloc(num_particles * sizeof(struct particle));
    float spacing_increment = 359 / (float) num_particles;
    float curr_loc_num = 0;
    int i;

    printf("\nGENERATING PARTICLES\n\n");

    for (i = 0; i < num_particles; i++)
    {
        particles[i].location = curr_loc_num;
        curr_loc_num += spacing_increment;
    }

    return particles;
}


float calculate_particle_sd(struct particle* particles, int num_particles)
{
    float sum = 0.0;
    float mean;
    float SD = 0.0;
    int i;

    for (i = 0; i < num_particles; ++i) 
        sum += particles[i].location;

    mean = sum / num_particles;

    for (i = 0; i < num_particles; ++i)
        SD += pow(particles[i].location - mean, 2);

    return sqrt(SD / num_particles);
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


void normalize_weights(float weight_sum, struct particle* particles, int num_particles)
{
    int i;

    for (i = 0; i < num_particles; i++)
        particles[i].weight = particles[i].weight / weight_sum;
}


void resample_particles(struct particle* particles, struct particle* new_particles, int num_particles)
{
    int i, j, random_amt, index_to_replace, new_index;
    float curr_index;
    float running_sum = 0;

    print_particles(particles);

    // generate running sum 
    for (i = 0; i < num_particles; i++)
    {
        running_sum += particles[i].weight;
        particles[i].summed_weight = running_sum;
    } 

    // generate new particles
    for (i = 1; i <= num_particles; i++)
    {
        curr_index = ((float) i)/((float) num_particles);
        
        for (j = 0; j < num_particles; j++)
        {
            if (particles[j].summed_weight > curr_index)
            {
                if (j == 0)
                    new_particles[i - 1] = particles[0];
                else
                    new_particles[i - 1] = particles[j - 1];
                break;
            }
        }
    }

    // make 5% of new particles random
    random_amt = .05 * num_particles;
    for (i = 0; i < random_amt; i++)
    {
        index_to_replace = rand() % 100;
        new_index = rand() % 100;
        new_particles[index_to_replace] = particles[new_index];
    }

    particles = new_particles;
    print_particles(particles);
}

// TODO: delete
void print_particles(struct particle* particles)
{
    int i;
    printf("\nParticle #     Location       Weight         Summed Weight      Category\n");

    for (i = 0; i < NUM_PARTICLES; i++)
    {
        printf("%10d%13.3f%13.3f%22.3f%14d\n", i, particles[i].location, particles[i].weight, particles[i].summed_weight, particles[i].category);
    }
    
}