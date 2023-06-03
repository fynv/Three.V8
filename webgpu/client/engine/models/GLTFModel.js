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
}


