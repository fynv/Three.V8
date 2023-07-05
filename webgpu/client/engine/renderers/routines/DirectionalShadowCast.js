function condition(cond, a, b="")
{
    return cond? a: b;
}

function get_shader(options)
{
    let localtion_attrib = 0;
    let location_varying = 0;

    let location_attrib_pos = localtion_attrib++;
    let location_attrib_color = localtion_attrib;
    let location_varying_color = location_varying;
    if (options.has_color)
    {
        localtion_attrib++;
        location_varying++;
    }

    let mOpt = options.material_options;
    let location_attrib_uv = localtion_attrib;
    let location_varying_uv =  location_varying;

    if (mOpt.has_color_texture)
    {
        localtion_attrib++;
        location_varying++;
    }

    let material_binding = 3;
    
    let binding_tex_color = material_binding;
    if (mOpt.has_color_texture) material_binding++;

    let alpha = options.alphaMode != "Opaque";
    let alpha_mask = options.alphaMode == "Mask";
    
    return `
struct Shadow
{
    VPSBMat: mat4x4f,
    projMat: mat4x4f,
    viewMat: mat4x4f,
    left_right: vec2f,
    bottom_up: vec2f,
    near_far: vec2f,
    light_radius: f32,
    bias: f32
};
@group(0) @binding(0)
var<uniform> uShadow: Shadow;

struct Model
{
    modelMat: mat4x4f,
    normalMat: mat4x4f
};
@group(1) @binding(0)
var<uniform> uModel: Model;

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

@group(1) @binding(1)
var<uniform> uMaterial: Material;

struct VSIn
{
    @location(${location_attrib_pos}) pos: vec3f,

${condition(options.has_color,`
    @location(${location_attrib_color}) color: vec4f,
`)}

${condition(mOpt.has_color_texture,`
    @location(${location_attrib_uv}) uv: vec2f,
`)}
};

struct VSOut 
{
    @builtin(position) Position: vec4f,

${condition(options.has_color,`
    @location(${location_varying_color}) alpha: f32,
`)}

${condition(mOpt.has_color_texture,`
    @location(${location_varying_uv}) uv: vec2f,
`)}

    @location(${location_varying_uv + 1}) dummy: f32
};

@vertex
fn vs_main(input: VSIn) -> VSOut
{
    var output: VSOut;
    let world_pos = uModel.modelMat*vec4(input.pos, 1.0);
    let view_pos = uShadow.viewMat*world_pos;
    var proj_pos = uShadow.projMat*view_pos;    
    if (uMaterial.doubleSided!=0 && uShadow.bias > 0.0)
    {
        proj_pos.z += uShadow.bias * proj_pos.w;
    }
    proj_pos.y = -proj_pos.y;
    proj_pos.z = (proj_pos.z + proj_pos.w) * 0.5;    
    output.Position = proj_pos;    

${condition(options.has_color,`
    output.alpha = input.color.w;
`)}

${condition(mOpt.has_color_texture,`
    output.uv = input.uv;
`)}

    return output;
}

struct FSIn
{
${condition(options.has_color,`
    @location(${location_varying_color}) alpha: f32,
`)}

${condition(mOpt.has_color_texture,`
    @location(${location_varying_uv}) uv: vec2f,
`)}

    @location(${location_varying_uv + 1}) dummy: f32
};

@group(1) @binding(2)
var uSampler: sampler;

${condition(mOpt.has_color_texture,`
@group(1) @binding(${binding_tex_color})
var uTexColor: texture_2d<f32>;
`)}

@fragment
fn fs_main(input: FSIn)
{
${condition(alpha,`
    var base_alpha = uMaterial.color.w;
    
${condition(options.has_color,`
    base_alpha *= input.alpha;
`)}

${condition(mOpt.has_color_texture,`
    base_alpha *= textureSample(uTexColor, uSampler, input.uv).w;
`)}

${condition(alpha_mask,`
    base_alpha = base_alpha > uMaterial.alphaCutoff? 1.0 : 0.0;
`)}

    if (base_alpha<0.5)
    {
        discard;
    }
`)}
}
`;

}

function GetPipelineDirectionalShadow(options)
{
    let signature = JSON.stringify(options);
    if (!("directional_shadow" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.directional_shadow = {};
    }

    if (!(signature in engine_ctx.cache.pipelines.directional_shadow))
    {
        let prim_options = {
            material: options.material_options,
            has_lightmap: options.has_lightmap,
            has_reflector: options.has_reflector,
            has_envmap: options.has_primtive_probe
        };
        let prim_signature = JSON.stringify(prim_options);
        let primitive_layout = engine_ctx.cache.bindGroupLayouts.primitive[prim_signature];
        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.directional_light_shadow, primitive_layout];
        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);
        let code = get_shader(options);
        let shaderModule = engine_ctx.device.createShaderModule({ code });

        const depthStencil = {
            depthWriteEnabled: true,
            depthCompare: 'less-equal',
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
        if (mOpt.has_color_texture)
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

        const fragment = {
            module: shaderModule,
            entryPoint: 'fs_main',
            targets: []
        };

        const primitive = {
            frontFace: 'cw',
            cullMode:  (options.material_options.doubleSided && !options.force_cull) ? "none" : "front",
            topology: 'triangle-list'
        };

        const pipelineDesc = {
            layout: layout,
    
            vertex,
            fragment,
    
            primitive,
            depthStencil
        };

        engine_ctx.cache.pipelines.directional_shadow[signature] = engine_ctx.device.createRenderPipeline(pipelineDesc); 
    }

    return engine_ctx.cache.pipelines.directional_shadow[signature];

}

export function RenderDirectionalShadow(passEncoder, params)
{
    let index_type_map = { 1: 'uint8', 2: 'uint16', 4: 'uint32'};

    let primitive = params.primitive;   
    let material = params.material_list[primitive.material_idx];
    let geo = primitive.geometry[primitive.geometry.length - 1];    

    let options = {}; 
    options.alpha_mode = material.alphaMode;
    options.has_color = primitive.color_buf != null;
    options.force_cull = params.force_cull;
    options.has_lightmap = primitive.has_lightmap;
    options.has_reflector = primitive.has_reflector;
	options.material_options = primitive.material_options;
    options.has_primtive_probe = primitive.envMap!=null;

    let pipeline = GetPipelineDirectionalShadow(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, params.bind_group_shadow);
    passEncoder.setBindGroup(1, primitive.bind_group);

    let localtion_attrib = 0;
    passEncoder.setVertexBuffer(localtion_attrib++, geo.pos_buf);

    if (options.has_color)
    {
        passEncoder.setVertexBuffer(localtion_attrib++, primitive.color_buf);
    }

    let mOpt = options.material_options;
    if (mOpt.has_color_texture)
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

export function RenderDirectionalShadowBundle(params)
{
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [],
        depthStencilFormat: 'depth32float',        
    });
    RenderDirectionalShadow(renderBundleEncoder, params);
    return renderBundleEncoder.finish();
}

