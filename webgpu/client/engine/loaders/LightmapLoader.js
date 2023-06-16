import { HDRImageLoader } from "./HDRImageLoader.js"
import { ImageLoader } from "./ImageLoader.js"
import { CreateTexture, CreateHDRTexture } from "../renderers/GPUUtils.js"
import { Vector3 } from "../math/Vector3.js";

const shader_rgbm2hdr = `
@group(0) @binding(0)
var<uniform> uRate: f32;

@group(0) @binding(1) 
var uSampler: sampler;

@group(0) @binding(2)
var uTex: texture_2d<f32>;

struct VSOut 
{
    @builtin(position) Position: vec4f,
    @location(0) vUV: vec2f
};

@vertex
fn vs_main(@builtin(vertex_index) vertId: u32) -> VSOut 
{
    var vsOut: VSOut;
    let grid = vec2(f32((vertId<<1)&2), f32(vertId & 2));
    let pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);    
    vsOut.vUV = vec2(grid.x, 1.0 -grid.y);
    vsOut.Position = vec4(pos_proj, 0.0, 1.0);
    return vsOut;
}

@fragment
fn fs_main(@location(0) vUV: vec2f) -> @location(0) vec4f
{
    let rgbm = textureSampleLevel(uTex, uSampler, vUV, 0.0);
    return vec4(rgbm.xyz*rgbm.w*uRate, 1.0);
}
`;

function GetPipelineRGBM2HDR()
{
    if (!("rgbm2hdr" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_rgbm2hdr });

        const vertex = {
            module: shaderModule,
            entryPoint: 'vs_main',
            buffers: []
        };

        const colorState = {
            format: 'rgba16float',
            writeMask: GPUColorWrite.ALL
        };

        const fragment = {
            module: shaderModule,
            entryPoint: 'fs_main',
            targets: [colorState]
        };

        const primitive = {
            frontFace: 'cw',
            cullMode: 'none',
            topology: 'triangle-list'
        };

        const pipelineDesc = {
            layout: 'auto',
    
            vertex,
            fragment,
    
            primitive,
        };

        engine_ctx.cache.pipelines.rgbm2hdr  = engine_ctx.device.createRenderPipeline(pipelineDesc);
    }
    return engine_ctx.cache.pipelines.rgbm2hdr;
}

function RGBM2HDR(texIn, texOut, rate)
{    
    const uniform = new Float32Array(4);
    uniform[0] = rate;
    const constant = engine_ctx.createBuffer(uniform.buffer, GPUBufferUsage.UNIFORM);
    const sampler = engine_ctx.device.createSampler();

    let pipeline = GetPipelineRGBM2HDR();
    let bind_group = engine_ctx.device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
            {
                binding: 0,
                resource:{
                    buffer: constant
                }
            },
            {
                binding: 1,
                resource: sampler
            },
            {
                binding: 2,
                resource: texIn.createView()
            },            
        ],
    });

    let colorAttachment =  {
        view:  texOut.createView(), 
        loadOp: 'load',
        storeOp: 'store'
    };

    let renderPassDesc = {
        colorAttachments: [colorAttachment],            
    }; 
    
    let commandEncoder = engine_ctx.device.createCommandEncoder();

    let passEncoder = commandEncoder.beginRenderPass(renderPassDesc);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, bind_group); 
    passEncoder.setViewport(
        0,
        0,
        texIn.width,
        texIn.height,
        0,
        1
    );
    passEncoder.setScissorRect(
        0,
        0,
        texIn.width,
        texIn.height
    );

    passEncoder.draw(3, 1);
    passEncoder.end();

    let cmdBuf = commandEncoder.finish();
    engine_ctx.queue.submit([cmdBuf]);
}

const shader_image_add = `
struct Range
{
    low: vec4f,
    high: vec4f
};

@group(0) @binding(0)
var<uniform> uRange: Range;

@group(0) @binding(1) 
var uSampler: sampler;

@group(0) @binding(2)
var uTex: texture_2d<f32>;

struct VSOut 
{
    @builtin(position) Position: vec4f,
    @location(0) vUV: vec2f
};

@vertex
fn vs_main(@builtin(vertex_index) vertId: u32) -> VSOut 
{
    var vsOut: VSOut;
    let grid = vec2(f32((vertId<<1)&2), f32(vertId & 2));
    let pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);    
    vsOut.vUV = vec2(grid.x, 1.0 -grid.y);
    vsOut.Position = vec4(pos_proj, 0.0, 1.0);
    return vsOut;
}

@fragment
fn fs_main(@location(0) vUV: vec2f) -> @location(0) vec4f
{
    let rgb = textureSampleLevel(uTex, uSampler, vUV, 0.0).xyz;
    let scaled_rgb = rgb*(uRange.high.xyz- uRange.low.xyz) + uRange.low.xyz;
    return vec4(scaled_rgb, 0.0);
}
`;

function GetPipelineImageAdd()
{
    if (!("imageAdd" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_image_add });

        const vertex = {
            module: shaderModule,
            entryPoint: 'vs_main',
            buffers: []
        };

        const colorState = {
            format: 'rgba16float',
            blend: {
                color: {
                    srcFactor: "one",
                    dstFactor: "one"
                },
                alpha: {
                    srcFactor: "zero",
                    dstFactor: "one"
                }
            },
            writeMask: GPUColorWrite.ALL
        };

        const fragment = {
            module: shaderModule,
            entryPoint: 'fs_main',
            targets: [colorState]
        };

        const primitive = {
            frontFace: 'cw',
            cullMode: 'none',
            topology: 'triangle-list'
        };

        const pipelineDesc = {
            layout: 'auto',
    
            vertex,
            fragment,
    
            primitive,
        };

        engine_ctx.cache.pipelines.imageAdd  = engine_ctx.device.createRenderPipeline(pipelineDesc);
    }
    return engine_ctx.cache.pipelines.imageAdd;
}

class ImageAdd
{
    constructor(image0, range0)
    {
        this.constant = engine_ctx.createBuffer0(32, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);
        this.sampler = engine_ctx.device.createSampler();
        this.pipeline = GetPipelineImageAdd();

        this.texIn = engine_ctx.device.createTexture({
            dimension: '2d',
            size: [image0.width, image0.height],
            format: 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT 
        });

        this.tex_out = engine_ctx.device.createTexture({
            dimension: '2d',
            size: [image0.width, image0.height],
            format: "rgba16float",
            usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING 
        });

        this.view_out = this.tex_out.createView();

        this.bind_group = engine_ctx.device.createBindGroup({
            layout: this.pipeline.getBindGroupLayout(0),
            entries: [
                {
                    binding: 0,
                    resource:{
                        buffer: this.constant
                    }
                },
                {
                    binding: 1,
                    resource: this.sampler
                },
                {
                    binding: 2,
                    resource: this.texIn.createView()
                },            
            ],
        });

        this.addImage(image0, range0, true);
    }

    addImage(image, range, clear = false)
    {
        engine_ctx.device.queue.copyExternalImageToTexture(
            { source: image },
            { texture: this.texIn},
            [ image.width, image.height]
        );

        const uniform = new Float32Array(8);
        uniform[0] = range.low.x;
        uniform[1] = range.low.y;
        uniform[2] = range.low.z;
        uniform[4] = range.high.x;
        uniform[5] = range.high.y;
        uniform[6] = range.high.z;
        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);

        let colorAttachment =  {
            view:  this.view_out, 
            clearValue: { r: 0, g: 0, b: 0, a: 1 },
            loadOp: clear? 'clear':'load',
            storeOp: 'store'
        };

        let renderPassDesc = {
            colorAttachments: [colorAttachment],            
        }; 
        
        let commandEncoder = engine_ctx.device.createCommandEncoder();

        let passEncoder = commandEncoder.beginRenderPass(renderPassDesc); 
        passEncoder.setPipeline(this.pipeline);
        passEncoder.setBindGroup(0, this.bind_group); 

        passEncoder.setViewport(
            0,
            0,
            image.width,
            image.height,
            0,
            1
        );
        passEncoder.setScissorRect(
            0,
            0,
            image.width,
            image.height
        );

        passEncoder.draw(3, 1);
        passEncoder.end();

        let cmdBuf = commandEncoder.finish();
        engine_ctx.queue.submit([cmdBuf]);
    }
}


export class LightmapLoader
{
    constructor()
    {


    }

    async fromHDR(url)
    {
        let hdr_loader = new HDRImageLoader();
        let img = await hdr_loader.loadFile(url);
        return CreateHDRTexture(img);
    }

    async fromRGBM(url, rate = 16.0)
    {
        let image_loader = new ImageLoader();
        let rgbm = await image_loader.loadFile(url);        
        let tex_rgbm = CreateTexture(rgbm, false, false);
        let tex_out =  engine_ctx.device.createTexture({
            dimension: '2d',
            size: [rgbm.width, rgbm.height],
            format: "rgba16float",
            usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING 
        });
        RGBM2HDR(tex_rgbm, tex_out, rate);        
        return tex_out;
    }

    async fromImages(url)
    {
        const response = await fetch(url);
        const text = await response.text();

        let image_loader = new ImageLoader();
        let img_add = null;

        let path = url.match(/(.*)[\/\\]/)[1]||'';
        let lines = text.split(/\r?\n/);
        for(let i=0; i< lines.length; i++)
        {
            let line = lines[i];
            let fields = line.split(",");
            if (fields.length<7) continue;
            let fn_img = fields[0];
            let low = new Vector3(parseFloat(fields[1]), parseFloat(fields[2]), parseFloat(fields[3]));
            let high = new Vector3(parseFloat(fields[4]), parseFloat(fields[5]), parseFloat(fields[6]));
            let url_image = path + "/" + fn_img;
            let range = { low, high};

            if (i==0)
            {
                let image0 = await image_loader.loadFile(url_image);                
                img_add = new ImageAdd(image0, range);
            }
            else
            {
                (async()=>{
                    let image = await image_loader.loadFile(url_image);
                    img_add.addImage(image, range);
                })();
            }
        }

        return img_add.tex_out;

    }
    

}
