#ifndef GL_HELPERS_H
#define GL_HELPERS_H

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>

#define key_pressed(key) glfwGetKey(window, key) == GLFW_PRESS

uint create_and_bind_texture(uint bind_texture_type, uint wrap_method, uint filtering_method) {
    uint tex;
    
    glGenTextures(1, &tex);
    glBindTexture(bind_texture_type, tex);  

    glTexParameteri(bind_texture_type, GL_TEXTURE_WRAP_S,     wrap_method     );
    glTexParameteri(bind_texture_type, GL_TEXTURE_WRAP_T,     wrap_method     );
    glTexParameteri(bind_texture_type, GL_TEXTURE_MIN_FILTER, filtering_method);
    glTexParameteri(bind_texture_type, GL_TEXTURE_MAG_FILTER, filtering_method);

    return tex;
}

void load_shader_file(const char* filename, uint shader_program, int shader_type) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* source = (char*)malloc(length + 1);
    fread(source, 1, length, file);
    source[length] = '\0';
    const char* source_const = source;
    
    uint shader = glCreateShader(shader_type);
    if (Settings->verbose) printf("Created shader ID: %u, type: %d for file: %s\n", shader, shader_type, filename);
    
    if (shader == 0) {
        printf("Failed to create shader for %s\n", filename);
        return;
    }
    
    glShaderSource(shader, 1, &source_const, NULL);
    glCompileShader(shader);
    
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (Settings->verbose) printf("Compile status for %s: %d\n", filename, status);
    
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        fprintf(stderr, "Shader compilation for [%s] failed:\n\n%s\n", filename, buffer);
        exit(1);
    }
    
    if (Settings->verbose) printf("Attaching shader %u to program %u\n", shader, shader_program);
    glAttachShader(shader_program, shader);
    
    // Check if attachment worked
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error attaching shader %s: %d\n", filename, error);
    }
    
    glDeleteShader(shader);
    free(source);
    fclose(file);
}

#endif