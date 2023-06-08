import { Vector2 } from "../math/Vector2.js"
import { Vector3 } from "../math/Vector3.js"
import { Color } from "../math/Color.js"

export class MeshStandardMaterial
{
    constructor()
    {
        this.alphaMode = "Opaque";
        this.alphaCutoff = 0.5;
        this.doubleSided = false;
        this.specular_glossiness = false;

        this.color = new Color(1.0, 1.0, 1.0);
        this.alpha = 1.0;
        this.normalScale = new Vector2(1.0, 1.0);
        this.emissive = new Vector3(0,0,0);
        this.tex_idx_map = -1;
        this.tex_idx_normalMap = -1;
        this.tex_idx_emissiveMap = -1;

        this.metallicFactor = 0.0;
        this.roughnessFactor = 1.0;
        this.tex_idx_metalnessMap = -1;        
        
        this.specular = new Color(1.0, 1.0, 1.0);
        this.glossinessFactor = 0.0;
        this.tex_idx_specularMap = -1;        

        this.constant = engine_ctx.createBuffer0(80, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);

        this.sampler = engine_ctx.device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear',
            mipmapFilter: "linear",
            addressModeU: "repeat",
            addressModeV: "repeat",            
        });

    }

    get_options(tex_list)
    {
        return {
            doubleSided: this.doubleSided,
            specular_glossiness: this.specular_glossiness,
            has_color_texture: this.tex_idx_map >=0 && (this.tex_idx_map in tex_list),
            has_metalness_map: this.tex_idx_metalnessMap >=0 && (this.tex_idx_metalnessMap in tex_list),            
            has_specular_map: this.tex_idx_specularMap >=0 && (this.tex_idx_specularMap in tex_list),            
            has_normal_map: this.tex_idx_normalMap >=0 && (this.tex_idx_normalMap in tex_list),
            has_emissive_map: this.tex_idx_emissiveMap >=0 && (this.tex_idx_emissiveMap in tex_list)
        };
    }

    update_constant()
    {
        const uniform = new Float32Array(20);
        const iuniform = new Int32Array(uniform.buffer);
        uniform[0] = this.color.r;
        uniform[1] = this.color.g;
        uniform[2] = this.color.b;
        uniform[3] = this.alpha;
        uniform[4] = this.emissive.x;
        uniform[5] = this.emissive.y;
        uniform[6] = this.emissive.z;
        uniform[8] = this.specular.r;
        uniform[9] = this.specular.g;
        uniform[10] = this.specular.b;
        uniform[11] = this.glossinessFactor;
        uniform[12] = this.normalScale.x;
        uniform[13] = this.normalScale.y;
        uniform[14] = this.metallicFactor;
        uniform[15] = this.roughnessFactor;
        uniform[16] = this.alphaCutoff;
        iuniform[17] = this.doubleSided?1:0;

        engine_ctx.queue.writeBuffer(this.constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
    }

}

