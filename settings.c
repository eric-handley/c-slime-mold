#ifndef SETTINGS_H
#define SETTINGS_H

#include "main.h" // To suppress vscode errors, for some reason

int number_keys[] = {
    GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
    GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0
};

GLuint movementSettingsUBO;
GLuint speciesSettingsUBO;
bool RESET_FRAME_COUNTER = false;

typedef struct MovementSettings_T {
    uint32_t n_agents;
    float    speed;
    float    turn_randomness;
    float    turn_speed;
    float    sample_angle;
    float    sample_dist;
    uint32_t debug;
} MovementSettings_T;

typedef struct SpeciesSettings_T {
    uint32_t species_colours[MAX_SPECIES];
    uint32_t n_species;
} SpeciesSettings_T;

MovementSettings_T* MovementSettings;
SpeciesSettings_T* SpeciesSettings;

void randomize_species_settings() {
    SpeciesSettings->n_species = random_int(1, 16);
    
    uint32_t species_colours[MAX_SPECIES];
    
    for_range(0, MAX_SPECIES, i) {
        species_colours[i] = random_int(0x111111, 0xFFFFFF);
    }

    memcpy(SpeciesSettings->species_colours, species_colours, sizeof(species_colours));
}
    
void randomize_movement_settings() {
    MovementSettings->speed = random_float(0.3, 7);
    MovementSettings->turn_randomness = random_float(0.0001, 0.4);
    MovementSettings->turn_speed = random_float(0.025, 0.6);
    MovementSettings->sample_angle = random_float(0.08, 0.75);
    MovementSettings->sample_dist = random_int(1, 25);
}

void init_settings() {
    MovementSettings = malloc(sizeof(MovementSettings_T));
    SpeciesSettings  = malloc(sizeof(SpeciesSettings_T));

    MovementSettings->n_agents = NUM_AGENTS;

    randomize_movement_settings();
    randomize_species_settings();

    glGenBuffers(1, &movementSettingsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, movementSettingsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MovementSettings_T), MovementSettings, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, movementSettingsUBO);

    glGenBuffers(1, &speciesSettingsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, speciesSettingsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SpeciesSettings_T), SpeciesSettings, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, speciesSettingsUBO);
}

void update_shared_settings() {
    glBindBuffer(GL_UNIFORM_BUFFER, movementSettingsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MovementSettings_T), MovementSettings);
    
    glBindBuffer(GL_UNIFORM_BUFFER, speciesSettingsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SpeciesSettings_T), SpeciesSettings);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_I:
            randomize_movement_settings();
            break;
            
        case GLFW_KEY_O:
            randomize_species_settings(); 
            break;
            
        case GLFW_KEY_P:
            randomize_movement_settings();
            randomize_species_settings();
            break;
        
        case GLFW_KEY_V:
            if (SpeciesSettings->n_species < 16) {
                SpeciesSettings->n_species += 1;
            }
            break;
        
        case GLFW_KEY_C:
            if (SpeciesSettings->n_species > 1) {
                SpeciesSettings->n_species -= 1;
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
            MovementSettings->speed += 0.1;
            break;

        case GLFW_KEY_S:
            if (MovementSettings->speed > 0) {
                MovementSettings->speed -= 0.1;
            }
            break;

        case GLFW_KEY_D:
            MovementSettings->turn_speed += 0.001;
            break;

        case GLFW_KEY_A:
            if (MovementSettings->turn_speed > 0) {
                MovementSettings->turn_speed -= 0.001;
            }
            break;

        case GLFW_KEY_E:
            MovementSettings->turn_randomness += 0.001;
            break;

        case GLFW_KEY_Q:
            if (MovementSettings->turn_randomness > 0) {
                MovementSettings->turn_randomness -= 0.001;
            }
            break;

        case GLFW_KEY_T:
            MovementSettings->sample_dist += 0.1;
            break;

        case GLFW_KEY_R:
            if (MovementSettings->sample_dist > 0) {
                MovementSettings->sample_dist -= 0.1;
            }
            break;
        
        case GLFW_KEY_G:
            MovementSettings->sample_angle += 0.01;
            break;

        case GLFW_KEY_F:
            if (MovementSettings->sample_angle > 0) {
                MovementSettings->sample_angle -= 0.01;
            }
            break;

        default:
            for_range(0, 10, i) {
                if (key_pressed(number_keys[i])) {
                    int slot = (i == 9) ? 0 : (i + 1);  // Map index to slot: 0->1, 1->2, ..., 8->9, 9->0
                    
                    if (key_pressed(GLFW_KEY_LEFT_ALT)) {
                        if (key_pressed(GLFW_KEY_LEFT_SHIFT)) {
                            save_settings(SPECIES, slot);
                        } else {
                            load_settings(SPECIES, slot);
                        }
                    } else {
                        if (key_pressed(GLFW_KEY_LEFT_SHIFT)) {
                            save_settings(MOVEMENT, slot);
                        } else {
                            load_settings(MOVEMENT, slot);
                        }
                    }
                    break;  // Exit loop once we find the pressed key
                }
            }
            break;
        }
    }
    update_shared_settings();
}

#endif