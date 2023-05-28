import { Matrix4 } from "../math/Matrix4.js"

export class DirectionalLightShadow
{
    constructor(light, map_width, map_height)
    {
        this.light = light;
        this.map_width = map_width;
        this.map_height = map_height;

        const depthTextureDesc = {
            size: [map_width, map_height, 1],
            dimension: '2d',
            format: 'depth32float',
            usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING
        };
        this.lightTex = engine_ctx.device.createTexture(depthTextureDesc);
        this.lightTexView =  this.lightTex.createView();
        
        this.light_radius = 0.0;
        this.bias = 0.001;
        this.force_cull = true;

        this.constant = engine_ctx.createBuffer0(224, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        if (!("directional_light_shadow" in engine_ctx.cache.bindGroupLayouts))
        {
            engine_ctx.cache.bindGroupLayouts.directional_light_shadow = engine_ctx.device.createBindGroupLayout({
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

        const bindGroupLayout = engine_ctx.cache.bindGroupLayouts.directional_light_shadow;
        
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

    setProjection(left, right, bottom, top, near, far)
    {
        this.left = left;
        this.right = right;
        this.bottom = bottom;
        this.top = top;
        this.near = near;
        this.far = far;
        this.light_proj_matrix = new Matrix4();
        this.light_proj_matrix.makeOrthographic(left, right, top, bottom, near, far);
    }

    updateMatrices()
    {
        this.view_matrix = this.light.matrixWorld.clone();
        this.view_matrix.invert();
        let lightScale = new Matrix4();
        lightScale.makeScale(0.5, 0.5, 0.5);
        let lightBias = new Matrix4();
        lightBias.makeTranslation(0.5, 0.5, 0.5);
        this.lightVPSBMatrix = this.view_matrix.clone();
        this.lightVPSBMatrix.premultiply(this.light_proj_matrix);
        this.lightVPSBMatrix.premultiply(lightScale);
        this.lightVPSBMatrix.premultiply(lightBias);
        
    }

    updateConstant()
    {
        this.updateMatrices();

        const uniform = new Float32Array(56);
        for (let i=0; i<16; i++)
        {
            uniform[i] = this.lightVPSBMatrix.elements[i];
        }
        for (let i=0; i<16; i++)
        {
            uniform[16+i] = this.light_proj_matrix.elements[i];
        }
        for (let i=0; i<16; i++)
        {
            uniform[32+i] = this.view_matrix.elements[i];
        }
        uniform[48] = this.left;
        uniform[49] = this.right;
        uniform[50] = this.bottom;
        uniform[51] = this.top;
        uniform[52] = this.near;
        uniform[53] = this.far;
        uniform[54] = this.light_radius;
        uniform[55] = this.bias;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }

}
