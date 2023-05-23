export class GPURenderTarget
{
    constructor(canvas_ctx = null, msaa = true)
    {
        this.canvas_ctx = canvas_ctx;
        this.msaa = msaa;        
        this.width = -1;
        this.height = -1;
        this.view_format = 'rgba8unorm-srgb';
        this.view_video = null;
        this.view_msaa = null;
        this.view_depth = null;
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

            this.width = width;
            this.height = height;
        }
    }    
}