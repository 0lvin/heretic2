#version 450

// PN patch data
struct PnPatch
{
 float b210;
 float b120;
 float b021;
 float b012;
 float b102;
 float b201;
 float b111;
 float n110;
 float n011;
 float n101;
};

layout (triangles, fractional_odd_spacing, cw) in;

layout (location = 0) in vec4 inColor[];
layout (location = 1) in vec2 inUV[];
layout (location = 2) in flat int inTex[];
layout (location = 3) in vec3 inNormal[];
layout (location = 4) in PnPatch iPnPatch[];

layout(location = 0) out vec4 color;
layout(location = 1) out vec2 outUV;
layout(location = 2) out flat int textured;
layout(location = 3) out vec3 outNormal;

#define uvw gl_TessCoord

void main(void)
{
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
            (gl_TessCoord.y * gl_in[1].gl_Position) +
            (gl_TessCoord.z * gl_in[2].gl_Position);

    outNormal = gl_TessCoord.x*inNormal[0] + gl_TessCoord.y*inNormal[1] + gl_TessCoord.z*inNormal[2];
    outUV = gl_TessCoord.x*inUV[0] + gl_TessCoord.y*inUV[1] + gl_TessCoord.z*inUV[2];

    color = (inColor[0] + inColor[1] + inColor[2]) / 3;
    textured = inTex[0] | inTex[1] | inTex[2];
}

/*
void main()
{
    float tessAlpha = 0.0;

    vec3 uvwSquared = uvw * uvw;
    vec3 uvwCubed   = uvwSquared * uvw;

    // extract control points
    vec3 b210 = vec3(iPnPatch[0].b210, iPnPatch[1].b210, iPnPatch[2].b210);
    vec3 b120 = vec3(iPnPatch[0].b120, iPnPatch[1].b120, iPnPatch[2].b120);
    vec3 b021 = vec3(iPnPatch[0].b021, iPnPatch[1].b021, iPnPatch[2].b021);
    vec3 b012 = vec3(iPnPatch[0].b012, iPnPatch[1].b012, iPnPatch[2].b012);
    vec3 b102 = vec3(iPnPatch[0].b102, iPnPatch[1].b102, iPnPatch[2].b102);
    vec3 b201 = vec3(iPnPatch[0].b201, iPnPatch[1].b201, iPnPatch[2].b201);
    vec3 b111 = vec3(iPnPatch[0].b111, iPnPatch[1].b111, iPnPatch[2].b111);

    // extract control normals
    vec3 n110 = normalize(vec3(iPnPatch[0].n110, iPnPatch[1].n110, iPnPatch[2].n110));
    vec3 n011 = normalize(vec3(iPnPatch[0].n011, iPnPatch[1].n011, iPnPatch[2].n011));
    vec3 n101 = normalize(vec3(iPnPatch[0].n101, iPnPatch[1].n101, iPnPatch[2].n101));

    // compute texcoords
    outUV  = gl_TessCoord[2]*inUV[0] + gl_TessCoord[0]*inUV[1] + gl_TessCoord[1]*inUV[2];

    // normal
    // Barycentric normal
    vec3 barNormal = gl_TessCoord[2]*inNormal[0] + gl_TessCoord[0]*inNormal[1] + gl_TessCoord[1]*inNormal[2];
    vec3 pnNormal  = inNormal[0]*uvwSquared[2] + inNormal[1]*uvwSquared[0] + inNormal[2]*uvwSquared[1]
                   + n110*uvw[2]*uvw[0] + n011*uvw[0]*uvw[1]+ n101*uvw[2]*uvw[1];
    outNormal = tessAlpha*pnNormal + (1.0-tessAlpha) * barNormal;

    // compute interpolated pos
    vec3 barPos = gl_TessCoord[2]*gl_in[0].gl_Position.xyz
                + gl_TessCoord[0]*gl_in[1].gl_Position.xyz
                + gl_TessCoord[1]*gl_in[2].gl_Position.xyz;

    // save some computations
    uvwSquared *= 3.0;

    // compute PN position
    vec3 pnPos  = gl_in[0].gl_Position.xyz*uvwCubed[2]
                + gl_in[1].gl_Position.xyz*uvwCubed[0]
                + gl_in[2].gl_Position.xyz*uvwCubed[1]
                + b210*uvwSquared[2]*uvw[0]
                + b120*uvwSquared[0]*uvw[2]
                + b201*uvwSquared[2]*uvw[1]
                + b021*uvwSquared[0]*uvw[1]
                + b102*uvwSquared[1]*uvw[2]
                + b012*uvwSquared[1]*uvw[0]
                + b111*6.0*uvw[0]*uvw[1]*uvw[2];

    // final position and normal
    vec3 finalPos = (1.0-tessAlpha)*barPos + tessAlpha*pnPos;
    gl_Position   = vec4(finalPos,1.0);

    color = (inColor[0] + inColor[1] + inColor[2]) / 3;
    textured = inTex[0] | inTex[1] | inTex[2];
}
*/
