#include "main.h"
#include "utils.h"

int random_int(int min, int max) {
    float scale = rand() / (RAND_MAX + 1.0f);
    return min + (int)(scale * (max - min + 1));
}

float random_float(float min, float max) {
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

float randfloat(float scale) {
    return scale * rand() / (RAND_MAX + 1.0f);
}

float randfloat_range(float min, float max) {
    return min + (max - min) * ((float)rand() / RAND_MAX);
}