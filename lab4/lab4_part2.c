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
#define SD_THRESHOLD 10 // FIXME: choose a better value for this

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
};

struct sensor_info
{
    int a, b, c, d;
};

void get_user_inputs(struct sensor_info sensor, struct map_info map);
struct particle* generate_particles(int numParticles, struct map_info map);
float calculate_particle_sd(struct particle* particles, int num_particles);


int main(void)
{
    struct sensor_info sensor;
    struct map_info map;
    struct particle* particles;

    get_user_inputs(sensor, map);
    particles = generate_particles(100, map);

    while (calculate_particle_sd(particles, 100) > SD_THRESHOLD)
    {
        // TODO: do step 1-4 of "determine robot position" here
    }

    free(particles);
}


void get_user_inputs(struct sensor_info sensor, struct map_info map)
{
    int i;

    printf("\n---------------------------------\nLAB 4\n---------------------------------\n\n");

    printf("INITIALIZATION\n\n");

    printf("input the value of the sensor reading when facing FREE SPACE: ");
    scanf("%d", &(sensor.a));
    sensor.d = sensor.a;
    printf("input the value of the sensor reading when facing a TOWER: ");
    scanf("%d", &(sensor.b));
    sensor.c = sensor.b;

    printf("\n\t→ sensor values received: a=%d b=%d c=%d d=%d\n\n", sensor.a, sensor.b, sensor.c, sensor.d);

    printf("input the number of towers on the map (between 2 and 4): ");
    scanf("%d", &(map.num_towers));

    for (i = 0; i < map.num_towers; i++)
    {
        printf("input the location of tower #%d: ", (i + 1));
        scanf("%f", &(map.tower_locations[i]));
    }

    printf("input the target tower: ");
    scanf("%d", &(map.target_tower));
    map.target_tower--;

    printf("\n\t→ map values received: %d towers at locations ", map.num_towers);

    for (i = 0; i < map.num_towers; i++)
    {
        printf("%.1f, ", map.tower_locations[i]);
    }

    printf("and target tower is tower #%d\n\n", (map.target_tower + 1));
}


struct particle* generate_particles(int num_particles, struct map_info map)
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