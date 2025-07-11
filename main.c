#include "main.h"

#define N_AGENTS 1000
#define SCALE_FACTOR 0.5

uint quadVAO, quadVBO;
uint fbo;
uint agentsBO[N_AGENTS];

const uint TEXTURE_WIDTH  = 1920 * SCALE_FACTOR;
const uint TEXTURE_HEIGHT = 1080 * SCALE_FACTOR;
const uint N_PIXELS = TEXTURE_WIDTH * TEXTURE_HEIGHT;

typedef struct Agent {
    float x;
    float y;
    float angle;
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

    glBindVertexArray(quadVAO);
}

Agent* setup_agents(uint n) {
    Agent* agents = malloc(sizeof(Agent) * n);
    srand(TIME);
    for_range(0, n, i) {
        agents[i] = (Agent){            
            rand() % TEXTURE_WIDTH,
            rand() % TEXTURE_HEIGHT,
            ((float)rand() / RAND_MAX) * 2.0 * M_PI - M_PI
        };
    }
    
    glGenBuffers(1, agentsBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *agentsBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * n, agents, GL_DYNAMIC_DRAW); 
    
    return agents;
}

int main(int argc, char* argv[]) {
    GLFWwindow* window = init_window(true, true);

    pthread_t clock_thread_idx;
    pthread_create(&clock_thread_idx, NULL, clock_thread, window);    

    uint update_agents = glCreateProgram();
    load_shader_file("shaders/compute.comp", update_agents , GL_COMPUTE_SHADER);
    glLinkProgram(update_agents);

    uint post_processing = glCreateProgram();
    load_shader_file("shaders/quad.vert", post_processing, GL_VERTEX_SHADER);
    load_shader_file("shaders/quad.frag", post_processing, GL_FRAGMENT_SHADER);
    glLinkProgram(post_processing);
    
    uint screen_texture = create_and_bind_texture(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    
    glGenFramebuffers(1, &fbo);

    uint frame_n_location = glGetUniformLocation(update_agents, "frameNumber");
    uint frame_number = 0;

    uint post_processing_texture_location = glGetUniformLocation(post_processing, "screenTexture");

    Agent* agents = setup_agents(N_AGENTS);

    setup_quad();
    
    while(!glfwWindowShouldClose(window)) {
        if (key_pressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);
        
        int window_width, window_height;
        glfwGetFramebufferSize(window, &window_width, &window_height);
        
        glUseProgram(update_agents);
        glProgramUniform1ui(update_agents, frame_n_location, frame_number);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, *agentsBO);
        glBindImageTexture(0, screen_texture, 0, 0, 0, GL_READ_WRITE, GL_RGBA32F);
        
        glDispatchCompute(100, 100, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glTextureBarrier(); 
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

        glUseProgram(post_processing);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glUniform1i(post_processing_texture_location, 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame_number += 1;
    }

    cleanup();
}