import { Object3D } from "../core/Object3D.js"
import { UpdateConstant_Model } from "./ModelComponents.js"
import { Vector3 } from "../math/Vector3.js"
import { Vector4 } from "../math/Vector4.js"
import { Matrix4 } from "../math/Matrix4.js"

export class GLTFModel extends Object3D
{
    constructor()
    {
        super();
        this.min_pos = new Vector3(Infinity,Infinity,Infinity);
        this.max_pos = new Vector3(-Infinity, -Infinity, -Infinity);

        this.textures = [];
        this.tex_dict = {};
        this.materials = [];
        this.meshes = [];
        this.mesh_dict = {};
        this.nodes = [];
        this.node_dict = {};
        this.roots = [];
        this.skins = [];
        this.skins_loaded = false;
        this.needUpdateSkinnedMeshes = false;
        this.lightmap = null;
        this.reflector = null;
        this.animations = [];
        this.animation_dict = {};
        this.current_playing = [];

        this.is_meta_ready = false;
        this.meta_resolvers = [];
    }

    async meta_ready()
    {
        return new Promise((resolve, reject) => {
            if (this.is_meta_ready)
            {
                resolve(true);
            }
            else
            {
                this.meta_resolvers.push(resolve);
            }
        });

    }

    set_meta_ready()
    {
        this.is_meta_ready = true;
        for (let resolve of this.meta_resolvers) 
        {
            resolve(true);
        }
    }

    calculate_bounding_box()
    {
        this.min_pos = new Vector3(Infinity,Infinity,Infinity);
        this.max_pos = new Vector3(-Infinity, -Infinity, -Infinity);

        for (let mesh of this.meshes)
        {
            let mesh_min_pos = new Vector3(Infinity,Infinity,Infinity);
            let mesh_max_pos = new Vector3(-Infinity, -Infinity, -Infinity);

            for (let prim of mesh.primitives)
            {
                mesh_min_pos.min(prim.min_pos);
                mesh_max_pos.max(prim.max_pos);
            }            

            if (mesh.node_id>=0 && mesh.skin_id<0)
            {
                let node = this.nodes[mesh.node_id];
                let mesh_mat = node.g_trans;                

                let model_pos = [];
                {
                    let pos = new Vector4(mesh_min_pos.x, mesh_min_pos.y, mesh_min_pos.z, 1.0);                        
                    pos.applyMatrix4(mesh_mat);                    
                    model_pos.push(pos);
                }
                {
                    let pos = new Vector4(mesh_max_pos.x, mesh_min_pos.y, mesh_min_pos.z, 1.0);
                    pos.applyMatrix4(mesh_mat);
                    model_pos.push(pos);
                }
                {
                    let pos = new Vector4(mesh_min_pos.x, mesh_max_pos.y, mesh_min_pos.z, 1.0);
                    pos.applyMatrix4(mesh_mat);
                    model_pos.push(pos);
                }
                {
                    let pos = new Vector4(mesh_max_pos.x, mesh_max_pos.y, mesh_min_pos.z, 1.0);
                    pos.applyMatrix4(mesh_mat);
                    model_pos.push(pos);
                }
                {
                    let pos = new Vector4(mesh_min_pos.x, mesh_min_pos.y, mesh_max_pos.z, 1.0);
                    pos.applyMatrix4(mesh_mat);
                    model_pos.push(pos);
                }
                {
                    let pos = new Vector4(mesh_max_pos.x, mesh_min_pos.y, mesh_max_pos.z, 1.0);
                    pos.applyMatrix4(mesh_mat);
                    model_pos.push(pos);
                }
                {
                    let pos = new Vector4(mesh_min_pos.x, mesh_max_pos.y, mesh_max_pos.z, 1.0);
                    pos.applyMatrix4(mesh_mat);
                    model_pos.push(pos);
                }
                {
                    let pos = new Vector4(mesh_max_pos.x, mesh_max_pos.y, mesh_max_pos.z, 1.0);
                    pos.applyMatrix4(mesh_mat);
                    model_pos.push(pos);
                }

                for (let k=0; k<8; k++)
                {
                    let pos = new Vector3(model_pos[k].x, model_pos[k].y, model_pos[k].z);
                    this.min_pos.min(pos);
                    this.max_pos.max(pos);
                }

            }
            else
            {
                this.min_pos.min(mesh_min_pos);
                this.max_pos.max(mesh_max_pos);
            }           
        }        
    }

    updateMeshConstants()
    {        
        for (let mesh of this.meshes)
        {
            let matrix = this.matrixWorld.clone();
            if (mesh.node_id>=0 && mesh.skin_id<0)
            {                
                let node = this.nodes[mesh.node_id];
                matrix.multiply(node.g_trans);
            }
            UpdateConstant_Model(mesh.constant, matrix);
        }
    }

    updateNodes()
    {       
        let node_queue  = [];
        for (let idx_root of this.roots)
        {
            let node = this.nodes[idx_root];
            node.g_trans.identity();
            node_queue.push(idx_root);
        }

        while(node_queue.length>0)
        {
            let id_node = node_queue.shift();
            let node = this.nodes[id_node];            

            let local = new Matrix4();
            local.compose(node.translation, node.rotation, node.scale);
            node.g_trans.multiply(local);

            for (let id_child of node.children)
            {
                let child = this.nodes[id_child];
                child.g_trans.copy(node.g_trans);
                node_queue.push(id_child);
            }
        }        
    }

    updateSkinMatrices()
    {
        let num_skins = this.skins.length;
        for (let i=0; i < num_skins; i++)
        {
            let skin = this.skins[i];
            let num_joints = skin.joints.length;
            let rela_mats = new Float32Array(16*num_joints);
            for (let j=0; j< num_joints; j++)
            {
                let node = this.nodes[skin.joints[j]];
                let g_trans = node.g_trans;
                let inverse_bind = skin.inverseBindMatrices[j];
                let rela = new Matrix4();
                rela.multiplyMatrices(g_trans, inverse_bind);
                for (let k=0; k<16; k++)
                {
                    rela_mats[j * 16 + k] = rela.elements[k];
                }                
            }
            engine_ctx.queue.writeBuffer(skin.buf_rela_mat, 0, rela_mats.buffer, rela_mats.byteOffset, rela_mats.byteLength);
        }        

        this.needUpdateSkinnedMeshes = num_skins > 0;

    }

    setLightmap(lightmap)
    {
        this.lightmap = lightmap;
        for (let mesh of this.meshes)
        {       
            for (let prim of mesh.primitives)
            {
                prim.create_bind_group(mesh.constant, this.materials, this.textures, this.lightmap, this.reflector);
            }
        }
    }

    set_reflector(reflector)
    {
        this.reflector = reflector;
        for (let mesh of this.meshes)
        {       
            for (let prim of mesh.primitives)
            {
                prim.create_bind_group(mesh.constant, this.materials, this.textures, this.lightmap, this.reflector);
            }
        }

    }

    setAnimationFrame(frame, no_pending = false)
    {
        let resolved = true;

        if (!no_pending)
        {
            do
            {
                for (let morph of frame.morphs)
                {
                    if (!(morph.name in this.mesh_dict))
                    {
                        resolved = false;
                        break;
                    }
                }
                if (!resolved) break;

                for (let trans of frame.translations)
                {
                    if (!(trans.name in this.node_dict))
                    {
                        resolved = false;
                        break;
                    }
                }
                if (!resolved) break;

                for (let rot of frame.rotations)
                {
                    if (!(rot.name in this.node_dict))
                    {
                        resolved = false;
                        break;
                    }
                }
                if (!resolved) break;

                for (let scale of frame.scales)
                {
                    if (!(scale.name in this.node_dict))
                    {
                        resolved = false;
                        break;
                    }
                }
            }
            while(false);

            if (!resolved)        
            {
                this.pending_frame = frame;
                return;
            }
        }

        for (let morph of frame.morphs)
        {
            if (morph.name in this.mesh_dict)
            {
                let mesh_idx = this.mesh_dict[morph.name];
                let mesh = this.meshes[mesh_idx];
                mesh.weights = [...morph.weights];
                mesh.needUpdateMorphTargets = true;
            }
        }

        let need_update_nodes = false;

        for (let trans of frame.translations)
        {
            if (trans.name in this.node_dict)
            {
                let node_idx = this.node_dict[trans.name];
                let node = this.nodes[node_idx];
                node.translation.copy(trans.translation);
                need_update_nodes = true;
            }
        }

        for (let rot of frame.rotations)
        {
            if (rot.name in this.node_dict)
            {
                let node_idx = this.node_dict[rot.name];
                let node = this.nodes[node_idx];
                node.rotation.copy(rot.rotation);
                need_update_nodes = true;
            }
        }

        for (let scale of frame.scales)
        {
            if (scale.name in this.node_dict)
            {
                let node_idx = this.node_dict[scale.name];
                let node = this.nodes[node_idx];
                node.scale.copy(scale.scale);
                need_update_nodes = true;
            }
        }

        if (need_update_nodes)
        {
            this.updateNodes();
            if (this.skins_loaded)
            {
                this.updateSkinMatrices();
            }
        }
    }

    addAnimation(anim)
    {
        this.animations.push(anim);
        this.animation_dict[anim.name] = this.animations.length - 1;
    }

    addAnimations(anims)
    {
        for (let anim of anims)
        {
            this.addAnimation(anims);
        }
    }

    playAnimation(name)
    {
        for (let playback of this.current_playing)
        {
            if (playback.name == name)
            {
                playback.time_start = Date.now();
                return;
            }
        }
        this.current_playing.push({ name, time_start: Date.now()})
    }

    stopAnimation(name)
    {
        for (let i =0; i<this.current_playing.length; i++)
        {
            let playback = this.current_playing[i];        
            if (playback.name == name)
            {
                this.current_playing.splice(i,1);
                return;
            }
        }        
    }

    updateAnimation()
    {      
        let t = Date.now();
        for (let playback of this.current_playing)
        {
            if (!(playback.name in this.animation_dict)) continue;
            let id_anim = this.animation_dict[playback.name];
            let anim = this.animations[id_anim];
            let duration = anim.duration * 1000.0;
            let x = 0;
            if (duration >0)
            {
                while(t-playback.time_start>=duration)
                {
                    playback.time_start += duration;
                }
                x= t - playback.time_start;
            }
            let frame = anim.get_frame(x*0.001);
            
            this.setAnimationFrame(frame);
        }
    }

}


