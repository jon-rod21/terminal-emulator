#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned int id;   // 0 means creation failed
} Shader;

// Reads a whole file into a malloc'd, null-terminated string.
// Returns NULL on failure. The caller must free() the result.
static char *shader_read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t n = fread(buf, 1, (size_t)size, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

// Returns 1 on success, 0 on failure (and prints the GL info log).
static int shader_check_errors(GLuint object, const char *type, int is_program) {
    GLint success;
    char info_log[1024];

    if (!is_program) {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, sizeof(info_log), NULL, info_log);
            fprintf(stderr,
                    "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n"
                    " -- --------------------------------------------------- -- \n",
                    type, info_log);
        }
    } else {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, sizeof(info_log), NULL, info_log);
            fprintf(stderr,
                    "ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n"
                    " -- --------------------------------------------------- -- \n",
                    type, info_log);
        }
    }
    return success;
}

// Loads and compiles one shader stage. Returns 0 on failure.
static GLuint shader_compile(GLenum stage, const char *path, const char *label) {
    char *source = shader_read_file(path);
    if (!source) {
        fprintf(stderr, "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: %s\n", path);
        return 0;
    }

    const char *src_ptr = source;
    GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &src_ptr, NULL);
    glCompileShader(shader);
    free(source);

    if (!shader_check_errors(shader, label, 0)) {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// C has no default arguments, so pass NULL for geometry_path if you don't have one.
// Check the result with: if (shader.id == 0) { ... }
static Shader shader_create(const char *vertex_path,
                            const char *fragment_path,
                            const char *geometry_path) {
    Shader shader = { 0 };

    GLuint vertex   = shader_compile(GL_VERTEX_SHADER,   vertex_path,   "VERTEX");
    GLuint fragment = shader_compile(GL_FRAGMENT_SHADER, fragment_path, "FRAGMENT");
    GLuint geometry = geometry_path
                    ? shader_compile(GL_GEOMETRY_SHADER, geometry_path, "GEOMETRY")
                    : 0;

    if (!vertex || !fragment || (geometry_path && !geometry)) {
        // glDeleteShader(0) is silently ignored, so this is safe for failed stages.
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteShader(geometry);
        return shader;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    if (geometry) glAttachShader(program, geometry);
    glLinkProgram(program);

    // Shaders are linked into the program now and no longer needed.
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry) glDeleteShader(geometry);

    if (!shader_check_errors(program, "PROGRAM", 1)) {
        glDeleteProgram(program);
        return shader;
    }

    shader.id = program;
    return shader;
}

static void shader_use(const Shader *s) {
    glUseProgram(s->id);
}

// Uniform setters. No std::string or GLM in C, so names are const char*
// and vectors/matrices are plain float arrays.
static void shader_set_bool(const Shader *s, const char *name, int value) {
    glUniform1i(glGetUniformLocation(s->id, name), value);
}

static void shader_set_int(const Shader *s, const char *name, int value) {
    glUniform1i(glGetUniformLocation(s->id, name), value);
}

static void shader_set_float(const Shader *s, const char *name, float value) {
    glUniform1f(glGetUniformLocation(s->id, name), value);
}

static void shader_set_vec2(const Shader *s, const char *name, float x, float y) {
    glUniform2f(glGetUniformLocation(s->id, name), x, y);
}

static void shader_set_vec3(const Shader *s, const char *name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(s->id, name), x, y, z);
}

static void shader_set_vec4(const Shader *s, const char *name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(s->id, name), x, y, z, w);
}

// mat points to 16 floats in column-major order (for cglm: pass (float *)m or &m[0][0]).
static void shader_set_mat4(const Shader *s, const char *name, const float *mat) {
    glUniformMatrix4fv(glGetUniformLocation(s->id, name), 1, GL_FALSE, mat);
}

#endif
