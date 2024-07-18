#include <GL/glew.h>     
#include <GLFW/glfw3.h>  
#include <stdio.h>       
#include <stdlib.h>      
#include <delo2d.h>      

int main() 
{
    Context context;
    SpriteBatch sb;

    delo2d_context_init(&context,1920,1080,"delo2d");

    delo2d_sprite_batch_init(&sb,100);
    
    while (!glfwWindowShouldClose(context.window)) 
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(context.window);
    }

    glfwDestroyWindow(context.window);
    glfwTerminate();
    return 0;
}
