#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "main.h"

uint create_and_bind_texture(uint bind_texture_type, uint wrap_method, uint filtering_method);
GLFWwindow* init_window(bool fullscreen, bool limit_60_fps);
void load_shader_file(const char* filename, GLuint shader_program, GLenum shader_type);
void update_time(void);

#endif