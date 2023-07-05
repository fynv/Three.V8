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

struct DirectionalLight
{
    color: vec4f,
    origin: vec4f,
    direction: vec4f,
    diffuse_thresh: f32,
    diffuse_high: f32,
    diffuse_low: f32,
    specular_thresh: f32,
    specular_high: f32,
    specular_low: f32
};

@group(3) @binding(0)
var<uniform> uDirectionalLight: DirectionalLight;

${condition(options.has_shadow,`
@group(3) @binding(1)
var uShadowSampler: sampler_comparison;

struct DirectionalShadow
{
    VPSBMat: mat4x4f,
    projMat: mat4x4f,
    viewMat: mat4x4f,
    left_right: vec2f,
    bottom_up: vec2f,
    near_far: vec2f,
    light_radius: f32,
    bias: f32
};

@group(3) @binding(2)
var<uniform> uDirectionalShadow: DirectionalShadow;

@group(3) @binding(3)
var uDirectionalShadowTex: texture_depth_2d;

fn borderPCFTexture(uvz : vec3f) -> f32
{
	return select(select(0.0, 1.0, uvz.z <= 1.0),  textureSampleCompareLevel(uDirectionalShadowTex, uShadowSampler, uvz.xy, uvz.z), 
        ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&  (uvz.x >= 0.0) && (uvz.y >= 0.0)));
}

`)}

var<private> seed : u32;
fn InitRandomSeed(val0: u32, val1: u32) 
{
    var v0 = val0;
    var v1 = val1;
    var s0 = 0u;

    for (var n = 0u; n<16u; n++)
    {
        s0 += 0x9e3779b9u;
		v0 += ((v1 << 4) + 0xa341316cu) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4u);
		v1 += ((v0 << 4) + 0xad90777du) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761eu);
    }

    seed = v0;
}

fn RandomInt() -> u32
{
    seed = 1664525u * seed + 1013904223u;
    return seed;
}

fn RandomFloat() -> f32
{
    return (f32(RandomInt() & 0x00FFFFFFu) / f32(0x01000000));
}

fn IGN(pixelX: i32, pixelY: i32) -> f32
{
    return fract(52.9829189 * fract(0.06711056*f32(pixelX) + 0.00583715*f32(pixelY)));
}

const RECIPROCAL_PI = 0.3183098861837907;


${condition(!options.has_shadow,`
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

    let dir = (uCamera.invViewMat * vec4(pos_view.xyz/dis, 0.0)).xyz;

    var start = 0.0;
${condition(options.is_reflect,`
    let pos_refl_eye = uMatrixReflector * vec4(uCamera.eyePos, 1.0);
    let dir_refl = uMatrixReflector * vec4(dir, 0.0);
    if (dir_refl.z * pos_refl_eye.z>0) 
    {
        return vec4(0.0);
    }
    start = -pos_refl_eye.z/dir_refl.z;
`)}

    var step = dis/f32(uFog.max_num_steps);
    if (step<uFog.min_step) 
    {
        step = uFog.min_step;
    }

    let step_alpha =  1.0 - pow(1.0 - uFog.rgba.w, step);

    let delta = IGN(i32(coord_pix.x), i32(coord_pix.y));

    var col = vec3(0.0);
    start +=  step * (delta - 0.5);
    for (var t = start; t<dis; t+= step)
    {
        let _step = min(step, dis - t);
        let _step_alpha = select(1.0 - pow(1.0 - uFog.rgba.w, _step), step_alpha, _step == step);
        let sample_t = max(t + _step*0.5, 0.0);
        
        let pos_world = uCamera.eyePos + dir * sample_t;
        var l_shadow = 1.0;
        let zEye = -dot(pos_world - uDirectionalLight.origin.xyz, uDirectionalLight.direction.xyz);
        if (zEye>0.0)
		{
			let att = pow(1.0 - uFog.rgba.w, zEye);
			l_shadow *= att;
		}	
        let att =  pow(1.0 - uFog.rgba.w, sample_t);
        let irradiance = uDirectionalLight.color.xyz * l_shadow * 0.25;
        col+=uFog.rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
    }
    return vec4(col, 0.0);
}
`,`
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

        let dir = (uCamera.invViewMat * vec4(pos_view.xyz/dis, 0.0)).xyz;

        var start = 0.0;
        var end = dis;
        {
            let origin_light = (uDirectionalShadow.viewMat * vec4(uCamera.eyePos, 1.0)).xyz;
            let dir_light = (uDirectionalShadow.viewMat * vec4(dir, 0.0)).xyz;

            var min_dis = vec3(0.0);
            var max_dis = vec3(dis);
            if (dir_light.x!=0.0)
            {
                let dis1 = (uDirectionalShadow.left_right.x - origin_light.x)/dir_light.x;
                let dis2 = (uDirectionalShadow.left_right.y - origin_light.x)/dir_light.x;
                min_dis.x = min(dis1, dis2);
			    max_dis.x = max(dis1, dis2);
            }

            if (dir_light.y!=0.0)
            {
                let dis1 = (uDirectionalShadow.bottom_up.x - origin_light.y)/dir_light.y;
                let dis2 = (uDirectionalShadow.bottom_up.y - origin_light.y)/dir_light.y;
                min_dis.y = min(dis1, dis2);
			    max_dis.y = max(dis1, dis2);
            }

            if (dir_light.z!=0.0)
            {
                let dis1 = (-uDirectionalShadow.near_far.x - origin_light.z)/dir_light.z;
                let dis2 = (-uDirectionalShadow.near_far.y - origin_light.z)/dir_light.z;
                min_dis.z = min(dis1, dis2);
			    max_dis.z = max(dis1, dis2);
            }

            start = max(max(0.0, min_dis.x), max(min_dis.y, min_dis.z));
		    end = min(min(dis, max_dis.x), min(max_dis.y, max_dis.z));
        }

${condition(options.is_reflect,`
        let pos_refl_eye = uMatrixReflector * vec4(uCamera.eyePos, 1.0);
        let dir_refl = uMatrixReflector * vec4(dir, 0.0);
        if (dir_refl.z * pos_refl_eye.z>0) 
        {
            return vec4(0.0);
        }
        start = max(start, -pos_refl_eye.z/dir_refl.z);
`)}
        var step = (end - start)/f32(uFog.max_num_steps);
        if (step<uFog.min_step) 
        {
            step = uFog.min_step;
        }
    
        let step_alpha =  1.0 - pow(1.0 - uFog.rgba.w, step);

        let delta = IGN(i32(coord_pix.x), i32(coord_pix.y));

        var col = vec3(0.0);
        start +=  step * (delta - 0.5);
        for (var t = start; t<end; t+= step)
        {
            let _step = min(step, dis - t);
            let _step_alpha = select(1.0 - pow(1.0 - uFog.rgba.w, _step), step_alpha, _step == step); 
            let sample_t = max(t + _step*0.5, 0.0);

            let pos_world = uCamera.eyePos + dir * sample_t;
            
            let shadowCoords = uDirectionalShadow.VPSBMat * vec4(pos_world, 1.0);   
            var l_shadow = borderPCFTexture(shadowCoords.xyz);
            if (l_shadow>0.0)
            {
                let zEye = -dot(pos_world - uDirectionalLight.origin.xyz, uDirectionalLight.direction.xyz);
                if (zEye>0.0)
                {
                    let att = pow(1.0 - uFog.rgba.w, zEye);
                    l_shadow *= att;
                }	
                let att =  pow(1.0 - uFog.rgba.w, sample_t);
                let irradiance = uDirectionalLight.color.xyz * l_shadow * 0.25;
                col+=uFog.rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
            }

        }
        return vec4(col, 0.0);
}
`)}
`;
}

function GetPipelineFogRayMarching(options)
{
    let signature = JSON.stringify(options);
    if (!("fog_ray_marching" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.fog_ray_marching = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.fog_ray_marching))
    {
        let camera_options = { has_reflector: options.is_reflect };
        let camera_signature =  JSON.stringify(camera_options);
        
        let options2 = { msaa: options.is_msaa};
        let signature2 = JSON.stringify(options2);
        let options3 = { has_shadow: options.has_shadow};
        let signature3 = JSON.stringify(options3);
        let bindGroupLayouts = [
            engine_ctx.cache.bindGroupLayouts.perspective_camera[camera_signature], 
            engine_ctx.cache.bindGroupLayouts.fog, 
            engine_ctx.cache.bindGroupLayouts.depth[signature2], 
            engine_ctx.cache.bindGroupLayouts.fog_directional[signature3]
        ];

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

        engine_ctx.cache.pipelines.fog_ray_marching[signature] =  engine_ctx.device.createRenderPipeline(pipelineDesc);

    }

    return engine_ctx.cache.pipelines.fog_ray_marching[signature];
}

export function FogRayMarching(passEncoder, params)
{
    let num_lights = params.lights.directional_lights.length;
    for (let i=0; i<num_lights; i++)
    {        
        let light = params.lights.directional_lights[i];
        let options = {}; 
        options.is_msaa  = params.target.msaa;
        options.view_format= params.target.view_format;
        options.has_shadow = light.shadow!=null;
        options.is_reflect = params.camera.reflector!=null;

        let pipeline = GetPipelineFogRayMarching(options);
        passEncoder.setPipeline(pipeline);
        passEncoder.setBindGroup(0, params.camera.bind_group);
        passEncoder.setBindGroup(1, params.bind_group_fog);
        passEncoder.setBindGroup(2, params.target.bind_group_depth);
        passEncoder.setBindGroup(3, params.lights.bind_group_fog_directional[i]);
        passEncoder.draw(3, 1);    
    }
}

export function FogRayMarchingBundle(params)
{
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [params.target.view_format],                
    });
    FogRayMarching(renderBundleEncoder, params);
    return renderBundleEncoder.finish();
}


