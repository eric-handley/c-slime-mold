#include "main.h"

#define N_AGENTS 100000
#define SCALE_FACTOR 1.5

const uint TEXTURE_WIDTH  = 1920 * SCALE_FACTOR;
const uint TEXTURE_HEIGHT = 1080 * SCALE_FACTOR;
const uint N_PIXELS = TEXTURE_WIDTH * TEXTURE_HEIGHT;

uint quadVAO, quadVBO;
uint FBO;
uint agentsBO[N_AGENTS];

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

    glBindVertexArray(quadVAO);
}

void setup_agents(uint n) {
    glGenBuffers(1, agentsBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *agentsBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * n, NULL, GL_DYNAMIC_DRAW); 
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
    
    glGenFramebuffers(1, &FBO);

    uint frame_n_location = glGetUniformLocation(update_agents, "frameNumber");
    uint frame_number = 0;

    uint num_agents_location = glGetUniformLocation(update_agents, "numAgents");
    glProgramUniform1ui(update_agents, num_agents_location, N_AGENTS);

    uint post_processing_texture_location = glGetUniformLocation(post_processing, "screenTexture");

    setup_agents(N_AGENTS);
    int threads_per_group = 10 * 10; 
    int num_groups_needed = (N_AGENTS + threads_per_group - 1) / threads_per_group; 

    setup_quad();
    
    while(!glfwWindowShouldClose(window)) {
        if (key_pressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);
        
        int window_width, window_height;
        glfwGetFramebufferSize(window, &window_width, &window_height);
        
        glUseProgram(update_agents);
        glProgramUniform1ui(update_agents, frame_n_location, frame_number);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, *agentsBO);
        glBindImageTexture(0, screen_texture, 0, 0, 0, GL_READ_WRITE, GL_RGBA32F);
        
        glDispatchCompute(num_groups_needed, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glTextureBarrier(); 
        
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
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