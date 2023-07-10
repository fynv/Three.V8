import { LightsIsEmpty } from "../../lights/Lights.js"

function condition(cond, a, b="")
{
    return cond? a: b;
}

function get_shader(options)
{
    let location_attrib = 0;
    let location_varying = 0;

    let location_attrib_pos = location_attrib++;
    let location_attrib_norm = location_attrib++;
    let location_varying_viewdir = location_varying++;
    let location_varying_norm = location_varying++;

    let alpha_mask = options.alpha_mode == "Mask";
    let alpha_blend = options.alpha_mode == "Blend";

    let location_attrib_color = location_attrib;
    let location_varying_color = location_varying;
    if (options.has_color)
    {
        location_attrib++;
        location_varying++;
    }

    let mOpt = options.material_options;
    let has_uv = mOpt.has_color_texture || mOpt.has_metalness_map || mOpt.has_specular_map || mOpt.has_normal_map || mOpt.has_emissive_map;

    let location_attrib_uv = location_attrib;
    let location_varying_uv =  location_varying;

    if (has_uv)
    {
        location_attrib++;
        location_varying++;
    }
    
    let location_attrib_atlas_uv = location_attrib;
    let location_varying_atlas_uv = location_varying;
    
    if (options.has_lightmap)
    {
        location_attrib++;
        location_varying++;
    }

    let primitive_binding = 3;
    
    let binding_tex_color = primitive_binding;
    if (mOpt.has_color_texture) primitive_binding++;

    let binding_tex_metalness = primitive_binding;
    if (mOpt.has_metalness_map) primitive_binding++;

    let binding_tex_specular = primitive_binding;
    if (mOpt.has_specular_map) primitive_binding++;

    let binding_tex_normal = primitive_binding;
    let location_attrib_tangent = location_attrib;
    let location_varying_tangent = location_varying;
    let location_attrib_bitangent = location_attrib;
    let location_varying_bitangent = location_varying;

    if (mOpt.has_normal_map)
    {
        primitive_binding++;
        location_attrib++;
        location_attrib_bitangent = location_attrib++;
        location_varying++;
        location_varying_bitangent = location_varying++;
    }

    let binding_tex_emissive = primitive_binding;
    if (mOpt.has_emissive_map) primitive_binding++;    

    let binding_lightmap = primitive_binding;
    if (options.has_lightmap) primitive_binding++;

    let binding_reflector = primitive_binding;
    if (options.has_reflector) primitive_binding+=3;

    let binding_primitive_probe = primitive_binding;
    if (options.has_primtive_probe) primitive_binding++;
    
    let location_varying_world_pos = location_varying++;   
    
    let lights_options = options.lights_options;    
    let directional_lights = lights_options.directional_lights;

    let binding_lights = 0;
    let binding_shadow_sampler = binding_lights;
    if (lights_options.has_shadow) binding_lights++;

    let binding_directional_light = binding_lights;
    for (let i=0; i<directional_lights.length; i++)
    {        
        let has_shadow = directional_lights[i];
        binding_lights++;

        if (has_shadow)
        {            
            binding_lights+=2;
        }
    }

    let binding_reflection_map =  binding_lights;
    if (lights_options.has_reflection_map) binding_lights++; 
    
    let has_indirect_light = lights_options.has_ambient_light || lights_options.has_hemisphere_light || 
        lights_options.has_environment_map || lights_options.has_probe_grid || lights_options.has_lod_probe_grid;

    let binding_ambient_light = binding_lights;
    if (lights_options.has_ambient_light) binding_lights++;    

    let binding_hemisphere_light = binding_lights;
    if (lights_options.has_hemisphere_light) binding_lights++;

    let binding_environment_map =  binding_lights;
    if (lights_options.has_environment_map) binding_lights++;
    
    let binding_probe_grid = binding_lights;
    if (lights_options.has_probe_grid) binding_lights+=3;
    if (lights_options.has_lod_probe_grid) binding_lights+=5;

    let binding_fog = binding_lights;
    if (lights_options.has_fog) binding_lights++;

    return `
struct Camera
{
    projMat: mat4x4f, 
    viewMat: mat4x4f,
    invProjMat: mat4x4f,
    invViewMat: mat4x4f,
    eyePos: vec4f,
    scissor: vec4f
};

@group(0) @binding(0)
var<uniform> uCamera: Camera;

${condition(options.is_reflect,`
@group(0) @binding(1)
var<uniform> uMatrixReflector: mat4x4f;
`)}
   
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

${condition(options.has_color,`
    @location(${location_attrib_color}) color: vec4f,
`)}

${condition(has_uv,`
    @location(${location_attrib_uv}) uv: vec2f,
`)}

${condition(options.has_lightmap,`
    @location(${location_attrib_atlas_uv}) atlasUV: vec2f,
`)}

${condition(mOpt.has_normal_map,`
    @location(${location_attrib_tangent}) tangent: vec3f,
    @location(${location_attrib_bitangent}) bitangent: vec3f,
`)}
}

struct VSOut 
{
    @builtin(position) Position: vec4f,
    @location(${location_varying_viewdir}) viewDir: vec3f,
    @location(${location_varying_norm}) norm: vec3f,

${condition(options.has_color,`
    @location(${location_varying_color}) color: vec4f,
`)}

${condition(has_uv,`
    @location(${location_varying_uv}) uv: vec2f,
`)}

${condition(options.has_lightmap,`
    @location(${location_varying_atlas_uv}) atlasUV: vec2f,
`)}

${condition(mOpt.has_normal_map,`
    @location(${location_varying_tangent}) tangent: vec3f,
    @location(${location_varying_bitangent}) bitangent: vec3f,
`)}

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
    output.viewDir = uCamera.eyePos.xyz - world_pos.xyz;
    let world_norm = uModel.normalMat * vec4(input.norm, 0.0);
    output.norm = world_norm.xyz;

${condition(options.has_color,`
    output.color = input.color;
`)}

${condition(has_uv,`
    output.uv = input.uv;
`)}

${condition(options.has_lightmap,`
    output.atlasUV = input.atlasUV;
`)}

${condition(mOpt.has_normal_map,`
    let world_tangent = uModel.modelMat * vec4(input.tangent, 0.0);
    output.tangent = world_tangent.xyz;

    let world_bitangent = uModel.modelMat * vec4(input.bitangent, 0.0);
    output.bitangent = world_bitangent.xyz;
`)}

    return output;
}

const PI = 3.14159265359;
const RECIPROCAL_PI = 0.3183098861837907;
const EPSILON = 1e-6;

var<private> seed : u32;
var<private> viewDir: vec3f;

fn InitRandomSeed(val0: u32, val1: u32) 
{
    var v0 = val0;
    var v1 = val1;
    var s0 = 0u;

    for (var n = 0u; n<16u; n++)
    {
        s0 += 0x9e3779b9u;
		v0 += ((v1 << 4) + 0xa341316cu) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4u);
		v1 += ((v0 << 4) + 0xad90777du) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761eu);
    }

    seed = v0;
}

fn RandomInt() -> u32
{
    seed = 1664525u * seed + 1013904223u;
    return seed;
}

fn RandomFloat() -> f32
{
    return (f32(RandomInt() & 0x00FFFFFFu) / f32(0x01000000));
}

fn IGN(pixelX: i32, pixelY: i32) -> f32
{
    return fract(52.9829189 * fract(0.06711056*f32(pixelX) + 0.00583715*f32(pixelY)));
}

var<private> jitter: f32 = 0.0;

fn vogel_sample(j: f32, N: f32, jitter: f32) -> vec2f
{
    let r = sqrt((j+0.5)/N);
    let theta = j * 2.4 + jitter * 2.0 * PI;
    return vec2(r * cos(theta), r * sin(theta));
}

var<private> N: vec3f;

struct FSIn
{
    @builtin(position) coord_pix: vec4f,
    @builtin(front_facing) front_facing: bool,
    @location(${location_varying_viewdir}) viewDir: vec3f,
    @location(${location_varying_norm}) norm: vec3f,

${condition(options.has_color,`
    @location(${location_varying_color}) color: vec4f,
`)}

${condition(has_uv,`
    @location(${location_varying_uv}) uv: vec2f,
`)}

${condition(options.has_lightmap,`
    @location(${location_varying_atlas_uv}) atlasUV: vec2f,
`)}

${condition(mOpt.has_normal_map,`
    @location(${location_varying_tangent}) tangent: vec3f,
    @location(${location_varying_bitangent}) bitangent: vec3f,
`)}

    @location(${location_varying_world_pos}) worldPos: vec3f,
};

struct FSOut
{
    @location(0) color: vec4f,
${condition(alpha_blend,`
    @location(1) oit0: vec4f,
    @location(2) oit1: vec4f,
`)}
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

${condition(mOpt.has_color_texture,`
@group(1) @binding(${binding_tex_color})
var uTexColor: texture_2d<f32>;
`)}

${condition(mOpt.has_metalness_map,`
@group(1) @binding(${binding_tex_metalness})
var uTexMetalness: texture_2d<f32>;
`)}

${condition(mOpt.has_specular_map,`
@group(1) @binding(${binding_tex_specular})
var uTexSpecular: texture_2d<f32>;
`)}

${condition(mOpt.has_normal_map,`
@group(1) @binding(${binding_tex_normal})
var uTexNormal: texture_2d<f32>;
`)}

${condition(mOpt.has_emissive_map,`
@group(1) @binding(${binding_tex_emissive})
var uTexEmissive: texture_2d<f32>;
`)}

${condition(options.has_lightmap,`
@group(1) @binding(${binding_lightmap})
var uTexLightmap: texture_2d<f32>;
`)}

${condition(options.has_reflector, `
@group(1) @binding(${binding_reflector})
var uTexReflector: texture_2d<f32>;

@group(1) @binding(${binding_reflector + 1})
var uTexReflectorDepth: texture_depth_2d;

@group(1) @binding(${binding_reflector + 2})
var<uniform> uCameraReflector: Camera;

fn getRadiance(world_pos: vec3f, viewDir: vec3f, norm: vec3f, f0: vec3f, f90: f32, roughness: f32) -> vec3f
{
    var reflectVec = reflect(-viewDir, norm);

    let size_view = textureDimensions(uTexReflectorDepth, 0);   
    let view_origin = (uCameraReflector.viewMat * vec4(world_pos, 1.0)).xyz;
    var view_dir = (uCameraReflector.viewMat * vec4(reflectVec, 0.0)).xyz;

    let rows_proj = transpose(uCameraReflector.projMat);
    let dx = dot(rows_proj[0].xyz, view_dir);
    let dy = dot(rows_proj[1].xyz, view_dir);
    let dw = dot(rows_proj[3], vec4(view_origin, 1.0));
    let dxdt = dx/dw * f32(size_view.x)*0.5;
    let dydt = dy/dw * f32(size_view.y)*0.5;
    let dldt = sqrt(dxdt*dxdt + dydt*dydt);
        
    var t = 0.0;

    var view_pos = view_origin + t*view_dir;    
    var proj = uCameraReflector.projMat * vec4(view_pos, 1.0);
    proj*= 1.0/proj.w;

    proj.x = clamp(proj.x, uCameraReflector.scissor.x, uCameraReflector.scissor.z);
    proj.y = clamp(proj.y, uCameraReflector.scissor.y, uCameraReflector.scissor.w);

    var uvz = vec3((proj.x + 1.0)*0.5, (1.0 - proj.y)*0.5, (1.0 - proj.z)*0.5);
    var pix_pos = uvz.xy * vec2f(size_view);

    if (dldt>0.001)
    {
        let step = 5.0/dldt; 
        var old_t = t;
        t+=step*jitter;

        while(view_pos.z <0.0)
        {
            view_pos = view_origin + t*view_dir;
            proj = uCameraReflector.projMat * vec4(view_pos, 1.0);
            proj*= 1.0/proj.w;

            proj.x = clamp(proj.x, uCameraReflector.scissor.x, uCameraReflector.scissor.z);
            proj.y = clamp(proj.y, uCameraReflector.scissor.y, uCameraReflector.scissor.w);

            let old_z = uvz.z;
            uvz = vec3((proj.x + 1.0)*0.5, (1.0 - proj.y)*0.5, (proj.z + 1.0)*0.5);
            
            let depth = textureSampleLevel(uTexReflectorDepth, uSampler, uvz.xy, 0);
            if (uvz.z>=depth)
            {
                let k = (uvz.z-depth)/(uvz.z - old_z);
                t = old_t*k + t*(1.0-k);

                view_pos = view_origin + t*view_dir;
                proj = uCameraReflector.projMat * vec4(view_pos, 1.0);
                proj*= 1.0/proj.w;

                proj.x = clamp(proj.x, uCameraReflector.scissor.x, uCameraReflector.scissor.z);
                proj.y = clamp(proj.y, uCameraReflector.scissor.y, uCameraReflector.scissor.w);
                uvz = vec3((proj.x + 1.0)*0.5, (1.0 - proj.y)*0.5, (proj.z + 1.0)*0.5);
                break;
            }

            let old_pix_pos = pix_pos;
            pix_pos = uvz.xy * vec2f(size_view);
            let delta = length(pix_pos - old_pix_pos);
            if (delta<1.0)
            {
                break;
            }

            old_t = t;
            t+=step;
        }
    }

    let depth = textureSampleLevel(uTexReflectorDepth, uSampler, uvz.xy, 0);
    proj.z = depth*2.0 - 1.0;

    let _view_pos = uCameraReflector.invProjMat * proj;
	view_pos = _view_pos.xyz / _view_pos.w;
	t = length(view_pos - view_origin);	

    var up = vec3(1.0, 0.0, 0.0);
    if (norm.y<norm.x)
	{
		if (norm.z<norm.y)
		{
			up = vec3(0.0, 0.0, 1.0);
		}
		else
		{
			up = vec3(0.0, 1.0, 0.0);
		}
	}
	else if (norm.z < norm.x)
	{
		up = vec3(0.0, 0.0, 1.0);
	}

    let axis_x = normalize(cross(up, norm));
	let axis_y = cross(norm, axis_x);

    let alpha = roughness * roughness;
	let alpha2 = alpha*alpha;

    let count_samples = i32(alpha2/(1.0 + alpha2) * 2.0 * 64.0) + 1;

    var acc_weight = 0.0;
	var acc_col = vec3(0.0);

    for (var i=0; i<count_samples; i++)
	{
        let r = RandomFloat();
		let theta = acos(sqrt((1.0-r)/(1.0-(1.0- alpha2)*r)));
		let phi = RandomFloat() * 2.0 * PI;

        let z = cos(theta);
		let xy = sin(theta);	

		let H = xy * cos(phi) * axis_x + xy * sin(phi) * axis_y + z * norm;

        reflectVec = reflect(-viewDir, H);
        view_dir = (uCameraReflector.viewMat * vec4(reflectVec, 0.0)).xyz;

        view_pos = view_origin + t*view_dir;
        proj = uCameraReflector.projMat * vec4(view_pos, 1.0);
        proj*= 1.0/proj.w;

        proj.x = clamp(proj.x, uCameraReflector.scissor.x, uCameraReflector.scissor.z);
        proj.y = clamp(proj.y, uCameraReflector.scissor.y, uCameraReflector.scissor.w);
        
        uvz = vec3((proj.x + 1.0)*0.5, (1.0 - proj.y)*0.5, (proj.z + 1.0)*0.5);

        let col = textureSampleLevel(uTexReflector, uSampler, uvz.xy, 0).xyz;

        let dotVH = dot(viewDir, H);
		let dotNL = dot(norm, reflectVec);				
		let dotNH = dot(norm, H); 

        let weight = abs(dotVH) / (dotNL * dotNH);
		acc_weight += weight;
		acc_col += weight * col;

    }
    
    return acc_col/acc_weight;	
}

`)}

struct EnvironmentMap
{
    SHCoefficients: array<vec4f, 9>,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

${condition(options.has_primtive_probe,`
@group(1) @binding(${binding_primitive_probe})
var<uniform> uIndirectLight: EnvironmentMap;
`)}

struct PhysicalMaterial
{
    diffuseColor: vec3f,
    roughness: f32,
    specularColor: vec3f,
    specularF90: f32
};

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

fn luminance(color: vec3f) -> f32
{
    return color.x * 0.2126 + color.y * 0.7152 + color.z *0.0722;
}

${condition(lights_options.has_shadow,`
@group(2) @binding(${binding_shadow_sampler})
var uShadowSampler: sampler_comparison;

fn borderDepthTexture(shadowTex: texture_depth_2d, uv: vec2f) -> f32
{
    return select(1.0, textureSampleLevel(shadowTex, uSampler, uv, 0),  
    ((uv.x <= 1.0) && (uv.y <= 1.0) && (uv.x >= 0.0) && (uv.y >= 0.0)));
}

fn borderPCFTexture(shadowTex: texture_depth_2d, uvz : vec3f) -> f32
{
	return select(select(0.0, 1.0, uvz.z <= 1.0),  textureSampleCompareLevel(shadowTex, uShadowSampler, uvz.xy, uvz.z), 
        ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&  (uvz.x >= 0.0) && (uvz.y >= 0.0)));
}

fn findBlocker(shadowTex: texture_depth_2d, uv : vec2f, z: f32, searchRegionRadiusUV: vec2f) -> vec3f
{
    var ret = vec3(0.0, 0.0, 32.0);

    for (var i=0; i<32; i++)
    {
        let offset = vogel_sample(f32(i), 32.0, 0.5)  * searchRegionRadiusUV;
        let shadowMapDepth = borderDepthTexture(shadowTex, uv + offset);
        if (shadowMapDepth<z)
        {
            ret.x+=shadowMapDepth;
            ret.y+=1.0;
        }
    }

    return ret;
}

fn pcfFilter(shadowTex: texture_depth_2d, uv : vec2f, z: f32,  filterRadiusUV: vec2f) -> f32
{
    var sum = 0.0;

    for (var i=0; i<64; i++)
    {
        let offset = vogel_sample(f32(i), 64.0, jitter) * filterRadiusUV;
        sum += borderPCFTexture(shadowTex, vec3(uv + offset, z));
    }    

    return sum/64.0;
}

fn computePCSSShadowCoef(shadow: DirectionalShadow, zEye: f32, uvz : vec3f, shadowTex: texture_depth_2d)  -> f32
{       
    let uv = uvz.xy;
    let z = uvz.z;

    let frustum_size = vec2(shadow.left_right.y - shadow.left_right.x, shadow.bottom_up.y - shadow.bottom_up.x);
    let light_radius_uv =  vec2(shadow.light_radius) / frustum_size;
    let searchRegionRadiusUV = light_radius_uv* (zEye - shadow.near_far.x);
    let blocker = findBlocker(shadowTex, uv, z, searchRegionRadiusUV);

    if (blocker.y==0.0) 
    {
        return 1.0;
    }

    let avgBlockerDepth = blocker.x / blocker.y;
    let avgBlockerDepthWorld = (shadow.near_far.x + (shadow.near_far.y - shadow.near_far.x) * avgBlockerDepth);

    let penumbraRadius =  light_radius_uv * (zEye - avgBlockerDepthWorld);

    return pcfFilter(shadowTex, uv, z, penumbraRadius);
}
`)}


${(()=>{
    let code = "";
    let binding = binding_directional_light;
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

${condition(lights_options.has_reflection_map && !options.has_reflector,`
@group(2) @binding(${binding_reflection_map})
var uReflectionMap: texture_cube<f32>;

fn getReflRadiance(reflectVec: vec3f, roughness: f32) -> vec3f
{
    var gloss : f32;
    if (roughness < 0.053)
    {
        gloss = 1.0;        
    }
    else
    {
        let r2 = roughness * roughness;
        let r4 = r2*r2;
        gloss = log(2.0/r4 - 1.0)/log(2.0)/18.0;
    }
    let mip = (1.0-gloss)*6.0;
    return textureSampleLevel(uReflectionMap, uSampler, reflectVec, mip).xyz;
}

fn getRadiance(world_pos: vec3f, reflectVec: vec3f, roughness: f32, irradiance: vec3f) -> vec3f
{
    var rad = getReflRadiance(reflectVec, roughness);
    if (roughness > 0.053)
    {        
        let lum1 = luminance(rad);
        if (lum1>0.0)
        {
            var rad2 = irradiance * RECIPROCAL_PI;
            let lum2 = luminance(rad2);
            let r2 = roughness*roughness;
            let r4 = r2*r2;
            let gloss = log(2.0/r4 - 1.0)/log(2.0)/18.0;
            rad *= gloss + lum2/lum1 * (1.0-gloss);
        }
    }
    return rad;
}
`)}

${condition(lights_options.has_ambient_light,`
struct AmbientLight
{
    color: vec4f,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

@group(2) @binding(${binding_ambient_light})
var<uniform> uIndirectLight: AmbientLight;

fn getIrradiance(world_pos: vec3f, normal: vec3f) -> vec3f
{
    return uIndirectLight.color.xyz * PI;
}
`)}

${condition(lights_options.has_hemisphere_light,`
struct HemisphereLight
{
    skyColor: vec4f,
    groundColor: vec4f,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

@group(2) @binding(${binding_hemisphere_light})
var<uniform> uIndirectLight: HemisphereLight;

fn getIrradiance(world_pos: vec3f, normal: vec3f) -> vec3f
{
    let k = normal.y * 0.5 + 0.5;
    return mix( uIndirectLight.groundColor.xyz, uIndirectLight.skyColor.xyz, k) * PI;
}
`)}

${condition(lights_options.has_environment_map,`
@group(2) @binding(${binding_environment_map})
var<uniform> uIndirectLight: EnvironmentMap;
`)}

${condition(lights_options.has_environment_map || options.has_primtive_probe,`
fn getIrradiance(world_pos: vec3f, normal: vec3f) -> vec3f
{
    let x = normal.x;
    let y = normal.y;
    let z = normal.z;

    // band 0
    var result = uIndirectLight.SHCoefficients[0].xyz * 0.886227;

    // band 1
	result += uIndirectLight.SHCoefficients[1].xyz * 2.0 * 0.511664 * y;
	result += uIndirectLight.SHCoefficients[2].xyz * 2.0 * 0.511664 * z;
	result += uIndirectLight.SHCoefficients[3].xyz * 2.0 * 0.511664 * x;

    // band 2
	result += uIndirectLight.SHCoefficients[4].xyz * 2.0 * 0.429043 * x * y;
	result += uIndirectLight.SHCoefficients[5].xyz * 2.0 * 0.429043 * y * z;
	result += uIndirectLight.SHCoefficients[6].xyz * ( 0.743125 * z * z - 0.247708 );
	result += uIndirectLight.SHCoefficients[7].xyz * 2.0 * 0.429043 * x * z;
	result += uIndirectLight.SHCoefficients[8].xyz * 0.429043 * ( x * x - y * y );

    return result;
}
`)}

${condition(lights_options.has_probe_grid && !options.has_primtive_probe,`
struct ProbeGrid
{
    coverageMin: vec4f,
    coverageMax: vec4f,
    divisions: vec4i,
    ypower: f32,
    normalBias: f32,    
    visRes: i32,
    packSize: i32,
    packRes: i32,
    irrRes: i32,
    irrPackRes: i32,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};

@group(2) @binding(${binding_probe_grid})
var<uniform> uProbeGrid: ProbeGrid;
`)}

${condition(lights_options.has_lod_probe_grid && !options.has_primtive_probe,`
struct ProbeGrid
{
    coverageMin: vec4f,
    coverageMax: vec4f,
    baseDivisions: vec4i,
    subDivisionLevel: i32,
    normalBias: f32,
    numProbes: i32,
    visRes: i32,
    packSize: i32,
    packRes: i32,
    irrRes: i32,
    irrPackRes: i32,
    diffuseThresh: f32,
    diffuseHigh: f32,
    diffuseLow: f32,
    specularThresh: f32,
    specularHigh: f32,
    specularLow: f32,
};
@group(2) @binding(${binding_probe_grid})
var<uniform> uProbeGrid: ProbeGrid;
`)}

${condition((lights_options.has_probe_grid || lights_options.has_lod_probe_grid) && !options.has_primtive_probe,`
@group(2) @binding(${binding_probe_grid + 1})
var uTexIrr: texture_2d<f32>;

@group(2) @binding(${binding_probe_grid + 2})
var uTexVis: texture_2d<f32>;

fn signNotZero(v: vec2f) -> vec2f
{
    return vec2(select(-1.0, 1.0, v.x >= 0.0), select(-1.0, 1.0, v.y >= 0.0));
}

fn vec3_to_oct(v: vec3f) -> vec2f
{
    let p = v.xy * (1.0/ (abs(v.x) + abs(v.y) + abs(v.z)));
    return select(p, ((1.0 - abs(p.yx)) * signNotZero(p)), v.z <= 0.0);
}

fn get_irradiance_common(idx: i32, dir: vec3f) -> vec3f
{
    let probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
    let pack_x = idx % uProbeGrid.packSize;
    let pack_y = idx / uProbeGrid.packSize;
    let uv = (vec2(f32(pack_x), f32(pack_y)) * f32(uProbeGrid.irrRes + 2) + (probe_uv * f32(uProbeGrid.irrRes) + 1.0))/f32(uProbeGrid.irrPackRes);
    return textureSampleLevel(uTexIrr, uSampler, uv, 0).xyz;
}

fn get_mean_dis_common(dir: vec3f, idx: i32) -> vec2f
{
    let probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
    let pack_x = idx % uProbeGrid.packSize;
    let pack_y = idx / uProbeGrid.packSize;
    let uv = (vec2(f32(pack_x), f32(pack_y)) * f32(uProbeGrid.visRes + 2) + (probe_uv * f32(uProbeGrid.visRes) + 1.0))/f32(uProbeGrid.packRes);
    return textureSampleLevel(uTexVis, uSampler, uv, 0).xy;
}

fn get_visibility_common(wpos: vec3f, idx: i32, vert_world: vec3f, scale: f32) -> f32
{
    var dir = wpos - vert_world;
    let dis = length(dir);
    dir = normalize(dir);

    let mean_dis_var = get_mean_dis_common(dir, idx);
    let mean_dis = mean_dis_var.x;
    let mean_var = mean_dis_var.y;
    let mean_var2 =  mean_var * mean_var;
    let delta = max(dis - mean_dis, 0.0);
    let delta2 = delta*delta * scale * scale;	
    return mean_var2/(mean_var2 + delta2);
}

`)}

${condition(lights_options.has_probe_grid && !options.has_primtive_probe,`
fn get_visibility(wpos: vec3f, vert: vec3i,  vert_world: vec3f) -> f32
{
    let idx = vert.x + (vert.y + vert.z*uProbeGrid.divisions.y)*uProbeGrid.divisions.x;
    return get_visibility_common(wpos, idx, vert_world, 1.0);
}

fn get_irradiance(vert: vec3i, dir: vec3f) -> vec3f
{    
    let idx = vert.x + (vert.y + vert.z*uProbeGrid.divisions.y)*uProbeGrid.divisions.x;
    return get_irradiance_common(idx, dir);
}

fn getIrradiance(world_pos: vec3f, normal: vec3f) -> vec3f
{
    let wpos = world_pos + (N + 3.0 * viewDir) * uProbeGrid.normalBias;
    
    let size_grid = uProbeGrid.coverageMax.xyz - uProbeGrid.coverageMin.xyz;
    var pos_normalized = (wpos - uProbeGrid.coverageMin.xyz)/size_grid;
    pos_normalized.y = pow(pos_normalized.y, 1.0/uProbeGrid.ypower);	
    var pos_voxel = pos_normalized * vec3f(uProbeGrid.divisions.xyz) - vec3(0.5);
    pos_voxel = clamp(pos_voxel, vec3(0.0), vec3f(uProbeGrid.divisions.xyz) - vec3(1.0));

    let i_voxel = clamp(vec3i(pos_voxel), vec3i(0), uProbeGrid.divisions.xyz - vec3i(2));
    let frac_voxel = pos_voxel - vec3f(i_voxel);

    var sum_weight = 0.0;
    var irr = vec3(0.0);

    for (var i=0; i<8; i++)
    {
        let x = i & 1;
        let y = (i>>1)&1;
        let z = i>>2;
        let vert = i_voxel + vec3i(x,y,z);
        var vert_normalized = (vec3f(vert) + vec3f(0.5))/vec3f(uProbeGrid.divisions.xyz);
        vert_normalized.y = pow(vert_normalized.y, uProbeGrid.ypower); 
        let vert_world = vert_normalized * size_grid + uProbeGrid.coverageMin.xyz;
        let dir = normalize(vert_world - world_pos);
        var dotDirN = dot(dir, N);
        let k = 0.9;
        dotDirN = (k*dotDirN + sqrt(1.0 - (1.0-dotDirN*dotDirN)*k*k))/(k+1.0);
        var weight = dotDirN * get_visibility(wpos, vert, vert_world);	

        let crushThreshold = 0.2;
        if (weight < crushThreshold) {
            weight *= weight * weight / (crushThreshold*crushThreshold); 
        }

        let w = vec3(1.0) - abs(vec3f(f32(x),f32(y),f32(z)) - frac_voxel);
        weight *= w.x * w.y * w.z;
        if (weight> 0.0)
        {					
            sum_weight += weight;
            irr += get_irradiance(vert, normal) * weight;			
        }

    }
    
    if (sum_weight>0.0)
	{        
		return irr/sum_weight;
	}	

	return vec3(0.0);
}
`)}

${condition(lights_options.has_lod_probe_grid && !options.has_primtive_probe,`
@group(2) @binding(${binding_probe_grid + 3})
var<storage, read> bPosLod: array<vec4f>;

@group(2) @binding(${binding_probe_grid + 4})
var<storage, read> bIndexData: array<i32>;

fn get_visibility(wpos: vec3f, idx: i32, lod: i32, vert_world: vec3f) -> f32
{
    let scale = f32(1<<u32(uProbeGrid.subDivisionLevel -lod));
    return get_visibility_common(wpos, idx, vert_world, scale);
}

fn get_probe_idx(ipos: vec3i) -> i32
{
    let base_offset = uProbeGrid.baseDivisions.x * uProbeGrid.baseDivisions.y * uProbeGrid.baseDivisions.z;
    let ipos_base = ipos / (1<<u32(uProbeGrid.subDivisionLevel));
    var node_idx = ipos_base.x + (ipos_base.y + ipos_base.z *  uProbeGrid.baseDivisions.y) *  uProbeGrid.baseDivisions.x;
    var probe_idx = bIndexData[node_idx];

    var lod = 0;
    var digit_mask = 1 << u32(uProbeGrid.subDivisionLevel -1);
    while(lod<uProbeGrid.subDivisionLevel && probe_idx>=uProbeGrid.numProbes)
    {
        let offset = base_offset + (probe_idx - uProbeGrid.numProbes)*8;
        var sub = 0;
        if ((ipos.x & digit_mask) !=0) 
        {
            sub+=1;
        }
		if ((ipos.y & digit_mask) !=0) 
        {
            sub+=2;
        }
		if ((ipos.z & digit_mask) !=0) 
        {
            sub+=4;
        }
        node_idx = offset + sub;
        probe_idx = bIndexData[node_idx];

        lod++;
		digit_mask >>=1;
    }
    return probe_idx;
}

fn getIrradiance(world_pos: vec3f, normal: vec3f) -> vec3f
{
    let wpos = world_pos + (N + 3.0 * viewDir) * uProbeGrid.normalBias;
    let divs = uProbeGrid.baseDivisions.xyz * (1<<u32(uProbeGrid.subDivisionLevel));

    let size_grid = uProbeGrid.coverageMax.xyz - uProbeGrid.coverageMin.xyz;
    var pos_normalized = (wpos - uProbeGrid.coverageMin.xyz)/size_grid;    
    var pos_voxel = pos_normalized * vec3f(divs) - vec3(0.5);
    pos_voxel = clamp(pos_voxel, vec3(0.0), vec3f(divs) - vec3(1.0));

    let i_voxel = clamp(vec3i(pos_voxel), vec3i(0), divs - vec3i(2));
    let frac_voxel = pos_voxel - vec3f(i_voxel);

    var sum_weight = 0.0;
    var irr = vec3(0.0);

    for (var i=0; i<8; i++)
    {
        let x = i&1;
        let y = (i>>1)&1;
        let z = i>>2;
        let vert = i_voxel + vec3i(x,y,z);
        let idx_probe = get_probe_idx(vert);
        let pos_lod = bPosLod[idx_probe];
        let probe_world = pos_lod.xyz;
        let dir = normalize(probe_world - world_pos);
        var dotDirN = dot(dir, N);
        let k = 0.9;
        dotDirN = (k*dotDirN + sqrt(1.0 - (1.0-dotDirN*dotDirN)*k*k))/(k+1.0);
        var weight = dotDirN * get_visibility(wpos, idx_probe, i32(pos_lod.w), probe_world);

        let crushThreshold = 0.2;
        if (weight < crushThreshold) {
            weight *= weight * weight / (crushThreshold*crushThreshold); 
        }

        let w = vec3(1.0) - abs(vec3f(f32(x),f32(y),f32(z)) - frac_voxel);
        weight *= w.x * w.y * w.z;
        if (weight> 0.0)
        {	
            var sample_dir = normal;

            let distance = get_mean_dis_common(sample_dir, idx_probe).x;
            let pos_to = wpos + distance * sample_dir;
            sample_dir = normalize(pos_to - probe_world);

            sum_weight += weight;
            irr += get_irradiance_common(idx_probe, sample_dir) * weight;			
        }
    }

    if (sum_weight>0.0)
	{   
		return irr/sum_weight;
	}    
	return vec3(0.0);
}

`)}

${condition(lights_options.has_fog,`
struct Fog
{
    rgba: vec4f,  
    max_num_steps: i32,
    min_step: f32
};
@group(2) @binding(${binding_fog})
var<uniform> uFog: Fog;
`)}

@fragment
fn fs_main(input: FSIn) -> FSOut
{
    jitter = IGN(i32(input.coord_pix.x), i32(input.coord_pix.y));
    InitRandomSeed(u32(input.coord_pix.x), u32(input.coord_pix.y));
    //jitter = RandomFloat();

    var output: FSOut;

    var base_color = uMaterial.color;

${condition(options.has_color,`
    base_color *= input.color;
`)}

    var tex_alpha = 1.0;

${condition(mOpt.has_color_texture,`
    let tex_color = textureSample(uTexColor, uSampler, input.uv);
    tex_alpha = tex_color.w;
    base_color *= tex_color;
`)}

${condition(alpha_mask,`
    base_color.w = select(0.0, 1.0, base_color.w > uMaterial.alphaCutoff);
`)}


${condition(mOpt.specular_glossiness,`
    var specularFactor = uMaterial.specularGlossiness.xyz;
    var glossinessFactor = uMaterial.specularGlossiness.w;

${condition(mOpt.has_specular_map,`
    specularFactor *= textureSample(uTexSpecular, uSampler, input.uv).xyz;
    glossinessFactor *= textureSample(uTexSpecular, uSampler, input.uv).w;
`)}

`,`

    var metallicFactor = uMaterial.metalicFactor;
    var roughnessFactor = uMaterial.roughnessFactor;

${condition(mOpt.has_metalness_map,`
    metallicFactor *= textureSample(uTexMetalness, uSampler, input.uv).z;
    roughnessFactor *= textureSample(uTexMetalness, uSampler, input.uv).y;
`)}

`)}

    viewDir= normalize(input.viewDir);
    var norm = normalize(input.norm);


${condition(mOpt.has_normal_map,`
    {
        let T = normalize(input.tangent);
        let B = normalize(input.bitangent);
        var bump = textureSample(uTexNormal, uSampler, input.uv).xyz;
        bump = (2.0*bump - 1.0) * vec3(uMaterial.normalScale, 1.0 );
        norm = normalize(bump.x*T + bump.y*B + bump.z*norm);        
    }
`)}

    if (uMaterial.doubleSided!=0 && !input.front_facing)
    {
        norm = -norm;
    }
    let dxy =  max(abs(dpdx(norm)), abs(dpdy(norm)));
    let geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);	

    let dx = dpdx(input.worldPos);
    let dy = -dpdy(input.worldPos);
    N = normalize(cross(dx,dy));

${condition(alpha_mask,`
    if (base_color.w == 0.0)
    {
        discard;
    }
`)}

${condition(options.is_reflect,`
    let pos_refl_eye = uMatrixReflector * vec4(uCamera.eyePos.xyz, 1.0);
    let pos_refl_frag = uMatrixReflector * vec4(input.worldPos, 1.0);
    if (pos_refl_eye.z * pos_refl_frag.z > 0.0)
    {
        discard;
    }
`)}
    var material : PhysicalMaterial;

${condition(mOpt.specular_glossiness,`
    material.diffuseColor = base_color.xyz * ( 1.0 -
        max( max( specularFactor.r, specularFactor.g ), specularFactor.b ) );
    material.roughness = max( 1.0 - glossinessFactor, 0.0525 );	
    material.specularColor = specularFactor.rgb;
`,`
    material.diffuseColor = base_color.xyz * ( 1.0 - metallicFactor );	
	material.roughness = max( roughnessFactor, 0.0525 );	
	material.specularColor = mix( vec3( 0.04 ), base_color.xyz, metallicFactor );	
`)}    
   
    material.roughness += geometryRoughness;
	material.roughness = min( material.roughness, 1.0 );
	material.specularF90 = 1.0;

    let emissive = uMaterial.emissive.xyz;    

${condition(mOpt.has_emissive_map,`
    emissive *= textureSample(uTexEmissive, uSampler, input.uv).xyz;
`)}

    var specular = vec3(0.0);
    var diffuse = vec3(0.0);
${(()=>{
    let code = "";
    for (let i=0; i<directional_lights.length; i++)
    {
        let has_shadow = directional_lights[i];
        code += `
    {        
        var l_shadow = 1.0;

${condition(has_shadow,`
        let shadowCoords = uDirectionalShadow_${i}.VPSBMat * vec4(input.worldPos, 1.0);        
        if (uDirectionalShadow_${i}.light_radius > 0.0)        
        {
            let zEye = -(uDirectionalShadow_${i}.viewMat * vec4(input.worldPos, 1.0)).z;
            l_shadow = computePCSSShadowCoef(uDirectionalShadow_${i}, zEye, shadowCoords.xyz, uDirectionalShadowTex_${i});
        }
        else
        {
            l_shadow = borderPCFTexture(uDirectionalShadowTex_${i}, shadowCoords.xyz);
        }
`)}

${condition(lights_options.has_fog,`
        if (l_shadow>0.0)
        {
            let zEye = -dot(input.worldPos - uDirectionalLight_${i}.origin.xyz, uDirectionalLight_${i}.direction.xyz);
            if (zEye>0.0)
			{
				let att = pow(1.0 - uFog.rgba.w, zEye);
				l_shadow *= att;
			}
        }
`)}
        var directLight: IncidentLight;
        directLight.color = uDirectionalLight_${i}.color.xyz *l_shadow;
        directLight.direction = uDirectionalLight_${i}.direction.xyz;
        let dotNL = clamp(dot(norm, directLight.direction), 0.0, 1.0);
        let irradiance = dotNL * directLight.color;
        diffuse += irradiance * BRDF_Lambert(material.diffuseColor);
        specular += irradiance * BRDF_GGX( directLight.direction, viewDir, norm, material.specularColor, material.specularF90, material.roughness );
    }
`;
    }
    return code;
})()}

${condition(options.has_lightmap,`
    {    
        let light_color = textureSampleLevel(uTexLightmap, uSampler, input.atlasUV, 0).xyz;
        diffuse += material.diffuseColor * light_color;

${condition(options.has_reflector,`        
        let radiance = getRadiance(input.worldPos, viewDir, norm, material.specularColor, material.specularF90, material.roughness);
        specular += material.specularColor * radiance;
`,`
${condition(lights_options.has_reflection_map,`
        var reflectVec = reflect(-viewDir, norm);
        reflectVec = normalize( mix( reflectVec, norm, material.roughness * material.roughness) );	
        let radiance = getRadiance(input.worldPos, reflectVec, material.roughness,  light_color * PI);
        specular += material.specularColor * radiance;
`,`
        specular += material.specularColor * light_color;
`)}
`)}

    }
`,condition(has_indirect_light, `
    {
        let irradiance = getIrradiance(input.worldPos, norm);
        var radiance = vec3(0.0);

${condition(options.has_reflector,`        
        radiance = getRadiance(input.worldPos, viewDir, norm, material.specularColor, material.specularF90, material.roughness);
`,`
${condition(lights_options.has_reflection_map,`
        var reflectVec = reflect(-viewDir, norm);	
        reflectVec = normalize( mix( reflectVec, norm, material.roughness * material.roughness) );	
        radiance = getRadiance(input.worldPos, reflectVec, material.roughness, irradiance);
`,`
        radiance = irradiance * RECIPROCAL_PI;
`)}
`)}
        diffuse += material.diffuseColor * irradiance * RECIPROCAL_PI;
        specular +=  material.specularColor * radiance;
    }
`))}

    var col = emissive + specular;

${condition(alpha_blend,` 
    col = clamp(col, vec3(0.0), vec3(1.0));
    output.color = vec4(col * tex_alpha, 0.0);
    col += diffuse;

    let alpha = base_color.w;
    let a = min(1.0, alpha)*8.0 + 0.01;
    let b = -input.coord_pix.z *0.95 + 1.0;
    let weight = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
    output.oit0 = vec4(col * alpha, alpha) * weight;
    output.oit1 = vec4(alpha);    
`,`
    col += diffuse;
    col = clamp(col, vec3(0.0), vec3(1.0));
    output.color = vec4(col, 1.0);
`)}    
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
            depthWriteEnabled: options.alpha_mode == "Mask" || (options.alpha_mode == "Opaque" && options.is_reflect),
            depthCompare: 'less-equal',
            format: 'depth32float'
        };

        let vertex_bufs = [];

        let location_attrib = 0;

        const positionAttribDesc = {
            shaderLocation: location_attrib++,
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
            shaderLocation: location_attrib++, 
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
                shaderLocation: location_attrib++, 
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
        let has_uv = mOpt.has_color_texture || mOpt.has_metalness_map || mOpt.has_specular_map || mOpt.has_normal_map || mOpt.has_emissive_map;
        if (has_uv)
        {
            const UVAttribDesc = {
                shaderLocation: location_attrib++, 
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

        if (options.has_lightmap)
        {
            const UVAttribDesc = {
                shaderLocation: location_attrib++, 
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
                shaderLocation: location_attrib++, 
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
                shaderLocation: location_attrib++, 
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
            frontFace: options.is_reflect?'cw':'ccw',
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

        if (options.alpha_mode == "Blend")
        {                        
            const colorState0 = {
                format: options.view_format,
                blend: {
                    color: {
                        srcFactor: "one",
                        dstFactor: "one"
                    },
                    alpha: {
                        srcFactor: "zero",
                        dstFactor: "one"
                    }
                },
                writeMask: GPUColorWrite.ALL
            };

            const colorState1 = {
                format: 'rgba16float',
                blend: {
                    color: {
                        srcFactor: "one",
                        dstFactor: "one"
                    },
                    alpha: {
                        srcFactor: "one",
                        dstFactor: "one"
                    }
                },
                writeMask: GPUColorWrite.ALL
            };

            const colorState2 = {
                format: 'r8unorm',
                blend: {
                    color: {
                        srcFactor: "zero",
                        dstFactor: "one-minus-src"
                    },
                    alpha: {
                        srcFactor: "zero",
                        dstFactor: "one-minus-src"
                    }
                },
                writeMask: GPUColorWrite.ALL
            };

            fragment.targets = [colorState0, colorState1, colorState2];
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
    options.has_color = primitive.color_buf != null;
    options.has_lightmap = primitive.has_lightmap;
    options.has_reflector = primitive.has_reflector;
    options.material_options = primitive.material_options;
    options.lights_options = params.lights.get_options();
    options.has_primtive_probe = primitive.envMap!=null;
    options.is_reflect = params.camera.reflector!=null;

    let pipeline = GetPipelineStandard(options);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, params.camera.bind_group);
    passEncoder.setBindGroup(1, primitive.bind_group);   

    if (params.lights.bind_group!=null)
    {
        passEncoder.setBindGroup(2, params.lights.bind_group);   
    }

    let location_attrib = 0;

    passEncoder.setVertexBuffer(location_attrib++, geo.pos_buf);
    passEncoder.setVertexBuffer(location_attrib++, geo.normal_buf);

    if (options.has_color)
    {
        passEncoder.setVertexBuffer(location_attrib++, primitive.color_buf);
    }

    let mOpt = options.material_options;
    let has_uv = mOpt.has_color_texture || mOpt.has_metalness_map || mOpt.has_specular_map || mOpt.has_normal_map || mOpt.has_emissive_map;
    if (has_uv)
	{
        passEncoder.setVertexBuffer(location_attrib++, primitive.uv_buf);
    }

    if (options.has_lightmap)
    {
        passEncoder.setVertexBuffer(location_attrib++, primitive.lightmap_uv_buf);
    }

    if (mOpt.has_normal_map)
    {
        passEncoder.setVertexBuffer(location_attrib++, geo.tangent_buf);
        passEncoder.setVertexBuffer(location_attrib++, geo.bitangent_buf);
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
    let primitive = params.primitive;   
    let material = params.material_list[primitive.material_idx];

    let color_formats = [params.target.view_format];
    if (material.alphaMode == "Blend")
    {
        color_formats.push('rgba16float');
        color_formats.push('r8unorm');
    }

    const renderBundleEncoder = engine_ctx.device.createRenderBundleEncoder({
        colorFormats: color_formats,
        depthStencilFormat: 'depth32float',
        sampleCount: params.target.msaa?4:1
    });
    

    RenderStandard(renderBundleEncoder, params);

    return renderBundleEncoder.finish();

}
