const shader_code =`
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

struct Hemisphere
{
    skyColor: vec4f,
    groundColor: vec4f
};

@group(1) @binding(0)
var<uniform> uHemisphere: Hemisphere;

struct VSOut 
{
    @builtin(position) Position: vec4f,
    @location(0) cubemapCoord: vec3f
};

@vertex
fn vs_main(@builtin(vertex_index) vertId: u32) -> VSOut 
{
    var vsOut: VSOut;
    let grid = vec2(f32((vertId<<1)&2), f32(vertId & 2));
    let pos_proj = vec4(grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0), 0.5, 1.0);    
    vsOut.Position = pos_proj;
    var pos_view = uCamera.invProjMat * pos_proj;    
    pos_view = pos_view / pos_view.w;
    let matrix = mat3x3(uCamera.invViewMat[0].xyz, uCamera.invViewMat[1].xyz, uCamera.invViewMat[2].xyz);
    vsOut.cubemapCoord = matrix * pos_view.xyz;
    return vsOut;
}

@fragment
fn fs_main(@location(0) vCubemapCoord: vec3f) -> @location(0) vec4f
{  
    let dir = normalize(vCubemapCoord);
    let k = dir.y * 0.5 + 0.5;    
    return vec4( mix( uHemisphere.groundColor.xyz, uHemisphere.skyColor.xyz, k), 1.0);
}

`;

function GetPipelineDrawHemisphere(options)
{
    let signature = JSON.stringify(options);

    if (!("draw_hemisphere" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.draw_hemisphere = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.draw_hemisphere))
    {        
        const pipelineLayoutDesc = { bindGroupLayouts: [engine_ctx.cache.bindGroupLayouts.perspective_camera, engine_ctx.cache.bindGroupLayouts.hemisphere_background] };        
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        const depthStencil = {
            depthWriteEnabled: false,
            depthCompare: 'less',
            format: 'depth32float'
        };

        const vertex = {
            module: shaderModule,
            entryPoint: 'vs_main',
            buffers: []
        };

        const colorState = {
            format: options.view_format,
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
            layout: layout,
    
            vertex,
            fragment,
    
            primitive,
            depthStencil
        };

        if (options.msaa)
        {
            pipelineDesc.multisample ={
                count: 4,
            };
        }

        engine_ctx.cache.pipelines.draw_hemisphere[signature] = engine_ctx.device.createRenderPipeline(pipelineDesc); 

    }

    return engine_ctx.cache.pipelines.draw_hemisphere[signature];

}

export function DrawHemisphere(passEncoder, target, camera, bg)
{
    let options = {
        msaa: target.msaa,
        view_format: target.view_format
    };

    let pipeline = GetPipelineDrawHemisphere(options);

    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, camera.bind_group);
    passEncoder.setBindGroup(1, bg.bind_group);

    passEncoder.setViewport(
        0,
        0,
        target.width,
        target.height,
        0,
        1
    );

    passEncoder.setScissorRect(
        0,
        0,
        target.width,
        target.height,
    );

    passEncoder.draw(3, 1);

}

