import { Object3D } from "../core/Object3D.js"
import { PerspectiveCameraEx } from "./PerspectiveCameraEx.js";
import { GPURenderTarget } from "../renderers/GPURenderTarget.js"
import { Vector2 } from "../math/Vector2.js";
import { Vector3 } from "../math/Vector3.js";
import { Vector4 } from "../math/Vector4.js";
import { DepthDownsample, DepthDownsampleBundle} from "../renderers/routines/DepthDownsample.js"

function toViewAABB(MV, min_pos, max_pos)
{
    let view_pos = [];
    {
        let pos = new Vector4(min_pos.x, min_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(max_pos.x, min_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(min_pos.x, max_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }
    
    {
        let pos = new Vector4(max_pos.x, max_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(min_pos.x, min_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(max_pos.x, min_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(min_pos.x, max_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }
    
    {
        let pos = new Vector4(max_pos.x, max_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    let min_pos_view = new Vector3(Number.MAX_VALUE, Number.MAX_VALUE, Number.MAX_VALUE);
    let max_pos_view = new Vector3(-Number.MAX_VALUE, -Number.MAX_VALUE, -Number.MAX_VALUE);

    for (let k=0; k<8; k++)
    {
        let pos = view_pos[k];
        if (pos.x < min_pos_view.x) min_pos_view.x = pos.x;
        if (pos.x > max_pos_view.x) max_pos_view.x = pos.x;
        if (pos.y < min_pos_view.y) min_pos_view.y = pos.y;
        if (pos.y > max_pos_view.y) max_pos_view.y = pos.y;
        if (pos.z < min_pos_view.z) min_pos_view.z = pos.z;
        if (pos.z > max_pos_view.z) max_pos_view.z = pos.z;
    }

    return { min_pos_view, max_pos_view };

}

export class Reflector extends Object3D
{
    constructor()
    {
        super();
        this.width = 1.0;
        this.height = 1.0;        
        this.constant = engine_ctx.createBuffer0(64, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);
        this.target = new GPURenderTarget(null, true);
        this.tex_depth_1x = null;
        this.view_depth_1x = null;
        this.resized = false;
        this.camera = new PerspectiveCameraEx(50, 1, 0.1, 2000, this);
    }

    updateConstant()
    {
        let mat_inv = this.matrixWorld.clone();
        mat_inv.invert();

        const uniform = new Float32Array(16);
        for (let i=0; i<16; i++)
        {
            uniform[i] = mat_inv.elements[i];
        }        
        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }

    updateTarget(width, height)
    {
        this.target.update(width, height);
        this.tex_depth_1x = engine_ctx.device.createTexture({
            size: [width, height],
            dimension: "2d",
            format: 'depth32float',
            usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING
        });
        this.view_depth_1x = this.tex_depth_1x.createView();
        this.bundle_depth_downsample = DepthDownsampleBundle(this.target);
    }

    depthDownsample()
    {
        let commandEncoder = engine_ctx.device.createCommandEncoder();

        let depthAttachment = {
            view: this.view_depth_1x,
            depthClearValue: 1,
            depthLoadOp: 'clear',
            depthStoreOp: 'store',
        };

        let renderPassDesc = {
            colorAttachments: [],
            depthStencilAttachment: depthAttachment
        }; 
        let passEncoder = commandEncoder.beginRenderPass(renderPassDesc);

        passEncoder.setViewport(
            0,
            0,
            this.target.width,
            this.target.height,
            0,
            1
        );

        passEncoder.setScissorRect(
            this.camera.scissor.origin.x,
            this.camera.scissor.origin.y,
            this.camera.scissor.size.x,
            this.camera.scissor.size.y,
        );
        
        passEncoder.executeBundles([this.bundle_depth_downsample]);
        passEncoder.end();

        let cmdBuf = commandEncoder.finish();
        engine_ctx.queue.submit([cmdBuf]);
    }

    calc_scissor()
    {
        let origin = new Vector2(0,0);
        let size = new Vector2(0,0);

        let min_pos = new Vector3(-this.width*0.5, -this.height*0.5, 0.0);
        let max_pos = new Vector3(this.width*0.5, this.height*0.5, 0.0);

        let MV = this.camera.matrixWorldInverse.clone();
        MV.multiply(this.matrixWorld);

        let {min_pos_view, max_pos_view }=  toViewAABB(MV, min_pos, max_pos);

        let invP = this.camera.projectionMatrixInverse;
        let view_far = new Vector4(0,0,1,1);
        view_far.applyMatrix4(invP);
        view_far.multiplyScalar(1.0/view_far.w);
        let view_near = new Vector4(0,0,-1,1);
        view_near.applyMatrix4(invP);
        view_near.multiplyScalar(1.0/view_near.w);

        if (min_pos_view.z < view_far.z)
        {
            min_pos_view.z = view_far.z;
        }

        if (max_pos_view.z > view_near.z)
        {
            max_pos_view.z = view_near.z;
        }

        if (min_pos_view.z > max_pos_view.z) return {origin, size};

        let P = this.camera.projectionMatrix;

        let min_pos_proj = new Vector4(min_pos_view.x, min_pos_view.y, min_pos_view.z, 1.0);
        min_pos_proj.applyMatrix4(P);
        min_pos_proj.multiplyScalar(1.0/min_pos_proj.w);

        let max_pos_proj = new Vector4(max_pos_view.x, max_pos_view.y, min_pos_view.z, 1.0);
        max_pos_proj.applyMatrix4(P);
        max_pos_proj.multiplyScalar(1.0/max_pos_proj.w);

        let min_pos_proj2 = new Vector4(min_pos_view.x, min_pos_view.y, max_pos_view.z, 1.0);
        min_pos_proj2.applyMatrix4(P);
        min_pos_proj2.multiplyScalar(1.0/min_pos_proj2.w);
        
        let max_pos_proj2 = new Vector4(max_pos_view.x, max_pos_view.y, max_pos_view.z, 1.0);
        max_pos_proj2.applyMatrix4(P);
        max_pos_proj2.multiplyScalar(1.0/max_pos_proj2.w);

        let min_proj = new Vector2(Math.min( min_pos_proj.x, min_pos_proj2.x), Math.min( min_pos_proj.y, min_pos_proj2.y));
        let max_proj = new Vector2(Math.max( max_pos_proj.x, max_pos_proj2.x), Math.max( max_pos_proj.y, max_pos_proj2.y));

        if (min_proj.x < -1.0) min_proj.x = -1.0
        if (min_proj.y < -1.0) min_proj.y = -1.0;
        if (max_proj.x > 1.0) max_proj.x = 1.0;
        if (max_proj.y > 1.0) max_proj.y = 1.0;

        if (min_proj.x > max_proj.x || min_proj.y > max_proj.y) 
        {
            this.camera.scissor= {min_proj, max_proj, origin, size};
            return;
        }

        let min_screen = new Vector2( (min_proj.x  + 1.0) *0.5 * this.target.width, (min_proj.y  + 1.0) *0.5 * this.target.height);
        let max_screen = new Vector2( (max_proj.x  + 1.0) *0.5 * this.target.width, (max_proj.y  + 1.0) *0.5 * this.target.height);

        origin.x = Math.floor(min_screen.x);
        origin.y = Math.floor(min_screen.y);        

        size.x = Math.ceil(max_screen.x) - origin.x;
        size.y = Math.ceil(max_screen.y) - origin.y;

        origin.y = this.target.height - (origin.y + size.y);

        this.camera.scissor= {min_proj, max_proj, origin, size};


    }

}
