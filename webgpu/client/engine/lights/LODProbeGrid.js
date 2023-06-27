import {IndirectLight} from "./IndirectLight.js"
import {Vector3} from "../math/Vector3.js"
import get_module from './IrrPresample.js'

export class LODProbeGrid extends IndirectLight
{
    constructor()
    {
        super();
        this.coverage_min = new Vector3(-10.0, 0.0, -10.0);
        this.coverage_max = new Vector3(10.0, 10.0, 10.0);
        this.base_divisions = new Vector3(10,5,10);
        this.sub_division_level = 2;
        this.normalBias = 0.2;
        this.vis_res = 16;
        this.pack_size = 0;
        this.pack_res = 0;
        this.irr_res = 8;
        this.irr_pack_res = 0;

        this.constant = engine_ctx.createBuffer0(112, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        this.sub_index = null;
        this.probe_data = null;
        this.visibility_data = null;
        this.perPrimitive = false;
        this.sub_index_buf = null;
        this.tex_irradiance = null;
        this.tex_visibility = null;
        this.pos_lod_buf = null;
        this.probe_buf0 = null;

        this.sampler = engine_ctx.device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear',
        });
    }

    getNumberOfProbes()
    {
        return this.probe_data.length/40;

    }

    allocate_probes()
    {        
        this.sub_index_buf = engine_ctx.createBuffer(this.sub_index.buffer, GPUBufferUsage.STORAGE, 0, this.sub_index.byteLength);
        let num_probes = this.getNumberOfProbes();       
        let arr_buf_pos_lod =  new Float32Array(num_probes*4);
        let arr_buf0 = new Float32Array(num_probes*4);
        for (let i=0; i<num_probes; i++)
        {
            arr_buf_pos_lod[i*4] = this.probe_data[i*40];
            arr_buf_pos_lod[i*4 + 1] = this.probe_data[i*40 + 1];
            arr_buf_pos_lod[i*4 + 2] = this.probe_data[i*40 + 2];
            arr_buf_pos_lod[i*4 + 3] = this.probe_data[i*40 + 3];
            arr_buf0[i*4] = this.probe_data[i*40 + 4];
            arr_buf0[i*4 + 1] = this.probe_data[i*40 + 5];
            arr_buf0[i*4 + 2] = this.probe_data[i*40 + 6];
            arr_buf0[i*4 + 3] = this.probe_data[i*40 + 7];
        }        
        this.pos_lod_buf = engine_ctx.createBuffer(arr_buf_pos_lod.buffer, GPUBufferUsage.STORAGE, 0, num_probes*16);
        this.probe_buf0 = engine_ctx.createBuffer(arr_buf0.buffer, GPUBufferUsage.STORAGE, 0, num_probes*16);
        
        this.pack_size = Math.floor(Math.ceil(Math.sqrt(num_probes)));
        this.pack_res = this.pack_size * (this.vis_res + 2);
        this.irr_pack_res = this.pack_size * (this.irr_res + 2);
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
            module.ccall("presample_irradiance", null, ["number", "number", "number", "number", "number", "number"], [num_probes, this.irr_res, p_probe_data, p_u32, 10, 1]);

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
        iuniform[8] = this.base_divisions.x;
        iuniform[9] = this.base_divisions.y;
        iuniform[10] = this.base_divisions.z;
        iuniform[12] = this.sub_division_level;
        uniform[13] = this.normalBias;
        iuniform[14] = this.getNumberOfProbes();
        iuniform[15] = this.vis_res;
        iuniform[16] = this.pack_size;
        iuniform[17] = this.pack_res;
        iuniform[18] = this.irr_res;
        iuniform[19] = this.irr_pack_res;
        uniform[20] = this.diffuse_thresh;
        uniform[21] = this.diffuse_high;
        uniform[22] = this.diffuse_low;
        uniform[23] = this.specular_thresh;
        uniform[24] = this.specular_high;
        uniform[25] = this.specular_low;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);

    }

    get_probe_idx(ipos)
    {        
        let base_offset = this.base_divisions.x * this.base_divisions.y * this.base_divisions.z;
        
        let ipos_base = ipos.clone();
        ipos_base.multiplyScalar(1<<this.sub_division_level);
        ipos_base.floor();
      
        let node_idx = ipos_base.x + (ipos_base.y + ipos_base.z * this.base_divisions.y) * this.base_divisions.x;        
        let probe_idx = this.sub_index[node_idx];        

        let lod = 0;
        let digit_mask = 1 << (this.sub_division_level - 1);

        let num_probes = this.getNumberOfProbes();
        while (lod < this.sub_division_level && probe_idx >= num_probes)
        {
            let offset = base_offset + (probe_idx - num_probes) * 8;
            let sub = 0;
            if ((ipos.x & digit_mask) != 0) sub += 1;
            if ((ipos.y & digit_mask) != 0) sub += 2;
            if ((ipos.z & digit_mask) != 0) sub += 4;
            node_idx = offset + sub;
		    probe_idx = this.sub_index[node_idx];

            lod++;
		    digit_mask >>= 1;
        }

        return probe_idx;
    }

    get_probe(position, envMap)
    {
        let divs = this.base_divisions.clone();
        divs.multiplyScalar(1<<this.sub_division_level);

        let size_grid = new Vector3();
        size_grid.subVectors(this.coverage_max, this.coverage_min);
        let pos_normalized = new Vector3();
        pos_normalized.subVectors(position, this.coverage_min);
        pos_normalized.divide(size_grid);
        let pos_voxel = new Vector3();
        pos_voxel.multiplyVectors(pos_normalized, divs);
        pos_voxel.subScalar(0.5);
        pos_voxel.clamp(new Vector3(0,0,0), new Vector3(divs.x - 1, divs.y - 1, divs.z - 1));
        let i_voxel = pos_voxel.clone(); i_voxel.floor();
        i_voxel.clamp(new Vector3(0,0,0), new Vector3(divs.x - 2, divs.y - 2, divs.z - 2));
        let frac_voxel = new Vector3();
        frac_voxel.subVectors(pos_voxel, i_voxel);

        let sum_weight = 0;
        let sumSH = new Array(9);
        for (let i=0;i<9;i++)
        {
            sumSH[i] = [0,0,0];
        }

        for (let i=0;i<8;i++)
        {
            let x = i & 1;
            let y = (i>>1)&1;
            let z = i>>2;

            let delta = new Vector3();
            delta.subVectors(new Vector3(x,y,z), frac_voxel);
            let wx = 1.0 - Math.abs(delta.x);
            let wy = 1.0 - Math.abs(delta.y);
            let wz = 1.0 - Math.abs(delta.z);
            let weight = wx*wy*wz;
            if (weight > 0)
            {
                sum_weight += weight;
                let vert = new Vector3();
                vert.addVectors(i_voxel, new Vector3(x,y,z));
                let idx_probe = this.get_probe_idx(vert);
                for (let k=0; k<9; k++)
                {
                    sumSH[k][0] += this.probe_data[idx_probe*40 + (k+1)*4]*weight;
                    sumSH[k][1] += this.probe_data[idx_probe*40 + (k+1)*4 + 1]*weight;
                    sumSH[k][2] += this.probe_data[idx_probe*40 + (k+1)*4 + 2]*weight;
                }
            }
        }

        if (sum_weight>0)
        {
            for (let k=0; k<9; k++)
            {
                sumSH[k][0]/=sum_weight;
                sumSH[k][1]/=sum_weight;
                sumSH[k][2]/=sum_weight;
            }
        }            
        envMap.shCoefficients = sumSH;
        envMap.updateConstant();


    }

}

