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

struct Model
{
    modelMat: mat4x4f,
    normalMat: mat4x4f
};
@group(1) @binding(0)
var<uniform> uModel: Model;

@vertex
fn vs_main(@location(0) aPos: vec3f) -> @builtin(position) vec4f
{    
    let world_pos = uModel.modelMat*vec4(aPos, 1.0);
    let view_pos = uCamera.viewMat*world_pos;
    var proj_pos = uCamera.projMat*view_pos;
    proj_pos.z = (proj_pos.z + proj_pos.w) * 0.5;
    return proj_pos; 
}

@fragment
fn fs_main()
{

}
`;

function GetPipelineDepth(options)
{
    let signature = JSON.stringify(options);
    if (!("depth" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.depth = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.depth))
    {
        let camera_options = { has_reflector: options.is_reflect };
        let camera_signature =  JSON.stringify(camera_options);
        let camera_layout = engine_ctx.cache.bindGroupLayouts.perspective_camera[camera_signature];
        
        let prim_options = {
            material: options.material_options,
            has_lightmap: options.has_lightmap,
            has_reflector: options.has_reflector,
            has_envmap: options.has_primtive_probe
        };
        let prim_signature = JSON.stringify(prim_options);
        let primitive_layout = engine_ctx.cache.bindGroupLayouts.primitive[prim_signature];
        
        let bindGroupLayouts = [camera_layout, primitive_layout];

        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);        
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        const depthStencil = {
            depthWriteEnabled: true,
            depthCompare: 'less-equal',
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

        const vertex = {
            module: shaderModule,
            entryPoint: 'vs_main',
            buffers: vertex_bufs
        };

        const fragment = {
            module: shaderModule,
            entryPoint: 'fs_main',
            targets: []
        };

        const primitive = {
            frontFace: options.is_reflect?'cw':'ccw',
            cullMode:  options.material_options.doubleSided ? "none" : "back",
            topology: 'triangle-list'
        };

        const pipelineDesc = {
            layout,
    
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

        engine_ctx.cache.pipelines.depth[signature] = engine_ctx.device.createRenderPipeline(pipelineDesc); 
    }
    return engine_ctx.cache.pipelines.depth[signature];
}

export function RenderDepth(passEncoder, params)
{
    let index_type_map = { 1: 'uint8', 2: 'uint16', 4: 'uint32'};

    let primitive = params.primitive;   
    let material = params.material_list[primitive.material_idx];
    let geo = primitive.geometry[primitive.geometry.length - 1]; 

    let options = {};    
    options.is_msaa  = params.target.msaa; 
    options.has_lightmap = primitive.has_lightmap;
    options.has_reflector = primitive.has_reflector;
    options.material_options = primitive.material_options;
    options.has_primtive_probe = primitive.envMap!=null;    
    options.is_reflect = params.camera.reflector!=null;
    
    let pipeline = GetPipelineDepth(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, params.camera.bind_group);
    passEncoder.setBindGroup(1, primitive.bind_group); 
    passEncoder.setVertexBuffer(0, geo.pos_buf);

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



export function RenderDepthBundle(params)
{    
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [],
        depthStencilFormat: 'depth32float',
        sampleCount: params.target.msaa?4:1
    });    

    RenderDepth(renderBundleEncoder, params);
    return renderBundleEncoder.finish();

}
