export function LightsIsEmpty(options)
{
    return options.directional_lights.length == 0;
}

let Poisson32_64 = new Float32Array([
    -0.975402, -0.0711386,
    -0.920347, -0.41142,
    -0.883908, 0.217872,
    -0.884518, 0.568041,
    -0.811945, 0.90521,
    -0.792474, -0.779962,
    -0.614856, 0.386578,
    -0.580859, -0.208777,
    -0.53795, 0.716666,
    -0.515427, 0.0899991,
    -0.454634, -0.707938,
    -0.420942, 0.991272,
    -0.261147, 0.588488,
    -0.211219, 0.114841,
    -0.146336, -0.259194,
    -0.139439, -0.888668,
    0.0116886, 0.326395,
    0.0380566, 0.625477,
    0.0625935, -0.50853,
    0.125584, 0.0469069,
    0.169469, -0.997253,
    0.320597, 0.291055,
    0.359172, -0.633717,
    0.435713, -0.250832,
    0.507797, -0.916562,
    0.545763, 0.730216,
    0.56859, 0.11655,
    0.743156, -0.505173,
    0.736442, -0.189734,
    0.843562, 0.357036,
    0.865413, 0.763726,
    0.872005, -0.927,

    -0.934812, 0.366741,
    -0.918943, -0.0941496,
    -0.873226, 0.62389,
    -0.8352, 0.937803,
    -0.822138, -0.281655,
    -0.812983, 0.10416,
    -0.786126, -0.767632,
    -0.739494, -0.535813,
    -0.681692, 0.284707,
    -0.61742, -0.234535,
    -0.601184, 0.562426,
    -0.607105, 0.847591,
    -0.581835, -0.00485244,
    -0.554247, -0.771111,
    -0.483383, -0.976928,
    -0.476669, -0.395672,
    -0.439802, 0.362407,
    -0.409772, -0.175695,
    -0.367534, 0.102451,
    -0.35313, 0.58153,
    -0.341594, -0.737541,
    -0.275979, 0.981567,
    -0.230811, 0.305094,
    -0.221656, 0.751152,
    -0.214393, -0.0592364,
    -0.204932, -0.483566,
    -0.183569, -0.266274,
    -0.123936, -0.754448,
    -0.0859096, 0.118625,
    -0.0610675, 0.460555,
    -0.0234687, -0.962523,
    -0.00485244, -0.373394,
    0.0213324, 0.760247,
    0.0359813, -0.0834071,
    0.0877407, -0.730766,
    0.14597, 0.281045,
    0.18186, -0.529649,
    0.188208, -0.289529,
    0.212928, 0.063509,
    0.23661, 0.566027,
    0.266579, 0.867061,
    0.320597, -0.883358,
    0.353557, 0.322733,
    0.404157, -0.651479,
    0.410443, -0.413068,
    0.413556, 0.123325,
    0.46556, -0.176183,
    0.49266, 0.55388,
    0.506333, 0.876888,
    0.535875, -0.885556,
    0.615894, 0.0703452,
    0.637135, -0.637623,
    0.677236, -0.174291,
    0.67626, 0.7116,
    0.686331, -0.389935,
    0.691031, 0.330729,
    0.715629, 0.999939,
    0.8493, -0.0485549,
    0.863582, -0.85229,
    0.890622, 0.850581,
    0.898068, 0.633778,
    0.92053, -0.355693,
    0.933348, -0.62981,
    0.95294, 0.156896
]);

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

        this.constant_poisson = engine_ctx.createBuffer(Poisson32_64.buffer, GPUBufferUsage.UNIFORM, 0, (32+64)*2*4);
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
        return { has_shadow, directional_lights };
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

                entries.push({
                    binding,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
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

            entries.push({
                binding,
                resource:{
                    buffer: this.constant_poisson
                }
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

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.lights[signature];
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });
       
    }
}



