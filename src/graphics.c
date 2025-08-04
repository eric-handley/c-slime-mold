#include "main.h"
#include "graphics.h"

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

GLFWwindow* init_window(bool fullscreen, bool limit_60_fps) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : NULL;
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Slime Mold Simulation", monitor, NULL);

    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    
    if (limit_60_fps) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }

    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        return NULL;
    }
    
    return window;
}

void load_shader_file(const char* filename, GLuint shader_program, GLenum shader_type) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = malloc(length + 1);
    fread(source, 1, length, file);
    source[length] = '\0';

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const char**)&source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("Shader compilation failed (%s): %s\n", filename, info_log);
    }

    glAttachShader(shader_program, shader);

    glDeleteShader(shader);
    free(source);
    fclose(file);
}


void update_time() {
    static clock_t start_time = 0;
    static bool initialized = false;
    
    if (!initialized) {
        start_time = clock();
        initialized = true;
        TIME = 0;
    } else {
        clock_t current_time = clock();
        TIME = (double)(current_time - start_time) / CLOCKS_PER_SEC;
    }
}