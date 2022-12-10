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
    vec2 T030;                                                                         
    vec2 T021;                                                                         
    vec2 T012;                                                                         
    vec2 T003;                                                                         
    vec2 T102;                                                                         
    vec2 T201;                                                                         
    vec2 T300;                                                                         
    vec2 T210;                                                                         
    vec2 T120;                                                                         
    vec2 T111;   
    vec3 Normal[3];                                                                             
    vec2 TexCoord[3];
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
                                                                                                
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)                                                   
{                                                                                               
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;   
}  


void main() {
    fragdata.normal = interpolate3D(oPatch.Normal[0], oPatch.Normal[1], oPatch.Normal[2]);                                                                                    
    
    float u = gl_TessCoord.x;                                                                   
    float v = gl_TessCoord.y;                                                                   
    float w = gl_TessCoord.z;        

    float u3 = pow(u, 3);                                                                    
    float v3 = pow(v, 3);                                                                    
    float w3 = pow(w, 3);                                                                    
    float u2 = pow(u, 2);                                                                    
    float v2 = pow(v, 2);                                                                    
    float w2 = pow(w, 2);
    
    vec3 pos = oPatch.B300 * w3 + oPatch.B030 * u3 + oPatch.B003 * v3 +                               
                     oPatch.B210 * 3.0 * w2 * u + oPatch.B120 * 3.0 * w * u2 + oPatch.B201 * 3.0 * w2 * v + 
                     oPatch.B021 * 3.0 * u2 * v + oPatch.B102 * 3.0 * w * v2 + oPatch.B012 * 3.0 * u * v2 + 
                     oPatch.B111 * 6.0 * w * u * v;

    vec2 tex = oPatch.T300 * w3 + oPatch.T030 * u3 + oPatch.T003 * v3 +                               
                     oPatch.T210 * 3.0 * w2 * u + oPatch.T120 * 3.0 * w * u2 + oPatch.T201 * 3.0 * w2 * v + 
                     oPatch.T021 * 3.0 * u2 * v + oPatch.T102 * 3.0 * w * v2 + oPatch.T012 * 3.0 * u * v2 + 
                     oPatch.T111 * 6.0 * w * u * v;

    fragdata.position = pos;
    fragdata.uv = tex;

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
