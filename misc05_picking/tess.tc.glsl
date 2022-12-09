#version 440 core

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
    vec3 Color[3];                                                                        
};  


layout (vertices = 3) out;
uniform float TessellationLevel;

in vec3 WorldPos_CS_in[];    
in vec3 Normal_CS_in[];                                                                         
in vec2 TexCoord_CS_in[];
//in vec4 Color_CS_in[];


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
    /*oPatch.B030 = WorldPos_CS_in[0];                                                   
    oPatch.B003 = WorldPos_CS_in[1];                                                   
    oPatch.B300 = WorldPos_CS_in[2];
    
    vec3 EdgeB300 = oPatch.B003 - oPatch.B030;                                
    vec3 EdgeB030 = oPatch.B300 - oPatch.B003;                                
    vec3 EdgeB003 = oPatch.B030 - oPatch.B300;                                
                                                                                                
    oPatch.B021 = oPatch.B030 + EdgeB300 / 3.0;                               
    oPatch.B012 = oPatch.B030 + EdgeB300 * 2.0 / 3.0;                         
    oPatch.B102 = oPatch.B003 + EdgeB030 / 3.0;                               
    oPatch.B201 = oPatch.B003 + EdgeB030 * 2.0 / 3.0;                         
    oPatch.B210 = oPatch.B300 + EdgeB003 / 3.0;                               
    oPatch.B120 = oPatch.B300 + EdgeB003 * 2.0 / 3.0;                         
                                                                                                
    oPatch.B021 = ProjectToPlane(oPatch.B021, oPatch.B030, oPatch.Normal[0]);     
    oPatch.B012 = ProjectToPlane(oPatch.B012, oPatch.B003, oPatch.Normal[1]);     
    oPatch.B102 = ProjectToPlane(oPatch.B102, oPatch.B003, oPatch.Normal[1]);     
    oPatch.B201 = ProjectToPlane(oPatch.B201, oPatch.B300, oPatch.Normal[2]);     
    oPatch.B210 = ProjectToPlane(oPatch.B210, oPatch.B300, oPatch.Normal[2]);     
    oPatch.B120 = ProjectToPlane(oPatch.B120, oPatch.B030, oPatch.Normal[0]);
                                                                                                             
    vec3 Center = (oPatch.B003 + oPatch.B030 + oPatch.B300) / 3.0;                
    oPatch.B111 = (oPatch.B021 + oPatch.B012 + oPatch.B102 +             
                            oPatch.B201 + oPatch.B210 + oPatch.B120) / 6.0;       
    oPatch.B111 += (oPatch.B111 - Center) / 2.0;*/

    
    oPatch.B300 = WorldPos_CS_in[0];                                                   
    oPatch.B030 = WorldPos_CS_in[1];                                                   
    oPatch.B003 = WorldPos_CS_in[2];

    vec3 b300 = oPatch.B300, b003 = oPatch.B003, b030 = oPatch.B030;
    vec3 p1 = oPatch.B300, p2 = oPatch.B030, p3 = oPatch.B003;
    vec3 n1 = oPatch.Normal[0], n2 = oPatch.Normal[1], n3 = oPatch.Normal[2];

    float w12 = dot(p2 - p1, n1);
    float w21 = dot(p1 - p2, n2);
    float w23 = dot(p3 - p2, n2);
    float w32 = dot(p2 - p3, n3);
    float w31 = dot(p1 - p3, n3);
    float w13 = dot(p3 - p1, n1);

    vec3 b210 = (2.*p1 + p2 - w12*n1) / 3.;
    vec3 b120 = (2.*p2 + p1 - w21*n2) / 3.;
    vec3 b021 = (2.*p2 + p3 - w23*n2) / 3.;
    vec3 b012 = (2.*p3 + p2 - w32*n3) / 3.;
    vec3 b102 = (2.*p3 + p1 - w31*n3) / 3.;
    vec3 b201 = (2.*p1 + p3 - w13*n1) / 3.;

    /*b021 = ProjectToPlane(b021, b030, oPatch.Normal[0]);     
    b012 = ProjectToPlane(b012, b003, oPatch.Normal[1]);     
    b102 = ProjectToPlane(b102, b003, oPatch.Normal[1]);     
    b201 = ProjectToPlane(b201, b300, oPatch.Normal[2]);     
    b210 = ProjectToPlane(b210, b300, oPatch.Normal[2]);     
    b120 = ProjectToPlane(b120, b030, oPatch.Normal[0]);*/

    vec3 ee = (b120 + b120 + b021 + b012 + b102 + b210) / 6.;
    vec3 vv = (p1 + p2 + p3) / 3.;
    vec3 b111 = ee + (ee - vv) / 2.;

    oPatch.B021 = b021;                           
    oPatch.B012 = b012;                         
    oPatch.B102 = b102;                           
    oPatch.B201 = b201;                        
    oPatch.B210 = b210;                            
    oPatch.B120 = b120;
    oPatch.B111 = b111;

} 


void main() {
    for (int i = 0 ; i < 3 ; i++) {                                                             
        oPatch.Normal[i] = Normal_CS_in[i];                                                     
        oPatch.TexCoord[i] = TexCoord_CS_in[i];
        //oPatch.Color[i] = vec3(Color_CS_in[i]);
    }                                                                                           
                                                                                                
    CalcPositions();                                                                            
                                                                                                
    gl_TessLevelOuter[0] = TessellationLevel;                                
    gl_TessLevelOuter[1] = TessellationLevel;                                                 
    gl_TessLevelOuter[2] = TessellationLevel;                                                  
    gl_TessLevelInner[0] = TessellationLevel; 
}
