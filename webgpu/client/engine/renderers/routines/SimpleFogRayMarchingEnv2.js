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

struct ProbeGrid
{
    coverageMin: vec4f,
    coverageMax: vec4f,
    baseDivisions: vec4i,
    subDivisionLevel: i32,
    normalBias: f32,
    numProbes: i32,
    visRes: i32,
    packSize: i32,
    packRes: i32,
    irrRes: i32,
    irrPackRes: i32,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

@group(3) @binding(0)
var<uniform> uProbeGrid: ProbeGrid;

@group(3) @binding(1)
var<storage, read> bPosLod: array<vec4f>;

@group(3) @binding(2)
var<storage, read> bProbeSH0: array<vec4f>;

@group(3) @binding(3)
var<storage, read> bIndexData: array<i32>;

@group(3) @binding(4)
var uSampler: sampler;

@group(3) @binding(5)
var uTexVis: texture_2d<f32>;

fn signNotZero(v: vec2f) -> vec2f
{
    return vec2(select(-1.0, 1.0, v.x >= 0.0), select(-1.0, 1.0, v.y >= 0.0));
}

fn vec3_to_oct(v: vec3f) -> vec2f
{
    let p = v.xy * (1.0/ (abs(v.x) + abs(v.y) + abs(v.z)));
    return select(p, ((1.0 - abs(p.yx)) * signNotZero(p)), v.z <= 0.0);
}


fn get_mean_dis_common(dir: vec3f, idx: i32) -> vec2f
{
    let probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
    let pack_x = idx % uProbeGrid.packSize;
    let pack_y = idx / uProbeGrid.packSize;
    let uv = (vec2(f32(pack_x), f32(pack_y)) * f32(uProbeGrid.visRes + 2) + (probe_uv * f32(uProbeGrid.visRes) + 1.0))/f32(uProbeGrid.packRes);
    return textureSampleLevel(uTexVis, uSampler, uv, 0).xy;
}

fn get_visibility_common(wpos: vec3f, idx: i32, vert_world: vec3f, scale: f32) -> f32
{
    var dir = wpos - vert_world;
    let dis = length(dir);
    dir = normalize(dir);

    let mean_dis_var = get_mean_dis_common(dir, idx);
    let mean_dis = mean_dis_var.x;
    let mean_var = mean_dis_var.y;
    let mean_var2 =  mean_var * mean_var;
    let delta = max(dis - mean_dis, 0.0);
    let delta2 = delta*delta * scale * scale;	
    return mean_var2/(mean_var2 + delta2);
}

fn get_visibility(wpos: vec3f, idx: i32, lod: i32, vert_world: vec3f) -> f32
{
    let scale = f32(1<<u32(uProbeGrid.subDivisionLevel -lod));
    return get_visibility_common(wpos, idx, vert_world, scale);
}

fn get_probe_idx(ipos: vec3i) -> i32
{
    let base_offset = uProbeGrid.baseDivisions.x * uProbeGrid.baseDivisions.y * uProbeGrid.baseDivisions.z;
    let ipos_base = ipos / (1<<u32(uProbeGrid.subDivisionLevel));
    var node_idx = ipos_base.x + (ipos_base.y + ipos_base.z *  uProbeGrid.baseDivisions.y) *  uProbeGrid.baseDivisions.x;
    var probe_idx = bIndexData[node_idx];

    var lod = 0;
    var digit_mask = 1 << u32(uProbeGrid.subDivisionLevel -1);
    while(lod<uProbeGrid.subDivisionLevel && probe_idx>=uProbeGrid.numProbes)
    {
        let offset = base_offset + (probe_idx - uProbeGrid.numProbes)*8;
        var sub = 0;
        if ((ipos.x & digit_mask) !=0) 
        {
            sub+=1;
        }
		if ((ipos.y & digit_mask) !=0) 
        {
            sub+=2;
        }
		if ((ipos.z & digit_mask) !=0) 
        {
            sub+=4;
        }
        node_idx = offset + sub;
        probe_idx = bIndexData[node_idx];

        lod++;
		digit_mask >>=1;
    }
    return probe_idx;
}

fn getIrradiance(world_pos: vec3f) -> vec3f
{
    let divs = uProbeGrid.baseDivisions.xyz * (1<<u32(uProbeGrid.subDivisionLevel));

    let size_grid = uProbeGrid.coverageMax.xyz - uProbeGrid.coverageMin.xyz;
    var pos_normalized = (world_pos - uProbeGrid.coverageMin.xyz)/size_grid;    
    var pos_voxel = pos_normalized * vec3f(divs) - vec3(0.5);
    pos_voxel = clamp(pos_voxel, vec3(0.0), vec3f(divs) - vec3(1.0));

    let i_voxel = clamp(vec3i(pos_voxel), vec3i(0), divs - vec3i(2));
    let frac_voxel = pos_voxel - vec3f(i_voxel);

    var sum_weight = 0.0;
    var coeffs0 = vec4(0.0);

    for (var i=0; i<8; i++)
    {
        let x = i & 1;
        let y = (i>>1)&1;
        let z = i>>2;
        let vert = i_voxel + vec3i(x,y,z);
        let idx_probe = get_probe_idx(vert);
        let pos_lod = bPosLod[idx_probe];
        let probe_world = pos_lod.xyz;
        var weight = get_visibility(world_pos, idx_probe, i32(pos_lod.w), probe_world);

        let crushThreshold = 0.2;
        if (weight < crushThreshold) {
            weight *= weight * weight / (crushThreshold*crushThreshold); 
        }

        let w = vec3(1.0) - abs(vec3f(f32(x),f32(y),f32(z)) - frac_voxel);
        weight *= w.x * w.y * w.z;
        if (weight> 0.0)
        {					
            sum_weight += weight;            
            coeffs0 += bProbeSH0[idx_probe]*weight;           
        }
    }

    if (sum_weight>0.0)
	{        
		coeffs0/=sum_weight;
		return coeffs0.xyz * 0.886227;
	}	

	return vec3(0.0);
}

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
    var step = dis/f32(uFog.max_num_steps)*8.0;
    if (step<uFog.min_step*8.0) 
    {
        step = uFog.min_step*8.0;
    }

    let step_alpha =  1.0 - pow(1.0 - uFog.rgba.w, step);

    let delta = IGN(i32(coord_pix.x), i32(coord_pix.y));

    var col = vec3(0.0);
    let start =  step * (delta - 0.5);
    for (var t = start; t<dis; t+= step)
    {
        let _step = min(step, dis - t);
        let _step_alpha = select(1.0 - pow(1.0 - uFog.rgba.w, _step), step_alpha, _step == step);
        let sample_t = max(t + _step*0.5, 0.0);

        let pos_world = uCamera.eyePos + dir * sample_t;
        let att =  pow(1.0 - uFog.rgba.w, sample_t);

        let irradiance = getIrradiance(pos_world);

		col+=uFog.rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
    }
    return vec4(col, 0.0);
}

`;
}

function GetPipelineSimpleFogRayMarchingEnv2(options)
{
    let signature = JSON.stringify(options);
    if (!("fog_ray_marching_env_simple2" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.fog_ray_marching_env_simple2 = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.fog_ray_marching_env_simple2))
    {
        let options2 = { msaa: options.is_msaa};
        let signature2 = JSON.stringify(options2);
        let bindGroupLayouts = [
            engine_ctx.cache.bindGroupLayouts.perspective_camera, 
            engine_ctx.cache.bindGroupLayouts.fog, 
            engine_ctx.cache.bindGroupLayouts.depth[signature2], 
            engine_ctx.cache.bindGroupLayouts.fog_lod_probe_grid
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

        engine_ctx.cache.pipelines.fog_ray_marching_env_simple2[signature] =  engine_ctx.device.createRenderPipeline(pipelineDesc);
    }

    return engine_ctx.cache.pipelines.fog_ray_marching_env_simple2[signature];
}

export function SimpleFogRayMarchingEnv2(passEncoder, params)
{
    let options = {}; 
    options.is_msaa  = params.target.msaa;
    options.view_format= params.target.view_format;

    let pipeline = GetPipelineSimpleFogRayMarchingEnv2(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, params.bind_group_camera);
    passEncoder.setBindGroup(1, params.bind_group_fog);
    passEncoder.setBindGroup(2, params.target.bind_group_depth);
    passEncoder.setBindGroup(3, params.lights.bind_group_fog_indirect);
    passEncoder.draw(3, 1);    

}


export function SimpleFogRayMarchingEnvBundle2(params)
{
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [params.target.view_format],                
    });
    SimpleFogRayMarchingEnv2(renderBundleEncoder, params);
    return renderBundleEncoder.finish();
}


