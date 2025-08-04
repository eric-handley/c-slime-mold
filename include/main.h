#ifndef MAIN_H
#define MAIN_H

#define LOCAL_SIZE 16

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
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <pthread.h>

#ifdef _WIN32
    #include <direct.h>
#else
    #include <sys/mman.h>
#endif

#define sleep(sec) usleep((sec) * (1e6))
#define for_range(start, end, iter) for (int iter = start; iter < end; iter++)
#define key_pressed(key) glfwGetKey(window, key) == GLFW_PRESS

typedef GLuint uint;

typedef struct Agent {
    float x;
    float y;
    float angle;
    uint32_t species;
} Agent;

extern GLuint agentsBO;
extern GLuint quadVAO, quadVBO;
extern GLuint frameBO;
extern uint64_t TIME;
extern const uint NUM_AGENTS;

extern const uint TEXTURE_WIDTH;
extern const uint TEXTURE_HEIGHT;

extern float mouse_x, mouse_y;
extern bool mouse_pressed;
extern bool right_mouse_pressed;
extern float fade_factor;
extern bool autoswap_mode;
extern bool autotweak_mode;
extern pthread_t autoswap_thread;
extern pthread_t autotweak_thread;
extern bool autoswap_thread_running;
extern bool autotweak_thread_running;
extern pthread_mutex_t settings_mutex;

#endif