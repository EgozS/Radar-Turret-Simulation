#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define M_PI 3.14159265358979323846
#define TURRET_DISTANCE 5
#define DEG2RAD(x) ((x) * (M_PI / 180.0f))
#define RAD2DEG(x) ((x) * (180.0f / M_PI))

typedef struct RadarRead {
    float angle;
    float height;
    float distance;
} RadarRead;

typedef struct TurretWrite {
    float angle;
    float height;
    float distance;
} TurretWrite;

/// @brief trigo function that calculates the turret angles based on the radar data
/// @param data radar data: angle, height and distance from a point
/// @return TurretWrite turret data: angle, height and distance to the point from the turret based on a fixed distance
/// @note this shit took me 2 hours to get right
TurretWrite GetAngles(RadarRead data) {
    TurretWrite res;

    float height_rad = DEG2RAD(data.height);
    float angle_rad = DEG2RAD(data.angle);

    float h = sinf(height_rad) * data.distance;
    float r_plane = cosf(height_rad) * data.distance;

    float x_radar = r_plane * cosf(angle_rad);
    float y_radar = r_plane * sinf(angle_rad);

    float dx = x_radar - TURRET_DISTANCE;
    float dy = y_radar;

    float flat_distance = sqrtf(dx * dx + dy * dy);
    float turret_angle_rad = atan2f(dy, dx);
    float turret_height_rad = atanf(h / flat_distance);
    float total_distance = sqrtf(flat_distance * flat_distance + h * h);

    float turret_angle_deg = RAD2DEG(turret_angle_rad);
    if (turret_angle_deg < 0) turret_angle_deg += 360.0f;

    res.angle = turret_angle_deg;
    res.height = RAD2DEG(turret_height_rad);
    res.distance = total_distance;

    return res;
}

int main() {
    RadarRead data = {0};
    TurretWrite output = {0};

    FILE *fp = fopen("turret.txt", "w");
    FILE *fp2 = fopen("radar.txt", "w");
    if (!fp || !fp2) {
        printf("Error opening file!\n");
        return 1;
    }

    srand(time(NULL));

    int stepper_steps = 200;
    float step_angle = 1.8f;

    int step = 0;
    float angle_deg = 0.0f;
    float variation = 0.0f;
    int i = 0;
    bool isUp = false;
    //main loop: for each step of the stepper motor, sweeping the servo 0-45 degrees
    //for each angle, getting turret path and writing to file for python simulation
    while (1) {
        angle_deg = fmodf(step * step_angle, 360.0f);
        if (isUp) {
            for (i = 0; i <= 45; i += 3) {
                data.angle = angle_deg;
                data.height = i;
        
                variation = ((rand() % 100) - 50) / 100.0f;
                data.distance = 12.0f + variation;
        
                output = GetAngles(data);
        
                if (!(isnan(output.angle) || isnan(output.height) || isnan(output.distance))) {
                    fprintf(fp, "%f,%f,%f#", output.angle, output.height, output.distance);
                    fprintf(fp2, "%f,%f,%f#", data.angle, data.height, data.distance);
                    fflush(fp);   
                    fflush(fp2);  
                }
                usleep(10000);
            }
        } else {
            for (i = 45; i >= 0; i -= 3) {
                data.angle = angle_deg;
                data.height = i;
        
                variation = ((rand() % 100) - 50) / 100.0f;
                data.distance = 12.0f + variation;
        
                output = GetAngles(data);
        
                if (!(isnan(output.angle) || isnan(output.height) || isnan(output.distance))) {
                    fprintf(fp, "%f,%f,%f#", output.angle, output.height, output.distance);
                    fprintf(fp2, "%f,%f,%f#", data.angle, data.height, data.distance);
                    fflush(fp);   
                    fflush(fp2);  
                }
                usleep(10000);
            }
        }        
        isUp = !isUp;
        step = (step + 1) % stepper_steps;
        usleep(300000); //300ms stepper delay
    }

    fclose(fp);
    fclose(fp2);
    return 0;
}
