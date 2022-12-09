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
    vec2 T0;                                                                         
    vec2 T03;                                                                         
    vec2 T30;                                                                         
    vec2 T3;                                                                         
    vec2 T01;                                                                         
    vec2 T02;                                                                         
    vec2 T31;                                                                         
    vec2 T32;                                                                         
    vec2 T10;                                                                         
    vec2 T13;
    vec2 T20;
    vec2 T23;
    vec2 T1;
    vec2 T12;
    vec2 T21;
    vec2 T2;
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


void main() {    
    float u = gl_TessCoord.x;                                                                   
    float v = gl_TessCoord.y;                                                                   
    vec3 p0 = oPatch.B0, p1 = oPatch.B1, p2 = oPatch.B2, p3 = oPatch.B3;


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
    
    vec2 tex = bu0 * (bv0* oPatch.T0 + bv1 * oPatch.T01 + bv2 * oPatch.T10 + bv3 * oPatch.T1) 
          + bu1 * (bv0 * oPatch.T03 + bv1 * oPatch.T02 + bv2 * oPatch.T13 + bv3 * oPatch.T12) 
          + bu2 * (bv0 * oPatch.T30 + bv1 * oPatch.T31 + bv2 * oPatch.T20 + bv3 * oPatch.T21) 
           + bu3 * (bv0 * oPatch.T3 + bv1 * oPatch.T32 + bv2 * oPatch.T23 + bv3 * oPatch.T2);

    fragdata.uv = tex;

    vec2 t0 = oPatch.TexCoord[0];
	vec2 t1 = oPatch.TexCoord[1];
	vec2 t2 = oPatch.TexCoord[2];
	vec2 t3 = oPatch.TexCoord[3];

    vec3 n0 = oPatch.Normal[0];
	vec3 n1 = oPatch.Normal[1];
	vec3 n2 = oPatch.Normal[2];
	vec3 n3 = oPatch.Normal[3];
	
	float v01 = (2.*(dot(p1 - p0, n0 + n1) / dot(p1 - p0, p1 - p0)));
	float v12 = (2.*(dot(p2 - p1, n1 + n2) / dot(p2 - p1, p2 - p1)));
	float v23 = (2.*(dot(p3 - p2, n2 + n3) / dot(p3 - p2, p3 - p2)));
	float v30 = (2.*(dot(p0 - p3, n3 + n0) / dot(p0 - p3, p0 - p3)));
	
	vec3 n01 = normalize(n0 + n1 - v01*(p1 - p0));
	vec3 n12 = normalize(n1 + n2 - v12*(p2 - p1));
	vec3 n23 = normalize(n2 + n3 - v23*(p3 - p2));
	vec3 n30 = normalize(n3 + n0 - v30*(p0 - p3));
	
	vec3 n0123 = ((2.*(n01 + n12 + n23 + n30)) + (n0 + n1 + n2 + n3)) / 12.;
	
	float nu0 = (1.-u) * (1.-u);
	float nu1 = 2. * u * (1.-u);
	float nu2 = u * u;
	
	float nv0 = (1.-v) * (1.-v);
	float nv1 = 2. * v * (1.-v);
	float nv2 = v * v;
	
	fragdata.normal = nu0*(nv0*n0 + nv1*n01 + nv2*n1)
                + nu1*(nv0*n30 + nv1*n0123 + nv2*n12)
                + nu2*(nv0*n3 + nv1*n23 + nv2*n2);

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
