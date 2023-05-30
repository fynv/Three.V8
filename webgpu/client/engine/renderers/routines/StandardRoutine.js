import { wgsl } from '../../wgsl-preprocessor.js';
import { LightsIsEmpty } from "../../lights/Lights.js"

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

    let material_binding = 3;
    
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
    
    let lights_options = options.lights_options;
    let directional_lights = lights_options.directional_lights;

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
    @location(${location_attrib_color}) color: vec4f,
#endif 

#if ${has_uv}
    @location(${location_attrib_uv}) uv: vec2f,
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
    let world_tangent = uModel.modelMat * vec4(input.tangent, 0.0);
    output.tangent = world_tangent.xyz;

    let world_bitangent = uModel.modelMat * vec4(input.bitangent, 0.0);
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

@group(1) @binding(1)
var<uniform> uMaterial: Material;

@group(1) @binding(2)
var uSampler: sampler;

#if ${mOpt.has_color_texture}
@group(1) @binding(${binding_tex_color})
var uTexColor: texture_2d<f32>;
#endif

#if ${mOpt.has_metalness_map}
@group(1) @binding(${binding_tex_metalness})
var uTexMetalness: texture_2d<f32>;
#endif

#if ${mOpt.has_roughness_map}
@group(1) @binding(${binding_tex_roughtness})
var uTexRoughness: texture_2d<f32>;
#endif

#if ${mOpt.has_specular_map}
@group(1) @binding(${binding_tex_specular})
var uTexSpecular: texture_2d<f32>;
#endif

#if ${mOpt.has_glossiness_map}
@group(1) @binding(${binding_tex_glossiness})
var uTexGlossiness: texture_2d<f32>;
#endif

#if ${mOpt.has_normal_map}
@group(1) @binding(${binding_tex_normal})
var uTexNormal: texture_2d<f32>;
#endif

#if ${mOpt.has_emissive_map}
@group(1) @binding(${binding_tex_emissive})
var uTexEmissive: texture_2d<f32>;
#endif

struct PhysicalMaterial
{
    diffuseColor: vec3f,
    roughness: f32,
    specularColor: vec3f,
    specularF90: f32
};

const RECIPROCAL_PI = 0.3183098861837907;
const EPSILON = 1e-6;

fn pow2(x: f32) -> f32
{
    return x*x;
}

struct DirectionalLight
{
    color: vec4f,
    origin: vec4f,
    direction: vec4f,
    diffuse_thresh: f32,
    diffuse_high: f32,
    diffuse_low: f32,
    specular_thresh: f32,
    specular_high: f32,
    specular_low: f32
};

struct DirectionalShadow
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

struct IncidentLight 
{
	color : vec3f,
	direction: vec3f
};

fn BRDF_Lambert(diffuseColor : vec3f) -> vec3f
{
    return RECIPROCAL_PI * diffuseColor;
}

fn F_Schlick(f0: vec3f, f90: f32, dotVH: f32) -> vec3f
{
    let fresnel = exp2( ( - 5.55473 * dotVH - 6.98316 ) * dotVH );
	return f0 * ( 1.0 - fresnel ) + ( f90 * fresnel );
}

fn V_GGX_SmithCorrelated( alpha: f32, dotNL: f32, dotNV: f32) -> f32
{
    let a2 = pow2( alpha );
	let gv = dotNL * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNV ) );
	let gl = dotNV * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNL ) );
	return 0.5 / max( gv + gl, EPSILON );

}

fn D_GGX(alpha: f32, dotNH: f32) -> f32
{
	let a2 = pow2( alpha );
	let denom = pow2( dotNH ) * ( a2 - 1.0 ) + 1.0; 
	return RECIPROCAL_PI * a2 / pow2( denom );
}


fn BRDF_GGX(lightDir: vec3f,  viewDir: vec3f, normal: vec3f, f0: vec3f, f90: f32, roughness: f32) -> vec3f
{
	let alpha = pow2(roughness);

	let halfDir = normalize(lightDir + viewDir);

	let dotNL = clamp(dot(normal, lightDir), 0.0, 1.0);
	let dotNV = clamp(dot(normal, viewDir), 0.0, 1.0);
	let dotNH = clamp(dot(normal, halfDir), 0.0, 1.0);
	let dotVH = clamp(dot(viewDir, halfDir), 0.0, 1.0);

	let F = F_Schlick(f0, f90, dotVH);
	let V = V_GGX_SmithCorrelated(alpha, dotNL, dotNV);
	let D = D_GGX( alpha, dotNH );
	return F*(V*D);
}

@group(2) @binding(0)
var uShadowSampler: sampler_comparison;


fn computeShadowCoords(VPSB: mat4x4f, world_pos: vec3f) -> vec3f
{
	let shadowCoords = VPSB * vec4(world_pos, 1.0);
	return shadowCoords.xyz;
}

fn borderPCFTexture(shadowTex: texture_depth_2d, uvz : vec3f) -> f32
{
	return select(select(0.0, 1.0, uvz.z <= 1.0),  textureSampleCompareLevel(shadowTex, uShadowSampler, uvz.xy, uvz.z), 
        ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&  (uvz.x >= 0.0) && (uvz.y >= 0.0)));
}

fn computeShadowCoef(VPSB: mat4x4f, world_pos: vec3f, shadowTex: texture_depth_2d)-> f32
{
    let shadowCoords = computeShadowCoords(VPSB, world_pos);
    return borderPCFTexture(shadowTex, shadowCoords);    
}

${(()=>{
    let code = "";
    let binding = 1;
    for (let i=0; i<directional_lights.length; i++)
    {        
        let has_shadow = directional_lights[i];

        code += `
@group(2) @binding(${binding})
var<uniform> uDirectionalLight_${i}: DirectionalLight;`
        binding++;

        if (has_shadow)
        {
            code += `
@group(2) @binding(${binding})
var<uniform> uDirectionalShadow_${i}: DirectionalShadow;`
            binding++;

            code += `
@group(2) @binding(${binding})
var uDirectionalShadowTex_${i}: texture_depth_2d;`
            binding++;
        }
    }
    return code;
    
})()}

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

#if ${mOpt.specular_glossiness}

    var specularFactor = uMaterial.specularGlossiness.xyz;
    var glossinessFactor = uMaterial.specularGlossiness.w;

#if ${mOpt.has_specular_map}
    specularFactor *= textureSample(uTexSpecular, uSampler, input.uv).xyz;
#endif
    
#if ${mOpt.has_glossiness_map}
    glossinessFactor *= textureSample(uTexGlossiness, uSampler, input.uv).w;
#endif

#else

    var metallicFactor = uMaterial.metalicFactor;
    var roughnessFactor = uMaterial.roughnessFactor;

#if ${mOpt.has_metalness_map}
    metallicFactor *= textureSample(uTexMetalness, uSampler, input.uv).z;
#endif

#if ${mOpt.has_roughness_map}
    roughnessFactor *= textureSample(uTexRoughness, uSampler, input.uv).y;
#endif

#endif

    let viewDir = normalize(input.viewDir);
    var norm = normalize(input.norm);

#if ${mOpt.has_normal_map}
    {
        let T = normalize(input.tangent);
        let B = normalize(input.bitangent);
        var bump = textureSample(uTexNormal, uSampler, input.uv).xyz;
        bump = (2.0*bump - 1.0) * vec3(uMaterial.normalScale, 1.0 );
        norm = normalize(bump.x*T + bump.y*B + bump.z*norm);        
    }
#endif

    if (uMaterial.doubleSided!=0)
    {
        if (dot(viewDir,norm)<0.0) 
        {
            norm = -norm;
        }
    }

    var material : PhysicalMaterial;
#if ${mOpt.specular_glossiness}
    material.diffuseColor = base_color.xyz * ( 1.0 -
        max( max( specularFactor.r, specularFactor.g ), specularFactor.b ) );
    material.roughness = max( 1.0 - glossinessFactor, 0.0525 );	
    material.specularColor = specularFactor.rgb;
#else
    material.diffuseColor = base_color.xyz * ( 1.0 - metallicFactor );	
	material.roughness = max( roughnessFactor, 0.0525 );	
	material.specularColor = mix( vec3( 0.04 ), base_color.xyz, metallicFactor );	
#endif

    let dxy =  max(abs(dpdx(norm)), abs(dpdy(norm)));
    let geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);	
    material.roughness += geometryRoughness;
	material.roughness = min( material.roughness, 1.0 );
	material.specularF90 = 1.0;

    let emissive = uMaterial.emissive.xyz;    
#if ${mOpt.has_emissive_map}
    emissive *= textureSample(uTexEmissive, uSampler, input.uv).xyz;
#endif

    var specular = vec3(0.0);
    var diffuse = vec3(0.0);
${(()=>{
    let code = "";
    for (let i=0; i<directional_lights.length; i++)
    {
        let has_shadow = directional_lights[i];
        code += wgsl`
    {
        let light_source = uDirectionalLight_${i};
        var l_shadow = 1.0;
#if ${has_shadow}
        let shadow = uDirectionalShadow_${i};
        l_shadow = computeShadowCoef(shadow.VPSBMat, input.worldPos, uDirectionalShadowTex_${i});
#endif
        var directLight: IncidentLight;
        directLight.color = light_source.color.xyz *l_shadow;
        directLight.direction = light_source.direction.xyz;
        let dotNL = clamp(dot(norm, directLight.direction), 0.0, 1.0);
        let irradiance = dotNL * directLight.color;
        diffuse += irradiance * BRDF_Lambert(material.diffuseColor);
        specular += irradiance * BRDF_GGX( directLight.direction, viewDir, norm, material.specularColor, material.specularF90, material.roughness );
    }
`;
    }
    return code;
})()}

    var col = emissive + specular;
    col += diffuse;
    col = clamp(col, vec3(0.0), vec3(1.0));

    output.color = vec4(col, 1.0);
    
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
        let primitive_layout = engine_ctx.cache.bindGroupLayouts.primitive[material_signature];
        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.perspective_camera, primitive_layout];
        if (!LightsIsEmpty(options.lights_options))
        {
            let lights_signature = JSON.stringify(options.lights_options);
            let lights_layout = engine_ctx.cache.bindGroupLayouts.lights[lights_signature];
            bindGroupLayouts.push(lights_layout);            
        }

        const pipelineLayoutDesc = { bindGroupLayouts };
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

        if (mOpt.has_normal_map)
        {
            const tangentAttribDesc = {
                shaderLocation: localtion_attrib++, 
                offset: 0,
                format: 'float32x4'
            };

            const tangentBufferDesc = {
                attributes: [tangentAttribDesc],
                arrayStride: 4 * 4,
                stepMode: 'vertex'
            };

            vertex_bufs.push(tangentBufferDesc);

            const bitangentAttribDesc = {
                shaderLocation: localtion_attrib++, 
                offset: 0,
                format: 'float32x4'
            };

            const bitangentBufferDesc = {
                attributes: [bitangentAttribDesc],
                arrayStride: 4 * 4,
                stepMode: 'vertex'
            };

            vertex_bufs.push(bitangentBufferDesc);

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
    let material = params.material_list[primitive.material_idx];
    let geo = primitive.geometry[primitive.geometry.length - 1];    
    
    let options = {};    
    options.alpha_mode = material.alphaMode;
    options.view_format= params.target.view_format;
    options.is_msaa  = params.target.msaa;
    options.is_highlight_pass = params.is_highlight_pass;
    options.has_color = primitive.color_buf != null;
    options.material_options = primitive.material_options;
    options.lights_options = params.lights.get_options();

    let pipeline = GetPipelineStandard(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, params.bind_group_camera);
    passEncoder.setBindGroup(1, primitive.bind_group);   

    if (params.lights.bind_group!=null)
    {
        passEncoder.setBindGroup(2, params.lights.bind_group);   
    }

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

    if (mOpt.has_normal_map)
    {
        passEncoder.setVertexBuffer(localtion_attrib++, geo.tangent_buf);
        passEncoder.setVertexBuffer(localtion_attrib++, geo.bitangent_buf);
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


export function RenderStandardBundle(params)
{
    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: [params.target.view_format],
        depthStencilFormat: 'depth32float',
        sampleCount: params.target.msaa?4:1
    });

    RenderStandard(renderBundleEncoder, params);

    return renderBundleEncoder.finish();

}
