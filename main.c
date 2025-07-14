#define LOCAL_SIZE 16 // Make sure to change in compute.comp
#define NUM_AGENTS 50000
#define RESOLUTION_SCALE_FACTOR 1

#include "main.h"

const uint TEXTURE_WIDTH  = 1920 * RESOLUTION_SCALE_FACTOR;
const uint TEXTURE_HEIGHT = 1080 * RESOLUTION_SCALE_FACTOR;

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
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * MovementSettings->n_agents, NULL, GL_DYNAMIC_DRAW); 
}

int main(int argc, char* argv[]) {
    GLFWwindow* window = init_window(true, true); // Fullscreen, limit to 60fps

    pthread_t clock_thread_idx;
    pthread_create(&clock_thread_idx, NULL, clock_thread, window);    

    glfwSetKeyCallback(window, key_callback);
    init_settings();
    init_save_struct();
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

    uint texture_uniform      = glGetUniformLocation(post_processing_program, "screenTexture");
    uint frame_number_uniform = glGetUniformLocation(update_agents_program,   "frameNumber"  );
    uint time_uniform         = glGetUniformLocation(update_agents_program,   "time"         );
    uint frame_number = 0;
    
    uint32_t n_work_groups = (MovementSettings->n_agents + LOCAL_SIZE - 1) / LOCAL_SIZE;  // ceiling division

    while(!glfwWindowShouldClose(window)) {
        int window_width, window_height;
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glGenFramebuffers(1, &frameBO);
        
        glBindFramebuffer(GL_FRAMEBUFFER, frameBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        
        // Fade & blur pixels
        glUseProgram(post_processing_program);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glUniform1i(texture_uniform, 0);
        
        // Draw updated texture to quad
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        
        // Update agent positions and draw
        glUseProgram(update_agents_program);
        
        if (RESET_FRAME_COUNTER) {
            frame_number = 0;
            RESET_FRAME_COUNTER = false;
        }

        // Update frame number, bind agent buffer object and screen texture
        glProgramUniform1ui(update_agents_program, frame_number_uniform, frame_number);
        glProgramUniform1ui(update_agents_program, time_uniform, (int)clock());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, agentsBO);
        glBindImageTexture(0, screen_texture, 0, 0, 0, GL_READ_WRITE, GL_RGBA32F);
        
        glDispatchCompute(n_work_groups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame_number += 1;
    }

    glfwTerminate();
}