import get_module from './BVH.js'
import { SimpleModel } from "../models/SimpleModel.js"
import { GLTFModel } from "../models/GLTFModel.js"

export class BoundingVolumeHierarchy
{
    constructor(objects = [])
    {
        this.module = null;
        this.lst_blas = [];
        this.lst_info = [];
        this.tlas = null;
        this.addModels(objects);
    }

    async _init()
    {
        if (this.module!=null) return;
        this.module = await get_module();
    }

    async addPrimitive(prim, matrix, info)
    {     
        await this._init();
        await prim.geometry_ready();

        let p_indices = this.module.ccall("alloc", "number", ["number"], [prim.cpu_indices.byteLength]);
        this.module.HEAPU8.set(new Uint8Array(prim.cpu_indices), p_indices);

        let p_pos = this.module.ccall("alloc", "number", ["number"], [prim.cpu_pos.byteLength]);
        this.module.HEAPU8.set(new Uint8Array(prim.cpu_pos), p_pos);

        let arr_matrix = new Float32Array(matrix.elements);        
        let p_matrix = this.module.ccall("alloc", "number", ["number"], [64]);
        this.module.HEAPU8.set(new Uint8Array(arr_matrix.buffer), p_matrix);

        let blas = this.module.ccall("CreateBLAS", "number", ["number", "number", "number", "number", "number", "number" ], [prim.num_face, prim.num_pos, p_indices, prim.type_indices, p_pos, p_matrix]);
        this.lst_blas.push(blas);

        this.module.ccall("dealloc", null, ["number"], [p_indices]);
        this.module.ccall("dealloc", null, ["number"], [p_pos]);
        this.module.ccall("dealloc", null, ["number"], [p_matrix]);

        this.lst_info.push(info);

        if (this.tlas!=null)
        {
            this.module.ccall("DestroyTLAS", null, ["number"], [this.tlas]);
        }

        let arr_blas = new Uint32Array(this.lst_blas);
        let p_lst_blas = this.module.ccall("alloc", "number", ["number"], [arr_blas.buffer.byteLength]);
        this.module.HEAPU8.set(new Uint8Array(arr_blas.buffer), p_lst_blas);
        this.tlas = this.module.ccall("CreateTLAS", "number", ["number", "number"], [this.lst_blas.length, p_lst_blas]);
        this.module.ccall("dealloc", null, ["number"], [p_lst_blas]);        
    }

    async addModel(model)
    {
        if (model instanceof SimpleModel)
        {
            let material = model.material;
            if (material.alphaMode!="Opaque") return;
            let primitive = model.geometry;
            await this.addPrimitive(primitive, model.matrixWorld, {model});
        }
        else if (model instanceof GLTFModel)
        {
            await model.meta_ready();
            let materials = model.materials;
            let num_mesh = model.meshes.length;
            let pendings = [];
            for (let i=0; i<num_mesh; i++)
            {
                let mesh = model.meshes[i];
                let matrix = model.matrixWorld;
                if (mesh.node_id>=0 && mesh.skin_id < 0)
                {
                    let node = model.nodes[mesh.node_id];
                    matrix.multiply(node.g_trans);
                }
                let num_primitives = mesh.primitives.length;
                for (let j=0; j<num_primitives; j++)
                {
                    let primitive = mesh.primitives[j];
                    let material = materials[primitive.material_idx];
                    if (material.alphaMode == "Opaque")
                    {
                        pendings.push(this.addPrimitive(primitive, matrix, {model, mesh_idx: i, prim_idx: j}));
                    }
                }
            }
            await Promise.all(pendings);
        }
    }

    async addModels(objects)
    {
        let object_set = new Set();
        for (let obj of objects)
        {
            obj.updateWorldMatrix(true, false);
            obj.traverse((child)=>{
                child.updateWorldMatrix(false, false);
                object_set.add(obj);
            });
        }
        let pendings = [];
        for (let model of object_set)
        {
            pendings.push(this.addModel(model));
        }
        await Promise.all(pendings);
    }

    intersect(ray, culling = 0)
    {
        if (this.module == null || this.tlas == null) return null;

        if (!("near" in ray))
        {
            ray.near = 0;
        }

        if (!("far" in ray))
        {
            ray.far = Number.MAX_VALUE;
        }

        let p_interection =  this.module.ccall("alloc", "number", ["number"], [20]);
        let result = this.module.ccall("IntersectTLAS", "number", ["number","number","number","number","number","number","number","number","number","number","number"], 
            [this.tlas, ray.origin.x, ray.origin.y, ray.origin.z,  ray.direction.x, ray.direction.y, ray.direction.z, ray.near, ray.far, culling, p_interection]);        

        if (result!=0)
        {
            let arr_intersection = new ArrayBuffer(20);
            {
                let view_in = new Uint8Array(this.module.HEAPU8.buffer, p_interection, 20);
                let view_out = new Uint8Array(arr_intersection);
                view_out.set(view_in);
            }
            this.module.ccall("dealloc", null, ["number"], [p_interection]);

            let iview = new Int32Array(arr_intersection);
            let fview = new Float32Array(arr_intersection);

            let prim_idx = iview[0];
            let tri_idx = iview[1];
            let t = fview[2];
            let u = fview[3];
            let v = fview[4];

            let info = this.lst_info[prim_idx];

            return { info, tri_idx, t, u, v};
        }
        else
        {
            this.module.ccall("dealloc", null, ["number"], [p_interection]);
            return null;
        }
    }    

}
