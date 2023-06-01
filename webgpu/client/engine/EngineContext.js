export class EngineContext
{
    constructor()
    {       
        this.device = null;
        this.queue = null;
        window.engine_ctx = this;

        this.cache = {};
        this.cache.bindGroupLayouts = {};
        this.cache.pipelines = {};
    }

    async initialize()
    {
        if (this.device!=null) return;
        const entry = navigator.gpu;
        if (!entry)
        {
            document.getElementById("error").innerHTML = `
<p>Doesn't look like your browser supports WebGPU.</p>
<p>Try upgrading your browser.</p>`;
            return;
        }
        const adapter = await entry.requestAdapter();
        this.device = await adapter.requestDevice();
        this.queue = this.device.queue;
    }

    createBuffer0(size, usage)
    {
        const desc = {
            size: (size + 3) & ~3,
            usage            
        };
        let buffer_out = this.device.createBuffer(desc);
        return buffer_out;
    }

    createBuffer(buffer_in, usage, offset = 0, size = buffer_in.byteLength)
    {
        usage|=GPUBufferUsage.COPY_DST;
        const desc = {
            size: (size + 3) & ~3,
            usage,           
        };
        let buffer_out = this.device.createBuffer(desc);      
        this.queue.writeBuffer(buffer_out, 0, buffer_in, offset, size);

        return buffer_out;
    }

    
}

