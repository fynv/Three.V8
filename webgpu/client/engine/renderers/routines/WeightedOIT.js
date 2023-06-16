function condition(cond, a, b="")
{
    return cond? a: b;
}

function get_shader(is_msaa)
{
    return `
${condition(is_msaa,`
@group(0) @binding(0)
var uTex0: texture_multisampled_2d<f32>;

@group(0) @binding(1)
var uTex1: texture_multisampled_2d<f32>;

`,`
@group(0) @binding(0)
var uTex0: texture_2d<f32>;

@group(0) @binding(1)
var uTex1: texture_2d<f32>;

`)}

@vertex
fn vs_main(@builtin(vertex_index) vertId: u32) -> @builtin(position) vec4f
{
    let grid = vec2(f32((vertId<<1)&2), f32(vertId & 2));
    let pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);    
    return vec4(pos_proj, 0.0, 1.0);
}

struct FS_IN
{
    @builtin(position) coord_pix: vec4f,

${condition(is_msaa,`
    @builtin(sample_index) sampleId: u32
`)}
}

@fragment
fn fs_main(input: FS_IN) -> @location(0) vec4f
{
    let ucoord2d = vec2u(input.coord_pix.xy);
${condition(is_msaa,`
    var reveal = textureLoad(uTex1, ucoord2d, input.sampleId).x;
`,`
    var reveal = textureLoad(uTex1, ucoord2d, 0).x;
`)}
    if (reveal>1.0) 
    {
        discard;
    }
    reveal = 1.0 - reveal;
${condition(is_msaa,`
    var col = textureLoad(uTex0, ucoord2d, input.sampleId);
`,`
    var col = textureLoad(uTex0, ucoord2d, 0);
`)}
    return vec4(col.xyz*reveal/max(col.w, 1e-5), reveal);

}
`;
}

function GetPipelineOITResolve(options)
{
    let signature = JSON.stringify(options);

    if (!("oit_resolve" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.oit_resolve = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.oit_resolve))
    {
        let options2 = { msaa: options.msaa};
        let signature2 = JSON.stringify(options2);
        const pipelineLayoutDesc = { bindGroupLayouts: [engine_ctx.cache.bindGroupLayouts.oit_resolve[signature2]] };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);
        let shaderModule = engine_ctx.device.createShaderModule({ code: get_shader(options.msaa) });

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

        if (options.msaa)
        {
            pipelineDesc.multisample ={
                count: 4,
            };
        }

        engine_ctx.cache.pipelines.oit_resolve[signature] = engine_ctx.device.createRenderPipeline(pipelineDesc);
    }
    return engine_ctx.cache.pipelines.oit_resolve[signature];
}

export function ResolveWeightedOIT(passEncoder, target)
{
    let options = {
        msaa: target.msaa,
        view_format: target.view_format
    };
    let pipeline = GetPipelineOITResolve(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, target.bind_group_oit);
    passEncoder.draw(3, 1);

}
