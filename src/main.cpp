#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "main.h"

#include "ecs/component_manager.h"
#include "assets/asset_manager.h"
#include "rendering/gpu_resource_manager.h"
#include "ecs/scene.h"
#include "shader.h"
#include "camera.h"


int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;
int win_w = 1280, win_h = 720;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 640, lastY = 360;
bool firstMouse = true;
bool cursorLocked = true;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
unsigned int numVerticesLoaded = 0;
unsigned int numTrianglesLoaded = 0;

void error_callback_glfw(int error, const char* description) {
  fprintf( stderr, "GLFW ERROR: code %i msg: %s.\n", error, description );
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
  if(cursorLocked == false) return;
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

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      cursorLocked = false;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      cursorLocked = true;
    }
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
  win_w = 1280;
  win_h = 720;

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
  
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 410");
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

  float cubeVertices[] = {
      //positions         //normals           //texture coords
      // Back face
     -0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
      0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f,   // top-right
      0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
      0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f,   // top-right
     -0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
     -0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,  // top-left
      // Front face
     -0.5f, -0.5f, 0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // bottom-left
      0.5f, -0.5f, 0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,  // bottom-right
      0.5f,  0.5f, 0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,   // top-right
      0.5f,  0.5f, 0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,   // top-right
     -0.5f,  0.5f, 0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,  // top-left
     -0.5f, -0.5f, 0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // bottom-left
      // Left face
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,   // top-right
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,  // top-left
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,  // bottom-right
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,   // top-right
      // Right face
      0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f,    // top-left
      0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right
      0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 1.0f,   // top-right
      0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right
      0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f,    // top-left
      0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f,   // bottom-left
      // Bottom face
      -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
       0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 1.0f,  // top-left
       0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   // bottom-left
       0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   // bottom-left
      -0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f,  // bottom-right
      -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
      // Top face
      -0.5f, 0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
       0.5f, 0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,   // bottom-right
       0.5f, 0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top-right
       0.5f, 0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,   // bottom-right
      -0.5f, 0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
      -0.5f, 0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f   // bottom-left
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

  float quadVertices[] = {  
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
  };	

  unsigned int VBO, cubeVAO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

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
  /*unsigned int diffuseMap = TextureFromFile("../assets/textures/container2.png");
  unsigned int specularMap = TextureFromFile("../assets/textures/container2_specular.png");*/

  double prev_s = glfwGetTime();  // Set the initial 'previous time'.
  double title_countdown_s = 0.2;

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
  stbi_set_flip_vertically_on_load(true);
  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);    
  //glEnable(GL_STENCIL_TEST);
  //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

  u32 framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  // generate texture
  u32 textureColorbuffer;
  glGenTextures(1, &textureColorbuffer);
  glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win_w, win_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach it to currently bound framebuffer object
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

  u32 rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, win_w, win_h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // screen quad VAO
  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

  //Shader lightingShader("lighting.vert", "lighting.frag");
  Shader lightCubeShader("lightCube.vert", "lightCube.frag");
  
  // load models
  // -----------
  Shader modelShader("modelShader.vert", "modelShader.frag");
  Shader screenShader("framebuffers_screen.vert", "framebuffers_screen.frag");

  screenShader.use();
  screenShader.setInt("screenTexture", 0);
  //Shader singleColorShader("modelShader.vert", "shaderSingleColor.frag");
  //Model ourModel("../assets/models/backpack/backpack.obj");

  //GameObjectManager GOManager;
  //GOManager.createGameObject("Backpack", &modelShader, &ourModel);

  //printf("GO: %s %i", GOManager.getGameObject("Backpack")->Name.c_str(), GOManager.getGameObject("Backpack")->id);

  // draw as wireframe
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  Scene scene;
  AssetManager assetManager;

  // Load a model
  ModelAssetID backpackModel = assetManager.LoadModel("../assets/models/backpack/backpack.obj");
  
  //printf("backpack materials: %d", assetManager.GetMaterial(2)->specularTexture);

  while ( !glfwWindowShouldClose( window ) ) 
  {
    glfwPollEvents(); // Update window events.
    processInput(window);

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
    {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;  

    // ===============
    // Dear ImGui START
    // ===============
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
      //ImGui::DragFloat3("position", glm::value_ptr(GOManager.getGameObject("Backpack")->position), 0.01f);
      //ImGui::DragFloat3("rotation", glm::value_ptr(GOManager.getGameObject("Backpack")->rotation), 0.01f);
      //ImGui::DragFloat3("scale", glm::value_ptr(GOManager.getGameObject("Backpack")->scale), 0.01f);
      if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);
      ImGui::Text("Loaded: %d models, %d vertices", assetManager.GetStats().modelsLoaded, assetManager.GetStats().totalVertices);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      //ImGui::Text("Vertices loaded %d Triangles loaded %d", numVerticesLoaded, numTrianglesLoaded);
      ImGui::End();
    }
    ImGui::Render();
    // ==============
    // Dear ImGui END
    // ==============

    // RENDER
    // ------
    // bind to framebuffer and draw scene as we normally would to color texture
    //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT /*| GL_STENCIL_BUFFER_BIT*/ );

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    //GOManager.update();
    //GOManager.render(view, projection);

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
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    lightCubeShader.setMat4("model", model);
    lightCubeShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    /*// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader.use();
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer); // use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);*/

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Put the stuff we've been drawing onto the visible area.
    glfwSwapBuffers( window );
    
  }
  // de-allocate all resources once they've outlived their purpose:
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteVertexArrays(1, &lightCubeVAO);
  glDeleteBuffers(1, &VBO);

  glDeleteVertexArrays(1, &quadVAO);
  glDeleteBuffers(1, &quadVBO);
  glDeleteRenderbuffers(1, &rbo);
  glDeleteFramebuffers(1, &framebuffer);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  // Close OpenGL window, context, and any other GLFW resources.
  glfwTerminate();
  return 0;
}