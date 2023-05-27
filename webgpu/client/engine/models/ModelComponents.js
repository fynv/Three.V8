import { Vector3 } from "../math/Vector3.js"
import { Matrix4 } from "../math/Matrix4.js"
import { Quaternion } from "../math/Quaternion.js";
import * as MathUtils from '../math/MathUtils.js';

export function UpdateConstant_Model(constant, matrixWorld)
{
    let normMatrix =  matrixWorld.clone();
    normMatrix.invert();
    normMatrix.transpose();
    const uniform = new Float32Array(16*2);
    for (let i=0; i<16; i++)
    {
        uniform[i] = matrixWorld.elements[i];
    }
    for (let i=0; i<16; i++)
    {
        uniform[16+i] = normMatrix.elements[i];
    }
    engine_ctx.queue.writeBuffer(constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
}

export class GeometrySet
{
    constructor()
    {
        this.pos_buf = null;
        this.normal_buf = null;
        this.tangent_buf = null;
        this.bitangent_buf = null;
    }
}

export class Primitive
{
    constructor()
    {
        this.num_pos = 0;

        this.geometry = [];
        this.color_buf = null;
        this.uv_buf = null;
        this.joints_buf = null;
        this.weights_buf = null;

        this.num_face = 0;
        this.type_indices = 2;
        this.index_buf = null;

        this.num_targets = 0;
        this.targets = null;
        this.none_zero_buf = null;

        this.material_idx = -1;

        this.min_pos = new Vector3(Infinity,Infinity,Infinity);
        this.max_pos = new Vector3(-Infinity, -Infinity, -Infinity);

        this.uuid = 0;
        this.bind_group = null;
    }

    updateUUID()
    {
        this.uuid = MathUtils.generateUUID();
    }

    create_bind_group(model_constant, material_list, tex_list)
    {        
        this.updateUUID();

        let material = material_list[this.material_idx];
       
        if (!("primitive" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.primitive = {};
        }

        let options = material.get_options();
        let signature = JSON.stringify(options);
        if (!(signature in engine_ctx.cache.bindGroupLayouts.primitive))
        {

            let entries = [
                {
                    binding: 0,
                    visibility: GPUShaderStage.VERTEX,
                    buffer:{
                        type: "uniform"
                    }
                },
                {
                    binding: 1,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                },
                {
                    binding: 2,
                    visibility: GPUShaderStage.FRAGMENT,
                    sampler:{}
                },
            ];

            let count_textures = 0;
            if (options.has_color_texture) count_textures++;
            if (options.has_metalness_map) count_textures++;
            if (options.has_roughness_map) count_textures++;
            if (options.has_specular_map) count_textures++;
            if (options.has_glossiness_map) count_textures++;
            if (options.has_normal_map) count_textures++;
            if (options.has_emissive_map) count_textures++;

            let binding = 3;
            for (let i=0; i<count_textures; i++)
            {
                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    texture:{
                        viewDimension: "2d"
                    }
                });
                binding++;
            }

            engine_ctx.cache.bindGroupLayouts.primitive[signature] = engine_ctx.device.createBindGroupLayout({ entries });
        }

        let entries = [
            {
                binding: 0,
                resource:{
                    buffer: model_constant
                }
            },
            {
                binding: 1,
                resource:{
                    buffer: material.constant
                }
            },
            {
                binding: 2,
                resource: material.sampler
            }
        ];

        let binding = 3;

        if (material.tex_idx_map>=0)
        {
            let tex = tex_list[material.tex_idx_map];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (material.tex_idx_metalnessMap>=0)
        {
            let tex = tex_list[material.tex_idx_metalnessMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (material.tex_idx_roughnessMap>=0)
        {
            let tex = tex_list[material.tex_idx_roughnessMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (material.tex_idx_specularMap>=0)
        {
            let tex = tex_list[material.tex_idx_specularMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (material.tex_idx_glossinessMap>=0)
        {
            let tex = tex_list[material.tex_idx_glossinessMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (material.tex_idx_normalMap>=0)
        {
            let tex = tex_list[material.tex_idx_normalMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (material.tex_idx_emissiveMap>=0)
        {
            let tex = tex_list[material.tex_idx_emissiveMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.primitive[signature];
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });
    }
    
}

export class Node
{
    constructor()
    {
        this.children = [];
        this.translation = new Vector3(0,0,0);
        this.rotation = new Quaternion(0,0,0,1);
        this.scale = new Vector3(1,1,1);
        this.g_trans = new Matrix4();    
    }

}

export class Skin
{
    constructor()
    {
        this.joints = [];
        this.inverseBindMatrices = new Matrix4();
    }
}

export class Mesh
{
    constructor()
    {
        this.node_id = -1;
        this.skin_id = -1;
        this.primitives = [];
        this.weights = [];
        this.needUpdateMorphTargets = false;
    }
}
