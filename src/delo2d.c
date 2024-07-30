
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION

#include <delo2d.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stb_image.h>


void delo2d_gl_clear_error()
{
    GLenum error;
    int maxIterations = 1000; // or another reasonable limit
    int iteration = 0;

    while ((error = glGetError()) != GL_NO_ERROR && iteration < maxIterations)
    {
        printf("OpenGL Error: %d\n", error);
        iteration++;
    }

    if (iteration >= maxIterations)
    {
        printf("GLClearError: Reached max iteration limit while clearing errors.\n");
    }
}
void delo2d_gl_check_error()
{
    GLenum error;
    while((error = glGetError()))
    {
        printf("%i",error);
        printf("%c",'\n');
    }
}

uint8_t delo2d_context_init(Context *context, uint16_t width, uint16_t height, char *window_title)
{
    if (!glfwInit()) 
    {
        printf("GLFW initialization failed!");
        glfwTerminate();
        return DELO_ERROR;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    context->window = glfwCreateWindow(width, height, window_title, NULL, NULL);

    if (!context->window) 
    {
        printf("GLFW window creation failed!");
        glfwTerminate();
        return DELO_ERROR;
    }

    glfwMakeContextCurrent(context->window);

    if (glewInit() != GLEW_OK) 
    {
        printf("GLEW initialization failed!");
        glfwDestroyWindow(context->window);
        glfwTerminate();
        return DELO_ERROR;
    }
    
    int buffer_width, buffer_height;
    glfwGetFramebufferSize(context->window, &buffer_width, &buffer_height);
    glViewport(0, 0, buffer_width, buffer_height);


    context->back_buffer_width  = buffer_width;
    context->back_buffer_height = buffer_height;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glEnable( GL_BLEND ); 
}

uint8_t delo2d_sprite_batch_init(SpriteBatch *sb
                                ,uint32_t     capacity
                                ,Context     *context
                                )
{
    
    sb->context = context;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        return DELO_ERROR;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &sb->vao);
    glBindVertexArray(sb->vao);

    GLfloat vertices[] = 
    {
    // positions     // texCoords
    -1.0f, -1.0f,0.0f,  0.0f, 0.0f,  // bottom-left
     1.0f, -1.0f,0.0f,  1.0f, 0.0f,  // bottom-right
     1.0f,  1.0f,0.0f,  1.0f, 1.0f,  // top-right
    -1.0f,  1.0f,0.0f,  0.0f, 1.0f   // top-left
    };

    glGenBuffers(1, &sb->vbo_vertices);
    glGenBuffers(1, &sb->vbo_colors);
    glGenBuffers(1, &sb->vbo_transforms);
    glGenBuffers(1, &sb->vbo_offsets);
    glGenBuffers(1, &sb->vbo_src_rects);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_vertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_colors);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);  
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_offsets);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);  
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_src_rects);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);  
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_transforms);
    for (int32_t i = 0; i < 4; i++) 
    {
        glEnableVertexAttribArray(5 + i);
        glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, (void*)(sizeof(float) * 4 * i));
        glVertexAttribDivisor(5 + i, 1);
    }
glBindVertexArray(0);
    glUseProgram(0);

    sb->capacity = capacity;
    sb->count = 0;

    sb->colors     = malloc(sizeof(Color)      *capacity);
    sb->transforms = malloc(sizeof(Matrix44)   *capacity);
    sb->offsets    = malloc(sizeof(Vector2f)   *capacity);
    sb->src_rects  = malloc(sizeof(Rectangle_f)*capacity);

    sb->change_mask = 0b11111111;

    delo2d_shader_from_files("shaders/sprite_batch.vert","shaders/sprite_batch.frag",&sb->shader);

    sb->texture = NULL;

}
uint8_t delo2d_sprite_batch_update(SpriteBatch *sb)
{
    if((sb->change_mask >> 0) & 1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_colors);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Color)*sb->count, (float*)sb->colors, GL_STATIC_DRAW);
        sb->change_mask &= ~(0 << 0);
    }

    if((sb->change_mask >> 1) & 1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_transforms);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix44)*sb->count, (float*)sb->transforms, GL_STATIC_DRAW); 
        sb->change_mask &= ~(1 << 0);
    }

    if((sb->change_mask >> 2) & 1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_offsets);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2f)*sb->count, (float*)sb->offsets, GL_STATIC_DRAW); 
        sb->change_mask &= ~(2 << 0);
    }

    if((sb->change_mask >> 3) & 1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_src_rects);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Rectangle_f)*sb->count, (float*)sb->src_rects, GL_STATIC_DRAW); 
        sb->change_mask &= ~(3 << 0);
    } 
}

uint8_t delo2d_sprite_batch_render(SpriteBatch *sb, Matrix44 *projection)
{
    /*------------------Draw instances-----------------*/
    glBindVertexArray(sb->vao);

    if(sb->texture != NULL)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sb->texture->renderer_id);
    }
    glUseProgram(sb->shader);
    glUniformMatrix4fv(glGetUniformLocation(sb->shader,"u_mvp"),1,GL_FALSE,&projection->x11);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, sb->count);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    /*------------------Draw instances-----------------*/
}

uint8_t delo2d_sprite_batch_add(SpriteBatch *sb
                               ,Color       *color
                               ,Matrix44    *transform
                               ,Vector2f    *offset
                               ,Rectangle_f *src_rect
                               )
{

    int32_t index = sb->count;

    if(index < sb->capacity)
    {
        uint16_t texture_width  = (sb->texture == NULL) ? 0.0:sb->texture->width;
        uint16_t texture_height = (sb->texture == NULL) ? 0.0:sb->texture->height;

        sb->colors    [index]        = *color;
        sb->transforms[index]        = *transform;
        sb->offsets   [index].x      = offset->x/(float)sb->context->back_buffer_width;
        sb->offsets   [index].y      = offset->y/(float)sb->context->back_buffer_height;
        sb->src_rects [index].x      = src_rect->x/texture_width;
        sb->src_rects [index].y      = src_rect->y/texture_height;
        sb->src_rects [index].width  = src_rect->width/texture_width;
        sb->src_rects [index].height = src_rect->height/texture_height;
        sb->change_mask              = 0b11111111;
        sb->count ++;
        printf("%f\n",sb->offsets   [index].x);
    }
}

uint8_t delo2d_sprite_batch_modify_color(SpriteBatch *sb
                                        ,Color       *color
                                        ,int32_t      index
                                        )
{
    if(index < sb->capacity)
    {
        sb->change_mask |= (0 << 1);
        sb->colors[index] = *color;
    }

}
uint8_t delo2d_sprite_batch_modify_transform(SpriteBatch *sb
                                            ,Matrix44    *transform
                                            ,int32_t      index
                                            )
{
    if(index < sb->capacity)
    {
        sb->change_mask |= (1 << 1);
        sb->transforms[index] = *transform;
    }

}
uint8_t delo2d_sprite_batch_modify_offset(SpriteBatch *sb
                                         ,Vector2f    *offset
                                         ,int32_t      index
                                         )
{
    if(index < sb->capacity)
    {
        sb->change_mask |= (2 << 1);
        sb->offsets[index] = *offset;
    }

}
uint8_t delo2d_sprite_batch_modify_src_rect(SpriteBatch *sb
                                           ,Rectangle_f *src_rect
                                           ,int32_t      index
                                           )
{
    if(index < sb->capacity)
    {
        sb->change_mask |= (3 << 1);
        sb->src_rects[index] = *src_rect;
    }

}
//shader code begin
void delo2d_shader_check_compile_status(GLuint shader)
{
    GLint compileStatus = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0)
        {
            char *infoLog = (char *)malloc(infoLogLength);
            if (infoLog)
            {
                glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
                printf("Shader compile error: %s\n", infoLog);
                free(infoLog);
            }
            else
            {
                printf("Failed to allocate memory for shader compile log\n");
            }
        }
        else
        {
            printf("Shader compile error: (no additional information available)\n");
        }
    }
    else
    {
        printf("Shader compiled successfully.\n");
    }
}
uint8_t delo2d_shader_compile(uint32_t type, char *shader_source_code, uint32_t *id)
{
    delo2d_gl_clear_error();
    
    if (shader_source_code == NULL) 
    {
        fprintf(stderr, "Error: Shader source code is NULL\n");
        return DELO_ERROR;
    }

    *id = glCreateShader(type);
    if (*id == 0) 
    {
        fprintf(stderr, "Error creating shader\n");
        return DELO_ERROR;
    }

    const char *src = shader_source_code;
    glShaderSource(*id, 1, &src, NULL);


    glCompileShader(*id);

    GLint compile_status;

    glGetShaderiv(*id, GL_COMPILE_STATUS, &compile_status);
    
    delo2d_shader_check_compile_status(*id);

    if (compile_status == GL_FALSE) 
    {
        fprintf(stderr,"Shader failed to compile\n");
        glDeleteShader(*id);
        *id = 0;
        return DELO_ERROR;
    }

    return DELO_SUCCESS;
}

uint8_t delo2d_shader_create(char *vertex_shader_source_code, char *fragment_shader_source_code, uint32_t *program)
{
    delo2d_gl_clear_error();

    *program = glCreateProgram();

    if (*program == 0) 
    {
        fprintf(stderr, "Error creating shader program\n");
        return DELO_ERROR;
    }

    uint32_t vs;
    uint32_t fs;

    printf("Compiling vertex shader...\n");
    if (delo2d_shader_compile(GL_VERTEX_SHADER, vertex_shader_source_code, &vs) == DELO_ERROR) 
    {
        printf("Could not compiler vertex shader.\n");
        glDeleteProgram(*program);
        return DELO_ERROR;
    }
    printf("Compiling fragment shader...\n");
    if (delo2d_shader_compile(GL_FRAGMENT_SHADER, fragment_shader_source_code, &fs) == DELO_ERROR) 
    {
        printf("Could not compiler fragment shader.\n");
        glDeleteProgram(*program);
        return DELO_ERROR;
    }

    glAttachShader(*program, vs);
    glAttachShader(*program, fs);

    glLinkProgram(*program);
    glValidateProgram(*program);

    GLint link_status;
    glGetProgramiv(*program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) 
    {
        fprintf(stderr, "Shader program linking failed.\n");
        
        glDeleteProgram(*program);
        return DELO_ERROR;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return DELO_SUCCESS;
}
uint8_t delo2d_shader_load(const char *path, char **source_code)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) 
    {
        fprintf(stderr, "Error opening file %s\n", path);
        return DELO_ERROR;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    *source_code = (char*)malloc(fsize + 1); // Allocate memory for source code
    if (*source_code == NULL) 
    {
        fprintf(stderr, "Error allocating memory\n");
        fclose(f);
        return DELO_ERROR;
    }
    
    fread(*source_code, fsize, 1, f);
    fclose(f);
    (*source_code)[fsize] = '\0'; // Null-terminate the string
    
    return DELO_SUCCESS; // Return success
}
uint8_t delo2d_shader_from_files(char *path_shader_vert,char *path_shader_frag, uint32_t *shader_id)
{
    *shader_id = 0;
    char *source_code_vert;
    char *source_code_frag;
    
    if(delo2d_shader_load(path_shader_vert,&source_code_vert) == DELO_ERROR)
    {
        printf("[delo2d] Could not load shader file %s\n",path_shader_vert);
        return DELO_ERROR;
    }
    if(delo2d_shader_load(path_shader_frag,&source_code_frag) == DELO_ERROR)
    {
        printf("[delo2d] Could not load shader file %s\n",path_shader_frag);
        return DELO_ERROR;
    }

    uint8_t status = DELO_SUCCESS;

    if(delo2d_shader_create(source_code_vert,source_code_frag,shader_id) == DELO_ERROR)
    {
        if(source_code_vert != NULL)free(source_code_vert);
        if(source_code_frag != NULL)free(source_code_frag);
        return DELO_ERROR;
    }
    else
    {
        return DELO_SUCCESS;   
    }
}
//shader code end

//texture code begin
uint8_t delo2d_texture_load(Texture *texture, char file_path[])
{
    stbi_set_flip_vertically_on_load(0);
    texture->local_buffer = stbi_load(file_path, &texture->width, &texture->height, &texture->bytes_per_pixel, 4);

    if (texture->local_buffer == NULL)
    {
        fprintf(stderr, "Error loading texture: %s\n", file_path);
        return DELO_ERROR;
    }

    glGenTextures(1, &texture->renderer_id);
    if (texture->renderer_id == 0)
    {
        fprintf(stderr, "Error generating texture ID\n");
        stbi_image_free(texture->local_buffer);
        return DELO_ERROR;
    }

    glBindTexture(GL_TEXTURE_2D, texture->renderer_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->local_buffer);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(texture->local_buffer);

    texture->initialized = 1;
    return DELO_SUCCESS;
}

//texture code end

//matrix code begin
Matrix44 matrix44_identity()
{
    struct Matrix44 matrix = 
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    return matrix;
}

Matrix44 matrix44_scale(float x, float y, float z) 
{
    struct Matrix44 matrix = 
    {
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1,
	};
    return matrix;
}
float* matrix44_to_gl_matrix(Matrix44 *matrix) 
{
	return &matrix->x11;
}
Matrix44 matrix44_translation(float x, float y, float z) 
{
	struct Matrix44 matrix = 
    {
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1
	};
    return matrix;
}

Matrix44 matrix44_rotation_z(float theta) 
{
	struct Matrix44 matrix = 
    {
		 cos(theta), sin(theta), 0, 0,
		-sin(theta), cos(theta), 0, 0,
		 0,          0,          1, 0,
		 0,          0,          0, 1
	};
    return matrix;
}

Matrix44 matrix44_rotation_y(float theta) 
{
	struct Matrix44 matrix = 
    {
		cos(theta), 0, -sin(theta), 0,
		0,          1,  0,          0,
		sin(theta), 0,  cos(theta), 0,
		0,          0,  0,          1
	};
    return matrix;
}

Matrix44 matrix44_rotation_x(float theta) 
{
	struct Matrix44 matrix = 
    {
		1,  0,           0,          0,
		0,  cos(theta),  sin(theta), 0,
		0, -sin(theta),  cos(theta), 0,
		0,  0,           0,          1
	};
    return matrix;
}
Matrix44 matrix44_multiply(Matrix44 a, Matrix44 b) 
{
	struct Matrix44 matrix = 
    {
        a.x11 * b.x11 + a.x12 * b.x21 + a.x13 * b.x31 + a.x14 * b.x41,
		a.x21 * b.x11 + a.x22 * b.x21 + a.x23 * b.x31 + a.x24 * b.x41,
		a.x31 * b.x11 + a.x32 * b.x21 + a.x33 * b.x31 + a.x34 * b.x41,
		a.x41 * b.x11 + a.x42 * b.x21 + a.x43 * b.x31 + a.x44 * b.x41,

		a.x11 * b.x12 + a.x12 * b.x22 + a.x13 * b.x32 + a.x14 * b.x42,
		a.x21 * b.x12 + a.x22 * b.x22 + a.x23 * b.x32 + a.x24 * b.x42,
		a.x31 * b.x12 + a.x32 * b.x22 + a.x33 * b.x32 + a.x34 * b.x42,
		a.x41 * b.x12 + a.x42 * b.x22 + a.x43 * b.x32 + a.x44 * b.x42,
  
		a.x11 * b.x13 + a.x12 * b.x23 + a.x13 * b.x33 + a.x14 * b.x43,
		a.x21 * b.x13 + a.x22 * b.x23 + a.x23 * b.x33 + a.x24 * b.x43,
		a.x31 * b.x13 + a.x32 * b.x23 + a.x33 * b.x33 + a.x34 * b.x43,
		a.x41 * b.x13 + a.x42 * b.x23 + a.x43 * b.x33 + a.x44 * b.x43,

		a.x11 * b.x14 + a.x12 * b.x24 + a.x13 * b.x34 + a.x14 * b.x44,
		a.x21 * b.x14 + a.x22 * b.x24 + a.x23 * b.x34 + a.x24 * b.x44,
		a.x31 * b.x14 + a.x32 * b.x24 + a.x33 * b.x34 + a.x34 * b.x44,
		a.x41 * b.x14 + a.x42 * b.x24 + a.x43 * b.x34 + a.x44 * b.x44,
	};
    return matrix;
}
Matrix44 matrix44_add(Matrix44 a, Matrix44 b)
{
    struct Matrix44 matrix = 
    {
        a.x11 + b.x11,
        a.x12 + b.x12,
        a.x13 + b.x13,
        a.x14 + b.x14,
        a.x21 + b.x21,
        a.x22 + b.x22,
        a.x23 + b.x23,
        a.x24 + b.x24,
        a.x31 + b.x31,
        a.x32 + b.x32,
        a.x33 + b.x33,
        a.x34 + b.x34,
        a.x41 + b.x41,
        a.x42 + b.x42,
        a.x43 + b.x43,
        a.x44 + b.x44
    };
    return matrix;
}
Matrix44 matrix44_perspective() 
{
    //Shit copied from the internet
	float r = 0.56; 
	float t = 0.33;
	float n = 1; 
	float f = 60;

	struct Matrix44 matrix = 
    {
		n / r, 0, 	  0, 					  0,
		0, 	   n / t, 0, 					  0,
		0, 	   0, 	  (-f - n) / (f - n)   , -1,
		0, 	   0, 	  (2 * f * n) / (n - f),  0
	};
    return matrix;
}
Vector2f matrix44_multilpy_vector2f(Vector2f vector, Matrix44 transform)
{
    Vector2f result = 
    {
        (vector.x * transform.x11) + (vector.y * transform.x21) + (1 * transform.x31),
        (vector.x * transform.x12) + (vector.y * transform.x22) + (1 * transform.x32)
    };
    return result;
}
Matrix44 matrix44_orthographic_projection(float l,float r,float t,float b,float f,float n)
{
    struct Matrix44 matrix = 
    {
        2.0f/(r-l),0,0,-((r+l)/(r-l)),

        0,2.0/(t-b),0,-((t+b)/(t-b)),

        0,0,-2 / (f-n),-((f+n)/(f-n)),
        0,0,0,1
    };
    return matrix;
}
float matrix44_calculate_sub_determinant(Matrix44 m, int a, int b, int c, int d) 
{
    float a11 = m.x11;
    float a12 = m.x12;
    float a13 = m.x13;

    float a21 = m.x21;
    float a22 = m.x22;
    float a23 = m.x23;

    float a31 = m.x31;
    float a32 = m.x32;
    float a33 = m.x33;

    float det = a11 * (a22 * a33 - a23 * a32) -
                a12 * (a21 * a33 - a23 * a31) +
                a13 * (a21 * a32 - a22 * a31);

    return det;
}
float matrix44_calculate_determinant(const Matrix44* m) 
{
    float a11 = m->x11;
    float a12 = m->x12;
    float a13 = m->x13;
    float a14 = m->x14;

    float a21 = m->x21;
    float a22 = m->x22;
    float a23 = m->x23;
    float a24 = m->x24;

    float a31 = m->x31;
    float a32 = m->x32;
    float a33 = m->x33;
    float a34 = m->x34;

    float a41 = m->x41;
    float a42 = m->x42;
    float a43 = m->x43;
    float a44 = m->x44;

    float det = a11 * (a22 * (a33 * a44 - a34 * a43) - a23 * (a32 * a44 - a34 * a42) + a24 * (a32 * a43 - a33 * a42)) -
                a12 * (a21 * (a33 * a44 - a34 * a43) - a23 * (a31 * a44 - a34 * a41) + a24 * (a31 * a43 - a33 * a41)) +
                a13 * (a21 * (a32 * a44 - a34 * a42) - a22 * (a31 * a44 - a34 * a41) + a24 * (a31 * a42 - a32 * a41)) -
                a14 * (a21 * (a32 * a43 - a33 * a42) - a22 * (a31 * a43 - a33 * a41) + a23 * (a31 * a42 - a32 * a41));

    return det;
}

Matrix44 matrix44_invert(Matrix44 input) 
{
    float det = matrix44_calculate_determinant(&input);

    if (det == 0.0f) 
    {
        printf("Matrix is not invertible (determinant is zero).\n");
        return input;
    }

    Matrix44 result = {0};

    result.x11 =  matrix44_calculate_sub_determinant(input, 1, 2, 3, 1) / det;
    result.x21 = -matrix44_calculate_sub_determinant(input, 0, 2, 3, 1) / det;
    result.x31 =  matrix44_calculate_sub_determinant(input, 0, 1, 3, 1) / det;
    result.x41 = -matrix44_calculate_sub_determinant(input, 0, 1, 2, 1) / det;

    result.x12 = -matrix44_calculate_sub_determinant(input, 1, 2, 3, 0) / det;
    result.x22 =  matrix44_calculate_sub_determinant(input, 0, 2, 3, 0) / det;
    result.x32 = -matrix44_calculate_sub_determinant(input, 0, 1, 3, 0) / det;
    result.x42 =  matrix44_calculate_sub_determinant(input, 0, 1, 2, 0) / det;

    result.x13 =  matrix44_calculate_sub_determinant(input, 1, 2, 3, 3) / det;
    result.x23 = -matrix44_calculate_sub_determinant(input, 0, 2, 3, 3) / det;
    result.x33 =  matrix44_calculate_sub_determinant(input, 0, 1, 3, 3) / det;
    result.x43 = -matrix44_calculate_sub_determinant(input, 0, 1, 2, 3) / det;

    result.x14 = -matrix44_calculate_sub_determinant(input, 1, 2, 3, 2) / det;
    result.x24 =  matrix44_calculate_sub_determinant(input, 0, 2, 3, 2) / det;
    result.x34 = -matrix44_calculate_sub_determinant(input, 0, 1, 3, 2) / det;
    result.x44 =  matrix44_calculate_sub_determinant(input, 0, 1, 2, 2) / det;

    return result;
}
//matrix code end

//camera code begin
uint8_t delo2d_camera_init(Camera *camera, Context *context)
{
    camera->projection = matrix44_orthographic_projection((float)0.0f
                                                         ,(float)context->back_buffer_width
                                                         ,(float)0.0f
                                                         ,(float)context->back_buffer_height
                                                         ,(float)1
                                                         ,(float)-1
                                                         );
    
    camera->projection = matrix44_multiply(camera->projection,matrix44_translation(1,-1,0));

    camera->context = context;
}
uint8_t delo2d_camera_move(Camera *camera,float tx, float ty)
{ 
    float w  = camera->context->back_buffer_width;
    float h  = camera->context->back_buffer_height;

    Matrix44 translation = matrix44_translation(tx/w,ty/h,0);

    camera->projection = matrix44_multiply(camera->projection,translation);
}  
//camera code end
