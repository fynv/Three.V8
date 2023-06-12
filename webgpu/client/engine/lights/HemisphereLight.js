import {Color} from "../math/Color.js"
import {IndirectLight} from "./IndirectLight.js"


export class HemisphereLight extends IndirectLight
{
    constructor()
    {
        super();
        this.skyColor = new Color(0.318, 0.318, 0.318 );
        this.groundColor = new Color(0.01, 0.025, 0.025);
        this.intensity = 1.0;

        if (!("hemisphere_light" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.hemisphere_light = engine_ctx.device.createBindGroupLayout({
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

        this.constant = engine_ctx.createBuffer0(64, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.hemisphere_light;
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
        const uniform = new Float32Array(16);
        uniform[0] = this.skyColor.r * this.intensity;
        uniform[1] = this.skyColor.g * this.intensity;
        uniform[2] = this.skyColor.b * this.intensity;
        uniform[3] = 1.0;

        uniform[4] = this.groundColor.r * this.intensity;
        uniform[5] = this.groundColor.g * this.intensity;
        uniform[6] = this.groundColor.b * this.intensity;
        uniform[7] = 1.0;

        uniform[8] = this.diffuse_thresh;
        uniform[9] = this.diffuse_high;
        uniform[10] = this.diffuse_low;
        uniform[11] = this.specular_thresh;
        uniform[12] = this.specular_high;
        uniform[13] = this.specular_low;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);

    }


}