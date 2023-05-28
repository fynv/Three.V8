export function LightsIsEmpty(options)
{
    return options.directional_lights.length == 0;
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
        return JSON.stringify(obj);
    }

    get_options()
    {
        let directional_lights = [];
        for (let light of this.directional_lights)
        {
            directional_lights.push(light.shadow!=null);
        }
        return { directional_lights };
    }

    clear_lists()
    {
        this.directional_lights = [];
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
            let entries = [
                {
                    binding: 0,
                    visibility: GPUShaderStage.FRAGMENT,
                    sampler:{
                        type: 'comparison',
                    }
                },
            ];

            let binding = 1;
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

            engine_ctx.cache.bindGroupLayouts.lights[signature] = engine_ctx.device.createBindGroupLayout({ entries });
        }               

        let entries = [
            {
                binding: 0,
                resource: this.sampler
            }
        ];

        let binding = 1;
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

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.lights[signature];
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });
       
    }
}



