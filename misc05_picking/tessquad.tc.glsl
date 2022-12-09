#version 440 core

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


layout (vertices = 3) out;
uniform float TessellationLevel;

in vec3 WorldPos_CS_in[];    
in vec3 Normal_CS_in[];                                                                         
in vec2 TexCoord_CS_in[];

out patch OutputPatch oPatch; 


void main() {                                                                                                                                                                           
    oPatch.B0 = WorldPos_CS_in[0];                                                   
    oPatch.B1 = WorldPos_CS_in[1];                                                   
    oPatch.B2 = WorldPos_CS_in[2];
    oPatch.B3 = WorldPos_CS_in[3];

    oPatch.T0 = TexCoord_CS_in[0];                                                   
    oPatch.T1 = TexCoord_CS_in[1];                                                   
    oPatch.T2 = TexCoord_CS_in[2];
    oPatch.T3 = TexCoord_CS_in[3];
    
    vec3 p0 = oPatch.B0, p1 = oPatch.B1, p2 = oPatch.B2, p3 = oPatch.B3;
    vec2 t0 = oPatch.T0, t1 = oPatch.T1, t2 = oPatch.T2, t3 = oPatch.T3;

    for (int i = 0 ; i < 4 ; i++) {                                                             
        oPatch.Normal[i] = Normal_CS_in[i];                                                     
        oPatch.TexCoord[i] = TexCoord_CS_in[i];
    }   

    vec3 n0 = oPatch.Normal[0];
	vec3 n1 = oPatch.Normal[1];
	vec3 n2 = oPatch.Normal[2];
	vec3 n3 = oPatch.Normal[3];

    float w01 = dot(p1 - p0, n0);
	float w10 = dot(p0 - p1, n1);
	float w12 = dot(p2 - p1, n1);
	float w21 = dot(p1 - p2, n2);
	float w23 = dot(p3 - p2, n2);
	float w32 = dot(p2 - p3, n3);
	float w30 = dot(p0 - p3, n3);
	float w03 = dot(p3 - p0, n0);

    float wt01 = dot(t1 - t0, vec2(n0));
	float wt10 = dot(t0 - t1, vec2(n1));
	float wt12 = dot(t2 - t1, vec2(n1));
	float wt21 = dot(t1 - t2, vec2(n2));
	float wt23 = dot(t3 - t2, vec2(n2));
	float wt32 = dot(t2 - t3, vec2(n3));
	float wt30 = dot(t0 - t3, vec2(n3));
	float wt03 = dot(t3 - t0, vec2(n0));

    vec3 b01 = (2. * p0 + p1 - w01 * n0) / 3.;
	vec3 b10 = (2. * p1 + p0 - w10 * n1) / 3.;
	vec3 b12 = (2. * p1 + p2 - w12 * n1) / 3.;
	vec3 b21 = (2. * p2 + p1 - w21 * n2) / 3.;
	vec3 b23 = (2. * p2 + p3 - w23 * n2) / 3.;
	vec3 b32 = (2. * p3 + p2 - w32 * n3) / 3.;
	vec3 b30 = (2. * p3 + p0 - w30 * n3) / 3.;
	vec3 b03 = (2. * p0 + p3 - w03 * n0) / 3.;

    vec2 t01 = (2. * t0 + t1 - wt01 * vec2(n0)) / 3.;
	vec2 t10 = (2. * t1 + t0 - wt10 * vec2(n1)) / 3.;
	vec2 t12 = (2. * t1 + t2 - wt12 * vec2(n1)) / 3.;
	vec2 t21 = (2. * t2 + t1 - wt21 * vec2(n2)) / 3.;
	vec2 t23 = (2. * t2 + t3 - wt23 * vec2(n2)) / 3.;
	vec2 t32 = (2. * t3 + t2 - wt32 * vec2(n3)) / 3.;
	vec2 t30 = (2. * t3 + t0 - wt30 * vec2(n3)) / 3.;
	vec2 t03 = (2. * t0 + t3 - wt03 * vec2(n0)) / 3.;

    oPatch.B01 = b01;
    oPatch.B10 = b10;
    oPatch.B12 = b12;
    oPatch.B21 = b21;
    oPatch.B23 = b23;
    oPatch.B32 = b32;
    oPatch.B30 = b30;
    oPatch.B03 = b03;

    oPatch.T01 = t01;
    oPatch.T10 = t10;
    oPatch.T12 = t12;
    oPatch.T21 = t21;
    oPatch.T23 = t23;
    oPatch.T32 = t32;
    oPatch.T30 = t30;
    oPatch.T03 = t03;
	
    vec3 q = b03 + b01 + b10 + b12 + b21 + b23 + b32 + b30;

    vec2 qt = t03 + t01 + t10 + t12 + t21 + t23 + t32 + t30;

	
    vec3 e0 = (2. * (b01 + b03 + q) - (b21 + b23)) / 18.;
	vec3 v0 = (4. * p0 + 2.*(p3 + p1) + p2) / 9.;
	vec3 b02 = e0 + (e0 - v0) / 2.;
    oPatch.B02 = b02;

	vec3 e1 = (2. * (b12 + b10 + q) - (b32 + b30)) / 18.;
	vec3 v1 = (4. * p1 + 2. * (p0 + p2) + p3) / 9.;
	vec3 b13 = e1 + (e1 - v1) / 2.;
	oPatch.B13 = b13;

	vec3 e2 = (2. * (b23 + b21 + q) - (b03 + b01)) / 18.;
	vec3 v2 = (4. * p2 + 2. * (p1 + p3) + p0) / 9.;
	vec3 b20 = e2 + (e2 - v2) / 2.;
	oPatch.B20 = b20;

	vec3 e3 = (2. * (b30 + b32 + q) - (b10 + b12)) / 18.;
	vec3 v3 = (4. * p3 + 2. * (p2 + p0) + p1) / 9.;
	vec3 b31 = e3 + (e3 - v3) / 2.;
	oPatch.B31 = b31;

    vec2 et0 = (2. * (t01 + t03 + qt) - (t21 + t23)) / 18.;
	vec2 vt0 = (4. * t0 + 2. * (t3 + t1) + t2) / 9.;
	vec2 t02 = et0 + (et0 - vt0) / 2.;
    oPatch.T02 = t02;

	vec2 et1 = (2. * (t12 + t10 + qt) - (t32 + t30)) / 18.;
	vec2 vt1 = (4. * t1 + 2. * (t0 + t2) + t3) / 9.;
	vec2 t13 = et1 + (et1 - vt1) / 2.;
	oPatch.T13 = t13;

	vec2 et2 = (2. * (t23 + t21 + qt) - (t03 + t01)) / 18.;
	vec2 vt2 = (4. * t2 + 2. * (t1 + t3) + t0) / 9.;
	vec2 t20 = et2 + (et2 - vt2) / 2.;
	oPatch.T20 = t20;

	vec2 et3 = (2. * (t30 + t32 + qt) - (t10 + t12)) / 18.;
	vec2 vt3 = (4. * t3 + 2. * (t2 + t0) + t1) / 9.;
	vec2 t31 = et3 + (et3 - vt3) / 2.;
	oPatch.T31 = t31;                                                                        
                                                                                                
    gl_TessLevelOuter[0] = TessellationLevel;
	gl_TessLevelOuter[1] = TessellationLevel;
	gl_TessLevelOuter[2] = TessellationLevel;
	gl_TessLevelOuter[3] = TessellationLevel;
	gl_TessLevelInner[0] = TessellationLevel;
	gl_TessLevelInner[1] = TessellationLevel;
}
