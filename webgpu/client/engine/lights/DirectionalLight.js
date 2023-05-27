import { Light } from "./Light.js"
import { Vector3 } from "../math/Vector3.js";

export class DirectionalLight extends Light
{
    constructor()
    {
        super();
        this.target = null;
        this.shadow = null;
        this.constant = engine_ctx.createBuffer0(80, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);
    }

    direction()
    {     
        this.updateWorldMatrix(true, false);
        let pos_target = new Vector3(0,0,0);
        if (this.target!=null)
        {
            pos_target.setFromMatrixPosition(this.target.matrixWorld);
        }
        let dir = new Vector3();
        dir.setFromMatrixPosition(this.matrixWorld);
        dir.sub(pos_target);
        dir.normalize();
        return dir;
    }

    lookAtTarget()
    {
        let dir = this.direction();
        if (Math.abs(dir.y)<Math.abs(dir.x))
        {
            if (Math.abs(dir.z)<Math.abs(dir.y))
            {
                this.up.set(0,0,1);
            }
            else
            {
                this.up.set(0,1,0);
            }
        }
        else if (Math.abs(dir.z)<Math.abs(dir.x))
        {
            this.up.set(0,0,1);
        }
        else
        {
            this.up.set(1,0,0);
        }    
        
        let pos_target = new Vector3(0,0,0);
        if (this.target!=null)
        {
            pos_target.setFromMatrixPosition(this.target.matrixWorld);
        }
        this.lookAt(pos_target);
    }

    updateConstant()
    {
        let uniform = new Float32Array(20);        

        let direction = this.direction();
        let position = new Vector3();
        position.setFromMatrixPosition(this.matrixWorld);        

        uniform[0] = this.color.r * this.intensity;
        uniform[1] = this.color.g * this.intensity;
        uniform[2] = this.color.b * this.intensity;
        uniform[3] = 1.0;
        uniform[4] = position.x;
        uniform[5] = position.y;
        uniform[6] = position.z;
        uniform[7] = 1.0;
        uniform[8] = direction.x;
        uniform[9] = direction.y;
        uniform[10] = direction.z;
        uniform[11] = 0.0;        
        uniform[12] = this.diffuse_thresh;
        uniform[13] = this.diffuse_high;
        uniform[14] = this.diffuse_low;
        uniform[15] = this.specular_thresh;
        uniform[16] = this.specular_high;
        uniform[17] = this.specular_low;
        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }


}
