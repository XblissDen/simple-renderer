#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <stdio.h>

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#define Assert(condition, message, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed at %s:%d: " message "\n", \
                    __FILE__, __LINE__, ##__VA_ARGS__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

void error_callback_glfw(int error, const char* description) {
  fprintf( stderr, "GLFW ERROR: code %i msg: %s.\n", error, description );
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

unsigned int CreateShaderProgram(const std::string vertexPath, const std::string fragmentPath){
  std::string shaderFolderPath = "../assets/shaders/";

  std::string vertexCode, fragmentCode, geometryCode;
  std::stringstream vShaderStream, fShaderStream, gShaderStream;
  std::ifstream vShaderFile(shaderFolderPath + vertexPath),
                fShaderFile(shaderFolderPath + fragmentPath);

  //check if everything OK   
  Assert(vShaderFile.good(), "Couldn't find vertex shader file: %s in shaders folder.\n", vertexPath.c_str());
  Assert(fShaderFile.good(), "Couldn't find fragment shader file: %s in shaders folder.\n", fragmentPath.c_str());             

  //proceed creating shader
  vShaderStream << vShaderFile.rdbuf();    
  fShaderStream << fShaderFile.rdbuf();

  vShaderFile.close();
  fShaderFile.close();

  vertexCode = vShaderStream.str();
  fragmentCode = fShaderStream.str();

  const char* vShaderCode = vertexCode.c_str();
  const char* fShaderCode = fragmentCode.c_str();

  // create VS shader
  GLuint vs = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vs, 1, &vShaderCode, NULL );
  glCompileShader( vs );

  // After glCompileShader check for errors.
  int params = -1;
  glGetShaderiv( vs, GL_COMPILE_STATUS, &params );

  // On error, capture the log and print it.
  if ( GL_TRUE != params ) {
    int max_length    = 2048, actual_length = 0;
    char slog[2048];
    glGetShaderInfoLog( vs, max_length, &actual_length, slog );
    fprintf( stderr, "ERROR: Shader index %u did not compile.\n%s\n", vs, slog );
    return 1;
  }

  //create FS Shader
  GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( fs, 1, &fShaderCode, NULL );
  glCompileShader( fs );

  // After glCompileShader check for errors.
  //int params = -1;
  glGetShaderiv( fs, GL_COMPILE_STATUS, &params );

  // On error, capture the log and print it.
  if ( GL_TRUE != params ) {
    int max_length    = 2048, actual_length = 0;
    char slog[2048];
    glGetShaderInfoLog( vs, max_length, &actual_length, slog );
    fprintf( stderr, "ERROR: Shader index %u did not compile.\n%s\n", fs, slog );
    return 1;
  }

  // create shader program
  unsigned int shader_program = glCreateProgram();
  glAttachShader( shader_program, fs );
  glAttachShader( shader_program, vs );
  glLinkProgram( shader_program );

  // Check for linking errors:
  glGetProgramiv( shader_program, GL_LINK_STATUS, &params );

  // Print the linking log:
  if ( GL_TRUE != params ) {    
    int max_length    = 2048, actual_length = 0;
    char plog[2048];
    glGetProgramInfoLog( shader_program, max_length, &actual_length, plog );
    fprintf( stderr, "ERROR: Could not link shader program GL index %u.\n%s\n", shader_program, plog );
    return 1;
  }

  glDeleteShader(fs);
  glDeleteShader(vs);

  return shader_program;
}

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
  int win_w = 800, win_h = 600;

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

  float points[] = {
    0.5f,  0.5f, 0.0f,  // top right
    0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
  };

  unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
  }; 

  GLuint vbo = 0;
  glGenBuffers( 1, &vbo );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glBufferData( GL_ARRAY_BUFFER, 12 * sizeof( float ), points, GL_STATIC_DRAW );
  
  GLuint vao = 0;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  glEnableVertexAttribArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

  unsigned int EBO;
  glGenBuffers(1, &EBO);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

  //glDeleteVertexArrays( 1, &vao );
  //glDeleteBuffers( 1, &vbo );

  unsigned int shader_program = CreateShaderProgram("triangle.vert","triangle.frag");

  double prev_s = glfwGetTime();  // Set the initial 'previous time'.
  double title_countdown_s = 0.2;

  //glfwSwapInterval( 1 );

  while ( !glfwWindowShouldClose( window ) ) {
    glfwPollEvents(); // Update window events.

    processInput(window);
    
    double curr_s     = glfwGetTime();   // Get the current time.
	  double elapsed_s  = curr_s - prev_s; // Work out the time elapsed over the last frame.
	  prev_s            = curr_s;          // Set the 'previous time' for the next frame to use.

    // Print the FPS, but not every frame, so it doesn't flicker too much.
    title_countdown_s -= elapsed_s;
    if ( title_countdown_s <= 0.0 && elapsed_s > 0.0 ) {
      double fps        = 1.0 / elapsed_s;

      // Create a string and put the FPS as the window title.
      char tmp[256];
      sprintf( tmp, "FPS %.2lf", fps );
      glfwSetWindowTitle(window, tmp );
      title_countdown_s = 0.2;
    }


    // Check if the window resized.
    //glfwGetWindowSize( window, &win_w, &win_h );
    // Update the viewport (drawing area) to fill the window dimensions.
		//glViewport( 0, 0, win_w, win_h );

    // Wipe the drawing surface clear.
    glClearColor( 0.2f, 0.3f, 0.3f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( shader_program );
    
    glBindVertexArray( vao );
    //glDrawArrays( GL_TRIANGLES, 0, 3 );
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw in wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // dram  filled
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    //glBindVertexArray(0);
    // Put the stuff we've been drawing onto the visible area.
    glfwSwapBuffers( window );
  }
  // Close OpenGL window, context, and any other GLFW resources.
  glfwTerminate();
  return 0;
}