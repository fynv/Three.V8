export class CanvasContext
{
    constructor(canvas)
    {        
        this.canvas = canvas;
        this.context = null;        
        this.view_format = 'rgba8unorm-srgb';
    }

    async initialize()
    {
        if (this.context!=null) return;
        await engine_ctx.initialize();

        this.context = this.canvas.getContext('webgpu');
        const presentationFormat = navigator.gpu.getPreferredCanvasFormat();
        if (presentationFormat == "bgra8unorm")
        {
            this.view_format = 'bgra8unorm-srgb';
        }        
        const canvasConfig = {
            device: engine_ctx.device,
            alphaMode: "opaque",
            format: presentationFormat,
            viewFormats: [this.view_format],
            usage:  GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC
        };
        this.context.configure(canvasConfig);
    }
    
}
