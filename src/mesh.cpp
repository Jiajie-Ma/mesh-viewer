
#include "mesh.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;
using namespace glm;
using namespace agl;

static const int max_line = 65535;

Mesh::Mesh() 
{
   v = 0;
   f = 0;
   _vertices = new float[0];
   _normals = new float[0];
   _faces = new unsigned int[0];
}

Mesh::~Mesh()
{
   clear();
}

bool Mesh::loadPLY(const std::string& filename)
{
   ifstream file(filename);
   if (!file)
   {
      cout << "ERROR: Cannot load file: " << filename << std::endl;
      return false;
   }

   // make sure the arrays are empty
   clear();

   // initialize a bin that reads all unnecessary information
   string line;
   file >> line;

   if (line != "ply"){
      cout << "ERROR: " << filename << " is not a ply file!" << std::endl;
      return false;
   }
   
   // skip the ascii line
   while (line != "element"){
      file.ignore(max_line, '\n');
      file >> line;
   }
   
   file >> line;
   file >> v;
   
   while (line != "element"){
      file.ignore(max_line, '\n');
      file >> line;
   }

   file >> line;
   file >> f;

   // skip to the end of the header
   for (int i = 0; i < 3; i ++){
      file.ignore(max_line, '\n');
   }

   // read the vertices and normals
   _vertices = new float[3 * v];
   _normals = new float[3 * v];
   for (int i = 0; i < v; i++){
      // read the vertices
      file >> _vertices[3*i + 0];
      file >> _vertices[3*i + 1];
      file >> _vertices[3*i + 2];
      file >> _normals[3*i + 0];
      file >> _normals[3*i + 1];
      file >> _normals[3*i + 2];
      file.ignore(max_line, '\n');

      // keep track of the maximum/minimum value of x,y,z
      if (i == 0){
         minpos[0] = _vertices[0];
         minpos[1] = _vertices[1];
         minpos[2] = _vertices[2];
         maxpos[0] = _vertices[0];
         maxpos[1] = _vertices[1];
         maxpos[2] = _vertices[2];
      }
      else{
         if (_vertices[3*i + 0] < minpos[0]){
            minpos[0] = _vertices[3*i + 0];
         }
         if (_vertices[3*i + 1] < minpos[1]){
            minpos[1] = _vertices[3*i + 1];
         }
         if (_vertices[3*i + 2] < minpos[2]){
            minpos[2] = _vertices[3*i + 2];
         }
         if (_vertices[3*i + 0] > maxpos[0]){
            maxpos[0] = _vertices[3*i + 0];
         }
         if (_vertices[3*i + 1] > maxpos[1]){
            maxpos[1] = _vertices[3*i + 1];
         }
         if (_vertices[3*i + 2] > maxpos[2]){
            maxpos[2] = _vertices[3*i + 2];
         }
      }
   }

   // read the faces (triangles)
   _faces = new unsigned int[3*f];
   for (int i = 0; i < f; i++){
      file >> line;
      file >> _faces[3*i + 0];
      file >> _faces[3*i + 1];
      file >> _faces[3*i + 2];
      file.ignore(max_line, '\n');
   }

   return true;
}

glm::vec3 Mesh::getMinBounds() const
{
  return minpos;
}

glm::vec3 Mesh::getMaxBounds() const
{
  return maxpos;
}

int Mesh::numVertices() const
{
   return v;
}

int Mesh::numTriangles() const
{
   return f;
}

float* Mesh::positions() const
{
   return _vertices;
}

float* Mesh::normals() const
{
   return _normals;
}

unsigned int* Mesh::indices() const
{
   return _faces;
}

void Mesh::clear()
{
   // clean up the memory
   delete[] _vertices;
   delete[] _normals;
   delete[] _faces;
}

