#version 440 core

layout(location = 0) in vec4 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec2 vertexUV;

out vec3 WorldPos_CS_in;                                                                                                                      
out vec3 Normal_CS_in;
out vec2 TexCoord_CS_in;
//out vec4 Color_CS_in;

void main() {
    WorldPos_CS_in = (vertexPosition_modelspace).xyz;
    Normal_CS_in = normalize(vertexNormal_modelspace);
    TexCoord_CS_in = vertexUV;
    //Color_CS_in = vertexColor;
}
