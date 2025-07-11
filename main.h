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

typedef GLuint uint;
#include "func.h"

#define sleep(sec) usleep((sec) * (1e6))
#define for_range(start, end, iter) for (int iter = start; iter < end; iter++)

double TIME;

void* clock_thread(void* arg) {
    GLFWwindow* window = (GLFWwindow*)arg;
    clock_t t_start = clock();
    while(!glfwWindowShouldClose(window)) {
        clock_t t_now = clock();
        TIME = (double)(t_now - t_start) / CLOCKS_PER_SEC;
    }
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

void cleanup() {
    glfwTerminate();
}

#endif