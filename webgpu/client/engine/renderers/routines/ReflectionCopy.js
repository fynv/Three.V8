const shader_code =`
@group(0) @binding(0)
var<uniform> uScissor: vec4i;

@group(0) @binding(1)
var uTexSrc: texture_2d<f32>;

@group(0) @binding(2)
var uTexDst : texture_storage_2d<rgba16float, write>;

@compute @workgroup_size(8,8, 1)
fn main(@builtin(global_invocation_id) id : vec3<u32>)
{
    let size = textureDimensions(uTexDst);
    if (id.x>=size.x || id.y>=size.y) 
    {
        return;
    }

    let coord = clamp(vec2i(id.xy), uScissor.xy,  uScissor.xy + uScissor.zw-1);
    let color = textureLoad(uTexSrc, coord, 0);
    textureStore(uTexDst, coord, color);
}
`;

function GetPipelineReflecionCopy()
{
    if (!("reflecion_copy" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.reflecion_copy];
        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);

        engine_ctx.cache.pipelines.reflecion_copy =  engine_ctx.device.createComputePipeline({
            layout,
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }
    return engine_ctx.cache.pipelines.reflecion_copy;
}


export function ReflecionCopy(passEncoder, width, height, bind_group)
{
    let pipeline = GetPipelineReflecionCopy();
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, bind_group);
    passEncoder.dispatchWorkgroups(Math.floor((width + 7) / 8), Math.floor((height + 7) / 8),1);
}




