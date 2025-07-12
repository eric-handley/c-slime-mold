#include "main.h"

#define SCALE_FACTOR 1.5
const uint TEXTURE_WIDTH  = 1920 * SCALE_FACTOR;
const uint TEXTURE_HEIGHT = 1080 * SCALE_FACTOR;

Settings_T* Settings = &(Settings_T){
    .n_agents        = 1000000,
    .n_species       = 4,  // Max 16 species and colours
    .species_colours = {0xFF3CAC, 0x784BA0, 0x2B86C5, 0x00FFC6},           // Neon Night  
    // .species_colours = {0xFF5733, 0xFF8D1A, 0xFFC300, 0xDAF7A6},           // Sunset Heat  
    // .species_colours = {0x9ADCFF, 0x80B3FF, 0x6886C5, 0x506D84},           // Arctic Drift  
    // .species_colours = {0x6C5B7B, 0xC06C84, 0xF67280, 0xF8B195},           // Rose Fade  
    // .species_colours = {0x114B5F, 0x028090, 0xE4FDE1, 0x456990},           // Oceanic Calm  
    // .species_colours = {0xFFD700, 0xFF8C00, 0xFF4500, 0xDC143C},           // Solar Flare  
    // .species_colours = {0x1A535C, 0x4ECDC4, 0xF7FFF7, 0xFF6B6B},           // Mint Punch  
    // .species_colours = {0x222222, 0x555555, 0xAAAAAA, 0xFFFFFF},           // Grayscale Mood  
    // .species_colours = {0x00FF00, 0xFFFF00, 0xFF00FF},                     // Toxic Pop  
    .speed           = 1,
    .turn_randomness = 0.02,
    .turn_speed      = 0.01,
    .sample_angle    = 0.4 * M_PI,
    .sample_dist     = 20,
    .debug           = false
};

GLuint agentsBO;
GLuint quadVAO, quadVBO;
GLuint FBO;

typedef struct Agent {
    float x;
    float y;
    float angle;
    uint32_t species;
} Agent;

void setup_quad() {
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void setup_agents() {
    glGenBuffers(1, &agentsBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, agentsBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * Settings->n_agents, NULL, GL_DYNAMIC_DRAW); 
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch(key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_UP:
                Settings->speed += 0.1;
                update_shared_settings();
                break;
            case GLFW_KEY_DOWN:
                if (Settings->speed > 0) {
                    Settings->speed -= 0.1;
                    update_shared_settings();
                }
                break;
            case GLFW_KEY_RIGHT:
                Settings->turn_speed += 0.001;
                update_shared_settings();
                if (Settings->debug) printf("turn_speed: %f\n", Settings->turn_speed);
                break;
            case GLFW_KEY_LEFT:
                if (Settings->turn_speed > 0) {
                    Settings->turn_speed -= 0.001;
                    update_shared_settings();
                    if (Settings->debug) printf("turn_speed: %f\n", Settings->turn_speed);
                }
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    GLFWwindow* window = init_window(true, false);

    pthread_t clock_thread_idx, input_thread_idx;
    pthread_create(&clock_thread_idx, NULL, clock_thread, window);    

    glfwSetKeyCallback(window, key_callback);
    configure_shared_settings();
    setup_agents();
    setup_quad();

    uint update_agents_program = glCreateProgram();
    load_shader_file("shaders/compute.comp", update_agents_program , GL_COMPUTE_SHADER);
    glLinkProgram(update_agents_program);

    uint post_processing_program = glCreateProgram();
    load_shader_file("shaders/quad.vert", post_processing_program, GL_VERTEX_SHADER);
    load_shader_file("shaders/quad.frag", post_processing_program, GL_FRAGMENT_SHADER);
    glLinkProgram(post_processing_program);

    uint screen_texture = create_and_bind_texture(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    uint texture_unif      = glGetUniformLocation(post_processing_program, "screenTexture");
    uint frame_number_unif = glGetUniformLocation(update_agents_program,   "frameNumber"  );
    uint time_unif         = glGetUniformLocation(update_agents_program,   "time"         );
    uint frame_number = 0;

    int num_groups_needed = (Settings->n_agents + 1024 - 1) / 1024; // 1024 -> threads per workgroup

    while(!glfwWindowShouldClose(window)) {
        int window_width, window_height;
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glGenFramebuffers(1, &FBO);
        
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

        // Fade & blur pixels
        glUseProgram(post_processing_program);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glUniform1i(texture_unif, 0);

        // Draw updated texture to quad
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


        // Update agent positions and draw
        glUseProgram(update_agents_program);
        
        // Update frame number, bind agent buffer object and screen texture
        glProgramUniform1ui(update_agents_program, frame_number_unif, frame_number);
        glProgramUniform1ui(update_agents_program, time_unif, (int)clock());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, agentsBO);
        glBindImageTexture(0, screen_texture, 0, 0, 0, GL_READ_WRITE, GL_RGBA32F);
        
        glDispatchCompute(num_groups_needed, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame_number += 1;
    }

    glfwTerminate();
}