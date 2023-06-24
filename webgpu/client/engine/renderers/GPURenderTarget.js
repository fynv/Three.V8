import * as MathUtils from '../math/MathUtils.js';

export class GPURenderTarget
{
    constructor(canvas_ctx = null, msaa = true)
    {
        this.uuid = MathUtils.generateUUID();
        this.canvas_ctx = canvas_ctx;
        this.msaa = msaa;        
        this.width = -1;
        this.height = -1;
        this.view_format = 'rgba8unorm-srgb';
        this.view_video = null;
        this.view_msaa = null;
        this.view_depth = null;
        this.oit_tex0 = null;
        this.oit_view0 = null;
        this.oit_tex1 = null;
        this.oit_view1 = null;
        this.bind_group_oit = null;
        this.bind_group_depth = null;
    }

    update(width = -1, height = -1)
    {
        if (this.canvas_ctx!=null)
        {
            width = this.canvas_ctx.canvas.width;
            height = this.canvas_ctx.canvas.height;
            this.view_format =  this.canvas_ctx.view_format;

            let colorTexture = this.canvas_ctx.context.getCurrentTexture();
            this.view_video =  colorTexture.createView({ format: this.view_format});
        }

        if (this.view_video == null || width!=this.width || height!=this.height)
        {
            if (this.canvas_ctx ==null)
            {
                this.tex_video = engine_ctx.device.createTexture({
                    size: { width, height},
                    dimension: "2d",
                    format: this.view_format,
                    usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_SRC
                });
                
                this.view_video =  this.tex_video.createView();
            }

            if (this.msaa)
            {
                this.tex_msaa = engine_ctx.device.createTexture({
                    size: [width, height],
                    dimension: "2d",
                    sampleCount: 4,
                    format: this.view_format,
                    usage: GPUTextureUsage.RENDER_ATTACHMENT                    
                });

                this.view_msaa = this.tex_msaa.createView();

                this.tex_depth = engine_ctx.device.createTexture({
                    size: [width, height],
                    dimension: "2d",
                    sampleCount: 4,
                    format: 'depth32float',
                    usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING
                });                
            }
            else
            {
                this.tex_depth = engine_ctx.device.createTexture({
                    size: [width, height],
                    dimension: "2d",
                    format: 'depth32float',
                    usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING
                });
            }
            this.view_depth = this.tex_depth.createView();

            if (!("depth" in engine_ctx.cache.bindGroupLayouts))
            {
                engine_ctx.cache.bindGroupLayouts.depth = {};
            }

            let options = { msaa: this.msaa  };
            let signature = JSON.stringify(options);

            if (!(signature in engine_ctx.cache.bindGroupLayouts.depth))
            {
                engine_ctx.cache.bindGroupLayouts.depth[signature] = engine_ctx.device.createBindGroupLayout({
                    entries: [
                        {
                            binding: 0,
                            visibility: GPUShaderStage.FRAGMENT,
                            texture:{
                                viewDimension: "2d",
                                multisampled: this.msaa,
                                sampleType: "unfilterable-float"
                            }
                        }
                    ]
                });
            }
            const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.depth[signature];
            this.bind_group_depth = engine_ctx.device.createBindGroup({
                layout: bindGroupLayout,
                entries: [
                    {
                        binding: 0,
                        resource: this.view_depth 
                    }                    
                ]
            });

            this.width = width;
            this.height = height;
            this.uuid = MathUtils.generateUUID();
        }
    } 
    
    update_oit_buffers()
    {
        let width = this.width;
        let height = this.height;

        if (this.oit_tex0== null || this.oit_tex0.width!=width || this.oit_tex0.height!=height)
        {
            this.oit_tex0 =  engine_ctx.device.createTexture({
                size: { width, height },
                dimension: '2d',
                sampleCount: this.msaa?4:1,
                format: 'rgba16float',
                usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING
            });
    
            this.oit_tex1 =  engine_ctx.device.createTexture({
                size: { width, height },
                dimension: '2d',
                sampleCount: this.msaa?4:1,
                format: 'r8unorm',
                usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING
            });

            this.oit_view0 =  this.oit_tex0.createView();
            this.oit_view1 =  this.oit_tex1.createView();

            if (!("oit_resolve" in engine_ctx.cache.bindGroupLayouts))
            {
                engine_ctx.cache.bindGroupLayouts.oit_resolve = {};
            }

            let options = { msaa: this.msaa  };
            let signature = JSON.stringify(options);

            if (!(signature in engine_ctx.cache.bindGroupLayouts.oit_resolve))
            {
                engine_ctx.cache.bindGroupLayouts.oit_resolve[signature] = engine_ctx.device.createBindGroupLayout({
                    entries: [
                        {
                            binding: 0,
                            visibility: GPUShaderStage.FRAGMENT,
                            texture:{
                                viewDimension: "2d",
                                multisampled: this.msaa,
                                sampleType: "unfilterable-float"
                            }
                        },
                        {
                            binding: 1,
                            visibility: GPUShaderStage.FRAGMENT,
                            texture:{
                                viewDimension: "2d",
                                multisampled: this.msaa,
                                sampleType: "unfilterable-float"
                            }
                        }
                    ]
                });
            }
           

            const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.oit_resolve[signature];
            this.bind_group_oit = engine_ctx.device.createBindGroup({
                layout: bindGroupLayout,
                entries: [
                    {
                        binding: 0,
                        resource: this.oit_view0 
                    },
                    {
                        binding: 1,
                        resource: this.oit_view1
                    }
                ]
            });

        }
    }
}