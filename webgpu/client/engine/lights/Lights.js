export function LightsIsEmpty(options)
{
    return options.directional_lights.length == 0 && !options.has_reflection_map 
        && !options.has_ambient_light && !options.has_hemisphere_light && !options.has_environment_map && !options.has_probe_grid && !options.has_lod_probe_grid;
}

export class Lights
{
    constructor()
    {        
        this.clear_lists();
        this.signature = this.get_signature();
        this.bind_group = null;  
        this.fog = null;
        this.bind_group_fog_indirect = null;   
        this.bind_group_fog_directional = [];
        this.sampler = engine_ctx.device.createSampler({
            compare: 'less',
            magFilter: 'linear',
            minFilter: 'linear',            
        });   
    }

    get_signature()
    {        
        let directional_lights = [];
        for (let light of  this.directional_lights)
        {
            directional_lights.push(light.uuid);
        }       
        let obj = { directional_lights };
        if (this.reflection_map!=null)
        {
            obj.reflection_map = this.reflection_map.uuid;
        }
        if (this.ambient_light!=null)
        {
            obj.ambient_light = this.ambient_light.uuid;
        }
        if (this.hemisphere_light!=null)
        {
            obj.hemisphere_light = this.hemisphere_light.uuid;
        }
        if (this.environment_map!=null)
        {
            obj.environment_map = this.environment_map.uuid;
        }
        if (this.probe_grid!=null)
        {
            obj.probe_grid = this.probe_grid.uuid;
        }
        if (this.lod_probe_grid!=null)
        {
            obj.lod_probe_grid = this.lod_probe_grid.uuid;
        }
        if (this.fog!=null)
        {
            obj.fog = this.fog.uuid;
        }
        return JSON.stringify(obj);
    }

    get_options()
    {
        let has_shadow = false;
        let directional_lights = [];
        for (let light of this.directional_lights)
        {
            if (light.shadow!=null)
            {
                has_shadow = true;
            }
            directional_lights.push(light.shadow!=null);            
        }
        let has_reflection_map = this.reflection_map!=null;
        let has_ambient_light = this.ambient_light!=null;
        let has_hemisphere_light = this.hemisphere_light!=null;
        let has_environment_map = this.environment_map!=null;
        let has_probe_grid = this.probe_grid!=null;
        let has_lod_probe_grid = this.lod_probe_grid!=null;
        let has_fog = this.fog!=null;
        return { has_shadow, directional_lights, has_reflection_map, has_ambient_light, has_hemisphere_light, has_environment_map, has_probe_grid, has_lod_probe_grid, has_fog};
    }

    clear_lists()
    {
        this.directional_lights = [];
        this.reflection_map = null;
        this.ambient_light = null;
        this.hemisphere_light = null;
        this.environment_map = null;
        this.probe_grid = null;
        this.lod_probe_grid = null;
        this.fog = null;
    }

    update_bind_group()
    {
        let new_signature = this.get_signature();
        if (new_signature == this.signature) return;
        this.signature = new_signature;

        let options = this.get_options();        

        if (LightsIsEmpty(options))
        {            
            this.bind_group = null;            
            return;
        }        

        if (!("lights" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.lights = {};
        }
       
        let signature = JSON.stringify(options);
        if (!(signature in engine_ctx.cache.bindGroupLayouts.lights))
        {            
            let entries = [];
            let binding = 0;

            if (options.has_shadow)
            {
                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    sampler:{
                        type: 'comparison',
                    }
                });
                binding++;
            }
            
            for (let light of this.directional_lights)
            {
                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                });
                binding++;

                if (light.shadow!=null)
                {
                    entries.push({
                        binding,
                        visibility: GPUShaderStage.FRAGMENT,
                        buffer:{
                            type: "uniform"
                        }
                    });
                    binding++;

                    entries.push({
                        binding,
                        visibility: GPUShaderStage.FRAGMENT,
                        texture:{
                            viewDimension: "2d",
                            sampleType : "depth"
                        }
                    });
                    binding++;

                }
            }

            if (this.reflection_map!=null)
            {
                entries.push({                
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    texture:{
                        viewDimension: "cube"
                    }
                });
                binding++;                
            }

            if (this.ambient_light!=null)
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

            if (this.hemisphere_light!=null)
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

            if (this.environment_map!=null)
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

            if (this.probe_grid!=null)
            {
                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                });
                binding++;

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    texture:{
                        viewDimension: "2d",                        
                    }
                });
                binding++;

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    texture:{
                        viewDimension: "2d",                        
                    }
                });
                binding++;
            }

            if (this.lod_probe_grid!=null)
            {
                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                });
                binding++;

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    texture:{
                        viewDimension: "2d",                        
                    }
                });
                binding++;

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    texture:{
                        viewDimension: "2d",                        
                    }
                });
                binding++;

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "read-only-storage"
                    }
                });
                binding++;

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "read-only-storage"
                    }
                });
                binding++;

            }

            if (this.fog!=null)
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

            engine_ctx.cache.bindGroupLayouts.lights[signature] = engine_ctx.device.createBindGroupLayout({ entries });
        }   
        
        let entries = [];
        let binding = 0;

        if (options.has_shadow)
        {
            entries.push({
                binding: 0,
                resource: this.sampler
            });
            binding++;
        }
       
        for (let light of this.directional_lights)
        {
            entries.push({
                binding,
                resource:{
                    buffer: light.constant
                }
            });
            binding++;

            if (light.shadow!=null)
            {
                entries.push({
                    binding,
                    resource:{
                        buffer: light.shadow.constant
                    }
                });
                binding++;

                entries.push({
                    binding,
                    resource: light.shadow.lightTexView
                });
                binding++;
            }
        }

        if (this.reflection_map!=null)
        {
            entries.push({                
                binding,
                resource: this.reflection_map.texture.createView({
                    dimension: 'cube'
                })
            });
            binding++;
        }

        if (this.ambient_light!=null)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.ambient_light.constant
                }
            });
            binding++;
        }

        if (this.hemisphere_light!=null)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.hemisphere_light.constant
                }
            });
            binding++;
        }

        if (this.environment_map!=null)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.environment_map.constant
                }
            });
            binding++;
        }

        if (this.probe_grid!=null)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.probe_grid.constant
                }
            });
            binding++;

            entries.push({                
                binding,
                resource: this.probe_grid.tex_irradiance.createView()
            });
            binding++;

            entries.push({                
                binding,
                resource: this.probe_grid.tex_visibility.createView()
            });
            binding++;
        }

        if (this.lod_probe_grid!=null)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.lod_probe_grid.constant
                }
            });
            binding++;

            entries.push({
                binding,
                resource: this.lod_probe_grid.tex_irradiance.createView()
            });
            binding++;

            entries.push({
                binding,
                resource: this.lod_probe_grid.tex_visibility.createView()
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: this.lod_probe_grid.pos_lod_buf
                }
            });
            binding++;

            entries.push({
                binding,
                resource:{
                    buffer: this.lod_probe_grid.sub_index_buf
                }
            });
            binding++;
        }

        if (this.fog!=null)
        {
            entries.push({
                binding,
                resource:{
                    buffer: this.fog.constant
                }
            });
            binding++;
        }

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.lights[signature];
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });

        if (this.fog!=null)
        {
            this.update_bind_group_fog();
        }
    }

    update_bind_group_fog()
    {
        let has_indirect = this.ambient_light!=null || this.hemisphere_light!=null || this.environment_map!=null;
        if (has_indirect)
        {
            if (!("fog_indirect" in engine_ctx.cache.bindGroupLayouts))
            {
                engine_ctx.cache.bindGroupLayouts.fog_indirect = engine_ctx.device.createBindGroupLayout({
                    entries: [
                        {
                            binding: 0,
                            visibility: GPUShaderStage.FRAGMENT,
                            buffer:{
                                type: "uniform"
                            }
                        }
                    ]
                });
            }

            const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.fog_indirect;

            if (this.ambient_light!=null)
            {
                this.bind_group_fog_indirect = engine_ctx.device.createBindGroup({
                    layout: bindGroupLayout,
                    entries: [
                        {
                            binding: 0,
                            resource:{
                                buffer: this.ambient_light.constant
                            }
                        }
                    ]
                });
            }

            if (this.hemisphere_light!=null)
            {
                this.bind_group_fog_indirect = engine_ctx.device.createBindGroup({
                    layout: bindGroupLayout,
                    entries: [
                        {
                            binding: 0,
                            resource:{
                                buffer: this.hemisphere_light.constant
                            }
                        }
                    ]
                });
            }

            if (this.environment_map!=null)
            {
                this.bind_group_fog_indirect = engine_ctx.device.createBindGroup({
                    layout: bindGroupLayout,
                    entries: [
                        {
                            binding: 0,
                            resource:{
                                buffer: this.environment_map.constant
                            }
                        }
                    ]
                });
            }
        }

        this.bind_group_fog_directional = [];
        for (let light of this.directional_lights)
        {
            let has_shadow = light.shadow!=null;
            let options = { has_shadow };
            let signature = JSON.stringify(options);

            if (!("fog_directional" in engine_ctx.cache.bindGroupLayouts))
            {
                engine_ctx.cache.bindGroupLayouts.fog_directional = {};
            }

            if (!(signature in engine_ctx.cache.bindGroupLayouts.fog_directional))
            {
                let entries = [];
                let binding = 0;

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                });
                binding++;

                if (light.shadow!=null)
                {
                    entries.push({
                        binding,
                        visibility: GPUShaderStage.FRAGMENT,
                        sampler:{
                            type: 'comparison',
                        }
                    });
                    binding++;
                    
                    entries.push({
                        binding,
                        visibility: GPUShaderStage.FRAGMENT,
                        buffer:{
                            type: "uniform"
                        }
                    });
                    binding++;

                    entries.push({
                        binding,
                        visibility: GPUShaderStage.FRAGMENT,
                        texture:{
                            viewDimension: "2d",
                            sampleType : "depth"
                        }
                    });
                    binding++;

                }

                engine_ctx.cache.bindGroupLayouts.fog_directional[signature] = engine_ctx.device.createBindGroupLayout({entries});

            }
            
            const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.fog_directional[signature];

            let entries = [];
            let binding = 0;

            entries.push({
                binding,
                resource:{
                    buffer: light.constant
                }
            });
            binding++;

            if (has_shadow)
            {
                entries.push({
                    binding,
                    resource: this.sampler
                });
                binding++;

                entries.push({
                    binding,
                    resource:{
                        buffer: light.shadow.constant
                    }
                });
                binding++;

                entries.push({
                    binding,
                    resource: light.shadow.lightTexView
                });
                binding++;
            }

            let bind_group = engine_ctx.device.createBindGroup({
                layout: bindGroupLayout,
                entries
            });

            this.bind_group_fog_directional.push(bind_group);
            
        } 

        if (this.probe_grid != null)
        {
            if (!("fog_probe_grid" in engine_ctx.cache.bindGroupLayouts))
            {
                engine_ctx.cache.bindGroupLayouts.fog_probe_grid = engine_ctx.device.createBindGroupLayout({
                    entries: [
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
                            buffer:{
                                type: "read-only-storage"
                            }
                        },
                        {
                            binding: 2,
                            visibility: GPUShaderStage.FRAGMENT,
                            sampler:{}
                        },
                        {
                            binding: 3,
                            visibility: GPUShaderStage.FRAGMENT,
                            texture:{
                                viewDimension: "2d",                        
                            }
                        }

                    ]
                });                
            }

            const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.fog_probe_grid;
            this.bind_group_fog_indirect = engine_ctx.device.createBindGroup({
                layout: bindGroupLayout,
                entries: [
                    {
                        binding: 0,
                        resource:{
                            buffer: this.probe_grid.constant
                        }
                    },
                    {
                        binding: 1,
                        resource:{
                            buffer: this.probe_grid.probe_buf0
                        }
                    },
                    {
                        binding: 2,
                        resource: this.probe_grid.sampler
                    },
                    {                
                        binding: 3,
                        resource: this.probe_grid.tex_visibility.createView()
                    }
                ]
            });
        }

        if (this.lod_probe_grid != null)
        {
            if (!("fog_lod_probe_grid" in engine_ctx.cache.bindGroupLayouts))
            {
                engine_ctx.cache.bindGroupLayouts.fog_lod_probe_grid = engine_ctx.device.createBindGroupLayout({
                    entries: [
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
                            buffer:{
                                type: "read-only-storage"
                            }
                        },
                        {
                            binding: 2,
                            visibility: GPUShaderStage.FRAGMENT,
                            buffer:{
                                type: "read-only-storage"
                            }
                        },
                        {
                            binding: 3,
                            visibility: GPUShaderStage.FRAGMENT,
                            buffer:{
                                type: "read-only-storage"
                            }
                        },
                        {
                            binding: 4,
                            visibility: GPUShaderStage.FRAGMENT,
                            sampler:{}
                        },
                        {
                            binding: 5,
                            visibility: GPUShaderStage.FRAGMENT,
                            texture:{
                                viewDimension: "2d",                        
                            }
                        },
                        
                    ]
                }); 
            }

            const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.fog_lod_probe_grid;
            this.bind_group_fog_indirect = engine_ctx.device.createBindGroup({
                layout: bindGroupLayout,
                entries: [
                    {
                        binding: 0,
                        resource:{
                            buffer: this.lod_probe_grid.constant
                        }
                    },
                    {
                        binding: 1,
                        resource:{
                            buffer: this.lod_probe_grid.pos_lod_buf
                        }
                    },
                    {
                        binding: 2,
                        resource:{
                            buffer: this.lod_probe_grid.probe_buf0
                        }
                    },
                    {
                        binding: 3,
                        resource:{
                            buffer: this.lod_probe_grid.sub_index_buf
                        }
                    },
                    {
                        binding: 4,
                        resource: this.lod_probe_grid.sampler
                    },
                    {                
                        binding: 5,
                        resource: this.lod_probe_grid.tex_visibility.createView()
                    },
                ]
            });

        }

        
    }
}



