#version 440 core

struct V2T
{
    vec3 position;
    vec3 normal;
    vec4 color;
    vec2 uv;
};

layout(location = 0) in vec4 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec2 vertexUV;

out V2T vdata;

void main() {
    vdata.position = (vertexPosition_modelspace).xyz;
    vdata.normal = normalize(vertexNormal_modelspace);
    vdata.color = vertexColor;
    vdata.uv = vertexUV;
}
