#include <delo2d.h>

uint8_t sprite_batch_init(SpriteBatch *sb, uint32_t capacity)
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

    sb->colors     = malloc(sizeof(Vector2f)*capacity);
    sb->transforms = malloc(sizeof(Matrix44)*capacity);
    sb->offsets    = malloc(sizeof(Vector2f)*capacity);
    sb->src_rects  = malloc(sizeof(Vector2f)*capacity);
}