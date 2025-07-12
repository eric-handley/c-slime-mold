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
#include <assert.h>

typedef GLuint uint;
typedef struct Settings_T {
    uint32_t n_agents;            // offset 0
    uint32_t n_species;           // offset 4
    uint32_t padding[2];          // offset 8, 12 (pad to 16-byte boundary)
    uint32_t species_colours[16]; // offset 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76
    float    speed;               // offset 80
    float    turn_randomness;     // offset 84
    float    turn_speed;          // offset 88
    float    sample_angle;        // offset 92
    uint32_t sample_dist;         // offset 96
    uint32_t debug;               // offset 100
} Settings_T;

Settings_T* Settings;

#include "func.h"

#define sleep(sec) usleep((sec) * (1e6))
#define for_range(start, end, iter) for (int iter = start; iter < end; iter++)

double TIME;
GLuint settingsUBO;

void configure_shared_settings() {
    glGenBuffers(1, &settingsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, settingsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Settings_T), Settings, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, settingsUBO);
}

void update_shared_settings() {
    glBindBuffer(GL_UNIFORM_BUFFER, settingsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Settings_T), Settings);
}

void* clock_thread(void* arg) {
    GLFWwindow* window = (GLFWwindow*)arg;
    clock_t t_start = clock();
    while(!glfwWindowShouldClose(window)) {
        clock_t t_now = clock();
        TIME = (double)(t_now - t_start) / CLOCKS_PER_SEC;
    }
    return NULL;
}

GLFWwindow* init_window(bool fullscreen, bool disable_60_fps_limit) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window;
    if (fullscreen) {
        window = glfwCreateWindow(1920, 1080, "OpenGL", glfwGetPrimaryMonitor(), NULL);
    } else {
        window = glfwCreateWindow(1000, 800, "OpenGL", NULL, NULL);
    }
   
    if (window == NULL) {
        perror("init_window()");
        exit(1);
    }
    
    glfwMakeContextCurrent(window);
    if (disable_60_fps_limit) glfwSwapInterval(0);

    glewExperimental = true;
    glewInit();
    
    // Clear any GLEW initialization errors
    while (glGetError() != GL_NO_ERROR) {}
    
    return window;
}

#endif