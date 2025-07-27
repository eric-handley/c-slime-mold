#ifndef SAVE_H
#define SAVE_H

#include "main.h"
#include "libs/mman.h"
#include <fcntl.h>
#include <unistd.h>

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

int open_fd(char* filename, int size, bool file_exists) {
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("save.c: open_fd()");
        return fd;
    }
    
    // Only truncate if file doesn't exist or we're saving
    if (!file_exists && ftruncate(fd, size) == -1) {
        perror("save.c: ftruncate()");
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
    sprintf(filename, "save/%s/%d.%s", type, key_pressed, type);
    create_directory_path(filename);
    
    bool file_exists = (access(filename, F_OK) == 0);
    int fd = open_fd(filename, sizeof_t, file_exists);

    char* file_ptr = mmap(NULL, sizeof_t, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_ptr == MAP_FAILED) {
        perror("save.c: MovementSettings_T mmap()");
        exit(1);
    }

    if (do_load_operation) {
        memcpy(settings_ptr, file_ptr, sizeof_t);
    } else {
        memcpy(file_ptr, settings_ptr, sizeof_t);
    }
    
    munmap(file_ptr, sizeof_t);
    close(fd);
}

#endif