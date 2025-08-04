#include "main.h"
#include "settings.h"
#include "utils.h"
#ifdef _WIN32
#include "../libs/mman.h"
#endif

int number_keys[] = {
    GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
    GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0
};

GLuint movementSettingsUBO;
GLuint speciesSettingsUBO;
bool RESET_AGENT_POSITIONS = false;
bool RESET_TO_CIRCLE = false;
bool CLEAR_SCREEN = false;

MovementSettings_T* MovementSettings;
SpeciesSettings_T* SpeciesSettings;

void randomize_species_settings() {
    // SpeciesSettings->n_species = random_int(1, 16);
    
    uint32_t species_colours[MAX_SPECIES];
    
    for_range(0, MAX_SPECIES, i) {
        species_colours[i] = random_int(0x111111, 0xFFFFFF);
    }

    memcpy(SpeciesSettings->species_colours, species_colours, sizeof(species_colours));
}

static void clamp_float(float* value, float min, float max) {
    if (*value < min) *value = min;
    if (*value > max) *value = max;
}

static void clamp_uint(uint32_t* value, uint32_t min, uint32_t max) {
    if (*value < min) *value = min;
    if (*value > max) *value = max;
}

void randomize_movement_settings() {
    MovementSettings->speed = random_float(0.3, 7);
    MovementSettings->turn_randomness = random_float(0.0001, 0.25);
    MovementSettings->turn_speed = random_float(0.025, 0.6);
    MovementSettings->sample_angle = random_float(0.08, 0.75);
    MovementSettings->sample_dist = random_int(1, 25);
    MovementSettings->fade_factor = random_float(0.00005, 0.001);
    fade_factor = MovementSettings->fade_factor;
}

void tweak_movement_settings() {
    MovementSettings->speed += random_float(-0.1, 0.1);
    clamp_float(&MovementSettings->speed, 0.3f, 7.0f);
    
    MovementSettings->turn_randomness += random_float(-0.01, 0.01);
    clamp_float(&MovementSettings->turn_randomness, 0.0001f, 0.25f);
    
    MovementSettings->turn_speed += random_float(-0.02, 0.02);
    clamp_float(&MovementSettings->turn_speed, 0.025f, 0.6f);
    
    MovementSettings->sample_angle += random_float(-0.02, 0.02);
    clamp_float(&MovementSettings->sample_angle, 0.08f, 0.75f);
    
    MovementSettings->sample_dist += random_float(-1.0, 1.0);
    clamp_float(&MovementSettings->sample_dist, 1.0f, 25.0f);
    
    MovementSettings->fade_factor += random_float(-0.0002, 0.0002);
    clamp_float(&MovementSettings->fade_factor, 0.00005f, 0.001f);
    fade_factor = MovementSettings->fade_factor;
}

void tweak_species_settings() {
    // Randomly adjust number of species
    // if (random_float(0, 1) < 0.025) {
    //     int change = (random_float(0, 1) < 0.0125) ? -1 : 1;
    //     SpeciesSettings->n_species += change;
    //     clamp_uint(&SpeciesSettings->n_species, 1, 16);
    // }
    
    // Gradually shift species colors
    for (int i = 0; i < SpeciesSettings->n_species; i++) {
        uint32_t color = SpeciesSettings->species_colours[i];
        
        int r = (color >> 16) & 0xFF;
        int g = (color >> 8) & 0xFF;  
        int b = color & 0xFF;
        
        r += random_int(-5, 5);
        g += random_int(-5, 5);
        b += random_int(-5, 5);
        
        r = (r < 0x11) ? 0x11 : (r > 0xFF) ? 0xFF : r;
        g = (g < 0x11) ? 0x11 : (g > 0xFF) ? 0xFF : g;
        b = (b < 0x11) ? 0x11 : (b > 0xFF) ? 0xFF : b;
        
        SpeciesSettings->species_colours[i] = (r << 16) | (g << 8) | b;
    }
}

void init_settings() {
    MovementSettings = malloc(sizeof(MovementSettings_T));
    SpeciesSettings  = malloc(sizeof(SpeciesSettings_T));

    MovementSettings->n_agents = NUM_AGENTS;

    randomize_movement_settings();
    randomize_species_settings();

    SpeciesSettings->n_species = random_int(1, 16);    
    MovementSettings->fade_factor = fade_factor;

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

static void handle_number_key(int key, int mods) {
    for (int i = 0; i < 10; i++) {
        if (key == number_keys[i]) {
            int slot = (i == 9) ? 0 : (i + 1);
            bool is_save = (mods & GLFW_MOD_SHIFT) != 0;
            bool is_species = (mods & GLFW_MOD_ALT) != 0;
            
            rw_settings(is_species ? SPECIES : MOVEMENT, slot, !is_save);
            update_shared_settings();
            return;
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_I:     randomize_movement_settings();
                             break;

        case GLFW_KEY_O:     randomize_species_settings();
                             break;

        case GLFW_KEY_P:     randomize_movement_settings(); 
                             randomize_species_settings();
                             break;

        case GLFW_KEY_V:     SpeciesSettings->n_species++; 
                             clamp_uint(&SpeciesSettings->n_species, 1, 16);
                             break;

        case GLFW_KEY_C:     SpeciesSettings->n_species--; 
                             clamp_uint(&SpeciesSettings->n_species, 1, 16);
                             break;

        case GLFW_KEY_SPACE: CLEAR_SCREEN = true;
                             break;

        case GLFW_KEY_K:     RESET_AGENT_POSITIONS = true;
                             break;

        case GLFW_KEY_L:     RESET_TO_CIRCLE = true;
                             break;

        case GLFW_KEY_M:     autoswap_mode = !autoswap_mode;
                             if (autoswap_mode) autotweak_mode = false;
                             break;

        case GLFW_KEY_N:     autotweak_mode = !autotweak_mode;
                             if (autotweak_mode) autoswap_mode = false;
                             break;

        default:             handle_number_key(key, mods);
                             return;

        }
    }
    
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE);
                              break;
                              
        case GLFW_KEY_W:      MovementSettings->speed += 0.1f;
                              break;
                              
        case GLFW_KEY_S:      MovementSettings->speed -= 0.1f; 
                              clamp_float(&MovementSettings->speed, 0.0f, 100.0f);
                              break;
                              
        case GLFW_KEY_D:      MovementSettings->turn_speed += 0.001f;
                              break;
                              
        case GLFW_KEY_A:      MovementSettings->turn_speed -= 0.001f; 
                              clamp_float(&MovementSettings->turn_speed, 0.0f, 10.0f);
                              break;
                              
        case GLFW_KEY_E:      MovementSettings->turn_randomness += 0.001f;
                              break;
                              
        case GLFW_KEY_Q:      MovementSettings->turn_randomness -= 0.001f; 
                              clamp_float(&MovementSettings->turn_randomness, 0.0f, 1.0f);
                              break;
                              
        case GLFW_KEY_T:      MovementSettings->sample_dist += 0.1f;
                              break;
                              
        case GLFW_KEY_R:      MovementSettings->sample_dist -= 0.1f; 
                              clamp_float(&MovementSettings->sample_dist, 0.0f, 100.0f);
                              break;
                              
        case GLFW_KEY_G:      MovementSettings->sample_angle += 0.01f;
                              break;
                              
        case GLFW_KEY_F:      MovementSettings->sample_angle -= 0.01f; 
                              clamp_float(&MovementSettings->sample_angle, 0.0f, 3.14f);
                              break;
                              
        case GLFW_KEY_Z:      MovementSettings->fade_factor += 0.0001f;
                              clamp_float(&MovementSettings->fade_factor, 0.00005f, 0.001f);
                              fade_factor = MovementSettings->fade_factor;
                              break;
                              
        case GLFW_KEY_X:      MovementSettings->fade_factor -= 0.0001f;
                              clamp_float(&MovementSettings->fade_factor, 0.00005f, 0.001f);
                              fade_factor = MovementSettings->fade_factor;
                              break;
                              
        default:              handle_number_key(key, mods);
                              return;
                              
        }
    }
    update_shared_settings();
}

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#endif

int create_directory_path(const char* path) {
    char* dir_path = malloc(strlen(path) + 1);
    strcpy(dir_path, path);
    
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash == NULL) {
        free(dir_path);
        return 0;  // No directory to create
    }
    
    *last_slash = '\0';  // Terminate string at last slash
    
    char* temp_path = malloc(strlen(dir_path) + 1);
    strcpy(temp_path, dir_path);
    
    for (char* p = temp_path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';  // Temporarily end the string
            mkdir(temp_path, 0755);  // Create this level
            *p = '/';   // Restore the slash
        }
    }
    
    int result = mkdir(temp_path, 0755);
    
    free(dir_path);
    free(temp_path);
    return result;
}

int open_fd(char* filename, int size, bool file_exists) {
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("settings.c: open_fd()");
        return fd;
    }
    
    if (!file_exists && ftruncate(fd, size) == -1) {
        perror("settings.c: ftruncate()");
        close(fd);
        return -1;
    }
    
    return fd;
}


void rw_settings(SettingsType which, int key_pressed, bool do_load_operation) {  
    char* type         = (which) ? "species" : "movement";
    int sizeof_t       = (which) ? sizeof(SpeciesSettings_T) : sizeof(MovementSettings_T);
    char* settings_ptr = (which) ? (char*)(SpeciesSettings) : (char*)(MovementSettings);
    
    char filename[100];
    sprintf(filename, "data/presets/%s/%d.%s", type, key_pressed, type);
    create_directory_path(filename);
    
    bool file_exists = (access(filename, F_OK) == 0);
    int fd = open_fd(filename, sizeof_t, file_exists);

    char* file_ptr = mmap(NULL, sizeof_t, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_ptr == MAP_FAILED) {
        perror("settings.c: mmap()");
        exit(1);
    }

    if (do_load_operation) {
        memcpy(settings_ptr, file_ptr, sizeof_t);
        if (!which) {
            fade_factor = MovementSettings->fade_factor;
        }
    } else {
        if (!which) {
            MovementSettings->fade_factor = fade_factor;
        }
        memcpy(file_ptr, settings_ptr, sizeof_t);
    }
    
    munmap(file_ptr, sizeof_t);
    close(fd);
}