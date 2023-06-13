import * as MathUtils from '../math/MathUtils.js';

export class ReflectionMap
{
    constructor()
    {
        this.uuid = MathUtils.generateUUID();
        this.texture =  engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [128, 128, 6],
            format: 'rgba16float',
            mipLevelCount: 7,
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.COPY_SRC
        });
    }
}

export class IndirectLight
{
    constructor()
    {
        this.uuid = MathUtils.generateUUID();
        
        this.diffuse_thresh = 0.2;
		this.diffuse_high = 0.8;
		this.diffuse_low = 0.2;

		this.specular_thresh = 0.2;
		this.specular_high = 0.8;
		this.specular_low = 0.2;

        this.reflection = null;
    }
}

