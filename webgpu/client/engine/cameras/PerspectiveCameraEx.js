import {PerspectiveCamera} from "./PerspectiveCamera.js"

export class PerspectiveCameraEx extends PerspectiveCamera
{
    constructor(fov = 50, aspect = 1, near = 0.1, far = 2000)
    {
        super(fov, aspect, near, far);
        const const_size = 4*16 * 4 + 4 *4;
        this.constant = engine_ctx.createBuffer0(const_size, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        if (!("perspective_camera" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.perspective_camera = engine_ctx.device.createBindGroupLayout({
                entries: [
                    {
                        binding: 0,
                        visibility: GPUShaderStage.VERTEX | GPUShaderStage.FRAGMENT,
                        buffer:{
                            type: "uniform"
                        }
                    }
                ]
            });
        }

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.perspective_camera;
        
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
        const uniform = new Float32Array(16*4 + 4);
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
        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }
}
