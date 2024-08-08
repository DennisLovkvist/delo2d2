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
    delo2d_context_init(&context,1920,1080,"delo2d");



    SpriteBatch sb;
    delo2d_renderer_sprite_init(&sb,100,&context);


    SpriteBatch sb2;
    delo2d_renderer_sprite_init(&sb2,100,&context);

    Camera camera;
    delo2d_camera_init(&camera,&context);

    Texture tex;
    delo2d_texture_load(&tex,"textures/sample.png");
    sb.texture = &tex;

    Color c = (Color){1,1,1,1};
    Matrix44 t = matrix44_scale(64/2,64/2,1);
    Vector2f o = (Vector2f){0,0};
    Rectangle_f sr = (Rectangle_f){0,0,64,64};

    delo2d_renderer_sprite_add(&sb,&c,&t,&o,&sr);

o.x = 200;
o.y = 0;

    delo2d_renderer_sprite_add(&sb,&c,&t,&o,&sr);

    RenderTarget rt;

    delo2d_render_target_init(&rt,1920,1080);


    sb2.texture = &rt.texture;


    Color c2 = (Color){1,1,1,1};
    Matrix44 t2 = matrix44_scale(1920/2,1080/2,1);
    Vector2f o2 = (Vector2f){0,0};
    Rectangle_f sr2 = (Rectangle_f){0,0,1920,1080};

    delo2d_renderer_sprite_add(&sb2,&c2,&t2,&o2,&sr2);


    float time = 0;
    while (!glfwWindowShouldClose(context.window)) 
    {
        glfwPollEvents();

        glBindFramebuffer(GL_FRAMEBUFFER, rt.fbo);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);



        delo2d_sprite_transform(64,64, &sb.transforms[1],(Vector2f){1,1},(Vector2f){0,0},0.6);
        sb.change_mask |= (1 << 1);
        
        delo2d_renderer_sprite_update(&sb);
        delo2d_renderer_sprite_render(&sb, &camera.projection);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        delo2d_renderer_sprite_update(&sb2);
        delo2d_renderer_sprite_render(&sb2, &camera.projection);

        delo2d_gl_check_error();

    //delo2d_camera_rotate(&camera,0.01);

        time += 0.1f;
        glfwSwapBuffers(context.window);
    }

    glfwDestroyWindow(context.window);
    glfwTerminate();
    return 0;
}
