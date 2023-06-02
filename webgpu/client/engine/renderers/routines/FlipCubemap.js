const shader_code = `

@group(0) @binding(0)
var uTexIn : texture_2d_array<f32>;

@group(0) @binding(1)
var uTexOut : texture_storage_2d_array<rgba8unorm, write>;

@compute @workgroup_size(8,8, 1)
fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>)
{
    let size = textureDimensions(uTexIn);
    if ( GlobalInvocationID.x>=size.x || GlobalInvocationID.y>=size.y ) 
    {
        return;
    }
    let v = textureLoad(uTexIn, GlobalInvocationID.xy, GlobalInvocationID.z, 0);    
    var face_idx = GlobalInvocationID.z;
    if (face_idx<2)
    {
        face_idx = 1- face_idx;
    }

    textureStore(uTexOut, vec2u(size.x - 1 - GlobalInvocationID.x, GlobalInvocationID.y), face_idx, v);
}
`;

function GetPipelineFlipCubemap()
{
    if (!("flip_cubemap" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });
        engine_ctx.cache.pipelines.flip_cubemap = engine_ctx.device.createComputePipeline({
            layout: 'auto',
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }
    return engine_ctx.cache.pipelines.flip_cubemap;
}

export function FlipCubemap(texIn, texOut)
{

    let pipeline = GetPipelineFlipCubemap();

    let bind_group = engine_ctx.device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
            {
                binding: 0,
                resource: texIn.createView({
                    dimension: '2d-array'
                })
            },
            {
                binding: 1,
                resource: texOut.createView({
                    dimension: '2d-array'
                })
            },
        ],

    });

    const commandEncoder = engine_ctx.device.createCommandEncoder();

    const passEncoder = commandEncoder.beginComputePass();
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, bind_group);
    passEncoder.dispatchWorkgroups(Math.floor((texIn.width+7)/8), Math.floor((texIn.height+7)/8), 6);

    passEncoder.end();


    engine_ctx.device.queue.submit([commandEncoder.finish()]);


}


