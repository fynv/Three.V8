import { wgsl } from '../../wgsl-preprocessor.js';

function get_shader()
{
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

@group(0) @binding(7)
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
}
`;
}


function GetPipelineSkin()
{    
    if (!("skin" in engine_ctx.cache.pipelines))   
    {
        let shader_code = get_shader();
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.skin];
        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);

        engine_ctx.cache.pipelines.skin = engine_ctx.device.createComputePipeline({
            layout,
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }

    return engine_ctx.cache.pipelines.skin;

}

export function SkinUpdate(passEncoder, primitive)
{
    let pipeline = GetPipelineSkin();

    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, primitive.bind_group_skin);
    passEncoder.dispatchWorkgroups(Math.floor((primitive.num_pos + 127) / 128), 1,1);        

    if (primitive.geometry[0].tangent_buf != null)
    {
        passEncoder.setBindGroup(0, primitive.bind_group_skin2);
        passEncoder.dispatchWorkgroups(Math.floor((primitive.num_pos + 127) / 128), 1,1);        

    }

}
