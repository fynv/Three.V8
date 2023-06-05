import { wgsl } from '../../wgsl-preprocessor.js';

function get_shader(has_tangent)
{
    let idx_uniform = has_tangent?11:7;

    return wgsl`
@group(0) @binding(0)
var<storage, read> rela_mats : array<mat4x4f>;

@group(0) @binding(1)
var<storage, read> joints : array<vec4u>;

@group(0) @binding(2)
var<storage, read> weights : array<vec4f>;

@group(0) @binding(3)
var<storage, read> pos_rests : array<vec4f>;

@group(0) @binding(4)
var<storage, read_write> pos_out : array<vec4f>;

@group(0) @binding(5)
var<storage, read> norm_rests : array<vec4f>;

@group(0) @binding(6)
var<storage, read_write> norm_out : array<vec4f>;

#if ${has_tangent}
@group(0) @binding(7)
var<storage, read> tangent_rests : array<vec4f>;

@group(0) @binding(8)
var<storage, read_write> tangent_out : array<vec4f>;

@group(0) @binding(9)
var<storage, read> bitangent_rests : array<vec4f>;

@group(0) @binding(10)
var<storage, read_write> bitangent_out : array<vec4f>;

#endif

@group(0) @binding(${idx_uniform})
var<uniform> num_verts: i32;

@compute @workgroup_size(128,1,1)
fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>)
{
    let idx = i32(GlobalInvocationID.x);
    if (idx>=num_verts) 
    {
        return;
    }

    let joint = joints[idx];
    let weight = weights[idx];

    let mat = 
        weight.x * rela_mats[joint.x] +
        weight.y * rela_mats[joint.y] +
        weight.z * rela_mats[joint.z] + 
        weight.w * rela_mats[joint.w];

    pos_out[idx] = mat * pos_rests[idx];
    norm_out[idx] = mat * norm_rests[idx];

#if ${has_tangent}
    tangent_out[idx] = mat * tangent_rests[idx];
    bitangent_out[idx] = mat * bitangent_rests[idx];
#endif

}
`;
}


function GetPipelineSkin(options)
{
    let signature = JSON.stringify(options);
    if (!("skin" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.skin = {};        
    }
    
    if (!(signature in engine_ctx.cache.pipelines.skin))
    {
        let shader_code = get_shader(options.has_tangent);
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.skin[signature]];
        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);

        engine_ctx.cache.pipelines.skin[signature]= engine_ctx.device.createComputePipeline({
            layout,
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }

    return engine_ctx.cache.pipelines.skin[signature];

}

export function SkinUpdate(passEncoder, primitive)
{
    let options = {
        has_tangent: primitive.geometry[0].tangent_buf != null,
    };

    let pipeline = GetPipelineSkin(options);

    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, primitive.bind_group_skin);
    passEncoder.dispatchWorkgroups(Math.floor((primitive.num_pos + 127) / 128), 1,1);        

}
