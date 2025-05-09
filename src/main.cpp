#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <glad/gl.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <shader.h>
#include <camera.h>

#define Assert(condition, message, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed at %s:%d: " message "\n", \
                    __FILE__, __LINE__, ##__VA_ARGS__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float lastX = 640, lastY = 360;
bool firstMouse = true;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

void error_callback_glfw(int error, const char* description) {
  fprintf( stderr, "GLFW ERROR: code %i msg: %s.\n", error, description );
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camera.ProcessKeyboard(RIGHT, deltaTime);
}

unsigned int loadTexture(const char *path);

int main( void ) {
  // Register the error callback function that we wrote earlier.
  glfwSetErrorCallback( error_callback_glfw );
  // Start OpenGL context and OS window using the GLFW helper library.
  if ( !glfwInit() ) {
    fprintf( stderr, "ERROR: could not start GLFW3.\n" );
    return 1;
  }

  // Request an OpenGL 4.1, core, context from GLFW.
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

  //glfwWindowHint( GLFW_SAMPLES, 8 );
  bool full_screen = false;
  GLFWmonitor *mon = NULL;
  int win_w = 1280, win_h = 720;

  if ( full_screen ) {
    mon = glfwGetPrimaryMonitor();
  
    const GLFWvidmode* mode = glfwGetVideoMode( mon );
  
    // Hinting these properties lets us use "borderless full screen" mode.
    glfwWindowHint( GLFW_RED_BITS, mode->redBits );
    glfwWindowHint( GLFW_GREEN_BITS, mode->greenBits );
    glfwWindowHint( GLFW_BLUE_BITS, mode->blueBits );
    glfwWindowHint( GLFW_REFRESH_RATE, mode->refreshRate );
  
    win_w = mode->width;  // Use our 'desktop' resolution for window size
    win_h = mode->height; // to get a 'full screen borderless' window.
  }

  // Create a window on the operating system, then tie the OpenGL context to it.
  GLFWwindow *window = glfwCreateWindow(
    win_w,
    win_h,
    "Extended OpenGL Init",
    mon,
    NULL
  );
  if ( !window ) {
    fprintf( stderr, "ERROR: Could not open window with GLFW3.\n" );
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent( window );
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback); 
  glfwSetScrollCallback(window, scroll_callback);    
                                  
  // Start Glad, so we can call OpenGL functions.
  int version_glad = gladLoadGL( glfwGetProcAddress );
  if ( version_glad == 0 ) {
    fprintf( stderr, "ERROR: Failed to initialize OpenGL context.\n" );
    return 1;
  }
  printf( "Loaded OpenGL %i.%i\n", GLAD_VERSION_MAJOR( version_glad ), GLAD_VERSION_MINOR( version_glad ) );

  // Try to call some OpenGL functions, and print some more version info.
  printf( "Renderer: %s.\n", glGetString( GL_RENDERER ) );
  printf( "OpenGL version supported %s.\n", glGetString( GL_VERSION ) );

  float vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
  };
  unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
  };
  
  glm::vec3 cubePositions[] = {
      glm::vec3( 0.0f,  0.0f,  0.0f),
      glm::vec3( 2.0f,  5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f),
      glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3( 2.4f, -0.4f, -3.5f),
      glm::vec3(-1.7f,  3.0f, -7.5f),
      glm::vec3( 1.3f, -2.0f, -2.5f),
      glm::vec3( 1.5f,  2.0f, -2.5f),
      glm::vec3( 1.5f,  0.2f, -1.5f),
      glm::vec3(-1.3f,  1.0f, -1.5f)
  };

  unsigned int VBO, cubeVAO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(cubeVAO);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
  unsigned int lightCubeVAO;
  glGenVertexArrays(1, &lightCubeVAO);
  glBindVertexArray(lightCubeVAO);

  // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  /*unsigned int ebo;
  glGenBuffers(1, &ebo);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

  // load textures (we now use a utility function to keep the code more organized)
  // -----------------------------------------------------------------------------
  unsigned int diffuseMap = loadTexture("../assets/textures/container2.png");
  unsigned int specularMap = loadTexture("../assets/textures/container2_specular.png");

  double prev_s = glfwGetTime();  // Set the initial 'previous time'.
  double title_countdown_s = 0.2;

  glEnable(GL_DEPTH_TEST); 

  Shader lightingShader("lighting.vert", "lighting.frag");
  Shader lightCubeShader("lightCube.vert", "lightCube.frag");
  
  lightingShader.use();
  lightingShader.setInt("material.diffuse", 0);
  lightingShader.setInt("material.specular", 1);

  glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
    glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
  };

  glm::vec3 pointLightColors[] = {
      glm::vec3(1.0f, 0.6f, 0.0f),
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 1.0, 0.0),
      glm::vec3(0.2f, 0.2f, 1.0f)
  };

  while ( !glfwWindowShouldClose( window ) ) {
    processInput(window);
    
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;  

    // Print the FPS, but not every frame, so it doesn't flicker too much.
    title_countdown_s -= deltaTime;
    if ( title_countdown_s <= 0.0 && deltaTime > 0.0 ) {
      double fps        = 1.0 / deltaTime;

      // Create a string and put the FPS as the window title.
      char tmp[256];
      sprintf_s( tmp, "FPS: %.2lf %.2lf ms", fps, deltaTime );
      glfwSetWindowTitle(window, tmp );
      title_countdown_s = 0.2;
    }

    // Wipe the drawing surface clear.
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    lightingShader.use();
    lightingShader.setVec3("viewPos", camera.Position);
    lightingShader.setFloat("material.shininess", 32.0f);

    /*
       Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
       the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
       by defining light types as classes and set their values in there, or by using a more efficient uniform approach
       by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
    */
    // directional light
    lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    lightingShader.setVec3("pointLights[0].ambient", pointLightColors[0].x * 0.1f, pointLightColors[0].y * 0.1f, pointLightColors[0].z * 0.1f);
    lightingShader.setVec3("pointLights[0].diffuse", pointLightColors[0].x, pointLightColors[0].y, pointLightColors[0].z);
    lightingShader.setVec3("pointLights[0].specular", pointLightColors[0].x, pointLightColors[0].y, pointLightColors[0].z);
    lightingShader.setFloat("pointLights[0].constant", 1.0f);
    lightingShader.setFloat("pointLights[0].linear", 0.09f);
    lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader.setVec3("pointLights[1].ambient", pointLightColors[1].x * 0.1f, pointLightColors[1].y * 0.1f, pointLightColors[1].z * 0.1f);
    lightingShader.setVec3("pointLights[1].diffuse", pointLightColors[1].x, pointLightColors[1].y, pointLightColors[1].z);
    lightingShader.setVec3("pointLights[1].specular", pointLightColors[1].x, pointLightColors[1].y, pointLightColors[1].z);
    lightingShader.setFloat("pointLights[1].constant", 1.0f);
    lightingShader.setFloat("pointLights[1].linear", 0.09f);
    lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
    // point light 3
    lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
    lightingShader.setVec3("pointLights[2].ambient", pointLightColors[2].x * 0.1f, pointLightColors[2].y * 0.1f, pointLightColors[2].z * 0.1f);
    lightingShader.setVec3("pointLights[2].diffuse", pointLightColors[2].x, pointLightColors[2].y, pointLightColors[2].z);
    lightingShader.setVec3("pointLights[2].specular", pointLightColors[2].x, pointLightColors[2].y, pointLightColors[2].z);
    lightingShader.setFloat("pointLights[2].constant", 1.0f);
    lightingShader.setFloat("pointLights[2].linear", 0.09f);
    lightingShader.setFloat("pointLights[2].quadratic", 0.032f);
    // point light 4
    lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
    lightingShader.setVec3("pointLights[3].ambient", pointLightColors[3].x * 0.1f, pointLightColors[3].y * 0.1f, pointLightColors[3].z * 0.1f);
    lightingShader.setVec3("pointLights[3].diffuse", pointLightColors[3].x, pointLightColors[3].y, pointLightColors[3].z);
    lightingShader.setVec3("pointLights[3].specular", pointLightColors[3].x, pointLightColors[3].y, pointLightColors[3].z);
    lightingShader.setFloat("pointLights[3].constant", 1.0f);
    lightingShader.setFloat("pointLights[3].linear", 0.09f);
    lightingShader.setFloat("pointLights[3].quadratic", 0.032f);
    // spotLight
    lightingShader.setVec3("spotLight.position", camera.Position);
    lightingShader.setVec3("spotLight.direction", camera.Front);
    lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLight.constant", 1.0f);
    lightingShader.setFloat("spotLight.linear", 0.09f);
    lightingShader.setFloat("spotLight.quadratic", 0.032f);
    lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)win_w / (float)win_h, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);

    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    lightingShader.setMat4("model", model);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);  

    // render the cube
    glBindVertexArray(cubeVAO);
    for(unsigned int i = 0; i < 10; i++)
    {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      float angle = 20.0f * i;
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      lightingShader.setMat4("model", model);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // also draw the lamp object(s)
    lightCubeShader.use();
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setMat4("view", view);

    // we now draw as many light bulbs as we have point lights.
    glBindVertexArray(lightCubeVAO);
    for (unsigned int i = 0; i < 4; i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[i]);
        model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        lightCubeShader.setMat4("model", model);
        lightCubeShader.setVec3("lightColor", pointLightColors[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // Put the stuff we've been drawing onto the visible area.
    glfwSwapBuffers( window );
    glfwPollEvents(); // Update window events.
  }
  // de-allocate all resources once they've outlived their purpose:
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteVertexArrays(1, &lightCubeVAO);
  glDeleteBuffers(1, &VBO);
  // Close OpenGL window, context, and any other GLFW resources.
  glfwTerminate();
  return 0;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}