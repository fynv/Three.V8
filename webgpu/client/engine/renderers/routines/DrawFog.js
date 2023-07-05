function condition(cond, a, b="")
{
    return cond? a: b;
}

function get_shader(options)
{
    return `
struct Camera
{
    projMat: mat4x4f, 
    viewMat: mat4x4f,
    invProjMat: mat4x4f,
    invViewMat: mat4x4f,
    eyePos: vec3f
};

@group(0) @binding(0)
var<uniform> uCamera: Camera;

${condition(options.is_reflect,`
@group(0) @binding(1)
var<uniform> uMatrixReflector: mat4x4f;
`)}

struct VSOut 
{
    @builtin(position) Position: vec4f,
    @location(0) posProj: vec2f
};

@vertex
fn vs_main(@builtin(vertex_index) vertId: u32) -> VSOut 
{
    var vsOut: VSOut;
    let grid = vec2(f32((vertId<<1)&2), f32(vertId & 2));
    let pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
    vsOut.posProj = pos_proj;
    vsOut.Position = vec4(pos_proj, 0.0, 1.0);
    return vsOut;
}

struct Fog
{
    rgba: vec4f,  
    max_num_steps: i32,
    min_step: f32
};

@group(1) @binding(0)
var<uniform> uFog: Fog;

${condition(options.is_msaa,`
@group(2) @binding(0)
var uDepthTex: texture_multisampled_2d<f32>;
`,`
@group(2) @binding(0)
var uDepthTex: texture_2d<f32>;
`)}

const PI = 3.14159265359;
const RECIPROCAL_PI = 0.3183098861837907;

${condition(options.has_ambient_light,`
struct AmbientLight
{
    color: vec4f,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

@group(3) @binding(0)
var<uniform> uIndirectLight: AmbientLight;

fn GetIrradiance() -> vec3f
{
    return uIndirectLight.color.xyz * PI;
}
`,`

${condition(options.has_hemisphere_light,`
struct HemisphereLight
{
    skyColor: vec4f,
    groundColor: vec4f,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

@group(3) @binding(0)
var<uniform> uIndirectLight: HemisphereLight;

fn GetIrradiance() -> vec3f
{
    return (uIndirectLight.skyColor.xyz + uIndirectLight.groundColor.xyz)*0.5*PI;
}
`,`
${condition(options.has_environment_light,`

struct EnvironmentMap
{
    SHCoefficients: array<vec4f, 9>,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

@group(3) @binding(0)
var<uniform> uIndirectLight: EnvironmentMap;

fn GetIrradiance() -> vec3f
{
    return uIndirectLight.SHCoefficients[0].xyz * 0.886227;
}
`,`
fn GetIrradiance() -> vec3f
{
    return vec3(0.0);
}
`)}
`)}
`)}

@fragment
fn fs_main(@builtin(position) coord_pix: vec4f, @location(0) vPosProj: vec2f) -> @location(0) vec4f
{
    let ucoord2d = vec2u(coord_pix.xy);
${condition(options.is_msaa,`
    let depth0 = textureLoad(uDepthTex, ucoord2d, 0).x;
    let depth1 = textureLoad(uDepthTex, ucoord2d, 1).x;
    let depth2 = textureLoad(uDepthTex, ucoord2d, 2).x;
    let depth3 = textureLoad(uDepthTex, ucoord2d, 3).x;
    let depth = 0.5 * (depth0 + depth1 + depth2 + depth3) - 1.0;
`,`
    let depth = textureLoad(uDepthTex, ucoord2d, 0).x * 2.0 - 1.0;
`)}
    var pos_view = uCamera.invProjMat * vec4(vPosProj, depth, 1.0);
    pos_view *= 1.0/pos_view.w;
    let dis = length(pos_view.xyz);

${condition(!options.is_reflect,`
    let length = dis;
`,`
    let dir = (uCamera.invViewMat * vec4(pos_view.xyz/dis, 0.0)).xyz;
    let pos_refl_eye = uMatrixReflector * vec4(uCamera.eyePos, 1.0);
    let dir_refl = uMatrixReflector * vec4(dir, 0.0);
    if (dir_refl.z * pos_refl_eye.z>0) 
    {
        return vec4(0.0);
    }
    let length = dis + pos_refl_eye.z/dir_refl.z;

`)}

    let alpha = 1.0 - pow(1.0 - uFog.rgba.w, length);
    let irradiance = GetIrradiance();
    let col = uFog.rgba.xyz* irradiance * RECIPROCAL_PI * alpha;

    return vec4(col, alpha);
}
`;
}

function GetPipelineDrawFog(options)
{
    let signature = JSON.stringify(options);
    if (!("draw_fog" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.draw_fog = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.draw_fog))
    {
        let camera_options = { has_reflector: options.is_reflect };
        let camera_signature =  JSON.stringify(camera_options);        
        
        let options2 = { msaa: options.is_msaa};
        let signature2 = JSON.stringify(options2);
        let bindGroupLayouts = [
            engine_ctx.cache.bindGroupLayouts.perspective_camera[camera_signature], 
            engine_ctx.cache.bindGroupLayouts.fog, 
            engine_ctx.cache.bindGroupLayouts.depth[signature2], 
        ];        
                
        if (options.has_ambient_light || options.has_hemisphere_light || options.has_environment_light)
        {
            bindGroupLayouts.push(engine_ctx.cache.bindGroupLayouts.fog_indirect);
        }

        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);
        let shaderModule = engine_ctx.device.createShaderModule({ code: get_shader(options) });

        const vertex = {
            module: shaderModule,
            entryPoint: 'vs_main',
            buffers: []
        };

        const colorState = {
            format:  options.view_format,
            blend: {
                color: {
                    srcFactor: "one",
                    dstFactor: "one-minus-src-alpha"
                },
                alpha: {
                    srcFactor: "one",
                    dstFactor: "one-minus-src-alpha"
                },
            },
            writeMask: GPUColorWrite.ALL
        };

        const fragment = {
            module: shaderModule,
            entryPoint: 'fs_main',
            targets: [colorState]
        };

        const primitive = {
            frontFace: 'cw',
            cullMode: 'none',
            topology: 'triangle-list'
        };

        const pipelineDesc = {
            layout,
    
            vertex,
            fragment,
    
            primitive,
        };

        engine_ctx.cache.pipelines.draw_fog[signature] = engine_ctx.device.createRenderPipeline(pipelineDesc);

    }

    return engine_ctx.cache.pipelines.draw_fog[signature];
}

export function DrawFog(passEncoder, params)
{
    let options = {}; 
    options.is_msaa  = params.target.msaa;
    options.view_format= params.target.view_format;
    options.has_ambient_light = params.lights.ambient_light!=null;
    options.has_hemisphere_light = params.lights.hemisphere_light!=null;
    options.has_environment_light = params.lights.environment_map!=null;
    options.is_reflect = params.camera.reflector!=null;

    let pipeline = GetPipelineDrawFog(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, params.camera.bind_group);
    passEncoder.setBindGroup(1, params.bind_group_fog);
    passEncoder.setBindGroup(2, params.target.bind_group_depth);
    if (options.has_ambient_light || options.has_hemisphere_light || options.has_environment_light)
    {
        passEncoder.setBindGroup(3, params.lights.bind_group_fog_indirect);
    }
    passEncoder.draw(3, 1);
}

export function DrawFogBundle(params)
{    
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [params.target.view_format],                
    });
    DrawFog(renderBundleEncoder, params);
    return renderBundleEncoder.finish();
}


