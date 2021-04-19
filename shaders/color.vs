#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexColor;

out vec3 Position;
out vec3 Normal;
out vec3 Color;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;

void main()
{
   Color = VertexColor;
   Normal = normalize( NormalMatrix * VertexNormal);
   Position = vec3(ModelViewMatrix * vec4(VertexPosition,1.0));
   gl_Position = MVP * vec4(VertexPosition,1.0);
}