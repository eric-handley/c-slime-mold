#include "main.h"

uint quadVAO, quadVBO;

void setup_quad() {
    float quadVertices[] = {
        // positions   // texCoords
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);  // Changed from 3 to 2
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void render_quad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

int main(int argc, char* argv[]) {
    GLFWwindow* window = init_window(false, true);

    while (glGetError() != GL_NO_ERROR);

    pthread_t clock_thread_idx;
    pthread_create(&clock_thread_idx, NULL, clock_thread, window);    

    uint screen_shader_program = glCreateProgram();
    load_shader_file("shaders/quad.vert", screen_shader_program, GL_VERTEX_SHADER);
    load_shader_file("shaders/quad.frag", screen_shader_program, GL_FRAGMENT_SHADER);
    glLinkProgram(screen_shader_program);
    
    uint compute_shader_program  = glCreateProgram();
    load_shader_file("shaders/compute.comp", compute_shader_program , GL_COMPUTE_SHADER);
    glLinkProgram(compute_shader_program);

    uint time_location = glGetUniformLocation(compute_shader_program, "t");

    const uint TEXTURE_WIDTH = 1920, TEXTURE_HEIGHT = 1080;
    uint texture = create_texture(GL_TEXTURE_2D, GL_TEXTURE0, GL_CLAMP_TO_EDGE, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    
    setup_quad();
    
    while(!glfwWindowShouldClose(window)) {
        if (key_pressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);
        
        glUseProgram(compute_shader_program );
        glUniform1f(time_location, TIME);
        glBindImageTexture(0, texture, 0, 0, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute((uint)TEXTURE_WIDTH/10, (uint)TEXTURE_HEIGHT/10, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        glUseProgram(screen_shader_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        render_quad();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
}