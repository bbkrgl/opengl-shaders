#version 430

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 NormalMatrix;
uniform mat4 MVP;

uniform sampler2D TexColor;
uniform sampler2D TexGrey;
uniform float textureOffset;
uniform float orbitDegree;

uniform float heightFactor;
uniform float imageWidth;
uniform float imageHeight;

out Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;


out vec3 LightVector;// Vector from Vertex to Light;
out vec3 CameraVector;// Vector from Vertex to Camera;


void main()
{
    // get orbitDegree value, compute new x, y coordinates
    // there won't be height in moon shader

   // set gl_Position variable correctly to give the transformed vertex position

    vec4 pos = MVP * vec4(VertexPosition, 1);
    pos = vec4(pos.x * cos(orbitDegree) - pos.y*sin(orbitDegree),
            pos.x * sin(orbitDegree) + pos.y*cos(orbitDegree),
            pos.z, 1);

	vec4 normal = NormalMatrix * vec4(VertexNormal, 1);
    normal = vec4(normal.x * cos(orbitDegree) - normal.y*sin(orbitDegree),
            normal.x * sin(orbitDegree) + normal.y*cos(orbitDegree),
            normal.z, 1);


    LightVector = normalize(lightPosition - pos.xyz);
    CameraVector = normalize(cameraPosition - pos.xyz);

    data.Position = pos.xyz;
    data.Normal = normal.xyz;
    data.TexCoord = VertexTex;

    gl_Position = ProjectionMatrix * ViewMatrix * pos;
}
