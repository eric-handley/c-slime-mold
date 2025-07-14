#ifndef MAIN_H
#define MAIN_H

#define _GNU_SOURCE
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>

typedef enum SettingsType {
    MOVEMENT,
    SPECIES
} SettingsType;

void save_settings(SettingsType which, int key_pressed);
void load_settings(SettingsType which, int key_pressed);

#define sleep(sec) usleep((sec) * (1e6))
#define for_range(start, end, iter) for (int iter = start; iter < end; iter++)
#define key_pressed(key) glfwGetKey(window, key) == GLFW_PRESS
#define MAX_SPECIES 16

GLuint agentsBO;
GLuint quadVAO, quadVBO;
GLuint frameBO;

typedef GLuint uint;
uint64_t TIME = 0;

typedef struct Agent {
    float x;
    float y;
    float angle;
    uint32_t species;
} Agent;

#include "functions.c"
#include "settings.c"
#include "save.c"

#endif