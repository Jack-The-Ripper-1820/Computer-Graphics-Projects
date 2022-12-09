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
    vec3 Normal[4];                                                                             
    vec2 TexCoord[4];   
};  


layout (vertices = 3) out;
uniform float TessellationLevel;

in vec3 WorldPos_CS_in[];    
in vec3 Normal_CS_in[];                                                                         
in vec2 TexCoord_CS_in[];

out patch OutputPatch oPatch; 

vec3 ProjectToPlane(vec3 Point, vec3 PlanePoint, vec3 PlaneNormal)                              
{                                                                                               
    vec3 v = Point - PlanePoint;                                                                
    float Len = dot(v, PlaneNormal);                                                            
    vec3 d = Len * PlaneNormal;                                                                 
    return (Point - d);                                                                         
}

void CalcPositions()                                                                            
{                                                                                               
    oPatch.B0 = WorldPos_CS_in[0];                                                   
    oPatch.B1 = WorldPos_CS_in[1];                                                   
    oPatch.B2 = WorldPos_CS_in[2];
    oPatch.B3 = WorldPos_CS_in[3];
    
    vec3 p0 = oPatch.B0, p1 = oPatch.B1, p2 = oPatch.B2, p3 = oPatch.B3;

    for (int i = 0 ; i < 4 ; i++) {                                                             
        oPatch.Normal[i] = Normal_CS_in[i];                                                     
        oPatch.TexCoord[i] = TexCoord_CS_in[i];
    }   

    vec3 n0 = oPatch.Normal[0];
	vec3 n1 = oPatch.Normal[1];
	vec3 n2 = oPatch.Normal[2];
	vec3 n3 = oPatch.Normal[3];

    vec3 EdgeB01 = p0 - p1;
    vec3 EdgeB12 = p1 - p2;
    vec3 EdgeB23 = p2 - p3;
    vec3 EdgeB30 = p3 - p0;

	

    /*oPatch.B10 = oPatch.B1 + EdgeB01 / 3.0;
    oPatch.B01 = oPatch.B1 + EdgeB01 * 2.0 / 3.0;
    oPatch.B21 = oPatch.B2 + EdgeB12 / 3.0;
    oPatch.B12 = oPatch.B2 + EdgeB12 * 2.0 / 3.0;
    oPatch.B32 = oPatch.B3 + EdgeB23 / 3.0;
    oPatch.B23 = oPatch.B3 + EdgeB23 * 2.0 / 3.0;
    oPatch.B03 = oPatch.B0 + EdgeB30 / 3.0;
    oPatch.B30 = oPatch.B0 + EdgeB30 * 2.0 / 3.0;
    

    oPatch.B10 = ProjectToPlane(oPatch.B10, oPatch.B1, n0);
    oPatch.B01 = ProjectToPlane(oPatch.B01, oPatch.B1, n1);
    oPatch.B21 = ProjectToPlane(oPatch.B21, oPatch.B2, n1);
    oPatch.B12 = ProjectToPlane(oPatch.B12, oPatch.B2, n2);
    oPatch.B32 = ProjectToPlane(oPatch.B32, oPatch.B3, n2);
    oPatch.B23 = ProjectToPlane(oPatch.B23, oPatch.B3, n3);
    oPatch.B03 = ProjectToPlane(oPatch.B03, oPatch.B0, n3);
    oPatch.B30 = ProjectToPlane(oPatch.B30, oPatch.B0, n0);
    */

    float w01 = dot(p1 - p0, n0);
	float w10 = dot(p0 - p1, n1);
	float w12 = dot(p2 - p1, n1);
	float w21 = dot(p1 - p2, n2);
	float w23 = dot(p3 - p2, n2);
	float w32 = dot(p2 - p3, n3);
	float w30 = dot(p0 - p3, n3);
	float w03 = dot(p3 - p0, n0);

    vec3 b01 = (2.*p0 + p1 - w01*n0) / 3.;
	vec3 b10 = (2.*p1 + p0 - w10*n1) / 3.;
	vec3 b12 = (2.*p1 + p2 - w12*n1) / 3.;
	vec3 b21 = (2.*p2 + p1 - w21*n2) / 3.;
	vec3 b23 = (2.*p2 + p3 - w23*n2) / 3.;
	vec3 b32 = (2.*p3 + p2 - w32*n3) / 3.;
	vec3 b30 = (2.*p3 + p0 - w30*n3) / 3.;
	vec3 b03 = (2.*p0 + p3 - w03*n0) / 3.;

    oPatch.B01 = b01;
    oPatch.B10 = b10;
    oPatch.B12 = b12;
    oPatch.B21 = b21;
    oPatch.B23 = b23;
    oPatch.B32 = b32;
    oPatch.B30 = b30;
    oPatch.B03 = b03;

    /*vec3 b01 = oPatch.B01;
    vec3 b10 = oPatch.B10;
    vec3 b12 = oPatch.B12;
    vec3 b21 = oPatch.B21;
    vec3 b23 = oPatch.B23;
    vec3 b32 = oPatch.B32;
    vec3 b30 = oPatch.B30;
    vec3 b03 = oPatch.B03;
    */

    //vec3 q = b01 + b10 + b12 + b21 + b23 + b32 + b30 + b03;
	
    vec3 q = b03 + b01 + b10 + b12 + b21 + b23 + b32 + b30;
	
    vec3 e0 = (2.*(b01 + b03 + q) - (b21 + b23)) / 18.;
	vec3 v0 = (4.*p0 + 2.*(p3 + p1) + p2) / 9.;
	vec3 b02 = e0 + (e0 - v0) / 2.;
    oPatch.B02 = b02;

	vec3 e1 = (2.*(b12 + b10 + q) - (b32 + b30)) / 18.;
	vec3 v1 = (4.*p1 + 2.*(p0 + p2) + p3) / 9.;
	vec3 b13 = e1 + (e1 - v1) / 2.;
	oPatch.B13 = b13;

	vec3 e2 = (2.*(b23 + b21 + q) - (b03 + b01)) / 18.;
	vec3 v2 = (4.*p2 + 2.*(p1 + p3) + p0) / 9.;
	vec3 b20 = e2 + (e2 - v2) / 2.;
	oPatch.B20 = b20;

	vec3 e3 = (2.*(b30 + b32 + q) - (b10 + b12)) / 18.;
	vec3 v3 = (4.*p3 + 2.*(p2 + p0) + p1) / 9.;
	vec3 b31 = e3 + (e3 - v3) / 2.;
	oPatch.B31 = b31;
} 


void main() {                                                                                                                                                                           
    CalcPositions();                                                                            
                                                                                                
    gl_TessLevelOuter[0] = TessellationLevel;
	gl_TessLevelOuter[1] = TessellationLevel;
	gl_TessLevelOuter[2] = TessellationLevel;
	gl_TessLevelOuter[3] = TessellationLevel;
	gl_TessLevelInner[0] = TessellationLevel;
	gl_TessLevelInner[1] = TessellationLevel;
}
