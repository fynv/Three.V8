import { coeffs } from "./filter_coeffs.js"

export class EnvironmentMapCreator
{
    constructor()
    {        
        this.buf_coeffs = engine_ctx.createBuffer(coeffs.buffer, GPUBufferUsage.UNIFORM);          
    }

}

