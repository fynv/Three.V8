const shader_code =`
@group(0) @binding(0)
var uSampler: sampler;

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

    let pixel_size = vec2(1.0, 1.0)/vec2f(size);

    let uv = (vec2f(id.xy) + 0.5)/vec2f(size);
    var color = vec3(0.0);

    color += 0.0625 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(-pixel_size.x, -pixel_size.y), 0).xyz;
    color += 0.125 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(0.0, -pixel_size.y), 0).xyz;
    color += 0.0625 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(pixel_size.x, -pixel_size.y), 0).xyz;
    
    color += 0.125 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(-pixel_size.x, 0.0), 0).xyz;
    color += 0.25 * textureSampleLevel(uTexSrc, uSampler, uv, 0).xyz;
    color += 0.125 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(pixel_size.x, 0.0), 0).xyz;

    color += 0.0625 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(-pixel_size.x, pixel_size.y), 0).xyz;
    color += 0.125 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(0.0, pixel_size.y), 0).xyz;
    color += 0.0625 * textureSampleLevel(uTexSrc, uSampler, uv + vec2(pixel_size.x, pixel_size.y), 0).xyz;

    let coord = vec2i(id.xy);
    textureStore(uTexDst, coord, vec4(color, 1.0));
}
`;


function GetPipelineReflecionMipmaps()
{
    if (!("reflecion_mipmaps" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.reflecion_mipmaps];
        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);

        engine_ctx.cache.pipelines.reflecion_mipmaps =  engine_ctx.device.createComputePipeline({
            layout,
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }
    return engine_ctx.cache.pipelines.reflecion_mipmaps;
}

export function ReflecionMipmaps(passEncoder, width, height, bind_group)
{
    let pipeline = GetPipelineReflecionMipmaps();
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, bind_group);
    passEncoder.dispatchWorkgroups(Math.floor((width + 7) / 8), Math.floor((height + 7) / 8),1);
}




