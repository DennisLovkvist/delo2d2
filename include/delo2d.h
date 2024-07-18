#pragma once
#include <stdint.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define DELO_SUCCESS 0
#define DELO_FAILURE 1

typedef struct Context Context;
struct Context
{
    GLint back_buffer_width,back_buffer_height,screen_width,screen_height;
    GLFWwindow* window; 
};

typedef struct Rectangle_f Rectangle_f;
struct Rectangle_f
{
    float x,y, width,height;
};
typedef struct Vector2f Vector2f;
struct Vector2f
{
    float x;
    float y;
};
typedef struct Vector3f Vector3f;
struct Vector3f
{
    float x;
    float y;
    float z;
};
typedef struct Vector4f Vector4f;
struct Vector4f
{
    float x;
    float y;
    float z;
    float w;
};
typedef struct Matrix44 Matrix44;
struct Matrix44
{	// Column-major order
	float x11, x21, x31, x41,
		  x12, x22, x32, x42,
		  x13, x23, x33, x43,
		  x14, x24, x34, x44;
};

typedef struct Texture Texture;
struct Texture
{
    uint8_t initialized;
    uint32_t renderer_id;
    unsigned char* local_buffer;
    int width,height,bytes_per_pixel;

};
typedef struct SpriteBatch SpriteBatch;
struct SpriteBatch
{
    GLuint capacity,count;
    GLuint shader_program;
    GLuint uniform_projection;

    GLuint vao;
    GLuint vbo_vertices;
    GLuint vbo_colors;
    GLuint vbo_transforms;
    GLuint vbo_offsets;
    GLuint vbo_src_rects;

    Vector4f    *colors;
    Matrix44    *transforms;
    Vector2f    *offsets;
    Rectangle_f *src_rects;

    GLuint shader;
    Matrix44 projection;
    Texture *texture;

};

uint8_t delo2d_context_init(Context *context, uint16_t width, uint16_t height, char *window_title);

uint8_t delo2d_sprite_batch_init(SpriteBatch *sb, uint32_t capacity);
