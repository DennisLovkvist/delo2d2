#include <GL/glew.h>     
#include <GLFW/glfw3.h>  
#include <stdio.h>       
#include <stdlib.h>      
#include <delo2d.h>  
#include <math.h>    

int main() 
{
    int bit_position = 0;
    uint8_t byte = 0b01111110; 
    unsigned char bit = (byte >> bit_position) & 1;

    // Print the bit
    printf("Bit at position %d: %d\n", bit_position, bit);

    Context context;
    SpriteBatch sb;

    delo2d_context_init(&context,1920,1080,"delo2d");



    delo2d_renderer_sprite_init(&sb,100,&context);

    Camera camera;
    delo2d_camera_init(&camera,&context);

    Texture tex;
    delo2d_texture_load(&tex,"textures/sample.png");
    sb.texture = &tex;

    Color c = (Color){1,1,1,1};
    Matrix44 t = matrix44_scale(64,64,1);
    Vector2f o = (Vector2f){200,0};
    Rectangle_f sr = (Rectangle_f){0,0,64,64};

    delo2d_renderer_sprite_add(&sb,&c,&t,&o,&sr);

o.x = 400;
o.y = 0;

    delo2d_renderer_sprite_add(&sb,&c,&t,&o,&sr);


    float time = 0;
    while (!glfwWindowShouldClose(context.window)) 
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        delo2d_renderer_sprite_update(&sb);
        delo2d_renderer_sprite_render(&sb, &camera.projection);
        delo2d_gl_check_error();

    //delo2d_camera_rotate(&camera,0.01);

        time += 0.1f;

        delo2d_sprite_transform(&sb.transforms[1],(Vector2f){1,1},(Vector2f){0,0},0.6);
        sb.change_mask |= (1 << 1);
        glfwSwapBuffers(context.window);
    }

    glfwDestroyWindow(context.window);
    glfwTerminate();
    return 0;
}
