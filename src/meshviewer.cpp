// Bryn Mawr College, alinen, 2020
// Haverford College, Jiajie Ma, 2021

#include "AGL.h"
#include "AGLM.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include "mesh.h"
#include "osutils.h"

using namespace std;
using namespace glm;
using namespace agl;

// globals
Mesh theModel;
int theCurrentModel = 0;
vector<string> theModelNames;
const float cameraSpeed = 0.25f;
float lastX, lastY, dist, azimuth, elevation;
bool control = false, zoom = false;
glm::vec3 lookfrom;

// OpenGL IDs
GLuint theVboPosId;
GLuint theVboNormalId;
GLuint theElementbuffer;

static void LoadModel(int modelId)
{
   assert(modelId >= 0 && modelId < theModelNames.size());
   theModel.loadPLY(theModelNames[theCurrentModel]);

   glBindBuffer(GL_ARRAY_BUFFER, theVboPosId);
   glBufferData(GL_ARRAY_BUFFER, theModel.numVertices() * 3 * sizeof(float), theModel.positions(), GL_DYNAMIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, theVboNormalId);
   glBufferData(GL_ARRAY_BUFFER, theModel.numVertices() * 3 * sizeof(float), theModel.normals(), GL_DYNAMIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theElementbuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, theModel.numTriangles() * 3 * sizeof(unsigned int), theModel.indices(), GL_DYNAMIC_DRAW);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   if (key == GLFW_KEY_ESCAPE)
   {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }
   else if (key == 'P')
   {
      if (--theCurrentModel < 0)
      {
         theCurrentModel = theModelNames.size() - 1;
      }
      cout << "Current file: " << theModelNames[theCurrentModel] << endl;
      LoadModel(theCurrentModel);
      azimuth = 0;
      elevation = 0;
      dist = 3.0f;
   }
   else if (key == 'N')
   {
      theCurrentModel = (theCurrentModel + 1) % theModelNames.size(); 
      cout << "Current file: " << theModelNames[theCurrentModel] << endl;
      LoadModel(theCurrentModel);
      azimuth = 0;
      elevation = 0;
      dist = 3.0f;
   }
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
   // Prevent a divide by zero
   if(height == 0) height = 1;
	
   // Set Viewport to window dimensions
   glViewport(0, 0, width, height);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
   double xpos, ypos;
   glfwGetCursorPos(window, &xpos, &ypos);

   int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
   if (state == GLFW_PRESS)
   {
      control = true;
      int keyPress = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
      if (keyPress == GLFW_PRESS) {
         zoom = true;
      }
   }
   else if (state == GLFW_RELEASE)
   {
      control = false;
      zoom = false;
   }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
   if (control)
   {
      if (zoom){
         // zoom in/out with shift and cursor
         float distDelta = ypos - lastY;
         
         distDelta *= cameraSpeed;

         dist += distDelta;

         if (dist < 1.0f){
            dist = 1.0f;
         }
      }
      else{
         // rotate with cursor
         float xDelta = lastX - xpos;
         float yDelta = ypos - lastY;

         xDelta *= cameraSpeed;
         yDelta *= cameraSpeed;

         azimuth += xDelta;
         elevation += yDelta;

         if(elevation > 89.9f){
            elevation = 89.9f;
         }   
         else if(elevation < -89.9f){
            elevation = -89.9f;
         }   
      }
   }

   lastX = xpos;
   lastY = ypos;
}

static void PrintShaderErrors(GLuint id, const std::string label)
{
   std::cerr << label << " failed\n";
   GLint logLen;
   glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLen);
   if (logLen > 0)
   {
      char* log = (char*)malloc(logLen);
      GLsizei written;
      glGetShaderInfoLog(id, logLen, &written, log);
      std::cerr << "Shader log: " << log << std::endl;
      free(log);
   }
}

static std::string LoadShaderFromFile(const std::string& fileName)
{
   std::ifstream file(fileName);
   if (!file)
   {
      std::cout << "Cannot load file: " << fileName << std::endl;
      return "";
   }

   std::stringstream code;
   code << file.rdbuf();
   file.close();

   return code.str();
}

static void LoadModels(const std::string& dir)
{
   std::vector<std::string> filenames = GetFilenamesInDir(dir, "ply");
   for (int i = 0; i < filenames.size(); i++)
   {
      std::string filename = filenames[i];
      theModelNames.push_back(dir+filename);
   }
}

static GLuint LoadShader(const std::string& vertex, const std::string& fragment)
{
   GLint result;
   std::string vertexShader = LoadShaderFromFile(vertex);
   const char* vertexShaderRaw = vertexShader.c_str();
   GLuint vshaderId = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vshaderId, 1, &vertexShaderRaw, NULL);
   glCompileShader(vshaderId);
   glGetShaderiv(vshaderId, GL_COMPILE_STATUS, &result);
   if (result == GL_FALSE)
   {
      PrintShaderErrors(vshaderId, "Vertex shader");
      return -1;
   }

   std::string fragmentShader = LoadShaderFromFile(fragment);
   const char* fragmentShaderRaw = fragmentShader.c_str();
   GLuint fshaderId = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fshaderId, 1, &fragmentShaderRaw, NULL);
   glCompileShader(fshaderId);
   glGetShaderiv(fshaderId, GL_COMPILE_STATUS, &result);
   if (result == GL_FALSE)
   {
      PrintShaderErrors(fshaderId, "Fragment shader");
      return -1;
   }

   GLuint shaderId = glCreateProgram();
   glAttachShader(shaderId, vshaderId);
   glAttachShader(shaderId, fshaderId);
   glLinkProgram(shaderId);
   glGetShaderiv(shaderId, GL_LINK_STATUS, &result);
   if (result == GL_FALSE)
   {
      PrintShaderErrors(shaderId, "Shader link");
      return -1;
   }
   return shaderId;
}


int main(int argc, char** argv)
{
   GLFWwindow* window;

   if (!glfwInit())
   {
      return -1;
   }

   // Explicitly ask for a 4.0 context 
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(500, 500, "Mesh Viewer", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      return -1;
   }

   // Make the window's context current 
   glfwMakeContextCurrent(window);

   glfwSetKeyCallback(window, key_callback);
   glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
   glfwSetMouseButtonCallback(window, mouse_button_callback);
   glfwSetScrollCallback(window, scroll_callback);
   glfwSetCursorPosCallback(window, cursor_position_callback);

#ifndef APPLE
   if (glewInit() != GLEW_OK)
   {
      return -1;
   }
#endif

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glClearColor(0, 0, 0, 1);

   glGenBuffers(1, &theVboPosId);
   glGenBuffers(1, &theVboNormalId);
   glGenBuffers(1, &theElementbuffer);

   GLuint vaoId;
   glGenVertexArrays(1, &vaoId);
   glBindVertexArray(vaoId);

   glEnableVertexAttribArray(0); // 0 -> Sending VertexPositions to array #0 in the active shader
   glBindBuffer(GL_ARRAY_BUFFER, theVboPosId); // always bind before setting data
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

   glEnableVertexAttribArray(1); // 1 -> Sending Normals to array #1 in the active shader
   glBindBuffer(GL_ARRAY_BUFFER, theVboNormalId); // always bind before setting data
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

   LoadModels("../models/");
   LoadModel(0);

   GLuint shaderId = LoadShader("../shaders/phong.vs", "../shaders/phong.fs");
   glUseProgram(shaderId);

   // set up the viewer
   GLuint mvpId = glGetUniformLocation(shaderId, "MVP");
   GLuint mvId = glGetUniformLocation(shaderId, "ModelViewMatrix");
   GLuint nmvId = glGetUniformLocation(shaderId, "NormalMatrix");
   dist = 3.0f;
   azimuth = 0;
   elevation = 0;
   lastX = 250.0f;
   lastY = 250.0f;
   glm::mat4 transform(1.0); // initialize to identity
   glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);

   // Loop until the user closes the window 
   while (!glfwWindowShouldClose(window))
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

      // enable camera control
      glm::vec3 minpos = theModel.getMinBounds();
      glm::vec3 maxpos = theModel.getMaxBounds();
      glm::vec3 center = 0.5f * (maxpos + minpos);
      glm::mat4 translation = glm::translate(glm::mat4(1), -center);
      float xsize = maxpos[0]-minpos[0];
      float ysize = maxpos[1]-minpos[1];
      float zsize = maxpos[2]-minpos[2];
      float scalefactor = std::min(2.0f/xsize, std::min(2.0f/ysize, 2.0f/zsize));
      glm::mat4 scalematrix = glm::scale(glm::mat4(1), glm::vec3(scalefactor));
      transform = scalematrix * translation;

      lookfrom.x = dist * sin(glm::radians(azimuth)) * cos(glm::radians(elevation));
      lookfrom.z = dist * cos(glm::radians(azimuth)) * cos(glm::radians(elevation));
      lookfrom.y = dist * sin(glm::radians(elevation));
      glm::mat4 camera = glm::lookAt(lookfrom, glm::vec3(0,0,0), glm::vec3(0,1.0f,0));
      glm::mat4 mvp = projection * camera * transform;
      glm::mat4 mv = camera * transform;
      glm::mat3 nmv = glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]));
      glUniformMatrix3fv(nmvId, 1, GL_FALSE, &nmv[0][0]);
      glUniformMatrix4fv(mvId, 1, GL_FALSE, &mv[0][0]);
      glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);

      glUniform3f(glGetUniformLocation(shaderId, "Material.Ks"), 1.0f, 1.0f, 1.0f);
      glUniform3f(glGetUniformLocation(shaderId, "Material.Kd"), 0.4f, 0.6f, 1.0f);
      glUniform3f(glGetUniformLocation(shaderId, "Material.Ka"), 0.1f, 0.1f, 0.1f);
      glUniform1f(glGetUniformLocation(shaderId, "Material.shininess"), 80.0f);
      glUniform4f(glGetUniformLocation(shaderId, "Light.position"), 100.0f, 100.0f, 100.0f, 1.0f);
      glUniform3f(glGetUniformLocation(shaderId, "Light.color"), 1.0f, 1.0f, 1.0f);

      // Draw primitive
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theElementbuffer);
      glDrawElements(GL_TRIANGLES, theModel.numTriangles() * 3, GL_UNSIGNED_INT, (void*)0);

      // Swap front and back buffers
      glfwSwapBuffers(window);

      // Poll for and process events
      glfwPollEvents();
   }

   glfwTerminate();
   return 0;
}


