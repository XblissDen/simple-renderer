#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <optional>

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

  //Shader lightingShader("lighting.vert", "lighting.frag");
  Shader lightCubeShader("lightCube.vert", "lightCube.frag");

  // draw as wireframe
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  // CREATE CORE SYSTEMS
  AssetManager assetManager;
  GPUResourceManager gpuManager(&assetManager);
  Renderer renderer(&assetManager, &gpuManager);

  // CREATE SCENE WITH ECS
  Scene scene;
  scene.AddSystem(std::make_unique<RenderSystem>(&renderer));
  //scene.AddSystem(std::make_unique<TransformSystem>());

  // Load a model
  ModelAssetID backpackModel = assetManager.LoadModel("../assets/models/backpack/backpack.obj");
  if (backpackModel == INVALID_MODEL) {
        std::cout << "Failed to load model!" << std::endl;
        return -1;
  }

  ModelAssetID cubeModel = assetManager.LoadModel("../assets/models/cube.glb");
  Assert(cubeModel != INVALID_MODEL, "Failed to load model!");

  EntityID backpackEntity = scene.CreateRenderableObject(
        "backpack",
        backpackModel,           // Model to render
        2,     // Material to use
        glm::vec3(0.0f, 0.0f, -1.0f)  // Position in world
  );

  EntityID e_lightCube1 = scene.CreateRenderableObject(
        "lightCube1",
        cubeModel,           // Model to render
        4,     // Material to use
        pointLightPositions[0]  // Position in world
  );

  EntityID e_lightCube2 = scene.CreateRenderableObject(
        "lightCube2",
        cubeModel,           // Model to render
        4,     // Material to use
        pointLightPositions[1]  // Position in world
  );

  EntityID e_lightCube3 = scene.CreateRenderableObject(
        "lightCube3",
        cubeModel,           // Model to render
        4,     // Material to use
        pointLightPositions[2]  // Position in world
  );

  EntityID e_lightCube4 = scene.CreateRenderableObject(
        "lightCube4",
        cubeModel,           // Model to render
        4,     // Material to use
        pointLightPositions[3]  // Position in world
  );

  printf("backpack entity: %d", scene.GetEntityByName("backpack"));
  // Create a basic material
  //MaterialID defaultMaterial = assetManager.CreateMaterial("default");
  
  //printf("backpack materials: %d", assetManager.GetMaterial(2)->specularTexture);

  std::optional<int> selectedEntityId;

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
      ImGui::Begin("Statistics");
    
      ImGui::Text("Loaded: %d models, %d vertices", assetManager.GetStats().modelsLoaded, assetManager.GetStats().totalVertices);
      ImGui::Text("Draw calls: %d, triangles: %d", renderer.GetDrawCalls(), renderer.GetTrianglesRendered());

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      
      ImGui::End();
    }

    {
      ImGui::Begin("Entity Editor");

      const char *comboPreviewValue = "Select Entity";
      if (selectedEntityId.has_value())
      {
        comboPreviewValue = scene.GetNameOfEntity(selectedEntityId.value()).c_str();
      }

      if(ImGui::BeginCombo("##entity_selector", comboPreviewValue)){
        for(const auto& pair: scene.GetEntitiesMap()){
          const std::string& entityName = pair.first;
          const int entityId = pair.second;

          const bool isSelected = (selectedEntityId.has_value() && selectedEntityId.value() == entityId);

          // This is the individual selectable item in the dropdown list.
          if (ImGui::Selectable(entityName.c_str(), isSelected))
          {
            selectedEntityId = entityId; // Store the selected entity's ID
          }

          // Set initial focus when opening the combo box
          if (isSelected)
          {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      // if an entity is selected
      if(selectedEntityId.has_value()){
        ImGui::Separator();
        ImGui::Text("Selected Entity: %s, ID: %d", scene.GetNameOfEntity(selectedEntityId.value()).c_str(), selectedEntityId.value());

        TransformComponent* transform = scene.GetComponentManager().GetTransform(selectedEntityId.value());
        if(ImGui::DragFloat3("Position", glm::value_ptr(transform->position), 0.1f)){
          transform->isDirty = true;
        }
        if(ImGui::DragFloat3("Rotation", glm::value_ptr(transform->rotation), 1.0f)){
          transform->isDirty = true;
        }
        if(ImGui::DragFloat3("Scale", glm::value_ptr(transform->scale), 0.1f)){
          transform->isDirty = true;
        }
      }

      ImGui::End();
    }

    ImGui::Render();
    // ==============
    // Dear ImGui END
    // ==============

    // RENDER
    // ------
    glEnable(GL_DEPTH_TEST);

    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT /*| GL_STENCIL_BUFFER_BIT*/ );

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    renderer.SetCamera(view, projection, camera.Position);

    scene.Update(deltaTime);

    renderer.RenderFrame();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Put the stuff we've been drawing onto the visible area.
    glfwSwapBuffers( window );
    
  }

  glDeleteRenderbuffers(1, &rbo);
  glDeleteFramebuffers(1, &framebuffer);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  // Close OpenGL window, context, and any other GLFW resources.
  glfwTerminate();
  return 0;
}