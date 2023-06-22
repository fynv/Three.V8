import {Color} from "../math/Color.js"
import {IndirectLight} from "./IndirectLight.js"

export class AmbientLight extends IndirectLight
{
    constructor()
    {
        super();
        this.color = new Color(1.0, 1.0, 1.0);
        this.intensity = 1.0;        
        this.constant = engine_ctx.createBuffer0(48, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);  
    }

    updateConstant()
    {
        const uniform = new Float32Array(12);
        uniform[0] = this.color.r * this.intensity;
        uniform[1] = this.color.g * this.intensity;
        uniform[2] = this.color.b * this.intensity;
        uniform[3] = 1.0;

        uniform[4] = this.diffuse_thresh;
        uniform[5] = this.diffuse_high;
        uniform[6] = this.diffuse_low;
        uniform[7] = this.specular_thresh;
        uniform[8] = this.specular_high;
        uniform[9] = this.specular_low;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);

    }

}
