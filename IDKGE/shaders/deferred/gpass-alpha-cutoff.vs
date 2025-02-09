#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/taa.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;

out vec3 fsin_fragpos;
out vec3 fsin_normal;
out vec3 fsin_tangent;
out vec2 fsin_texcoords;
flat out uint drawID;
out IDK_VelocityData fsin_vdata;

out vec3 TBN_viewpos;
out vec3 TBN_fragpos;
out mat3 TBN;
out mat3 TBNT;

uniform uint un_draw_offset;


void main()
{
    drawID = gl_DrawID + un_draw_offset;

    IDK_Camera camera = IDK_UBO_cameras[0];

    const uint offset = IDK_SSBO_transform_offsets[drawID];
    const mat4 model  = IDK_SSBO_transforms[offset + gl_InstanceID];
    const mat4 prev_T = IDK_SSBO_prev_transforms[offset + gl_InstanceID];

    vec4 position = model * vec4(vsin_pos,     1.0);
    vec4 normal   = model * vec4(vsin_normal,  0.0);
    vec4 tangent  = model * vec4(vsin_tangent, 0.0);

    vec3 curr = (model * vec4(vsin_pos, 1.0)).xyz;
    vec3 prev = (prev_T * vec4(vsin_pos, 1.0)).xyz;
    fsin_vdata = PackVData(camera, curr, prev);


    vec3 N = normalize(mat3(model) * normalize(vsin_normal));

    if (dot(N, normalize(camera.position.xyz - position.xyz)) < 0.0)
    {
        N *= -1.0;
    }


    vec3 T = normalize(mat3(model) * normalize(vsin_tangent));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    B = normalize(B - dot(B, N) * N);


    fsin_fragpos   = position.xyz;
    fsin_normal    = N;
    fsin_tangent   = T;
    fsin_texcoords = vsin_texcoords;

    TBN  = mat3(T, B, N);
    TBNT = transpose(TBN);
    TBN_fragpos = TBNT * fsin_fragpos;
    TBN_viewpos = TBNT * camera.position.xyz;

    gl_Position = (camera.P * camera.V) * position;
}