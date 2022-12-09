#version 440 core

struct FragData
{
    vec3 position;
    vec3 normal;
    vec4 color;
    vec2 uv;
};

struct OutputPatch                                                                              
{                                                                                               
    vec3 B030;                                                                         
    vec3 B021;                                                                         
    vec3 B012;                                                                         
    vec3 B003;                                                                         
    vec3 B102;                                                                         
    vec3 B201;                                                                         
    vec3 B300;                                                                         
    vec3 B210;                                                                         
    vec3 B120;                                                                         
    vec3 B111;                                                                         
    vec3 Normal[3];                                                                             
    vec2 TexCoord[3];
    vec3  Color[3];
};   

layout(triangles, equal_spacing, ccw) in;

in patch OutputPatch oPatch;     

out FragData fragdata;
out vec3 position_worldspace;
out vec3 normal_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 lightDirection_cameraspace[2];

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 lightPosition_worldspace[2];
uniform highp sampler2D myTextureSampler;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)                                                   
{                                                                                               
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;   
}                                                                                               
                                                                                                
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)                                                   
{                                                                                               
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;   
}  


void main() {
    fragdata.uv = interpolate2D(oPatch.TexCoord[0], oPatch.TexCoord[1], oPatch.TexCoord[2]); 
    fragdata.normal = interpolate3D(oPatch.Normal[0], oPatch.Normal[1], oPatch.Normal[2]);                                                                                    
    
    float u = gl_TessCoord.x;                                                                   
    float v = gl_TessCoord.y;                                                                   
    float w = gl_TessCoord.z;                                                                   
    float uPow3 = pow(u, 3);                                                                    
    float vPow3 = pow(v, 3);                                                                    
    float wPow3 = pow(w, 3);                                                                    
    float uPow2 = pow(u, 2);                                                                    
    float vPow2 = pow(v, 2);                                                                    
    float wPow2 = pow(w, 2);                                                                    
    vec3 pos = oPatch.B300 * wPow3 + oPatch.B030 * uPow3 + oPatch.B003 * vPow3 +                               
                     oPatch.B210 * 3.0 * wPow2 * u + oPatch.B120 * 3.0 * w * uPow2 + oPatch.B201 * 3.0 * wPow2 * v + 
                     oPatch.B021 * 3.0 * uPow2 * v + oPatch.B102 * 3.0 * w * vPow2 + oPatch.B012 * 3.0 * u * vPow2 + 
                     oPatch.B111 * 6.0 * w * u * v;

    fragdata.position = pos;
    gl_Position = P * V * M * vec4(pos, 1.0);

    position_worldspace = (M * vec4(pos, 1.0)).xyz;

    vec3 vertexPosition_cameraspace = (V * M * vec4(pos, 1.0)).xyz;
    eyeDirection_cameraspace = vec3(0.0f, 0.0f, 0.0f) - vertexPosition_cameraspace;

    for(int i = 0; i < 2; i++) {
		vec3 lightPosition_cameraspace = (V * vec4(lightPosition_worldspace[i], 1.0)).xyz;
		lightDirection_cameraspace[i] = lightPosition_cameraspace + eyeDirection_cameraspace;
		//cout << "vertex shader: " << i << endl;
	}

    //vec3 lightPosition_cameraspace = (V * vec4(lightPosition_worldspace, 1)).xyz;
    //lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace;

    normal_cameraspace = (V * M * vec4(fragdata.normal, 1.0)).xyz;
}
