
#ifndef meshmodel_H_
#define meshmodel_H_

#include "AGLM.h"

namespace agl {
   class Mesh
   {
   public:

      Mesh();

      virtual ~Mesh();

      // Initialize this object with the given file
      // Returns true if successfull. false otherwise.
      bool loadPLY(const std::string& filename);

      // load a specific .ply file that contains color information (instead of normals)
      bool loadwithColor(const std::string& filename);

      // Return the minimum point of the axis-aligned bounding box
      glm::vec3 getMinBounds() const;

      // Return the maximum point of the axis-aligned bounding box
      glm::vec3 getMaxBounds() const;

      // Return number of vertices in this model
      int numVertices() const;

      // Positions in this model
      float* positions() const;

      // Normals in this model
      float* normals() const;

      // Colors in this model
      float* colors() const;

      // Return number of faces in this model
      int numTriangles() const;

      // face indices in this model
      unsigned int* indices() const;

      // free all memories for member variables
      void clear();

   protected:
      int v; // number of vertices
      int f; // number of faces/polygons
      float* _vertices; // list of vertices
      float* _normals; // list of normals
      float* _colors; // list of colors
      unsigned int* _faces; // list of faces
      glm::vec3 minpos; // minimum values of x, y, and z
      glm::vec3 maxpos; // maximum values of x, y, and z
   };
}

#endif
