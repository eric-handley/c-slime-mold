#include "main.h"

#define SCALE_FACTOR 1
const uint TEXTURE_WIDTH  = 1920 * SCALE_FACTOR;
const uint TEXTURE_HEIGHT = 1080 * SCALE_FACTOR;

void init_settings() {
    Settings = malloc(sizeof(Settings_T));

    Settings->n_agents = 100000;
    Settings->n_species = 4;  // Max 16 species & colours
    
    uint32_t species_colours[Settings->n_species];
    for_range(0, Settings->n_species, i) {
        species_colours[i] = random_int(0x333333, 0xFFFFFF);
    }
    
    // Or set colours manually:
    // uint32_t species_colours[] = {
    //     random_int(0x333333, 0xFFFFFF),
    //     random_int(0x333333, 0xFFFFFF),
    //     random_int(0x333333, 0xFFFFFF),
    //     random_int(0x333333, 0xFFFFFF)
    // };
    
    Settings->speed = 1;
    Settings->turn_randomness = 0.02f;
    Settings->turn_speed = 0.01f;
    Settings->sample_angle = 0.4f * (float)M_PI;
    Settings->sample_dist = 20;
    Settings->debug = false;
    
    memcpy(Settings->species_colours, species_colours, sizeof(species_colours));
    glGenBuffers(1, &settingsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, settingsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Settings_T), Settings, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, settingsUBO);
}

void randomize_settings() {
    uint32_t species_colours[Settings->n_species];
    for_range(0, Settings->n_species, i) {
        species_colours[i] = random_int(0x333333, 0xFFFFFF);
    }
    Settings->speed = random_int(0.2, 3);
    Settings->turn_randomness = random_float(0.0001, 0.1);
    Settings->turn_speed = random_float(0.0001, 0.1);
    Settings->sample_angle = random_float(0.05, 0.5) * (float)M_PI;
    Settings->sample_dist = random_int(3, 50);
    
    memcpy(Settings->species_colours, species_colours, sizeof(species_colours));
    update_shared_settings();
}

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
    if (action == GLFW_RELEASE) {
        switch (key)
        {
        case GLFW_KEY_SPACE:
            randomize_settings();
            break;
        
        default:
            break;
        }
    }
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
    init_settings();
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