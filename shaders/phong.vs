#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 LightIntensity;

struct LightInfo{
   vec4 Position;
   vec3 color;
};
uniform LightInfo Light;

struct MaterialInfo{
   vec3 Ka;
   vec3 Kd;
   vec3 Ks;
   float shininess;
};
uniform MaterialInfo Material;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;

void main()
{
   vec3 tnorm = normalize( NormalMatrix * VertexNormal);
   vec4 eyeCoords = ModelViewMatrix * vec4 (VertexPosition, 1.0);
   vec3 s = normalize(vec3(Light.Position - eyeCoords));
   vec3 v = normalize(-eyeCoords.xyz);

   vec3 r = -reflect (s, tnorm);
   vec3 ambient = Light.color * Material.Ka;
   float sDotN = max( dot(s, tnorm), 0.0);
   vec3 diffuse = Light.color * Material.Kd * sDotN;
   vec3 spec = vec3(0.0);
   if(sDotN > 0.0){
      spec = Light.color * Material.Ks * pow(max(dot(r,v), 0.0), Material.shininess);
   }
   LightIntensity = ambient + diffuse + spec;
   gl_Position = MVP * vec4(VertexPosition, 1.0);
}