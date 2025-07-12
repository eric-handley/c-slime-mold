#ifndef SETTINGS_H
#define SETTINGS_H

#define RESOLUTION_SCALE_FACTOR 1.5
#define MAX_SPECIES 16

GLuint settingsUBO;
bool RESET_FRAME_COUNTER = false;

typedef struct Settings_T {
    uint32_t n_agents;                     // offset 0
    uint32_t n_species;                    // offset 4
    uint32_t padding[2];                   // offset 8, 12 (pad to 16-byte boundary)
    uint32_t species_colours[MAX_SPECIES]; // offset 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76
    float    speed;                        // offset 80
    float    turn_randomness;              // offset 84
    float    turn_speed;                   // offset 88
    float    sample_angle;                 // offset 92
    float    sample_dist;                  // offset 96
    uint32_t debug;                        // offset 100
} Settings_T;

Settings_T* Settings;

void init_settings() {
    Settings = malloc(sizeof(Settings_T));

    Settings->n_agents = 100000;
    Settings->n_species = 2;  // Max 16 species & colours
    
    uint32_t species_colours[MAX_SPECIES];
    for_range(0, MAX_SPECIES, i) {
        species_colours[i] = random_int(0x111111, 0xFFFFFF);
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

void randomize_settings(bool change_colours, bool change_movement) {
    if (change_colours) {
        uint32_t species_colours[MAX_SPECIES];
        for_range(0, MAX_SPECIES, i) {
            species_colours[i] = random_int(0x111111, 0xFFFFFF);
        }
        memcpy(Settings->species_colours, species_colours, sizeof(species_colours));
    }

    if (change_movement) {
        Settings->speed = random_float(0.6, 7.5);
        Settings->turn_randomness = random_float(0.0001, 0.5);
        Settings->turn_speed = random_float(0.0001, 0.5);
        Settings->sample_angle = random_float(0.05, 0.5) * (float)M_PI;
        Settings->sample_dist = random_int(3, 50);
    }
}

void update_shared_settings() {
    glBindBuffer(GL_UNIFORM_BUFFER, settingsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Settings_T), Settings);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
        switch (key)
        {
        case GLFW_KEY_1:
            randomize_settings(false, true);
            break;
        
        case GLFW_KEY_2:
            randomize_settings(true, true);
            break;

        case GLFW_KEY_3:
            randomize_settings(true, false);
            break;
        
        case GLFW_KEY_UP:
            if (Settings->n_species < 16) {
                Settings->n_species += 1;
            }
            break;
        
        case GLFW_KEY_DOWN:
            if (Settings->n_species > 1) {
                Settings->n_species -= 1;
            }
            break;

        case GLFW_KEY_SPACE:
            RESET_FRAME_COUNTER = true;
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

            case GLFW_KEY_W:
                Settings->speed += 0.1;
                update_shared_settings();
                break;

            case GLFW_KEY_S:
                if (Settings->speed > 0) {
                    Settings->speed -= 0.1;
                    update_shared_settings();
                }
                break;

            case GLFW_KEY_D:
                Settings->turn_speed += 0.001;
                update_shared_settings();
                if (Settings->debug) printf("turn_speed: %f\n", Settings->turn_speed);
                break;

            case GLFW_KEY_A:
                if (Settings->turn_speed > 0) {
                    Settings->turn_speed -= 0.001;
                    update_shared_settings();
                    if (Settings->debug) printf("turn_speed: %f\n", Settings->turn_speed);
                }
                break;

            case GLFW_KEY_E:
                Settings->turn_randomness += 0.001;
                update_shared_settings();
                break;

            case GLFW_KEY_Q:
                if (Settings->turn_randomness > 0) {
                    Settings->turn_randomness -= 0.001;
                    update_shared_settings();
                }
                break;

            case GLFW_KEY_T:
                Settings->sample_dist += 0.001;
                update_shared_settings();
                break;

            case GLFW_KEY_R:
                if (Settings->sample_dist > 0) {
                    Settings->sample_dist -= 0.001;
                    update_shared_settings();
                }
                break;
            
            case GLFW_KEY_G:
                Settings->sample_angle += 0.001;
                update_shared_settings();
                break;

            case GLFW_KEY_F:
                if (Settings->sample_angle > 0) {
                    Settings->sample_angle -= 0.001;
                    update_shared_settings();
                }
                break;

            default:
                break;
        }
    }
    update_shared_settings();
}

#endif