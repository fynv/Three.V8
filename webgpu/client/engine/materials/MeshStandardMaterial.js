import { Vector2 } from "../math/Vector2.js"
import { Vector3 } from "../math/Vector3.js"
import { Color } from "../math/Color.js"
import * as MathUtils from '../math/MathUtils.js';

export class MeshStandardMaterial
{
    constructor()
    {
        this.alphaMode = "Opaque";
        this.alphaCutoff = 0.5;
        this.doubleSided = false;
        this.specular_glossiness = false;

        this.color = new Color(1.0, 1.0, 1.0);
        this.alpha = 1.0;
        this.normalScale = new Vector2(1.0, 1.0);
        this.emissive = new Vector3(0,0,0);
        this.tex_idx_map = -1;
        this.tex_idx_normalMap = -1;
        this.tex_idx_emissiveMap = -1;

        this.metallicFactor = 0.0;
        this.roughnessFactor = 1.0;
        this.tex_idx_metalnessMap = -1;
        this.tex_idx_roughnessMap = -1;
        
        this.specular = new Color(1.0, 1.0, 1.0);
        this.glossinessFactor = 0.0;
        this.tex_idx_specularMap = -1;
        this.tex_idx_glossinessMap = -1;       

        this.constant = engine_ctx.createBuffer0(80, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        this.sampler = engine_ctx.device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear',
            mipmapFilter: "linear"
        });

        this.create_binding_group(null);

    }

    get_options()
    {
        return {
            doubleSided: this.doubleSided,
            specular_glossiness: this.specular_glossiness,
            has_color_texture: this.tex_idx_map >=0,
            has_metalness_map: this.tex_idx_metalnessMap >=0,
            has_roughness_map: this.tex_idx_roughnessMap >=0,
            has_specular_map: this.tex_idx_specularMap >=0,
            has_glossiness_map: this.tex_idx_glossinessMap >=0,
            has_normal_map: this.tex_idx_normalMap >=0,
            has_emissive_map: this.tex_idx_emissiveMap >=0
        };
    }

    update_constant()
    {
        const uniform = new Float32Array(20);
        const iuniform = new Int32Array(uniform.buffer);
        uniform[0] = this.color.r;
        uniform[1] = this.color.g;
        uniform[2] = this.color.b;
        uniform[3] = this.alpha;
        uniform[4] = this.emissive.x;
        uniform[5] = this.emissive.y;
        uniform[6] = this.emissive.z;
        uniform[8] = this.specular.r;
        uniform[9] = this.specular.g;
        uniform[10] = this.specular.b;
        uniform[11] = this.glossinessFactor;
        uniform[12] = this.normalScale.x;
        uniform[13] = this.normalScale.y;
        uniform[14] = this.metallicFactor;
        uniform[15] = this.roughnessFactor;
        uniform[16] = this.alphaCutoff;
        iuniform[17] = this.doubleSided?1:0;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }

    create_binding_group(tex_list)
    {
        this.uuid = MathUtils.generateUUID();
        if (!("mesh_standard_material" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.mesh_standard_material = {};
        }        
        
        let options = this.get_options();
        let signature = JSON.stringify(options);
        if (!(signature in engine_ctx.cache.bindGroupLayouts.mesh_standard_material))
        {

            let entries = [
                {
                    binding: 0,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                },
                {
                    binding: 1,
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

            let binding = 2;
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

            engine_ctx.cache.bindGroupLayouts.mesh_standard_material[signature] = engine_ctx.device.createBindGroupLayout({ entries });
        }

        let entries = [
            {
                binding: 0,
                resource:{
                    buffer: this.constant
                }
            },
            {
                binding: 1,
                resource: this.sampler
            }
        ];

        let binding = 2;
        
        if (this.tex_idx_map>=0)
        {
            let tex = tex_list[this.tex_idx_map];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.tex_idx_metalnessMap>=0)
        {
            let tex = tex_list[this.tex_idx_metalnessMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.tex_idx_roughnessMap>=0)
        {
            let tex = tex_list[this.tex_idx_roughnessMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.tex_idx_specularMap>=0)
        {
            let tex = tex_list[this.tex_idx_specularMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.tex_idx_glossinessMap>=0)
        {
            let tex = tex_list[this.tex_idx_glossinessMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.tex_idx_normalMap>=0)
        {
            let tex = tex_list[this.tex_idx_normalMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.tex_idx_emissiveMap>=0)
        {
            let tex = tex_list[this.tex_idx_emissiveMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.mesh_standard_material[signature];
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });
    }

}

