import {Color} from "../math/Color.js"
import * as MathUtils from '../math/MathUtils.js';
import { FlipCubemap } from "../renderers/routines/FlipCubemap.js"

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

        let cubemap_in = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [imgs[0].width, imgs[0].height, 6],
            format: 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
        });        

        this.cubemap = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [imgs[0].width, imgs[0].height, 6],
            format: 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.STORAGE_BINDING
        });

        for (let i=0; i<6; i++)
        {
            let imageBitmap = imgs[i];
            engine_ctx.device.queue.copyExternalImageToTexture(
                { source: imageBitmap },
                { texture: cubemap_in, origin: [0, 0, i] },
                [imageBitmap.width, imageBitmap.height]
            );
        }

        FlipCubemap(cubemap_in, this.cubemap);

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

