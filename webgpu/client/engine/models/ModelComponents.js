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
        this.lightmap_uv_buf = null;
        this.envMap = null;

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

        this.cpu_pos = null;
        this.cpu_indices = null;
                
        this.constant_morph = null;
        this.bind_group_morph = null;
        this.bind_group_morph2 = null;

        this.constant_skin = null;
        this.bind_group_skin = null;
        this.bind_group_skin2 = null;

        this.is_geometry_ready = false;
        this.geometry_resolvers = [];
        
    }

    async geometry_ready()
    {
        return new Promise((resolve, reject) => {
            if (this.is_geometry_ready)
            {
                resolve(true);
            }
            else
            {
                this.geometry_resolvers.push(resolve);
            }
        });
    }

    set_geometry_ready()
    {
        this.is_geometry_ready = true;
        for (let resolve of this.geometry_resolvers) 
        {
            resolve(true);
        }
    }

    updateUUID()
    {
        this.uuid = MathUtils.generateUUID();
    }

    create_bind_group(model_constant, material_list, tex_list, lightmap = null)
    {        
        let material = material_list[this.material_idx];
       
        if (!("primitive" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.primitive = {};
        }

        this.material_options = material.get_options(tex_list);
        this.has_lightmap = lightmap != null;
        let options = {
            material: this.material_options,
            has_lightmap: this.has_lightmap,
            has_envmap: this.envMap !=null
        };
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
                    visibility: GPUShaderStage.VERTEX | GPUShaderStage.FRAGMENT,
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
            if (this.material_options.has_color_texture) count_textures++;
            if (this.material_options.has_metalness_map) count_textures++;            
            if (this.material_options.has_specular_map) count_textures++;
            if (this.material_options.has_normal_map) count_textures++;
            if (this.material_options.has_emissive_map) count_textures++;

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

            if (lightmap!=null)
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

            if (this.envMap!=null)
            {
                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
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

        if (this.material_options.has_color_texture) 
        {
            let tex = tex_list[material.tex_idx_map];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.material_options.has_metalness_map)
        {
            let tex = tex_list[material.tex_idx_metalnessMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }        

        if (this.material_options.has_specular_map)
        {
            let tex = tex_list[material.tex_idx_specularMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.material_options.has_normal_map)
        {
            let tex = tex_list[material.tex_idx_normalMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (this.material_options.has_emissive_map)
        {
            let tex = tex_list[material.tex_idx_emissiveMap];
            entries.push({
                binding,
                resource: tex.createView()
            });
            binding++;
        }

        if (lightmap!=null)
        {
            entries.push({
                binding,
                resource: lightmap.createView()
            });
            binding++;
        }

        if (this.envMap!=null)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.envMap.constant
                }
            });
            binding++;
        }

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.primitive[signature];
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });

        if (this.uuid!=0) this.updateUUID();
    }    

    create_bind_group_morph(buf_weights)
    {       
        let options = {            
            sparse: this.none_zero_buf != null
        };
        let signature = JSON.stringify(options);

        if (!("morph" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.morph = {};
        }

        if (!(signature in engine_ctx.cache.bindGroupLayouts.morph))
        {

            let binding = 0;
            let entries = [];

            // Coefficients
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // PosBase
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // PosDelta
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // PosOut
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "storage"
                }
            });
            binding++;

            // NormBase
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // NormDelta
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // NormOut
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "storage"
                }
            });
            binding++;            

            if (options.sparse)
            {
                // Nonzero
                entries.push({
                    binding,
                    visibility: GPUShaderStage.COMPUTE,
                    buffer:{
                        type: "read-only-storage"
                    }
                });
                binding++;
            }

            // Uniforms
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "uniform"
                }
            });
            binding++;

            engine_ctx.cache.bindGroupLayouts.morph[signature] = engine_ctx.device.createBindGroupLayout({ entries });
        }

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.morph[signature];
       
        let uniform = new Int32Array(4);
        uniform[0] = this.num_pos;
        uniform[1] = this.num_targets;
        this.constant_morph = engine_ctx.createBuffer(uniform.buffer, GPUBufferUsage.UNIFORM, 0, 16);

        let geo_in = this.geometry[0];
        let geo_out = this.geometry[1];

        let binding = 0;
        let entries = [];          
        
        entries.push({
            binding,
            resource:{
                buffer: buf_weights
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: geo_in.pos_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: this.targets.pos_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: geo_out.pos_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: geo_in.normal_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: this.targets.normal_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: geo_out.normal_buf
            }
        });
        binding++;

        if (options.sparse)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.none_zero_buf
                }
            });
            binding++;
        }     

        entries.push({
            binding,
            resource:{
                buffer: this.constant_morph
            }
        });
        binding++;        
        
        this.bind_group_morph = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });      

        if (this.geometry[0].tangent_buf != null)
        {
            let binding = 0;
            let entries = [];          
            
            entries.push({
                binding,
                resource:{
                    buffer: buf_weights
                }
            });
            binding++;
            
            entries.push({
                binding,
                resource:{
                    buffer: geo_in.tangent_buf
                }
            });
            binding++;
    
            entries.push({
                binding,
                resource:{
                    buffer: this.targets.tangent_buf
                }
            });
            binding++;
    
            entries.push({
                binding,
                resource:{
                    buffer: geo_out.tangent_buf
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: geo_in.bitangent_buf
                }
            });
            binding++;
    
            entries.push({
                binding,
                resource:{
                    buffer: this.targets.bitangent_buf
                }
            });
            binding++;
    
            entries.push({
                binding,
                resource:{
                    buffer: geo_out.bitangent_buf
                }
            });
            binding++;

            if (options.sparse)
            {
                entries.push({
                    binding,
                    resource:{
                        buffer: this.none_zero_buf
                    }
                });
                binding++;
            }     

            entries.push({
                binding,
                resource:{
                    buffer: this.constant_morph
                }
            });
            binding++;        
            
            this.bind_group_morph2 = engine_ctx.device.createBindGroup({
                layout: bindGroupLayout,
                entries
            });      
        }
    }

    create_bind_group_skin(buf_rela_mat)
    {
        if (!("skin" in engine_ctx.cache.bindGroupLayouts))        
        {
            let binding = 0;
            let entries = [];

            // Rela-Mat
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // Joints
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // Weights
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // PosRest
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // PosOut
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "storage"
                }
            });
            binding++;


            // NormRest
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "read-only-storage"
                }
            });
            binding++;

            // NormOut
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "storage"
                }
            });
            binding++;            

            // Uniforms
            entries.push({
                binding,
                visibility: GPUShaderStage.COMPUTE,
                buffer:{
                    type: "uniform"
                }
            });
            binding++;            

            engine_ctx.cache.bindGroupLayouts.skin = engine_ctx.device.createBindGroupLayout({ entries });
        }
        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.skin;

        let uniform = new Int32Array(4);
        uniform[0] = this.num_pos;        
        this.constant_skin = engine_ctx.createBuffer(uniform.buffer, GPUBufferUsage.UNIFORM, 0, 16);

        let geo_id = this.num_targets > 0 ? 1: 0;
        let geo_in = this.geometry[geo_id];
        let geo_out = this.geometry[geo_id + 1];

        let binding = 0;
        let entries = [];

        entries.push({
            binding,
            resource:{
                buffer: buf_rela_mat
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: this.joints_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: this.weights_buf
            }
        });
        binding++;
        
        entries.push({
            binding,
            resource:{
                buffer: geo_in.pos_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: geo_out.pos_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: geo_in.normal_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: geo_out.normal_buf
            }
        });
        binding++;

        entries.push({
            binding,
            resource:{
                buffer: this.constant_skin
            }
        });
        binding++;  
        
        this.bind_group_skin = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });     

        if (this.geometry[0].tangent_buf != null)
        {
            let binding = 0;
            let entries = [];

            entries.push({
                binding,
                resource:{
                    buffer: buf_rela_mat
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: this.joints_buf
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: this.weights_buf
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: geo_in.tangent_buf
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: geo_out.tangent_buf
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: geo_in.bitangent_buf
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: geo_out.bitangent_buf
                }
            });
            binding++;      
            
            entries.push({
                binding,
                resource:{
                    buffer: this.constant_skin
                }
            });
            binding++;      
            
            this.bind_group_skin2 = engine_ctx.device.createBindGroup({
                layout: bindGroupLayout,
                entries
            });     
        }        
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
        this.inverseBindMatrices = [];
        this.buf_rela_mat = null;
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
        this.buf_weights = null;
        this.needUpdateMorphTargets = false;
        this.constant = engine_ctx.createBuffer0(128, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);
    }

    update_weights()
    {
        let arr = new Float32Array(this.weights);        
        engine_ctx.queue.writeBuffer(this.buf_weights, 0, arr.buffer, 0, arr.length*4);        
    }
}
