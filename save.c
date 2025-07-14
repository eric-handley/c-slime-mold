#ifndef SAVE_H
#define SAVE_H

#include "main.h"

typedef enum FieldType {
    FIELD_TYPE_FLOAT,
    FIELD_TYPE_UINT32,
    FIELD_TYPE_UINT32_HEX,
} FieldType;

typedef struct SettingsField {
    char* field_name;
    FieldType type;
    union {
        float* float_ptr;
        uint32_t* uint32_ptr;
    } value;
} SettingsField;

#define N_MOVEMENT_FIELDS 5
#define N_SPECIES_FIELDS 17

SettingsField movement_fields_save[N_MOVEMENT_FIELDS];
SettingsField species_fields_save[N_SPECIES_FIELDS];

void init_save_struct() {
    SettingsField movement_fields_tmp[] = {
        {"speed", FIELD_TYPE_FLOAT, .value.float_ptr = &MovementSettings->speed},
        {"turn_randomness", FIELD_TYPE_FLOAT, .value.float_ptr = &MovementSettings->turn_randomness},
        {"turn_speed", FIELD_TYPE_FLOAT, .value.float_ptr = &MovementSettings->turn_speed},
        {"sample_angle", FIELD_TYPE_FLOAT, .value.float_ptr = &MovementSettings->sample_angle},
        {"sample_dist", FIELD_TYPE_FLOAT, .value.float_ptr = &MovementSettings->sample_dist},
    };

    SettingsField species_fields_tmp[17];
    species_fields_tmp[0] = (SettingsField){"n_species", FIELD_TYPE_UINT32, .value.uint32_ptr = &SpeciesSettings->n_species};
    
    for_range(0, 16, i) {
        char* string = calloc(50, sizeof(char));
        sprintf(string, "species_colours[%d]", i);
        species_fields_tmp[i + 1] = (SettingsField){string, FIELD_TYPE_UINT32_HEX, .value.uint32_ptr = &SpeciesSettings->species_colours[i]};
    }

    memcpy(movement_fields_save, movement_fields_tmp, sizeof(movement_fields_tmp));
    memcpy(species_fields_save, species_fields_tmp, sizeof(species_fields_tmp));
}

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#endif

// Function to create directories recursively
int create_directory_path(const char* path) {
    char* dir_path = malloc(strlen(path) + 1);
    strcpy(dir_path, path);
    
    // Find the last slash to get directory path
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash == NULL) {
        free(dir_path);
        return 0;  // No directory to create
    }
    
    *last_slash = '\0';  // Terminate string at last slash
    
    // Create directories recursively
    char* temp_path = malloc(strlen(dir_path) + 1);
    strcpy(temp_path, dir_path);
    
    for (char* p = temp_path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';  // Temporarily end the string
            mkdir(temp_path, 0755);  // Create this level
            *p = '/';   // Restore the slash
        }
    }
    
    // Create the final directory
    int result = mkdir(temp_path, 0755);
    
    free(dir_path);
    free(temp_path);
    return result;
}

void save_settings_fields(SettingsField* fields, int sizeof_fields, const char* filename) {
    int count = sizeof_fields / sizeof(SettingsField);
    
    // Create directory if it doesn't exist
    create_directory_path(filename);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not create/open file '%s' for writing\n", filename);
        return;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s=", fields[i].field_name);
        switch (fields[i].type) {
            case FIELD_TYPE_FLOAT:
                fprintf(file, "%f\n", *fields[i].value.float_ptr);
                break;
            case FIELD_TYPE_UINT32:
                fprintf(file, "%u\n", *fields[i].value.uint32_ptr);
                break;
            case FIELD_TYPE_UINT32_HEX:
                fprintf(file, "#%06X\n", *fields[i].value.uint32_ptr);
                break;
        }
    }
    
    fclose(file);
}

void load_settings_fields(SettingsField* fields, int sizeof_fields, const char* filename) {
    int count = sizeof_fields / sizeof(SettingsField);
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file '%s' for reading\n", filename);
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* equals = strchr(line, '=');
        if (!equals) continue;
        
        *equals = '\0';
        char* field_name = line;
        char* value_str = equals + 1;
        
        for (int i = 0; i < count; i++) {
            if (strcmp(fields[i].field_name, field_name) == 0) {
                switch (fields[i].type) {
                    case FIELD_TYPE_FLOAT:
                        *fields[i].value.float_ptr = strtof(value_str, NULL);
                        break;
                    case FIELD_TYPE_UINT32:
                        *fields[i].value.uint32_ptr = strtoul(value_str, NULL, 10);
                        break;
                    case FIELD_TYPE_UINT32_HEX: // Handle different possible hex code formats
                        char* hex_start = value_str;
                        if (*hex_start == '#') hex_start++;
                        *fields[i].value.uint32_ptr = strtoul(hex_start, NULL, 16);
                        break;
                }
                break;
            }
        }
    }
    
    fclose(file);
}

void save_settings(SettingsType which, int key_pressed) {
    char filename[100];
    char* type = (which == MOVEMENT) ? "movement" : "species";
    sprintf(filename, "save/%s/%d.%s", type, key_pressed, type);

    if (which == MOVEMENT) {
        save_settings_fields(movement_fields_save, sizeof(movement_fields_save), filename);
    } else {
        save_settings_fields(species_fields_save, sizeof(species_fields_save), filename);
    }
}

void load_settings(SettingsType which, int key_pressed) {
    char filename[100];
    char* type = (which == MOVEMENT) ? "movement" : "species";
    sprintf(filename, "save/%s/%d.%s", type, key_pressed, type);

    if (which == MOVEMENT) {
        load_settings_fields(movement_fields_save, sizeof(movement_fields_save), filename);
    } else {
        load_settings_fields(species_fields_save, sizeof(species_fields_save), filename);
    }
}

#endif