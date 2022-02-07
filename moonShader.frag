#version 430

in Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;
in vec3 LightVector;
in vec3 CameraVector;

uniform vec3 lightPosition;
uniform sampler2D TexColor;
uniform sampler2D MoonTexColor;
uniform sampler2D TexGrey;
uniform float textureOffset;

out vec4 FragColor;

vec3 ambientReflectenceCoefficient = vec3(0.5f);
vec3 ambientLightColor = vec3(0.6f);
vec3 specularReflectenceCoefficient = vec3(1.0f);
vec3 specularLightColor = vec3(1.0f);
float SpecularExponent = 10;
vec3 diffuseReflectenceCoefficient = vec3(1.0f);
vec3 diffuseLightColor = vec3(1.0f);


void main()
{
    vec2 texCoord = data.TexCoord;
    vec4 texColor = texture(MoonTexColor, texCoord);

    vec3 ambient = ambientLightColor*ambientReflectenceCoefficient;

    float diff_c = max(dot(data.Normal, LightVector), 0.0);
    vec3 diffuse = diff_c*diffuseLightColor*diffuseReflectenceCoefficient;

    vec3 H = normalize(CameraVector + LightVector);
    float spec = pow(max(dot(H, data.Normal), 0.0), SpecularExponent);
    vec3 specular = spec*specularReflectenceCoefficient*specularLightColor;

    FragColor = vec4((diffuse+ambient+specular)*texColor.xyz, 1.0);
}
