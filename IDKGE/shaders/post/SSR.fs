#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"
#include "../include/noise.glsl"
#include "../include/denoise.glsl"
#include "../include/time.glsl"
#include "../include/taa.glsl"


#define SSGI_DIFFUSE


out vec4 fsout_frag_color;
in vec2 fsin_texcoords;

uniform sampler2D un_input[8];

uniform sampler2D un_albedo;
uniform sampler2D un_normal;
uniform sampler2D un_pbr;
uniform sampler2D un_velocity;
uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;
uniform sampler2D un_occlusion;
uniform sampler2D un_prev_SSGI;
uniform samplerCube un_skybox;
uniform samplerCube un_skybox_diffuse;
uniform samplerCube un_skybox_specular;

uniform float un_factor    = 4.0;
uniform float un_intensity = 1.0;


#define RAY_OFFSET 0.02
#define RAY_STEP_SIZE 1.0
#define RAY_MAX_STEPS 128


float getFadeFactor()
{
    vec2 texcoord = gl_FragCoord.xy / textureSize(un_albedo, 0);

    float edgeThickness = 0.1;

    float edgeFadeFactor = smoothstep(0.0, edgeThickness, texcoord.x) * 
                           smoothstep(0.0, edgeThickness, texcoord.y) *
                           smoothstep(1.0, 1.0 - edgeThickness, texcoord.x) * 
                           smoothstep(1.0, 1.0 - edgeThickness, texcoord.y);


    return edgeFadeFactor;
}


float DepthToViewZ( sampler2D depthtex, vec2 uv, mat4 P )
{
    float z = textureLod(depthtex, uv, 0.0).r * 2.0 - 1.0;

    vec4 pos  = vec4(uv * 2.0 - 1.0, z, 1.0);
         pos  = inverse(P) * vec4(pos.xyz, 1.0);
         pos /= pos.w;
    
    return pos.z;
}


vec2 rotateUV( vec2 uv, float rotation )
{
    float mid = 0.5;

    return vec2(
        cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
        cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
    );
}



vec2 bin_search( mat4 P, vec3 start, vec3 end )
{
    vec3 rmin = start;
    vec3 rmax = end;
    vec3 rmid;

    vec2 uv;

    for (int i=0; i<8; i++)
    {
        rmid = mix(rmin, rmax, 0.5);

        vec4 proj = P * vec4(rmid.xyz, 1.0);
        uv = (proj.xy / proj.w) * 0.5 + 0.5;

        float ray_z = rmid.z;
        float tex_z = DepthToViewZ(un_fragdepth, uv, P);

        if (rmid.z < tex_z)
        {
            rmax = rmid;
        }

        else
        {
            rmin = rmid;
        }
    }

    return uv;
}



vec3 raymarch( int steps, float maxdist, mat4 P, mat4 V, vec3 R, float roughness, float n1, vec3 rp, vec3 rd,
               vec3 N, vec3 Kd, vec3 brdf, vec3 albedo )
{
    // #define MAXDIST    32.0
    #define RESOLUTION (maxdist / float(steps))
    #define THICKNESS  -(RESOLUTION - 0.25)
    // #define THICKNESS  -0.2

    rp += n1*RESOLUTION*rd;

    int  level  = min(int(5.0*roughness), 5);
    vec2 uv     = vec2(0.0);
    vec3 result = vec3(0.0);

    for (int i=0; i<steps; i++)
    {
        rp += RESOLUTION*rd;

        vec4 proj = P * vec4(rp.xyz, 1.0);
        uv   = (proj.xy / proj.w) * 0.5 + 0.5;

        if (uv.x <= 0.0 || uv.x >= 1.0 || uv.y <= 0.0 || uv.y >= 1.0)
        {
            return vec3(0, 0, 0);
        }

        float ray_z = rp.z;
        float tex_z = DepthToViewZ(un_fragdepth, uv, P);
        float delta = ray_z - tex_z;

        if (delta < 0.0 && delta > THICKNESS)
        {
            uv = bin_search(P, rp-RESOLUTION*rd, rp);
    
            vec3 N = textureLod(un_normal, uv, 0.0).xyz;
                 N = normalize((V * vec4(N, 0.0)).xyz);

            result  = textureLod(un_input[level], uv, 0.0).rgb;
            result *= (dot(N, rd) > 0.0) ? 0.0 : 1.0;
            return result;
        }
    }

    // vec3 irradiance = textureLod(un_skybox_diffuse, N, roughness).rgb;
    // vec3 diffuse    = Kd * irradiance * albedo;
    // vec3 prefilter  = textureLod(un_skybox_specular, R, roughness*4.0).rgb;
    // vec3 specular   = prefilter * brdf;
    // vec3 ambient    = (specular); // * ao;

    // return ambient;
    return result;
}


const vec2 poisson16[] = vec2[]( // These are the Poisson Disk Samples
    vec2( -0.94201624,  -0.39906216 ),
    vec2(  0.94558609,  -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2(  0.34495938,   0.29387760 ),
    vec2( -0.91588581,   0.45771432 ),
    vec2( -0.81544232,  -0.87912464 ),
    vec2( -0.38277543,   0.27676845 ),
    vec2(  0.97484398,   0.75648379 ),
    vec2(  0.44323325,  -0.97511554 ),
    vec2(  0.53742981,  -0.47373420 ),
    vec2( -0.26496911,  -0.41893023 ),
    vec2(  0.79197514,   0.19090188 ),
    vec2( -0.24188840,   0.99706507 ),
    vec2( -0.81409955,   0.91437590 ),
    vec2(  0.19984126,   0.78641367 ),
    vec2(  0.14383161,  -0.14100790 )
);


vec2 randomSeed( vec2 texcoord )
{
    return vec2(fract(sin(dot(texcoord, vec2(12.9898, 78.233))) * 43758.5453));
}

vec2 fetchBlueNoise( vec2 uv, vec2 texcoord )
{
    vec2 seed = randomSeed(texcoord);
    return IDK_BlueNoise(uv + seed).rg;
}

vec3 sampleBlueNoiseHemisphere( vec2 texcoord )
{
    vec2 noise = fetchBlueNoise(texcoord.xy, texcoord);
    float theta = noise.x * 2.0 * PBR_PI;  // Azimuth angle
    float phi = acos(1.0 - 2.0 * noise.y); // Elevation angle
    return vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
}


void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();
    vec3 viewpos = camera.position.xyz;

    vec2 texcoord = fsin_texcoords;
    vec3 fragpos  = vec3(0.0);

    {
        float depth = textureLod(un_fragdepth, texcoord, 0.0).r;

        vec4 ndc;
             ndc.xy = texcoord * 2.0 - 1.0;
             ndc.z  = depth * 2.0 - 1.0;
             ndc.w  = 1.0;

        vec4 view = inverse(camera.V) * inverse(camera.P) * ndc;

        fragpos = (view.xyz / view.w);
    }


    vec4 albedo = textureLod(un_albedo, texcoord, 0.0).rgba;
    vec3 N      = normalize(textureLod(un_normal, texcoord, 0.0).xyz);
    vec3 V      = normalize(viewpos - fragpos);
    vec3 R      = reflect(-V, N);
    vec4 pbr    = textureLod(un_pbr, texcoord, 0.0).rgba;

    float roughness = pbr[0];
    float metallic  = pbr[1];

    vec3  F0      = clamp(mix(vec3(0.04), albedo.rgb, metallic), 0.0, 1.0);
    float NdotV   = max(dot(N, V), 0.0);
    vec3  fresnel = fresnelSchlickR(NdotV, F0, roughness);

    vec2  tsize = textureSize(un_albedo, 0);
    // vec2  texel = tsize*texcoord;
    vec2  texel = tsize*texcoord + float(IDK_GetFrame() % 5);
    vec3  bnoise = IDK_BlueNoiseTexel(ivec2(texel)).rgb;
    vec3  nnoise = bnoise * 2.0 - 1.0;
    float n1 = bnoise[0];



    if (albedo.a < 1.0)
    {
        fsout_frag_color = vec4(0.0);
        return;
    }


    vec3 Kd   = (vec3(1.0) - fresnel) * (1.0 - metallic);
    vec2 BRDF = textureLod(un_BRDF_LUT, vec2(NdotV, roughness), 0.0).rg;
    vec3 brdf = (fresnel * BRDF.x + BRDF.y);
    float ao  = textureLod(un_occlusion, texcoord, 0.0).r;



    vec3  ro = (camera.V * vec4(fragpos, 1.0)).xyz;
    vec3  spec_rd = normalize((camera.V * vec4(R, 0.0)).xyz);

    vec3 norm_vspace = normalize((camera.V * vec4(N, 0.0)).xyz);
    vec3 specular = raymarch(8, 16.0, camera.P, camera.V, R, roughness, n1, ro, spec_rd, N, Kd, brdf, albedo.rgb);
    vec3 diffuse  = vec3(0.0);


    {
        vec2  wnoise = vec2(IDK_GetIrrational()); // IDK_WhiteNoiseTexel(ivec2(texel)).rg;
        float theta  = 1.0 * 3.14159 * (wnoise.r);

        // vec3[4] noise = vec3[4](
        //     IDK_BlueNoiseTexel(ivec2(texel2) + ivec2( 0,  0)).rgb,
        //     IDK_BlueNoiseTexel(ivec2(texel2) + ivec2( 0, -1)).rgb,
        //     IDK_BlueNoiseTexel(ivec2(texel2) + ivec2(+1,  0)).rgb,
        //     IDK_BlueNoiseTexel(ivec2(texel2) + ivec2(-1, +1)).rgb
        // );

        vec2 offsets[9] = vec2[9](
            vec2(-1, -1) / tsize,
            vec2(-1,  0) / tsize,
            vec2(-1, +1) / tsize,
            vec2( 0, -1) / tsize,
            vec2( 0,  0) / tsize,
            vec2( 0, +1) / tsize,
            vec2(+1, -1) / tsize,
            vec2(+1,  0) / tsize,
            vec2(+1, +1) / tsize
        );

        for (int i=0; i<9; i++)
        {
            vec2  noise_uv    = texcoord + rotateUV((4.0*poisson16[i]) / tsize, theta);
            ivec2 noise_texel = ivec2(tsize * noise_uv);
            vec3  noise       = IDK_BlueNoiseTexel(noise_texel).rgb * 2.0 - 1.0;

            vec3 diff_rd  = normalize(noise);
                 diff_rd *= sign(dot(diff_rd, norm_vspace));

            diffuse += raymarch(8, 16.0, camera.P, camera.V, R, 0.0, n1, ro, diff_rd, N, Kd, brdf, albedo.rgb);
        }
        // diffuse *= albedo.rgb;
        diffuse *= (un_intensity / 9.0);
    }

    vec3 curr_ssgi = (Kd*diffuse + brdf*specular);
    vec2 curr_vel  = UnpackVelocity(textureLod(un_velocity, texcoord, 0.0)).xy;
    vec2 prev_uv   = clamp(texcoord-curr_vel, vec2(0.0), vec2(1.0));
    vec3 prev_ssgi = vec3(0.0);

    // if (prev_uv.x < 0.5)
    // {
    //     prev_ssgi = texture(un_prev_SSGI, prev_uv).rgb;
    // }

    // else
    {
        float d = 5.0;
        float s = 2.5;
        float t = 0.012;
        prev_ssgi = smartDeNoise(un_prev_SSGI, prev_uv, d, s, t).rgb;
    }

    vec3 result = mix(prev_ssgi, curr_ssgi, 1.0/un_factor);

    fsout_frag_color = vec4(result, 1.0);
}



