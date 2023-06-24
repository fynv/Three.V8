import {Color} from "../math/Color.js"
import * as MathUtils from '../math/MathUtils.js';

export class Fog
{
    constructor()
    {
        this.uuid = MathUtils.generateUUID();
        this.color = new Color(1.0, 1.0, 1.0);
        this.density = 0.1;
        this.max_num_steps = 50;
        this.min_step = 0.15;        
        this.constant = engine_ctx.createBuffer0(32, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        if (!("fog" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.fog = engine_ctx.device.createBindGroupLayout({
                entries: [
                    {
                        binding: 0,
                        visibility: GPUShaderStage.FRAGMENT,
                        buffer:{
                            type: "uniform"
                        }
                    }
                ]
            });
        }

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.fog;
        
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
        const uniform = new Float32Array(8);
        const iuniform = new Int32Array(uniform.buffer);
        uniform[0] = this.color.r;
        uniform[1] = this.color.g;
        uniform[2] = this.color.b;
        uniform[3] = this.density;

        iuniform[4] = this.max_num_steps;
        uniform[5] = this.min_step;       

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }   


}
