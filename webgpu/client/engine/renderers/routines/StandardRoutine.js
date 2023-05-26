import { wgsl } from '../../wgsl-preprocessor.js';

function get_shader(options)
{
    let localtion_attrib = 0;
    let location_varying = 0;

    let location_attrib_pos = localtion_attrib++;
    let location_attrib_norm = localtion_attrib++;
    let location_varying_viewdir = location_varying++;
    let location_varying_norm = location_varying++;

    let location_attrib_color = localtion_attrib;
    let location_varying_color = location_varying;
    if (options.has_color)
    {
        localtion_attrib++;
        location_varying++;
    }

    let mOpt = options.material_options;
    let has_uv = mOpt.has_color_texture || mOpt.has_metalness_map || mOpt.has_roughness_map || mOpt.has_normal_map || mOpt.has_emissive_map;

    let location_attrib_uv = localtion_attrib;
    let location_varying_uv =  location_varying;

    if (has_uv)
    {
        localtion_attrib++;
        location_varying++;
    }

    let material_binding = 0;
    let binding_material = material_binding++;
    let binding_sampler = material_binding++;

    let binding_tex_color = material_binding;
    if (mOpt.has_color_texture) material_binding++;

    let binding_tex_metalness = material_binding;
    if (mOpt.has_metalness_map) material_binding++;
    
    let binding_tex_roughtness = material_binding;
    if (mOpt.has_roughness_map) material_binding++;

    let binding_tex_specular = material_binding;
    if (mOpt.has_specular_map) material_binding++;

    let binding_tex_glossiness = material_binding;
    if (mOpt.has_glossiness_map) material_binding++;

    let binding_tex_normal = material_binding;
    let location_attrib_tangent = localtion_attrib;
    let location_varying_tangent = location_varying;
    let location_attrib_bitangent = localtion_attrib;
    let location_varying_bitangent = location_varying;

    if (mOpt.has_normal_map)
    {
        material_binding++;
        localtion_attrib++;
        location_attrib_bitangent = localtion_attrib++;
        location_varying++;
        location_varying_bitangent = localtion_attrib++;
    }

    let binding_tex_emissive = material_binding;
    if (mOpt.has_emissive_map) material_binding++;    

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

#if ${options.has_color}
    @location(${location_attrib_color}) color: vec4f
#endif 

#if ${has_uv}
    @location(${location_attrib_uv}) uv: vec2f
#endif

#if ${mOpt.has_normal_map}
    @location(${location_attrib_tangent}) tangent: vec3f,
    @location(${location_attrib_bitangent}) bitangent: vec3f,
#endif

}

struct VSOut 
{
    @builtin(position) Position: vec4f,
    @location(${location_varying_viewdir}) viewDir: vec3f,
    @location(${location_varying_norm}) norm: vec3f,

#if ${options.has_color}
    @location(${location_varying_color}) color: vec4f,
#endif 

#if ${has_uv}
    @location(${location_varying_uv}) uv: vec2f,
#endif

#if ${mOpt.has_normal_map}
    @location(${location_varying_tangent}) tangent: vec3f,
    @location(${location_varying_bitangent}) bitangent: vec3f,
#endif

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

#if ${options.has_color}
    output.color = input.color;
#endif

#if ${has_uv}
    output.uv = input.uv;
#endif

#if ${mOpt.has_normal_map}
    let world_tangent = uModelMat * vec4(input.tangent, 0.0);
    output.tangent = world_tangent.xyz;

    let world_bitangent = uModelMat * vec4(input.bitangent, 0.0);
    output.bitangent = world_bitangent.xyz;
#endif

    return output;
}

struct FSIn
{
    @location(${location_varying_viewdir}) viewDir: vec3f,
    @location(${location_varying_norm}) norm: vec3f,

#if ${options.has_color}
    @location(${location_varying_color}) color: vec4f,
#endif     

#if ${has_uv}
    @location(${location_varying_uv}) uv: vec2f,
#endif

#if ${mOpt.has_normal_map}
    @location(${location_varying_tangent}) tangent: vec3f,
    @location(${location_varying_bitangent}) bitangent: vec3f,
#endif

    @location(${location_varying_world_pos}) worldPos: vec3f,
};

struct FSOut
{
    @location(0) color: vec4f
};

struct Material
{
    color: vec4f,
    emissive: vec4f,
    specularGlossiness: vec4f,
    normalScale: vec2f,
    metalicFactor: f32,
    roughnessFactor: f32,
    alphaCutoff: f32,
    doubleSided: i32
};

@group(2) @binding(${binding_material})
var<uniform> uMaterial: Material;

@group(2) @binding(${binding_sampler})
var uSampler: sampler;

#if ${mOpt.has_color_texture}
@group(2) @binding(${binding_tex_color})
var uTexColor: texture_2d<f32>;
#endif

#if ${mOpt.has_metalness_map}
@group(2) @binding(${binding_tex_metalness})
var uTexMetalness: texture_2d<f32>;
#endif

#if ${mOpt.has_roughness_map}
@group(2) @binding(${binding_tex_roughtness})
var uTexRoughness: texture_2d<f32>;
#endif

#if ${mOpt.has_specular_map}
@group(2) @binding(${binding_tex_specular})
var uTexSpecular: texture_2d<f32>;
#endif

#if ${mOpt.has_glossiness_map}
@group(2) @binding(${binding_tex_glossiness})
var uTexGlossiness: texture_2d<f32>;
#endif

#if ${mOpt.has_normal_map}
@group(2) @binding(${binding_tex_normal})
var uTexNormal: texture_2d<f32>;
#endif

#if ${mOpt.has_emissive_map}
@group(2) @binding(${binding_tex_emissive})
var uTexEmissive: texture_2d<f32>;
#endif

@fragment
fn fs_main(input: FSIn) -> FSOut
{
    var output: FSOut;

    var base_color = uMaterial.color;

#if ${options.has_color}
    base_color *= input.color;
#endif

    var tex_alpha = 1.0;
#if ${mOpt.has_color_texture}
    let tex_color = textureSample(uTexColor, uSampler, input.uv);
    tex_alpha = tex_color.w;
    base_color *= tex_color;
#endif

    let viewDir = normalize(input.viewDir);
    var norm = normalize(input.norm);

    let k = norm.y*0.5 + 0.5;
    let l = mix(vec3(0.3), vec3(0.8), k);

    output.color = vec4(base_color.xyz * l, base_color.w);
    
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
        let material_signature = JSON.stringify(options.material_options);
        let material_layout = engine_ctx.cache.bindGroupLayouts.mesh_standard_material[material_signature];
        const pipelineLayoutDesc = { bindGroupLayouts: [engine_ctx.cache.bindGroupLayouts.perspective_camera, engine_ctx.cache.bindGroupLayouts.model, material_layout] };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);
        let code = get_shader(options);
        let shaderModule = engine_ctx.device.createShaderModule({ code });

        const depthStencil = {
            depthWriteEnabled: true,
            depthCompare: 'less',
            format: 'depth32float'
        };

        let vertex_bufs = [];

        let localtion_attrib = 0;

        const positionAttribDesc = {
            shaderLocation: localtion_attrib++,
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
            shaderLocation: localtion_attrib++, 
            offset: 0,
            format: 'float32x4'
        };        

        const normalBufferDesc = {
            attributes: [normalAttribDesc],
            arrayStride: 4 * 4,
            stepMode: 'vertex'
        };

        vertex_bufs.push(normalBufferDesc);

        if (options.has_color)
        {
            const colorAttribDesc = {
                shaderLocation: localtion_attrib++, 
                offset: 0,
                format: 'float32x4'
            };        
    
            const colorBufferDesc = {
                attributes: [colorAttribDesc],
                arrayStride: 4 * 4,
                stepMode: 'vertex'
            };

            vertex_bufs.push(colorBufferDesc);
        }

        let mOpt = options.material_options;
        let has_uv = mOpt.has_color_texture || mOpt.has_metalness_map || mOpt.has_roughness_map || mOpt.has_normal_map || mOpt.has_emissive_map;
        if (has_uv)
        {
            const UVAttribDesc = {
                shaderLocation: localtion_attrib++, 
                offset: 0,
                format: 'float32x2'
            };        
    
            const UVBufferDesc = {
                attributes: [UVAttribDesc],
                arrayStride: 4 * 2,
                stepMode: 'vertex'
            };

            vertex_bufs.push(UVBufferDesc);
        }

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
    passEncoder.setBindGroup(2, material.bind_group);   

    let localtion_attrib = 0;

    passEncoder.setVertexBuffer(localtion_attrib++, geo.pos_buf);
    passEncoder.setVertexBuffer(localtion_attrib++, geo.normal_buf);

    if (options.has_color)
    {
        passEncoder.setVertexBuffer(localtion_attrib++, primitive.color_buf);
    }

    let mOpt = options.material_options;
    let has_uv = mOpt.has_color_texture || mOpt.has_metalness_map || mOpt.has_roughness_map || mOpt.has_normal_map || mOpt.has_emissive_map;
    if (has_uv)
	{
        passEncoder.setVertexBuffer(localtion_attrib++, primitive.uv_buf);
    }

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
