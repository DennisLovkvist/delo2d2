#pragma once
#include <stdint.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define DELO_SUCCESS 0
#define DELO_ERROR   1

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
typedef struct Color Color;
struct Color
{
    float r;
    float g;
    float b;
    float a;
};
typedef struct Matrix44 Matrix44;
struct Matrix44
{	// Column-major order
	float x11, x21, x31, x41,
		  x12, x22, x32, x42,
		  x13, x23, x33, x43,
		  x14, x24, x34, x44;
};

typedef struct Camera Camera;
struct Camera
{
    Context *context;
    Matrix44 projection; 
    Vector2f position;
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
    GLuint uniform_projection;

    GLuint vao;
    GLuint vbo_vertices;
    GLuint vbo_colors;
    GLuint vbo_transforms;
    GLuint vbo_offsets;
    GLuint vbo_src_rects;

    Color       *colors;
    Matrix44    *transforms;
    Vector2f    *offsets;
    Rectangle_f *src_rects;

    uint8_t change_mask;
    
    Context *context;

    GLuint shader;
    Texture *texture;

};
typedef struct RenderTarget RenderTarget;
struct RenderTarget
{
    uint32_t fbo;
    Texture texture;
};

void delo2d_gl_clear_error();
void delo2d_gl_check_error();

uint8_t delo2d_context_init(Context *context, uint16_t width, uint16_t height, char *window_title);

uint8_t delo2d_renderer_sprite_init(SpriteBatch *sb
                                ,uint32_t     capacity
                                ,Context     *context
                                );


uint8_t delo2d_renderer_sprite_update(SpriteBatch *sb);

uint8_t delo2d_renderer_sprite_render(SpriteBatch *sb, Matrix44 *projection);

uint8_t delo2d_renderer_sprite_add(SpriteBatch *sb
                               ,Color       *color
                               ,Matrix44    *transform
                               ,Vector2f    *offset
                               ,Rectangle_f *src_rect
                               );

void delo2d_render_target_init(RenderTarget *rt, uint32_t width, uint32_t height);

//matrix code begin
Matrix44 matrix44_identity();
Matrix44 matrix44_skew(float sx, float sy);
Matrix44 matrix44_perspective();
Matrix44 matrix44_scale(float x, float y, float z);
Matrix44 matrix44_translation(float x, float y, float z);
Matrix44 matrix44_rotation_z(float theta);
Matrix44 matrix44_rotation_y(float theta);
Matrix44 matrix44_rotation_x(float theta);
Matrix44 matrix44_multiply(Matrix44 a, Matrix44 b);
Matrix44 matrix44_add(Matrix44 a, Matrix44 b);
Vector2f matrix44_multilpy_vector2f(Vector2f vector, Matrix44 transform);
Matrix44 matrix44_invert(Matrix44 input);
Matrix44 matrix44_orthographic_projection(float l,float r,float t,float b,float f,float n);
float matrix44_calculate_sub_determinant(Matrix44 m, int a, int b, int c, int d);
float matrix44_calculate_determinant(const Matrix44* m);
float* matrix44_to_gl_matrix(Matrix44 *matrix);
//matrix code end

//texture code begin
uint8_t delo2d_texture_load(Texture *texture, char file_path[]);
//texture code end

//shader code begin
uint8_t delo2d_shader_from_files(char *path_shader_vert,char *path_shader_frag, uint32_t *shader_id);
//shader code end

//camera code begin
uint8_t delo2d_camera_init(Camera *camera, Context *context);
void delo2d_camera_move(Camera *camera,float tx, float ty);
void delo2d_camera_zoom(Camera *camera,float z);
void delo2d_camera_rotate(Camera *camera,float t);
//camera code end

void delo2d_sprite_transform(uint32_t width,uint32_t height, Matrix44 *transform, Vector2f scale, Vector2f skew, float rotation);