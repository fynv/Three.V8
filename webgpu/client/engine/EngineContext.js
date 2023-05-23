export class EngineContext
{
    constructor()
    {       
        this.device = null;
        this.queue = null;
        window.engine_ctx = this;

        this.cache = {};
        this.cache.bindGroupLayouts = {};
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
        const desc = {
            size: (size + 3) & ~3,
            usage,
            mappedAtCreation: true
        };
        let buffer_out = this.device.createBuffer(desc);
        let buffer_mapped = buffer_out.getMappedRange();
        let view_in = new Uint8Array(buffer_in, offset, size);
        let view_out = new Uint8Array(buffer_mapped);
        view_out.set(view_in);                                        
        buffer_out.unmap();
        return buffer_out;
    }

    
}

