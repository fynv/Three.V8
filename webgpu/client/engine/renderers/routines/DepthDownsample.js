const shader_code =`
@group(0) @binding(0)
var uDepthTex: texture_multisampled_2d<f32>;

@vertex
fn vs_main(@builtin(vertex_index) vertId: u32) -> @builtin(position) vec4f 
{    
    let grid = vec2(f32((vertId<<1)&2), f32(vertId & 2));
    let pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
    return vec4(pos_proj, 0.0, 1.0);
}

@fragment
fn fs_main(@builtin(position) coord_pix: vec4f) -> @builtin(frag_depth) f32
{
    let ucoord2d = vec2u(coord_pix.xy);
    let depth0 = textureLoad(uDepthTex, ucoord2d, 0).x;
    let depth1 = textureLoad(uDepthTex, ucoord2d, 1).x;
    let depth2 = textureLoad(uDepthTex, ucoord2d, 2).x;
    let depth3 = textureLoad(uDepthTex, ucoord2d, 3).x;
    let depth = 0.25 * (depth0 + depth1 + depth2 + depth3);
    return depth;
}
`;

function GetPipelineDepthDownsample()
{
    if (!("depth_downsample" in engine_ctx.cache.pipelines))
    {
        let options_depth = { msaa: true};
        let signature_depth = JSON.stringify(options_depth);
        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.depth[signature_depth]];

        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        const depthStencil = {
            depthWriteEnabled: true,
            depthCompare: 'always',
            format: 'depth32float'
        };


        const vertex = {
            module: shaderModule,
            entryPoint: 'vs_main',
            buffers: []
        };

        const fragment = {
            module: shaderModule,
            entryPoint: 'fs_main',
            targets: []
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
            depthStencil
        };

        engine_ctx.cache.pipelines.depth_downsample =  engine_ctx.device.createRenderPipeline(pipelineDesc);

    }
    return engine_ctx.cache.pipelines.depth_downsample;
}

export function DepthDownsample(passEncoder, target)
{ 
    let pipeline = GetPipelineDepthDownsample();
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, target.bind_group_depth);
    passEncoder.draw(3, 1);   
}


export function DepthDownsampleBundle(target)
{    
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [],
        depthStencilFormat: 'depth32float'
    });    

    DepthDownsample(renderBundleEncoder, target);
    return renderBundleEncoder.finish();

}
