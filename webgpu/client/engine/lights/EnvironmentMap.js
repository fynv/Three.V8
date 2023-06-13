import {IndirectLight} from "./IndirectLight.js"

export class EnvironmentMap extends IndirectLight
{
    constructor()
    {
        super();
        this.shCoefficients = new Array(9);
        for (let i=0;i<9;i++)
        {
            this.shCoefficients[i] = [0,0,0];
        }
        this.constant = engine_ctx.createBuffer0(176, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);  
    }

    updateConstant()
    {
        const uniform = new Float32Array(44);

        for (let i=0;i<9;i++)
        {
            uniform[i*4] = this.shCoefficients[i][0];
            uniform[i*4 + 1] = this.shCoefficients[i][1];
            uniform[i*4 + 2] = this.shCoefficients[i][2];
            uniform[i*4 + 3] = 0.0;
        }

        uniform[37] = this.diffuse_thresh;
        uniform[38] = this.diffuse_high;
        uniform[39] = this.diffuse_low;
        uniform[40] = this.specular_thresh;
        uniform[41] = this.specular_high;
        uniform[42] = this.specular_low;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }

}

