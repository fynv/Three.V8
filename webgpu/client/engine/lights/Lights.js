export function LightsIsEmpty(options)
{
    return options.directional_lights.length == 0 && !options.has_reflection_map 
        && !options.has_ambient_light && !options.has_hemisphere_light && !options.has_environment_map && !options.has_probe_grid;
}

export class Lights
{
    constructor()
    {        
        this.clear_lists();
        this.signature = this.get_signature();
        this.bind_group = null;     
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
        return { has_shadow, directional_lights, has_reflection_map, has_ambient_light, has_hemisphere_light, has_environment_map, has_probe_grid};
    }

    clear_lists()
    {
        this.directional_lights = [];
        this.reflection_map = null;
        this.ambient_light = null;
        this.hemisphere_light = null;
        this.environment_map = null;
        this.probe_grid = null;
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

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.lights[signature];
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });
       
    }
}



