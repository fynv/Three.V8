import {PerspectiveCamera} from "./PerspectiveCamera.js"

export class PerspectiveCameraEx extends PerspectiveCamera
{
    constructor(fov = 50, aspect = 1, near = 0.1, far = 2000, reflector = null)
    {
        super(fov, aspect, near, far);
        this.reflector = reflector;

        const const_size = 4*16 * 4 + 4 * 2 * 4;
        this.constant = engine_ctx.createBuffer0(const_size, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);
        this.constant_scissor = engine_ctx.createBuffer0(16, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        let options = { has_reflector: reflector!=null};
        let signature = JSON.stringify(options);

        if (!("perspective_camera" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.perspective_camera = {};
        }
        
        if (!(signature in engine_ctx.cache.bindGroupLayouts.perspective_camera))
        {
            let entries = [
                {
                    binding: 0,
                    visibility: GPUShaderStage.VERTEX | GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                }
            ];

            if (reflector!=null)
            {
                entries.push({
                    binding: 1,
                    visibility: GPUShaderStage.FRAGMENT,
                    buffer:{
                        type: "uniform"
                    }
                });
            }

            engine_ctx.cache.bindGroupLayouts.perspective_camera[signature] = engine_ctx.device.createBindGroupLayout({ entries });

        }

        
        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.perspective_camera[signature];

        let entries = [
            {
                binding: 0,
                resource:{
                    buffer: this.constant
                }
            }
        ];

        if (reflector!=null)
        {
            entries.push({
                binding: 1,
                resource:{
                    buffer: reflector.constant
                }
            });
        }
        
        this.bind_group = engine_ctx.device.createBindGroup({ layout: bindGroupLayout, entries});
    }

    updateConstant()
    {
        const uniform = new Float32Array(16*4 + 2*4);
        for (let i=0; i<16; i++)
        {
            uniform[i] = this.projectionMatrix.elements[i];
        }
        for (let i=0; i<16; i++)
        {
            uniform[16 + i] = this.matrixWorldInverse.elements[i];
        }
        for (let i=0; i<16; i++)
        {
            uniform[32 + i] = this.projectionMatrixInverse.elements[i];
        }
        for (let i=0; i<16; i++)
        {
            uniform[48 + i] = this.matrixWorld.elements[i];
        }
        for (let i=0; i<4; i++)
        {
            uniform[64 + i] = this.matrixWorld.elements[i+12];
        }

        if( this.scissor!=null)
        {
            uniform[68] = this.scissor.min_proj.x;
            uniform[69] = this.scissor.min_proj.y;
            uniform[70] = this.scissor.max_proj.x;
            uniform[71] = this.scissor.max_proj.y;
        }
        else
        {
            uniform[68] = -1.0;
            uniform[69] = -1.0;
            uniform[70] = 1.0;
            uniform[71] = 1.0;
        }
        
        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);

        if( this.scissor!=null)
        {
            const uniform_scissor = new Int32Array(4);
            uniform_scissor[0] = this.scissor.origin.x;
            uniform_scissor[1] = this.scissor.origin.y;
            uniform_scissor[2] = this.scissor.size.x;
            uniform_scissor[3] = this.scissor.size.y;

            engine_ctx.queue.writeBuffer(this.constant_scissor, 0, uniform_scissor.buffer, uniform_scissor.byteOffset, uniform_scissor.byteLength);
        }
    }
}
