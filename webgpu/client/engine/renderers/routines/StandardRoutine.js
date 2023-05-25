import { wgsl } from '../../wgsl-preprocessor.js';

function get_shader(options)
{
    let localtion_attrib = 0;
    let location_varying = 0;

    let location_attrib_pos = localtion_attrib++;
    let location_attrib_norm = localtion_attrib++;

    let location_varying_viewdir = location_varying++;
    let location_varying_norm = location_varying++;
    let location_varying_world_pos = location_varying++;

    return wgsl`
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

    
struct Model
{
    modelMat: mat4x4f,
    normalMat: mat4x4f
};
@group(1) @binding(0)
var<uniform> uModel: Model;

struct VSIn
{
    @location(${location_attrib_pos}) pos: vec3f,
    @location(${location_attrib_norm}) norm: vec3f,
}

struct VSOut 
{
    @builtin(position) Position: vec4f,
    @location(${location_varying_viewdir}) viewDir: vec3f,
    @location(${location_varying_norm}) norm: vec3f,
    @location(${location_varying_world_pos}) worldPos: vec3f,
};

@vertex
fn vs_main(input: VSIn) -> VSOut
{
    var output: VSOut;
    let world_pos = uModel.modelMat*vec4(input.pos, 1.0);
    let view_pos = uCamera.viewMat*world_pos;
    var proj_pos = uCamera.projMat*view_pos;
    proj_pos.z = (proj_pos.z + proj_pos.w) * 0.5;
    output.Position = proj_pos;
    output.worldPos = world_pos.xyz;
    output.viewDir = uCamera.eyePos - world_pos.xyz;
    let world_norm = uModel.normalMat * vec4(input.norm, 0.0);
    output.norm = world_norm.xyz;
    return output;
}

struct FSIn
{
    @location(${location_varying_viewdir}) viewDir: vec3f,
    @location(${location_varying_norm}) norm: vec3f,
    @location(${location_varying_world_pos}) worldPos: vec3f,
};

struct FSOut
{
    @location(0) color: vec4f
};

@fragment
fn fs_main(input: FSIn) -> FSOut
{
    var output: FSOut;
    output.color = vec4((input.norm + 1.0)*0.5, 1.0);
    return output;
}
`;
}

function GetPipelineStandard(options)
{
    let signature = JSON.stringify(options);
    if (!("standard" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.standard = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.standard))
    {
        const pipelineLayoutDesc = { bindGroupLayouts: [engine_ctx.cache.bindGroupLayouts.perspective_camera, engine_ctx.cache.bindGroupLayouts.model] };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);
        let code = get_shader(options);
        let shaderModule = engine_ctx.device.createShaderModule({ code });

        const depthStencil = {
            depthWriteEnabled: true,
            depthCompare: 'less',
            format: 'depth32float'
        };

        let vertex_bufs = [];

        const positionAttribDesc = {
            shaderLocation: 0,
            offset: 0,
            format: 'float32x4'
        };

        const positionBufferDesc = {
            attributes: [positionAttribDesc],
            arrayStride: 4 * 4,
            stepMode: 'vertex'
        };

        vertex_bufs.push(positionBufferDesc);

        const normalAttribDesc = {
            shaderLocation: 1, 
            offset: 0,
            format: 'float32x4'
        };        

        const normalBufferDesc = {
            attributes: [normalAttribDesc],
            arrayStride: 4 * 4,
            stepMode: 'vertex'
        };

        vertex_bufs.push(normalBufferDesc);

        const vertex = {
            module: shaderModule,
            entryPoint: 'vs_main',
            buffers: vertex_bufs
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
            frontFace: 'ccw',
            cullMode:  options.material_options.doubleSided ? "none" : "back",
            topology: 'triangle-list'
        };

        const pipelineDesc = {
            layout: layout,
    
            vertex,
            fragment,
    
            primitive,
            depthStencil
        };

        if (options.is_msaa)
        {
            pipelineDesc.multisample ={
                count: 4,
            };
        }

        engine_ctx.cache.pipelines.standard[signature] = engine_ctx.device.createRenderPipeline(pipelineDesc); 
    }

    return engine_ctx.cache.pipelines.standard[signature];
}


export function RenderStandard(passEncoder, params)
{
    let index_type_map = { 1: 'uint8', 2: 'uint16', 4: 'uint32'};

    let primitive = params.primitive;   
    if (primitive.geometry.length<1) return;

    let material = params.material_list[primitive.material_idx];
    let geo = primitive.geometry[primitive.geometry.length - 1];    
    
    let options = {};    
    options.alpha_mode = material.alphaMode;
    options.view_format= params.target.view_format;
    options.is_msaa  = params.target.msaa;
    options.is_highlight_pass = params.is_highlight_pass;
    options.has_color = primitive.color_buf != null;
    options.material_options = material.get_options();

    let pipeline = GetPipelineStandard(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, params.bind_group_camera);
    passEncoder.setBindGroup(1, params.bind_group_model);   

    passEncoder.setVertexBuffer(0, geo.pos_buf);
    passEncoder.setVertexBuffer(1, geo.normal_buf);

    if (primitive.index_buf!=null)
    {
        passEncoder.setIndexBuffer(primitive.index_buf, index_type_map[primitive.type_indices]);        
        passEncoder.drawIndexed(primitive.num_face * 3, 1);
    }
    else
    {
        passEncoder.draw(primitive.num_pos, 1);
    }

}


export function RenderStandardBundle(passEncoder, params)
{
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [params.target.view_format],
        depthStencilFormat: 'depth32float',
        sampleCount: params.target.msaa?4:1
    });

    RenderStandard(renderBundleEncoder, params);

    return renderBundleEncoder.finish();

}
