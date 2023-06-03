import { wgsl } from '../../wgsl-preprocessor.js';

function get_shader(options)
{
    let idx_sparse = options.has_tangent?13:7;
    let idx_uniform = options.sparse? idx_sparse+1 : idx_sparse;

    return wgsl`
@group(0) @binding(0)
var<storage, read> coefs : array<f32>;

@group(0) @binding(1)
var<storage, read> pos_base : array<vec4f>;

@group(0) @binding(2)
var<storage, read> pos_delta : array<vec4f>;

@group(0) @binding(3)
var<storage, read_write> pos_out : array<vec4f>;

@group(0) @binding(4)
var<storage, read> norm_base : array<vec4f>;

@group(0) @binding(5)
var<storage, read> norm_delta : array<vec4f>;

@group(0) @binding(6)
var<storage, read_write> norm_out : array<vec4f>;

#if ${options.has_tangent}
@group(0) @binding(7)
var<storage, read> tangent_base : array<vec4f>;

@group(0) @binding(8)
var<storage, read> tangent_delta : array<vec4f>;

@group(0) @binding(9)
var<storage, read_write> tangent_out : array<vec4f>;

@group(0) @binding(10)
var<storage, read> bitangent_base : array<vec4f>;

@group(0) @binding(11)
var<storage, read> bitangent_delta : array<vec4f>;

@group(0) @binding(12)
var<storage, read_write> bitangent_out : array<vec4f>;
#endif

#if ${options.sparse}
@group(0) @binding(${idx_sparse})
var<storage, read> nonzero : array<i32>;
#endif

struct Params
{
    num_verts: i32,
    num_deltas: i32
};

@group(0) @binding(${idx_uniform})
var<uniform> uParams: Params;

@compute @workgroup_size(128,1,1)
fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>)
{
    let idx = i32(GlobalInvocationID.x);
    if (idx>=uParams.num_verts) 
    {
        return;
    }
    var pos = pos_base[idx];
    var norm = norm_base[idx];

#if ${options.has_tangent}
    var tangent = tangent_base[idx];
    var bitangent = bitangent_base[idx];
#endif

#if ${options.sparse}
    let to_morph = (nonzero[idx]!=0);
#else
    let to_morph = true;
#endif
    if (to_morph)
    {
        for (var i=0; i<uParams.num_deltas; i++)
        {
            let coef = clamp(coefs[i], 0.0,1.0);
            if (coef==0.0) 
            {
                continue;
            }

            {
                let delta = pos_delta[i*uParams.num_verts + idx];
                pos += delta * coef;
            }

            {
                let delta = norm_delta[i*uParams.num_verts + idx];
                norm += delta * coef;
            }

#if ${options.has_tangent}
            {
                let delta = tangent_delta[i*uParams.num_verts + idx];
                tangent += delta * coef;
            }

            {
                let delta = bitangent_delta[i*uParams.num_verts + idx];
                bitangent += delta * coef;
            }
#endif
        }
    }

    pos_out[idx] = pos;
	norm_out[idx] = norm;
#if ${options.has_tangent}
	tangent_out[idx] = tangent;
	bitangent_out[idx] = bitangent;
#endif
}
`;
}


function GetPipelineMorph(options)
{
    let signature = JSON.stringify(options);
    if (!("morph" in engine_ctx.cache.pipelines))
    {
        engine_ctx.cache.pipelines.morph = {};        
    }

    if (!(signature in engine_ctx.cache.pipelines.morph))
    {
        let shader_code = get_shader(options);      
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_code });

        let bindGroupLayouts = [engine_ctx.cache.bindGroupLayouts.morph[signature]];
        const pipelineLayoutDesc = { bindGroupLayouts };
        let layout = engine_ctx.device.createPipelineLayout(pipelineLayoutDesc);

        engine_ctx.cache.pipelines.morph[signature]= engine_ctx.device.createComputePipeline({
            layout,
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }
    return engine_ctx.cache.pipelines.morph[signature];
}


export function MorphUpdate(passEncoder, primitive)
{
    let options = {
        has_tangent: primitive.geometry[0].tangent_buf != null,
        sparse: primitive.none_zero_buf != null
    };

    let pipeline = GetPipelineMorph(options);

    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, primitive.bind_group_morph);
    passEncoder.dispatchWorkgroups(Math.floor((primitive.num_pos + 127) / 128), 1,1);        

}
