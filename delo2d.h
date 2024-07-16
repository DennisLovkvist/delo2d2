#pragma once
#include <stdint.h>
#include <GL/glew.h>

#define DELO_SUCCESS 0
#define DELO_FAILURE 1

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
typedef struct Matrix44 Matrix44;
struct Matrix44
{	// Column-major order
	float x11, x21, x31, x41,
		  x12, x22, x32, x42,
		  x13, x23, x33, x43,
		  x14, x24, x34, x44;
};
typedef struct SpriteBatch SpriteBatch;
struct SpriteBatch
{
    GLuint capacity;
    GLuint shader_program;
    GLuint uniform_projection;

    GLuint vao;
    GLuint vbo_vertices;
    GLuint vbo_colors;
    GLuint vbo_transforms;
    GLuint vbo_offsets;
    GLuint vbo_src_rects;

    float *positions;
    float *colors;
    float *transforms;
    float *offsets;
    float *src_rects;

};
