import {Color} from "../math/Color.js"
import * as MathUtils from '../math/MathUtils.js';
import { CreateCubeTexture } from "../renderers/GPUUtils.js"


export class Background
{
    constructor()
    {
        this.uuid = MathUtils.generateUUID();
    }
}

export class ColorBackground extends Background
{
    constructor()
    {
        super();
        this.color = new Color(1.0, 1.0, 1.0);
    }
}

export class HemisphereBackground extends Background
{
    constructor()
    {
        super();
        this.skyColor = new Color( 0.318, 0.318, 0.318 );
        this.groundColor = new Color( 0.01, 0.025, 0.025);

        if (!("hemisphere_background" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.hemisphere_background = engine_ctx.device.createBindGroupLayout({
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

        this.constant = engine_ctx.createBuffer0(32, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.hemisphere_background;
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries: [
                {
                    binding: 0,
                    resource:{
                        buffer: this.constant
                    }
                }
            ]
        });
    }

    updateConstant()
    {
        const uniform = new Float32Array(8);
        uniform[0] = this.skyColor.r;
        uniform[1] = this.skyColor.g;
        uniform[2] = this.skyColor.b;
        uniform[3] = 1.0;

        uniform[4] = this.groundColor.r;
        uniform[5] = this.groundColor.g;
        uniform[6] = this.groundColor.b;
        uniform[7] = 1.0;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);

    }
}

export class CubeBackground extends Background
{
    constructor()
    {
        super();
        this.cubemap = null;
        this.bind_group = null;

        if (!("cube_background" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.cube_background = engine_ctx.device.createBindGroupLayout({
                entries: [
                    {
                        binding: 0,
                        visibility: GPUShaderStage.FRAGMENT,
                        sampler:{}
                    },
                    {
                        binding: 1,
                        visibility: GPUShaderStage.FRAGMENT,
                        texture:{
                            viewDimension: "cube"
                        }
                    }
                ]
            });
        }

        this.sampler = engine_ctx.device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear',
        });
    }

    setCubemap(imgs)
    {
        this.uuid = MathUtils.generateUUID();
        this.cubemap = CreateCubeTexture(imgs);
        
        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.cube_background;
        this.bind_group = engine_ctx.device.createBindGroup({
            layout: bindGroupLayout,
            entries: [               
                {
                    binding: 0,
                    resource: this.sampler
                },
                {
                    binding: 1,
                    resource: this.cubemap.createView({
                        dimension: 'cube'
                    })
                }
            ]
        });

    }

}

