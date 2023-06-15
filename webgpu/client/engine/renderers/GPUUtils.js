import { FlipCubemap } from "../renderers/routines/FlipCubemap.js"

export function CreateTexture(image, generate_mipmaps = false, srgb = true)
{
    let mipLevelCount = generate_mipmaps? (Math.floor(Math.log2(Math.max(image.width, image.height))) + 1) : 1;        

    let texture = engine_ctx.device.createTexture({
        dimension: '2d',
        size: [image.width, image.height],
        format: srgb? 'rgba8unorm-srgb': 'rgba8unorm',
        mipLevelCount,
        usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
    });

    let width = image.width;
    let height = image.height;
    let source = image;

    for (let mipLevel =0; mipLevel<mipLevelCount; mipLevel++)
    {
        engine_ctx.device.queue.copyExternalImageToTexture(
            { source },
            { texture, origin: [0, 0], mipLevel},
            [ width, height]
        );

        if (mipLevel < mipLevelCount-1)
        {
            if (width > 1) width = Math.floor(width/2);
            if (height > 1) height = Math.floor(height/2);
            let canvas =  document.createElement("canvas");
            canvas.width = width;
            canvas.height = height;
            let ctx2d = canvas.getContext("2d");
            ctx2d.drawImage(source, 0,0,width,height);
            source = canvas;
        }        
    }

    return texture;
}

export function CreateCubeTexture(images, flip_x = true)
{
    let cubemap;
    if (flip_x)
    {
        let cubemap_in = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [images[0].width, images[0].height, 6],
            format: 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
        });

        cubemap = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [images[0].width, images[0].height, 6],
            format: 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.STORAGE_BINDING
        });

        for (let i=0; i<6; i++)
        {
            let imageBitmap = images[i];
            engine_ctx.device.queue.copyExternalImageToTexture(
                { source: imageBitmap },
                { texture: cubemap_in, origin: [0, 0, i] },
                [imageBitmap.width, imageBitmap.height]
            );
        }

        FlipCubemap(cubemap_in, cubemap);
    }
    else
    {
        cubemap = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [images[0].width, images[0].height, 6],
            format: 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
        });

        for (let i=0; i<6; i++)
        {
            let imageBitmap = images[i];
            engine_ctx.device.queue.copyExternalImageToTexture(
                { source: imageBitmap },
                { texture: cubemap, origin: [0, 0, i] },
                [imageBitmap.width, imageBitmap.height]
            );
        }

    }

    return cubemap;

}

export function CreateHDRTexture(image)
{
    let texture = engine_ctx.device.createTexture({
        dimension: '2d',
        size: [image.width, image.height],
        format: "rgb9e5ufloat",
        usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST
    });

    let width = image.width;
    let height = image.height;
    
    engine_ctx.queue.writeTexture(
        { texture},
        image.array_rgb9e5,
        { bytesPerRow: width*4},
        { width, height}
    );

    return texture;
}


export function CreateHDRCubeTexture(images)
{
    let cubemap = engine_ctx.device.createTexture({
        dimension: '2d',          
        size: [images[0].width, images[0].height, 6],
        format: 'rgb9e5ufloat',
        usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST
    });

    for (let i=0; i<6; i++)
    {
        let image = images[i];
        let width = image.width;
        let height = image.height;
        engine_ctx.queue.writeTexture(
            { texture: cubemap, origin: [0, 0, i]},
            image.array_rgb9e5,
            { bytesPerRow: width*4},
            { width, height}
        );
    }

    return cubemap;
}

