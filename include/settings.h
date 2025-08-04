#ifndef SETTINGS_H
#define SETTINGS_H

#include "main.h"

#define MAX_SPECIES 16

typedef enum SettingsType {
    MOVEMENT,
    SPECIES
} SettingsType;

extern int number_keys[10];
extern GLuint movementSettingsUBO;
extern GLuint speciesSettingsUBO;
extern bool RESET_AGENT_POSITIONS;
extern bool RESET_TO_CIRCLE;
extern bool CLEAR_SCREEN;

typedef struct MovementSettings_T {
    uint32_t n_agents;
    float    speed;
    float    turn_randomness;
    float    turn_speed;
    float    sample_angle;
    float    sample_dist;
    uint32_t debug;
    float    fade_factor;
} MovementSettings_T;

typedef struct SpeciesSettings_T {
    uint32_t species_colours[MAX_SPECIES];
    uint32_t n_species;
} SpeciesSettings_T;

extern MovementSettings_T* MovementSettings;
extern SpeciesSettings_T* SpeciesSettings;

void init_settings(void);
void randomize_species_settings(void);
void randomize_movement_settings(void);
void tweak_movement_settings(void);
void tweak_species_settings(void);
void update_shared_settings(void);
void rw_settings(SettingsType which, int key_pressed, bool do_load_operation);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif