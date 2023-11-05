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

void get_user_inputs(struct sensor_info* sensor, struct map_info* map);
struct particle* generate_particles(int numParticles);
float calculate_particle_sd(struct particle* particles, int num_particles);
float get_motion_noise();
void categorize_particle(struct map_info* map, struct particle* particle);
void compute_weight(int sensor_reading, struct sensor_info* sensor, struct particle* particle);
void resample_particles(struct particle* particles, struct particle* new_particles, int num_particles);
void normalize_weights(float weight_sum, struct particle* particles, int num_particles);
void print_particles(struct particle* particles); // TODO: delete

int main(void)
{
    int i, sensor_reading;
    float weight, weight_sum;
    struct sensor_info sensor;
    struct map_info map;
    struct particle* particles;
    struct particle* new_particles = malloc(NUM_PARTICLES * sizeof(struct particle));
    struct particle currParticle;

    get_user_inputs(&sensor, &map);

    // TODO: Replace with actual readings
    sensor.a = 5;
    sensor.b = 10;
    sensor.c = 15;
    sensor.d = 20;

    particles = generate_particles(NUM_PARTICLES);

    printf("DETERMINING ROBOT LOCATION\n\n");

    while (calculate_particle_sd(particles, NUM_PARTICLES) > SD_THRESHOLD)
    {
        printf("Enter a sensor reading: ");
        scanf("%d", &sensor_reading);

        weight_sum = 0.0;

        for (i = 0; i < NUM_PARTICLES; i++)
        {
            // advance the particle
            particles[i].location += ADVANCE_DEGREES;
            particles[i].location += get_motion_noise();

            // categorize
            categorize_particle(&map, &(particles[i]));

            // TODO: compute weight
            compute_weight(sensor_reading, &sensor, &(particles[i]));
            weight_sum += particles[i].weight;
        }

        normalize_weights(weight_sum, particles, NUM_PARTICLES);

        resample_particles(particles, new_particles, NUM_PARTICLES);

        printf("SD: %.3f\n", calculate_particle_sd(particles, NUM_PARTICLES));
    }

    free(particles);
    free(new_particles);
}


void get_user_inputs(struct sensor_info* sensor, struct map_info* map)
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

    for (i = 0; i < num_particles; ++i) {
        sum += particles[i].location;
    }

    mean = sum / num_particles;

    for (i = 0; i < num_particles; ++i) {
        SD += pow(particles[i].location - mean, 2);
    }

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

    particle->category = FREE_SPACE;

    for (i = 0; i < map->num_towers; i++)
        if (location < (map->tower_locations[i] + TOWER_TICK_WIDTH) && location > (map->tower_locations[i] - TOWER_TICK_WIDTH))
            particle->category = TOWER;
}

// FIXME: finish
void compute_weight(int sensor_reading, struct sensor_info* sensor, struct particle* particle)
{
    // float u = 2 / (sensor->d + sensor->c - sensor->b - sensor->a);

    // if (particle->category == TOWER)
    // {

    // }
    // else if (particle->category == FREE_SPACE)
    // {

    // }

    particle->weight = ((float) rand()) / ((float) RAND_MAX); 
}


void normalize_weights(float weight_sum, struct particle* particles, int num_particles)
{
    int i;

    for (i = 0; i < num_particles; i++)
        particles[i].weight = particles[i].weight / weight_sum;
}


// TODO: do we need to add 5% random?
void resample_particles(struct particle* particles, struct particle* new_particles, int num_particles)
{
    int i, j;
    float curr_index;
    float running_sum = 0;

    for (i = 0; i < num_particles; i++)
    {
        running_sum += particles[i].weight;
        particles[i].summed_weight = running_sum;
    } 

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

    particles = new_particles;
}


// TODO: delete
void print_particles(struct particle* particles)
{
    int i;
    printf("Particle #     Location       Weight         Summed Weight  \n");

    for (i = 0; i < NUM_PARTICLES; i++)
    {
        printf("%10d%13.3f%13.3f%22.3f\n", i, particles[i].location, particles[i].weight, particles[i].summed_weight);
    }
    
}