#include "main.h"
#include "graphics.h"
#include "utils.h"
#include "settings.h"

GLuint agentsBO;
GLuint quadVAO, quadVBO;
GLuint frameBO;
uint64_t TIME = 0;

const uint NUM_AGENTS = 500000;
const float RES_SCALE_FACTOR = 2;
const uint TEXTURE_WIDTH  = 1920 * RES_SCALE_FACTOR;
const uint TEXTURE_HEIGHT = 1080 * RES_SCALE_FACTOR;

float fade_factor = 0.001f;
bool autoswap_mode = false;
float autoswap_time_interval = 5;
bool autotweak_mode = false;
float autotweak_time_interval = 0.3;

float mouse_x = 0.0f, mouse_y = 0.0f;
bool mouse_pressed = false;
bool right_mouse_pressed = false;

pthread_t autoswap_thread;
pthread_t autotweak_thread;
bool autoswap_thread_running = false;
bool autotweak_thread_running = false;
pthread_mutex_t settings_mutex = PTHREAD_MUTEX_INITIALIZER;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mouse_pressed = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        right_mouse_pressed = (action == GLFW_PRESS);
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouse_x = (float)xpos * RES_SCALE_FACTOR;
    mouse_y = (float)(1080 - ypos) * RES_SCALE_FACTOR;
}

bool settings_need_update = false;

void* autoswap_thread_func(void* arg) {
    while (autoswap_thread_running) {
        if (autoswap_mode) {
            pthread_mutex_lock(&settings_mutex);
            randomize_movement_settings();
            randomize_species_settings();
            settings_need_update = true;
            pthread_mutex_unlock(&settings_mutex);
        }
        usleep(autoswap_time_interval * 1000000);
    }
    return NULL;
}

void* autotweak_thread_func(void* arg) {
    while (autotweak_thread_running) {
        if (autotweak_mode) {
            pthread_mutex_lock(&settings_mutex);
            tweak_movement_settings();
            tweak_species_settings();
            settings_need_update = true;
            pthread_mutex_unlock(&settings_mutex);
        }
        usleep(autotweak_time_interval * 1000000);
    }
    return NULL;
}

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
    srand(time(NULL));
    GLFWwindow* window = init_window(true, true);
    update_time();    

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
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

    uint filtering = (RES_SCALE_FACTOR <= 1.0f) ? GL_NEAREST : GL_LINEAR;
    uint screen_texture = create_and_bind_texture(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, filtering);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    uint texture_uniform             = glGetUniformLocation(post_processing_program, "screenTexture"    );
    uint frame_number_uniform        = glGetUniformLocation(update_agents_program,   "frameNumber"      );
    uint time_uniform                = glGetUniformLocation(update_agents_program,   "time"             );
    uint mouse_pos_uniform           = glGetUniformLocation(update_agents_program,   "mousePos"         );
    uint mouse_pressed_uniform       = glGetUniformLocation(update_agents_program,   "mousePressed"     );
    uint right_mouse_pressed_uniform = glGetUniformLocation(update_agents_program,   "rightMousePressed");
    uint fade_factor_uniform         = glGetUniformLocation(post_processing_program, "fadeFactor"       );
    uint mouse_pos_frag_uniform      = glGetUniformLocation(post_processing_program, "mousePos"         );
    uint mouse_pressed_frag_uniform  = glGetUniformLocation(post_processing_program, "mousePressed"     );
    uint reset_circle_uniform        = glGetUniformLocation(update_agents_program,   "resetToCircle"    );
    uint frame_number                = 0;
    
    uint32_t n_work_groups = (MovementSettings->n_agents + LOCAL_SIZE - 1) / LOCAL_SIZE;
    
    glGenFramebuffers(1, &frameBO);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);

    autoswap_thread_running = true;
    autotweak_thread_running = true;
    pthread_create(&autoswap_thread, NULL, autoswap_thread_func, NULL);
    pthread_create(&autotweak_thread, NULL, autotweak_thread_func, NULL);

    while(!glfwWindowShouldClose(window)) {
        update_time();
        
        if (settings_need_update) {
            pthread_mutex_lock(&settings_mutex);
            update_shared_settings();
            settings_need_update = false;
            pthread_mutex_unlock(&settings_mutex);
        }
        
        int window_width, window_height;
        glfwGetFramebufferSize(window, &window_width, &window_height);
        
        glBindFramebuffer(GL_FRAMEBUFFER, frameBO);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        
        glUseProgram(post_processing_program);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glUniform1i(texture_uniform, 0);
        glUniform1f(fade_factor_uniform, fade_factor);
        glUniform2f(mouse_pos_frag_uniform, mouse_x, mouse_y);
        glUniform1i(mouse_pressed_frag_uniform, mouse_pressed ? 1 : 0);
        
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        glUseProgram(update_agents_program);
        
        if (RESET_AGENT_POSITIONS) {
            frame_number = 0;
            RESET_AGENT_POSITIONS = false;
        }

        glProgramUniform1ui(update_agents_program, frame_number_uniform, frame_number);
        glProgramUniform1ui(update_agents_program, time_uniform, (int)clock());
        glProgramUniform2f(update_agents_program, mouse_pos_uniform, mouse_x, mouse_y);
        glProgramUniform1ui(update_agents_program, mouse_pressed_uniform, mouse_pressed ? 1 : 0);
        glProgramUniform1ui(update_agents_program, right_mouse_pressed_uniform, right_mouse_pressed ? 1 : 0);
        glProgramUniform1ui(update_agents_program, reset_circle_uniform, RESET_TO_CIRCLE ? 1 : 0);
        
        if (RESET_TO_CIRCLE) {
            // Tweak settings for better circle spreading
            if (MovementSettings->turn_randomness > 0.05f) {
                MovementSettings->turn_randomness *= 0.3f; // Reduce high randomness
            }
            if (MovementSettings->speed < 1.0f) {
                MovementSettings->speed = 1.5f; // Ensure minimum speed
            }
            if (MovementSettings->turn_speed > 0.2f) {
                MovementSettings->turn_speed *= 0.7f; // Reduce excessive turning
            }
            fade_factor = MovementSettings->fade_factor;
            RESET_TO_CIRCLE = false;
        }
        
        if (CLEAR_SCREEN) {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBO);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            CLEAR_SCREEN = false;
        }
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, agentsBO);
        glBindImageTexture(0, screen_texture, 0, 0, 0, GL_READ_WRITE, GL_RGBA32F);
        
        glDispatchCompute(n_work_groups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame_number += 1;
    }

    autoswap_thread_running = false;
    autotweak_thread_running = false;
    pthread_join(autoswap_thread, NULL);
    pthread_join(autotweak_thread, NULL);

    glfwTerminate();
}