import { Vector3 } from "../math/Vector3.js"
import { Matrix4 } from "../math/Matrix4.js"
import { Quaternion } from "../math/Quaternion.js";


export function CreateBindGroupLayout_Model()
{
    if(!("model" in engine_ctx.cache.bindGroupLayouts))
    {
        engine_ctx.cache.bindGroupLayouts.model = engine_ctx.device.createBindGroupLayout({
            entries: [
                {
                    binding: 0,
                    visibility: GPUShaderStage.VERTEX,
                    buffer:{
                        type: "uniform"
                    }
                }
            ]
        });
    }
    return engine_ctx.cache.bindGroupLayouts.model;
}

export function CreateBindGroup_Model(constant)
{
    let layout = CreateBindGroupLayout_Model();
    return engine_ctx.device.createBindGroup({
        layout,
        entries: [
            {
                binding: 0,
                resource:{
                    buffer: constant
                }
            }
        ]
    });
}

export function UpdateConstant_Model(constant, matrixWorld)
{
    let normMatrix =  matrixWorld.clone();
    normMatrix.invert();
    normMatrix.transpose();
    const uniform = new Float32Array(16*2);
    for (let i=0; i<16; i++)
    {
        uniform[i] = matrixWorld.elements[i];
    }
    for (let i=0; i<16; i++)
    {
        uniform[16+i] = normMatrix.elements[i];
    }
    engine_ctx.queue.writeBuffer(constant, 0, uniform.buffer, uniform.byteOffset, uniform.byteLength);
}

export class GeometrySet
{
    constructor()
    {
        this.pos_buf = null;
        this.normal_buf = null;
        this.tangent_buf = null;
        this.bitangent_buf = null;
    }
}

export class Primitive
{
    constructor()
    {
        this.num_pos = 0;

        this.geometry = [];
        this.color_buf = null;
        this.uv_buf = null;
        this.joints_buf = null;
        this.weights_buf = null;

        this.num_face = 0;
        this.type_indices = 2;
        this.index_buf = null;

        this.num_targets = 0;
        this.targets = null;
        this.none_zero_buf = null;

        this.material_idx = -1;

        this.min_pos = new Vector3(Infinity,Infinity,Infinity);
        this.max_pos = new Vector3(-Infinity, -Infinity, -Infinity);

    }
}

export class Node
{
    constructor()
    {
        this.children = [];
        this.translation = new Vector3(0,0,0);
        this.rotation = new Quaternion(0,0,0,1);
        this.scale = new Vector3(1,1,1);
        this.g_trans = new Matrix4();    
    }

}

export class Skin
{
    constructor()
    {
        this.joints = [];
        this.inverseBindMatrices = new Matrix4();
    }
}

export class Mesh
{
    constructor()
    {
        this.node_id = -1;
        this.skin_id = -1;
        this.primitives = [];
        this.weights = [];
        this.needUpdateMorphTargets = false;
    }
}
