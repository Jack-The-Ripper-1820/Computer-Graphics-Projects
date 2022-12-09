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
    vec3 B0;                                                                         
    vec3 B03;                                                                         
    vec3 B30;                                                                         
    vec3 B3;                                                                         
    vec3 B01;                                                                         
    vec3 B02;                                                                         
    vec3 B31;                                                                         
    vec3 B32;                                                                         
    vec3 B10;                                                                         
    vec3 B13;
    vec3 B20;
    vec3 B23;
    vec3 B1;
    vec3 B12;
    vec3 B21;
    vec3 B2;
    vec3 Normal[4];                                                                             
    vec2 TexCoord[4];   
};  

layout(quads, equal_spacing, ccw) in;

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

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2, vec2 v3)                                                   
{
    // Interpolate along bottom edge using x component of the
    // tessellation coordinate
    vec2 p1 = vec2(mix(v0, v1, gl_TessCoord.x));
    // Interpolate along top edge using x component of the
    // tessellation coordinate
    vec2 p2 = vec2(mix(v2, v3, gl_TessCoord.x));
    // Now interpolate those two results using the y component
    // of tessellation coordinate
    return vec2(mix(p1, p2, gl_TessCoord.y));
}                                                                                               
                                                                                                
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2, vec3 v3)                                                   
{
    // Interpolate along bottom edge using x component of the
    // tessellation coordinate
    vec3 p1 = vec3(mix(v0, v1, gl_TessCoord.x));
    // Interpolate along top edge using x component of the
    // tessellation coordinate
    vec3 p2 = vec3(mix(v2, v3, gl_TessCoord.x));
    // Now interpolate those two results using the y component
    // of tessellation coordinate
    return vec3(mix(p1, p2, gl_TessCoord.y));
}  


void main() {
    fragdata.uv = interpolate2D(oPatch.TexCoord[0], oPatch.TexCoord[1], oPatch.TexCoord[2], oPatch.TexCoord[3]); 
    fragdata.normal = interpolate3D(oPatch.Normal[0], oPatch.Normal[1], oPatch.Normal[2], oPatch.Normal[3]);                                                                                    
    
    float u = gl_TessCoord.x;                                                                   
    float v = gl_TessCoord.y;                                                                   
    
    float bu0 = (1. - u) * (1.-u) * (1.-u);
	float bu1 = 3. * u * (1.-u) * (1.-u);
	float bu2 = 3. * u * u * (1.-u);
	float bu3 = u * u * u;

	float bv0 = (1. - v) * (1.-v) * (1.-v);
	float bv1 = 3. * v * (1.-v) * (1.-v);
	float bv2 = 3. * v * v * (1.-v);
	float bv3 = v * v * v;
    
    vec3 pos = bu0 * (bv0* oPatch.B0 + bv1 * oPatch.B01 + bv2 * oPatch.B10 + bv3 * oPatch.B1) 
           + bu1 * (bv0 * oPatch.B03 + bv1 * oPatch.B02 + bv2 * oPatch.B13 + bv3 * oPatch.B12) 
           + bu2 * (bv0 * oPatch.B30 + bv1 * oPatch.B31 + bv2 * oPatch.B20 + bv3 * oPatch.B21) 
           + bu3 * (bv0 * oPatch.B3 + bv1 * oPatch.B32 + bv2 * oPatch.B23 + bv3 * oPatch.B2);

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
