import { Object3D } from "../core/Object3D.js"
import { UpdateConstant_Model } from "./ModelComponents.js"
import { Matrix4 } from "../math/Matrix4.js"

export class GLTFModel extends Object3D
{
    constructor()
    {
        super();
        this.textures = [];
        this.tex_dict = {};
        this.materials = [];
        this.meshes = [];
        this.mesh_dict = {};
        this.nodes = [];
        this.node_dict = {};
        this.roots = [];
        this.skins = [];
        this.animations = [];
        this.animation_dict = {};
        this.current_playing = [];
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

    setAnimationFrame(frame, no_pending = false)
    {
        let resolved = true;

        if (!no_pending)
        {
            for (let morph of frame.morphs)
            {
                if (!(morph.name in this.mesh_dict))
                {
                    resolved = false;
                    break;
                }
            }

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
                mesh.weights = morph.weights;
                mesh.needUpdateMorphTargets = true;
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

    playAnimation(name, no_pending = false)
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


