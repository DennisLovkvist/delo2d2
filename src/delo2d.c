#include <delo2d.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t delo2d_context_init(Context *context, uint16_t width, uint16_t height, char *window_title)
{
    if (!glfwInit()) 
    {
        printf("GLFW initialization failed!");
        glfwTerminate();
        return DELO_FAILURE;
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
        return DELO_FAILURE;
    }

    glfwMakeContextCurrent(context->window);

    if (glewInit() != GLEW_OK) 
    {
        printf("GLEW initialization failed!");
        glfwDestroyWindow(context->window);
        glfwTerminate();
        return DELO_FAILURE;
    }
    
    int buffer_width, buffer_height;
    glfwGetFramebufferSize(context->window, &buffer_width, &buffer_height);
    glViewport(0, 0, buffer_width, buffer_height);


    context->back_buffer_width  = buffer_width;
    context->back_buffer_height = buffer_height;
}

uint8_t delo2d_sprite_batch_init(SpriteBatch *sb
                                ,uint32_t     capacity
                                )
{
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        return DELO_FAILURE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &sb->vao);
    glBindVertexArray(sb->vao);

    GLfloat vertices[] = 
    {
    // positions     // texCoords
    -1.0f, -1.0f,  0.0f, 0.0f,  // bottom-left
     1.0f, -1.0f,  1.0f, 0.0f,  // bottom-right
     1.0f,  1.0f,  1.0f, 1.0f,  // top-right
    -1.0f,  1.0f,  0.0f, 1.0f   // top-left
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &sb->vbo_vertices);
    glGenBuffers(1, &sb->vbo_colors);
    glGenBuffers(1, &sb->vbo_transforms);
    glGenBuffers(1, &sb->vbo_offsets);
    glGenBuffers(1, &sb->vbo_src_rects);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_vertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_colors);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(5 * sizeof(float)));  
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_offsets);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(7 * sizeof(float)));  
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_src_rects);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(11 * sizeof(float)));  
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_transforms);
    for (int32_t i = 0; i < 4; i++) 
    {
        glEnableVertexAttribArray(5 + i);
        glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, (void*)(sizeof(float) * 4 * i));
        glVertexAttribDivisor(5 + i, 1);
    }

    glUseProgram(0);

    sb->capacity = capacity;
    sb->count = 0;

    sb->colors     = malloc(sizeof(Color)      *capacity);
    sb->transforms = malloc(sizeof(Matrix44)   *capacity);
    sb->offsets    = malloc(sizeof(Vector2f)   *capacity);
    sb->src_rects  = malloc(sizeof(Rectangle_f)*capacity);

    sb->change_mask = 0b11111111;

}
uint8_t delo2d_sprite_batch_update(SpriteBatch *sb
                                  ,Color       *colors
                                  ,Matrix44    *transforms
                                  ,Vector2f    *offsets
                                  ,Rectangle_f *src_rects
                                  ,uint32_t     count
                                  )
{
    if(sb->change_mask & 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_colors);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Color)*count, (float*)colors, GL_STATIC_DRAW); 
    }

    if(sb->change_mask & 1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_transforms);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix44)*count, (float*)transforms, GL_STATIC_DRAW); 
    }

    if(sb->change_mask & 2)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_offsets);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2f)*count, (float*)offsets, GL_STATIC_DRAW); 
    }

    if(sb->change_mask & 3)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_src_rects);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Rectangle_f)*count, (float*)src_rects, GL_STATIC_DRAW); 
    } 

    sb->count = count;
}

uint8_t delo2d_sprite_batch_render(SpriteBatch *sb)
{
    /*------------------Draw instances-----------------*/
    
    glBindVertexArray(sb->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sb->texture->renderer_id);
    glUseProgram(sb->shader);
    glUniformMatrix4fv(glGetUniformLocation(sb->shader,"u_mvp"),1,GL_FALSE,&sb->projection.x11);

    glDrawArraysInstanced(GL_QUADS, 0, 4, sb->count);

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
                               ,int32_t      index
                               )
{
    if(index < sb->capacity)
    {
        sb->colors    [index] = *color;
        sb->transforms[index] = *transform;
        sb->offsets   [index] = *offset;
        sb->src_rects [index] = *src_rect;
        sb->change_mask = 0b11111111;
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