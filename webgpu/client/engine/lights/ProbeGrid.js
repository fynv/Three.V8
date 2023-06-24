import {IndirectLight} from "./IndirectLight.js"
import {Vector3} from "../math/Vector3.js"
import get_module from './IrrPresample.js'

export class ProbeGrid extends IndirectLight
{
    constructor()
    {
        super();
        this.coverage_min = new Vector3(-10.0, 0.0, -10.0);
        this.coverage_max = new Vector3(10.0, 10.0, 10.0);
        this.divisions = new Vector3(10,5,10);
        this.ypower = 1.0;
        this.normalBias = 0.2;
        this.vis_res = 16;
        this.pack_size = 0;
        this.pack_res = 0;
        this.irr_res = 8;
        this.irr_pack_res = 0;

        this.constant = engine_ctx.createBuffer0(112, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        this.probe_data = null;
        this.visibility_data = null;
        this.perPrimitive = false;
        this.tex_irradiance = null;
        this.tex_visibility = null;
        this.probe_buf0 = null;
        this.sampler = engine_ctx.device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear',
        });

    }

    allocate_probes()
    {
        let num_probes =  this.divisions.x * this.divisions.y * this.divisions.z;
        let arr_buf0 = new Float32Array(num_probes*4);
        for (let i=0; i<num_probes; i++)
        {
            arr_buf0[i*4] = this.probe_data[i*36];
            arr_buf0[i*4 + 1] = this.probe_data[i*36 + 1];
            arr_buf0[i*4 + 2] = this.probe_data[i*36 + 2];
            arr_buf0[i*4 + 3] = this.probe_data[i*36 + 3];
        }
        this.probe_buf0 = engine_ctx.createBuffer(arr_buf0.buffer, GPUBufferUsage.STORAGE, 0, num_probes*16);

        let pack_size = Math.floor(Math.ceil(Math.sqrt(num_probes)));
        this.irr_pack_res = pack_size * (this.irr_res + 2);
        this.tex_irradiance = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [this.irr_pack_res, this.irr_pack_res],
            format: 'rgb9e5ufloat',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST
        });
        
        (async() =>{            
            let module = await get_module();
            let p_probe_data = module.ccall("alloc", "number", ["number"], [this.probe_data.byteLength]);            
            module.HEAPU8.set(new Uint8Array(this.probe_data.buffer), p_probe_data);
            let p_u32 = module.ccall("alloc", "number", ["number"], [this.irr_pack_res * this.irr_pack_res * 4]);
            module.ccall("presample_irradiance", null, ["number", "number", "number", "number", "number", "number"], [num_probes, this.irr_res, p_probe_data, p_u32, 9, 0]);

            let arr_u32 = new Uint32Array(this.irr_pack_res * this.irr_pack_res);
            {
                let view_in = new Uint8Array( module.HEAPU8.buffer, p_u32, this.irr_pack_res * this.irr_pack_res * 4);
                let view_out = new Uint8Array(arr_u32.buffer);
                view_out.set(view_in);
            }
            
            module.ccall("dealloc", null, ["number"], [p_probe_data]);
            module.ccall("dealloc", null, ["number"], [p_u32]);

            engine_ctx.queue.writeTexture(
                { texture: this.tex_irradiance },
                arr_u32.buffer,
                { bytesPerRow: this.irr_pack_res*4},
                { width: this.irr_pack_res, height: this.irr_pack_res}
            );
        })();

        this.pack_size = Math.ceil(Math.sqrt(num_probes));
        this.pack_res = this.pack_size * (this.vis_res + 2);

        this.tex_visibility = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [this.pack_res, this.pack_res],
            format: 'rg16float',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST
        });
        
        engine_ctx.queue.writeTexture(
            { texture: this.tex_visibility },
            this.visibility_data.buffer,
            { bytesPerRow: this.pack_res*4},
            { width: this.pack_res, height: this.pack_res}
        );

    }

    updateConstant()
    {
        const uniform = new Float32Array(28);
        const iuniform = new Int32Array(uniform.buffer);
        uniform[0] = this.coverage_min.x;
        uniform[1] = this.coverage_min.y;
        uniform[2] = this.coverage_min.z;
        uniform[4] = this.coverage_max.x;
        uniform[5] = this.coverage_max.y;
        uniform[6] = this.coverage_max.z;
        iuniform[8] = this.divisions.x;
        iuniform[9] = this.divisions.y;
        iuniform[10] = this.divisions.z;
        uniform[12] = this.ypower;
        uniform[13] = this.normalBias;
        iuniform[14] = this.vis_res;
        iuniform[15] = this.pack_size;
        iuniform[16] = this.pack_res;
        iuniform[17] = this.irr_res;
        iuniform[18] = this.irr_pack_res;
        uniform[19] = this.diffuse_thresh;
        uniform[20] = this.diffuse_high;
        uniform[21] = this.diffuse_low;
        uniform[22] = this.specular_thresh;
        uniform[23] = this.specular_high;
        uniform[24] = this.specular_low;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }

}
