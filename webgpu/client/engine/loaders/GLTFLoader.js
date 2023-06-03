import { Matrix4 } from "../math/Matrix4.js"
import { GLTFModel } from "../models/GLTFModel.js"
import { MeshStandardMaterial } from "../materials/MeshStandardMaterial.js"
import { GeometrySet, Primitive, Node, Skin, Mesh } from "../models/ModelComponents.js"
import { CreateTexture } from "../renderers/GPUUtils.js"
import get_module from './GltfLoad.js'

function get_url_extension( url ) {
    return url.split(/[#?]/)[0].split('.').pop().trim();
}

export class GLTFLoader
{
    constructor()
    {
        

    }

    loadModelFromFile(url)
    {
        let model = new GLTFModel;
        let gltf_version;
        let file_length;
        let json_length;

        let bin_uri = url;
        let bin_offset;
        
        let xhr;

        let material_affected_primitives;
        let tex_affected_materials;

        const load_image_url = async (url, idx, opts) =>
        {
            let img = document.createElement("img"); 
            img.src = url;       
            await img.decode();
            const imageBitmap = await createImageBitmap(img);
            let texture = CreateTexture(imageBitmap, true, opts.is_srgb);
            model.textures[idx] = texture;

            let affected_materials = tex_affected_materials[idx];
            for (let material_idx of affected_materials)
            {
                let affected_primitives = material_affected_primitives[material_idx];
                for (let {i,j} of affected_primitives)
                {
                    let mesh = model.meshes[i];
                    let prim = mesh.primitives[j];
                    prim.create_bind_group(mesh.constant, model.materials, model.textures);
                }
            }
        }

        const load_image = async (blob, idx, opts) =>
        {            
            let urlCreator = window.URL || window.webkitURL;
            var imageUrl = urlCreator.createObjectURL(blob);
            await load_image_url(imageUrl, idx, opts);
        };

        const load_targets = async (primitive_out, info) =>
        {
            let num_pos = primitive_out.num_pos;
            let type_indices = primitive_out.type_indices;
            let num_face = primitive_out.num_face;
            let num_targets = primitive_out.num_targets;

            let pendings = [];

            let pos_targets = new ArrayBuffer(num_pos*4*4 * num_targets);
            let norm_targets = new ArrayBuffer(num_pos*4*4 * num_targets);
            let non_zero = new Int32Array(num_pos);    
            
            for (let i=0; i< num_targets; i++)
            {
                let target_info = info.targets[i];
                if (!target_info.pos_is_sparse)
                {                        
                    pendings.push(new Promise((resolve, reject) => {
                        let xhr_pos;
                        const load_position = ()=>
                        {
                            const arrBuf = xhr_pos.response;

                            let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                            let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
            
                            this.module.HEAPU8.set(new Uint8Array(arrBuf), p_vec3);
                            this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 0.0]);                                
                            let src_view = new Uint8Array(this.module.HEAPU8.buffer, p_vec4, num_pos*4*4)
                            let dst_view = new Uint8Array(pos_targets, num_pos*4*4*i, num_pos*4*4);
                            dst_view.set(src_view)
            
                            this.module.ccall("dealloc", null, ["number"], [p_vec3]);
                            this.module.ccall("dealloc", null, ["number"], [p_vec4]);
    
                            resolve(true);
                        };
    
                        xhr_pos = new XMLHttpRequest(); 
                        xhr_pos.open("GET", bin_uri);
                        xhr_pos.responseType = "arraybuffer";            
                        xhr_pos.setRequestHeader('Range', `bytes=${bin_offset + target_info.position_offset}-${bin_offset + target_info.position_offset + num_pos*3*4 - 1}`);
                        xhr_pos.onload = load_position;
                        xhr_pos.send();
                    }));
                }
                else
                { 
                    pendings.push((async()=>{
                        let count = target_info.position_count;
                        let idx_bytes = 4;
                        if (target_info.position_idx_type == 5123)
                        {
                            idx_bytes = 2;
                        }
                        else if (target_info.position_idx_type == 5121)
                        {
                            idx_bytes = 1;
                        }  
                        
                        let pending_bufs = [
                            new Promise((resolve, reject) => {
                                let dataSize =  count*idx_bytes;
                                dataSize = (dataSize + 3) & ~3;

                                let xhr_pos_idx = new XMLHttpRequest(); 
                                xhr_pos_idx.open("GET", bin_uri);
                                xhr_pos_idx.responseType = "arraybuffer";            
                                xhr_pos_idx.setRequestHeader('Range', `bytes=${bin_offset + target_info.position_idx_offset}-${bin_offset + target_info.position_idx_offset + dataSize - 1}`);
                                xhr_pos_idx.onload = ()=> resolve( xhr_pos_idx.response);
                                xhr_pos_idx.send();
                            }),
                            new Promise((resolve, reject) => {
                                let xhr_pos_value = new XMLHttpRequest(); 
                                xhr_pos_value.open("GET", bin_uri);
                                xhr_pos_value.responseType = "arraybuffer";            
                                xhr_pos_value.setRequestHeader('Range', `bytes=${bin_offset + target_info.position_value_offset}-${bin_offset + target_info.position_value_offset + count*3*4 - 1}`);
                                xhr_pos_value.onload = ()=> resolve( xhr_pos_value.response);
                                xhr_pos_value.send();
                            })
                        ];

                        let [arr_idx, arr_value] = await Promise.all(pending_bufs);
                        let view_idx, view_value;
                        if (idx_bytes == 4)
                        {
                            view_idx = new Uint32Array(arr_idx);
                        }
                        else if (idx_bytes == 2)
                        {
                            view_idx = new Uint16Array(arr_idx);
                        }
                        else if (idx_bytes == 1)
                        {
                            view_idx = new Uint8Array(arr_idx);
                        }
                        view_value = new Float32Array(arr_value);

                        let dst_view = new Float32Array(pos_targets, num_pos*4*4*i, num_pos*4);
                        for (let j=0; j< view_idx.length; j++)
                        {
                            let idx = view_idx[j];
                            let value = [view_value[j*3], view_value[j*3+1], view_value[j*3+2]];
                            if (value[0]!=0 || value[1]!=0 || value[2]!=0)
                            {
                                non_zero[idx] = 1;
                                dst_view[idx*4] = value[0];
                                dst_view[idx*4+1] = value[1];
                                dst_view[idx*4+2] = value[2];
                            }
                        }

                    })());
                }

                if (target_info.has_normal)
                {
                    if (!target_info.norm_is_sparse)
                    {
                        pendings.push(new Promise((resolve, reject) => {
                            let xhr_norm;
                            
                            const load_normal = ()=>
                            {
                                const arrBuf = xhr_norm.response;

                                let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                                let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);

                                this.module.HEAPU8.set(new Uint8Array(arrBuf), p_vec3);
                                this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 0.0]);
                                let src_view = new Uint8Array(this.module.HEAPU8.buffer, p_vec4, num_pos*4*4)
                                let dst_view = new Uint8Array(norm_targets, num_pos*4*4*i, num_pos*4*4);
                                dst_view.set(src_view);
                                
                                this.module.ccall("dealloc", null, ["number"], [p_vec3]);
                                this.module.ccall("dealloc", null, ["number"], [p_vec4]);
        
                                resolve(true);

                            };

                            xhr_norm = new XMLHttpRequest(); 
                            xhr_norm.open("GET", bin_uri);
                            xhr_norm.responseType = "arraybuffer";            
                            xhr_norm.setRequestHeader('Range', `bytes=${bin_offset + target_info.normal_offset}-${bin_offset + target_info.normal_offset + num_pos*3*4 - 1}`);
                            xhr_norm.onload = load_normal;
                            xhr_norm.send();

                        }));
                    }
                    else
                    {
                        pendings.push((async()=>{
                            let count = target_info.normal_count;
                            let idx_bytes = 4;
                            if (target_info.normal_idx_type == 5123)
                            {
                                idx_bytes = 2;
                            }
                            else if (target_info.normal_idx_type == 5121)
                            {
                                idx_bytes = 1;
                            }

                            let pending_bufs = [
                                new Promise((resolve, reject) => {
                                    let dataSize =  count*idx_bytes;
                                    dataSize = (dataSize + 3) & ~3;

                                    let xhr_norm_idx = new XMLHttpRequest(); 
                                    xhr_norm_idx.open("GET", bin_uri);
                                    xhr_norm_idx.responseType = "arraybuffer";            
                                    xhr_norm_idx.setRequestHeader('Range', `bytes=${bin_offset + target_info.normal_idx_offset}-${bin_offset + target_info.normal_idx_offset + dataSize - 1}`);
                                    xhr_norm_idx.onload = ()=> resolve( xhr_norm_idx.response);
                                    xhr_norm_idx.send();

                                }),
                                new Promise((resolve, reject) => {
                                    let xhr_norm_value = new XMLHttpRequest(); 
                                    xhr_norm_value.open("GET", bin_uri);
                                    xhr_norm_value.responseType = "arraybuffer";            
                                    xhr_norm_value.setRequestHeader('Range', `bytes=${bin_offset + target_info.normal_value_offset}-${bin_offset + target_info.normal_value_offset + count*3*4 - 1}`);
                                    xhr_norm_value.onload = ()=> resolve( xhr_norm_value.response);
                                    xhr_norm_value.send();
                                })
                            ];

                            let [arr_idx, arr_value] = await Promise.all(pending_bufs);
                            let view_idx, view_value;
                            if (idx_bytes == 4)
                            {
                                view_idx = new Uint32Array(arr_idx);
                            }
                            else if (idx_bytes == 2)
                            {
                                view_idx = new Uint16Array(arr_idx);
                            }
                            else if (idx_bytes == 1)
                            {
                                view_idx = new Uint8Array(arr_idx);
                            }
                            view_value = new Float32Array(arr_value);
                            
                            let dst_view = new Float32Array(norm_targets, num_pos*4*4*i, num_pos*4);
                            for (let j=0; j< view_idx.length; j++)
                            {
                                let idx = view_idx[j];
                                let value = [view_value[j*3], view_value[j*3+1], view_value[j*3+2]];
                                if (value[0]!=0 || value[1]!=0 || value[2]!=0)
                                {
                                    non_zero[idx] = 1;
                                    dst_view[idx*4] = value[0];
                                    dst_view[idx*4+1] = value[1];
                                    dst_view[idx*4+2] = value[2];
                                }
                            }

                        })());
                    }
                }


            }

            await Promise.all(pendings);               

            let view_pos = new Float32Array(primitive_out.cpu_pos);
            let view_norm = new Float32Array(primitive_out.cpu_norm);

            let view_tangent; 
            let view_bitangent;
            let tangent_targets;
            let bitangent_targets;            
            if (info.has_tangent)
            {
                view_tangent = new Float32Array(primitive_out.cpu_tangent);
                view_bitangent = new Float32Array(primitive_out.cpu_bitangent);
                tangent_targets = new ArrayBuffer(num_pos*4*4 * num_targets); 
                bitangent_targets = new ArrayBuffer(num_pos*4*4 * num_targets); 
            }

            for (let i=0; i< num_targets; i++)
            {
                let target_info = info.targets[i];
                let view_pos_target;
                if (!target_info.has_normal || info.has_tangent)
                {
                    let view_pos_delta = new Float32Array(pos_targets, num_pos*4*4*i,  num_pos*4);
                    view_pos_target = new Float32Array(num_pos*3);
                    for (let j=0; j<num_pos; j++)
                    {
                        view_pos_target[j*3] = view_pos[j*3] + view_pos_delta[j*4];
                        view_pos_target[j*3+1] = view_pos[j*3+1] + view_pos_delta[j*4+1];
                        view_pos_target[j*3+2] = view_pos[j*3+2] + view_pos_delta[j*4+2];
                    }
                }

                if (!target_info.has_normal)
                {   
                    let p_pos3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                    this.module.HEAPU8.set(new Uint8Array(view_pos_target.buffer), p_pos3);

                    let p_indices = 0;
                    if (primitive_out.cpu_indices!=null)
                    {
                        p_indices = this.module.ccall("alloc", "number", ["number"], [num_face*type_indices*3]);
                        this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_indices), p_indices);
                    }

                    {
                        let p_norm4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                        this.module.ccall("zero", null, ["number", "number"], [p_norm4, num_pos*4*4]);                
                        this.module.ccall("calc_normal", null, ["number", "number", "number", "number", "number", "number"], [num_face, num_pos, type_indices, p_indices, p_pos3, p_norm4]);
                        
                        let src_view = new Uint8Array(this.module.HEAPU8.buffer, p_norm4, num_pos*4*4)
                        let dst_view = new Uint8Array(norm_targets, num_pos*4*4*i, num_pos*4*4);                   
                        dst_view.set(src_view);
                        this.module.ccall("dealloc", null, ["number"], [p_norm4]);
                    }
                    
                    this.module.ccall("dealloc", null, ["number"], [p_pos3]);
                    if (p_indices > 0)
                    {
                        this.module.ccall("dealloc", null, ["number"], [p_indices]);
                    }
                    

                    let view_norm_delta = new Float32Array(norm_targets, num_pos*4*4*i,  num_pos*4);

                    for (let j=0; j<num_pos; j++)
                    {
                        view_norm_delta[j*4]-=view_norm[j*3];
                        view_norm_delta[j*4+1]-=view_norm[j*3+1];
                        view_norm_delta[j*4+2]-=view_norm[j*3+2];
                        if (info.is_sparse && 
                            (view_norm_delta[j*4]!=0 || view_norm_delta[j*4 + 1]!=0 || view_norm_delta[j*4 + 2]!=0))
                        {
                            non_zero[j] = 1;
                        }
                    }
                    
                }

                if (info.has_tangent)
                {
                    let p_pos3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                    this.module.HEAPU8.set(new Uint8Array(view_pos_target.buffer), p_pos3);

                    let p_uv2 =  this.module.ccall("alloc", "number", ["number"], [num_pos*2*4]);
                    this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_uv), p_uv2);

                    let p_indices = 0;
                    if (primitive_out.cpu_indices!=null)
                    {
                        p_indices = this.module.ccall("alloc", "number", ["number"], [num_face*type_indices*3]);
                        this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_indices), p_indices);
                    }

                    {
                        let p_tangent4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                        let p_bitangent4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                        this.module.ccall("zero", null, ["number", "number"], [p_tangent4, num_pos*4*4]);
                        this.module.ccall("zero", null, ["number", "number"], [p_bitangent4, num_pos*4*4]);

                        this.module.ccall("calc_tangent", null, ["number", "number", "number", "number", "number", "number", "number", "number"], [num_face, num_pos, type_indices, p_indices, p_pos3, p_uv2, p_tangent4, p_bitangent4]);                

                        {
                            let src_view = new Uint8Array(this.module.HEAPU8.buffer, p_tangent4, num_pos*4*4)
                            let dst_view = new Uint8Array(tangent_targets, num_pos*4*4*i, num_pos*4*4);                   
                            dst_view.set(src_view);
                        }

                        {
                            let src_view = new Uint8Array(this.module.HEAPU8.buffer, p_bitangent4, num_pos*4*4)
                            let dst_view = new Uint8Array(bitangent_targets, num_pos*4*4*i, num_pos*4*4);                   
                            dst_view.set(src_view);
                        }

                        this.module.ccall("dealloc", null, ["number"], [p_tangent4]);
                        this.module.ccall("dealloc", null, ["number"], [p_bitangent4]);
                    }


                    this.module.ccall("dealloc", null, ["number"], [p_pos3]);
                    this.module.ccall("dealloc", null, ["number"], [p_uv2]);
                    if (p_indices > 0)
                    {
                        this.module.ccall("dealloc", null, ["number"], [p_indices]);
                    }

                    let view_tangent_delta = new Float32Array(tangent_targets, num_pos*4*4*i,  num_pos*4);
                    let view_bitangent_delta = new Float32Array(bitangent_targets, num_pos*4*4*i,  num_pos*4);

                    for (let j=0; j<num_pos; j++)
                    {
                        view_tangent_delta[j*4]-=view_tangent[j*3];
                        view_tangent_delta[j*4+1]-=view_tangent[j*3+1];
                        view_tangent_delta[j*4+2]-=view_tangent[j*3+2];
                        view_bitangent_delta[j*4]-=view_bitangent[j*3];
                        view_bitangent_delta[j*4+1]-=view_bitangent[j*3+1];
                        view_bitangent_delta[j*4+2]-=view_bitangent[j*3+2];
                        if (info.is_sparse && (
                            view_tangent_delta[j*4]!=0 || view_tangent_delta[j*4 + 1]!=0 || view_tangent_delta[j*4 + 2]!=0 ||
                            view_bitangent_delta[j*4]!=0 || view_bitangent_delta[j*4 + 1]!=0 || view_bitangent_delta[j*4 + 2]!=0))
                        {
                            non_zero[j] = 1;
                        }
                    }

                }
            }                     
            
            primitive_out.targets = new GeometrySet();
            primitive_out.targets.pos_buf = engine_ctx.createBuffer(pos_targets, GPUBufferUsage.STORAGE, 0, num_pos*4*4 * num_targets);
            primitive_out.targets.normal_buf = engine_ctx.createBuffer(norm_targets, GPUBufferUsage.STORAGE, 0, num_pos*4*4 * num_targets);
            
            if (info.has_tangent)
            {
                primitive_out.targets.tangent_buf = engine_ctx.createBuffer(tangent_targets, GPUBufferUsage.STORAGE, 0, num_pos*4*4 * num_targets);
                primitive_out.targets.bitangent_buf = engine_ctx.createBuffer(bitangent_targets, GPUBufferUsage.STORAGE, 0, num_pos*4*4 * num_targets);
            }

            if (info.is_sparse)
            {
                primitive_out.none_zero_buf = engine_ctx.createBuffer(non_zero.buffer, GPUBufferUsage.STORAGE, 0, num_pos*4);
            }
        }

        const load_primitive = async (primitive_out, info)=>
        {
            let num_pos = primitive_out.num_pos;
            let type_indices = primitive_out.type_indices;
            let num_face = primitive_out.num_face;
            let num_targets = primitive_out.num_targets;
            let geo_set = new GeometrySet();
            primitive_out.geometry.push(geo_set);

            let num_geo_sets = 1;
            if (num_targets>0)
            {
                num_geo_sets++;
            }

            if (info.is_skinned)
            {
                num_geo_sets++;
            }

            let usage0 = num_geo_sets>1? GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC : GPUBufferUsage.VERTEX;

            let pendings = [];

            if (info.draco_offset>=0)
            {           
                pendings.push(new Promise((resolve, reject) => {
                    let xhr_draco;
                    const load_draco = ()=>
                    {
                        const arrBuf = xhr_draco.response;
                        let decoderConfig = {};                                
                        decoderConfig.onModuleLoaded = (draco)=>
                        {
                            let decoderBuffer = new draco.DecoderBuffer();
                            decoderBuffer.Init( new Int8Array( arrBuf ), arrBuf.byteLength );

                            const decoder = new draco.Decoder();
                            let dracoGeometry = new draco.Mesh();
                            decoder.DecodeBufferToMesh(decoderBuffer, dracoGeometry);

                            {
                                let dataSize = num_face * 3 * type_indices;
                                dataSize = (dataSize + 3) & ~3;

                                let arrBuf = new ArrayBuffer(dataSize);
                                let view = new Uint8Array(arrBuf);

                                let ptr = draco._malloc( dataSize );  
                                if (type_indices==4)
                                {
                                    decoder.GetTrianglesUInt32Array( dracoGeometry, dataSize, ptr );
                                }
                                else
                                {
                                    decoder.GetTrianglesUInt16Array( dracoGeometry, dataSize, ptr );
                                }
                                let buf_in = new Uint8Array(draco.HEAPU8.buffer, ptr, dataSize);
                                view.set(buf_in);
                                draco._free( ptr );

                                primitive_out.cpu_indices = arrBuf;
                                primitive_out.index_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.INDEX, 0,  dataSize);

                            }

                            {
                                let dataSize =  num_pos * 3 *4;

                                let arrBuf = new ArrayBuffer(dataSize);
                                let view = new Uint8Array(arrBuf);
                                
                                let attribute = decoder.GetAttributeByUniqueId( dracoGeometry, info.draco_attributes.POSITION );                 
                                let ptr = draco._malloc( dataSize );                                
                                decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_FLOAT32, dataSize, ptr );
                                let buf_in = new Uint8Array(draco.HEAPU8.buffer, ptr, dataSize);
                                view.set(buf_in);
                                draco._free( ptr );

                                primitive_out.cpu_pos = arrBuf;
                                let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                                let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                
                                this.module.HEAPU8.set(view, p_vec3);
                                this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 1.0]);
                                geo_set.pos_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, usage0, p_vec4, num_pos*4*4);
                
                                this.module.ccall("dealloc", null, ["number"], [p_vec3]);
                                this.module.ccall("dealloc", null, ["number"], [p_vec4]);
                                
                            }

                            if ("NORMAL" in info.draco_attributes)
                            {                                
                                let dataSize =  num_pos * 3 *4;

                                let arrBuf = new ArrayBuffer(dataSize);
                                let view = new Uint8Array(arrBuf);
                                
                                let attribute = decoder.GetAttributeByUniqueId( dracoGeometry, info.draco_attributes.NORMAL );
                                let ptr = draco._malloc( dataSize );
                                decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_FLOAT32, dataSize, ptr );
                                let buf_in = new Uint8Array(draco.HEAPU8.buffer, ptr, dataSize);
                                view.set(buf_in);
                                draco._free( ptr );

                                primitive_out.cpu_norm = arrBuf;
                                let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                                let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
        
                                this.module.HEAPU8.set(new Uint8Array(arrBuf), p_vec3);
                                this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 0.0]);
                                geo_set.normal_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, usage0, p_vec4, num_pos*4*4);
        
                                this.module.ccall("dealloc", null, ["number"], [p_vec3]);
                                this.module.ccall("dealloc", null, ["number"], [p_vec4]);

                            }

                            if ("COLOR_0" in info.draco_attributes)
                            {
                                let chn = info.color_type == "VEC4"? 4:3;
                                let byte_per_ch = 4;
                                if (info.color_componentType == 5123)
                                {
                                    byte_per_ch = 2;
                                }
                                else if (info.color_componentType == 5121)
                                {
                                    byte_per_ch = 1;
                                }

                                let dataSize =  num_pos * chn * byte_per_ch;

                                let arrBuf = new ArrayBuffer(dataSize);
                                let view = new Uint8Array(arrBuf);

                                let attribute = decoder.GetAttributeByUniqueId( dracoGeometry, info.draco_attributes.NORMAL );
                                let ptr = draco._malloc( dataSize );
                                if (byte_per_ch == 4)
                                {
                                    decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_FLOAT32, dataSize, ptr );
                                }
                                else if (byte_per_ch == 2)
                                {
                                    decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_UINT16, dataSize, ptr );
                                }
                                else if (byte_per_ch == 1)
                                {
                                    decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_UINT8, dataSize, ptr );
                                }
                                let buf_in = new Uint8Array(draco.HEAPU8.buffer, ptr, dataSize);
                                view.set(buf_in);
                                draco._free( ptr );

                                if (chn==4 && byte_per_ch == 4)
                                {
                                    primitive_out.color_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.VERTEX, 0, num_pos*4*4);                                    
                                }
                                else
                                {
                                    let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                                    if (chn==4)
                                    {                            
                                        if (byte_per_ch == 2)
                                        {
                                            let p_u16vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*2*4]);
                                            this.module.HEAPU8.set(view, p_u16vec4);
                                            this.module.ccall("u16vec4_to_vec4", null, ["number", "number", "number"], [p_u16vec4, p_vec4, num_pos]);
                                            this.module.ccall("dealloc", null, ["number"], [p_u16vec4]);

                                        }
                                        else
                                        {
                                            let p_u8vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4]);
                                            this.module.HEAPU8.set(view, p_u8vec4);
                                            this.module.ccall("u8vec4_to_vec4", null, ["number", "number", "number"], [p_u8vec4, p_vec4, num_pos]);
                                            this.module.ccall("dealloc", null, ["number"], [p_u8vec4]);

                                        }
                                    }
                                    else
                                    {
                                        if (byte_per_ch == 4)
                                        {
                                            let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                                            this.module.HEAPU8.set(view, p_vec3);
                                            this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 1.0]);
                                            this.module.ccall("dealloc", null, ["number"], [p_vec3]);

                                        }
                                        else if (byte_per_ch == 2)
                                        {
                                            let p_u16vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*2*3]);
                                            this.module.HEAPU8.set(view, p_u16vec3);
                                            this.module.ccall("u16vec3_to_vec4", null, ["number", "number", "number", "number"], [p_u16vec3, p_vec4, num_pos, 1.0]);
                                            this.module.ccall("dealloc", null, ["number"], [p_u16vec3]);

                                        }
                                        else
                                        {
                                            let p_u8vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3]);
                                            this.module.HEAPU8.set(view, p_u8vec3);
                                            this.module.ccall("u8vec3_to_vec4", null, ["number", "number", "number", "number"], [p_u8vec3, p_vec4, num_pos, 1.0]);
                                            this.module.ccall("dealloc", null, ["number"], [p_u8vec3]);
                                        }
                                    }

                                    primitive_out.color_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, GPUBufferUsage.VERTEX, p_vec4, num_pos*4*4);
                                    this.module.ccall("dealloc", null, ["number"], [p_vec4]);
                                }                                
                            }
                            
                            if ("TEXCOORD_0" in info.draco_attributes)
                            {
                                let dataSize =  num_pos * 2 *4;

                                let arrBuf = new ArrayBuffer(dataSize);
                                let view = new Uint8Array(arrBuf);
                                
                                let attribute = decoder.GetAttributeByUniqueId( dracoGeometry, info.draco_attributes.TEXCOORD_0 );
                                let ptr = draco._malloc( dataSize );
                                decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_FLOAT32, dataSize, ptr );
                                let buf_in = new Uint8Array(draco.HEAPU8.buffer, ptr, dataSize);
                                view.set(buf_in);
                                draco._free( ptr );

                                primitive_out.cpu_uv = arrBuf;
                                primitive_out.uv_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.VERTEX, 0, num_pos*2*4);

                            }        
                            
                            if ("JOINTS_0" in info.draco_attributes)
                            {
                                let dataSize = num_pos * 4 * 4;

                                let arrBuf = new ArrayBuffer(dataSize);
                                let view = new Uint8Array(arrBuf);

                                let attribute = decoder.GetAttributeByUniqueId( dracoGeometry, info.draco_attributes.JOINTS_0 );
                                let ptr = draco._malloc( dataSize );
                                decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_UINT32, dataSize, ptr );
                                let buf_in = new Uint8Array(draco.HEAPU8.buffer, ptr, dataSize);
                                view.set(buf_in);
                                draco._free( ptr );

                                primitive_out.joints_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.STORAGE, 0, num_pos*4*4);
                            }

                            if ("WEIGHTS_0" in info.draco_attributes)
                            {
                                let dataSize = num_pos * 4 * 4;

                                let arrBuf = new ArrayBuffer(dataSize);
                                let view = new Uint8Array(arrBuf);

                                let attribute = decoder.GetAttributeByUniqueId( dracoGeometry, info.draco_attributes.WEIGHTS_0 );
                                let ptr = draco._malloc( dataSize );
                                decoder.GetAttributeDataArrayForAllPoints( dracoGeometry, attribute, draco.DT_FLOAT32, dataSize, ptr );
                                let buf_in = new Uint8Array(draco.HEAPU8.buffer, ptr, dataSize);
                                view.set(buf_in);
                                draco._free( ptr );

                                primitive_out.weights_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.STORAGE, 0, num_pos*4*4);
                            }

                            draco.destroy(dracoGeometry);
                            draco.destroy(decoder);
                            draco.destroy(decoderBuffer);

                            resolve(true);


                        };
                        DracoDecoderModule(decoderConfig);

                    };                    
                    
                    xhr_draco = new XMLHttpRequest(); 
                    xhr_draco.open("GET", bin_uri);
                    xhr_draco.responseType = "arraybuffer";            
                    xhr_draco.setRequestHeader('Range', `bytes=${bin_offset + info.draco_offset}-${bin_offset + info.draco_offset + info.draco_length - 1}`);
                    xhr_draco.onload = load_draco;
                    xhr_draco.send();
                }));
            }

            if (info.indices_offset>=0)
            {     
                pendings.push(new Promise((resolve, reject) => {

                    let dataSize =  num_face*type_indices*3;
                    dataSize = (dataSize + 3) & ~3;
                    
                    let xhr_indices;
                    const load_indices = ()=>
                    {
                        const arrBuf = xhr_indices.response;  
                        primitive_out.cpu_indices = arrBuf;
                        primitive_out.index_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.INDEX, 0,  dataSize);
                        resolve(true);
                    };    
                        
                    xhr_indices = new XMLHttpRequest(); 
                    xhr_indices.open("GET", bin_uri);
                    xhr_indices.responseType = "arraybuffer";            
                    xhr_indices.setRequestHeader('Range', `bytes=${bin_offset + info.indices_offset}-${bin_offset + info.indices_offset + dataSize - 1}`);
                    xhr_indices.onload = load_indices;
                    xhr_indices.send();
                }));
            }

            if (info.position_offset>=0)
            {
                pendings.push(new Promise((resolve, reject) => {
                    let xhr_pos;
                    const load_position = ()=>
                    {
                        const arrBuf = xhr_pos.response;  
                        primitive_out.cpu_pos = arrBuf;
                        let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                        let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
        
                        this.module.HEAPU8.set(new Uint8Array(arrBuf), p_vec3);
                        this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 1.0]);
                        geo_set.pos_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, usage0, p_vec4, num_pos*4*4);
        
                        this.module.ccall("dealloc", null, ["number"], [p_vec3]);
                        this.module.ccall("dealloc", null, ["number"], [p_vec4]);

                        resolve(true);
                    };

                    xhr_pos = new XMLHttpRequest(); 
                    xhr_pos.open("GET", bin_uri);
                    xhr_pos.responseType = "arraybuffer";            
                    xhr_pos.setRequestHeader('Range', `bytes=${bin_offset + info.position_offset}-${bin_offset + info.position_offset + num_pos*3*4 - 1}`);
                    xhr_pos.onload = load_position;
                    xhr_pos.send();
                }));
            }

            if (info.normal_offset>=0)
            {
                pendings.push(new Promise((resolve, reject) => {
                    let xhr_norm;
                    const load_normal = ()=>
                    {
                        const arrBuf = xhr_norm.response;  
                        primitive_out.cpu_norm = arrBuf;
                        let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                        let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);

                        this.module.HEAPU8.set(new Uint8Array(arrBuf), p_vec3);
                        this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 0.0]);
                        geo_set.normal_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, usage0, p_vec4, num_pos*4*4);

                        this.module.ccall("dealloc", null, ["number"], [p_vec3]);
                        this.module.ccall("dealloc", null, ["number"], [p_vec4]);

                        resolve(true);
                    };

                    xhr_norm = new XMLHttpRequest(); 
                    xhr_norm.open("GET", bin_uri);
                    xhr_norm.responseType = "arraybuffer";            
                    xhr_norm.setRequestHeader('Range', `bytes=${bin_offset + info.normal_offset}-${bin_offset + info.normal_offset + num_pos*3*4 - 1}`);
                    xhr_norm.onload = load_normal;
                    xhr_norm.send();
                }));
            }

            if (info.color_offset>=0)
            {
                pendings.push(new Promise((resolve, reject) => {

                    let xhr_color;
                    let chn = info.color_type == "VEC4"? 4:3;
                    let byte_per_ch = 4;
                    if (info.color_componentType == 5123)
                    {
                        byte_per_ch = 2;
                    }
                    else if (info.color_componentType == 5121)
                    {
                        byte_per_ch = 1;
                    }
                    const load_color = ()=>
                    {
                        const arrBuf = xhr_color.response;  

                        if (chn==4 && byte_per_ch == 4)
                        {
                            primitive_out.color_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.VERTEX, 0, num_pos*4*4);
                            
                        }
                        else
                        {
                            let p_vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                            if (chn==4)
                            {                            
                                if (byte_per_ch == 2)
                                {
                                    let p_u16vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*2*4]);
                                    this.module.HEAPU8.set(new Uint8Array(arrBuf), p_u16vec4);
                                    this.module.ccall("u16vec4_to_vec4", null, ["number", "number", "number"], [p_u16vec4, p_vec4, num_pos]);
                                    this.module.ccall("dealloc", null, ["number"], [p_u16vec4]);

                                }
                                else
                                {
                                    let p_u8vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4]);
                                    this.module.HEAPU8.set(new Uint8Array(arrBuf), p_u8vec4);
                                    this.module.ccall("u8vec4_to_vec4", null, ["number", "number", "number"], [p_u8vec4, p_vec4, num_pos]);
                                    this.module.ccall("dealloc", null, ["number"], [p_u8vec4]);

                                }
                            }
                            else
                            {
                                if (byte_per_ch == 4)
                                {
                                    let p_vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                                    this.module.HEAPU8.set(new Uint8Array(arrBuf), p_vec3);
                                    this.module.ccall("vec3_to_vec4", null, ["number", "number", "number", "number"], [p_vec3, p_vec4, num_pos, 1.0]);
                                    this.module.ccall("dealloc", null, ["number"], [p_vec3]);

                                }
                                else if (byte_per_ch == 2)
                                {
                                    let p_u16vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*2*3]);
                                    this.module.HEAPU8.set(new Uint8Array(arrBuf), p_u16vec3);
                                    this.module.ccall("u16vec3_to_vec4", null, ["number", "number", "number", "number"], [p_u16vec3, p_vec4, num_pos, 1.0]);
                                    this.module.ccall("dealloc", null, ["number"], [p_u16vec3]);

                                }
                                else
                                {
                                    let p_u8vec3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3]);
                                    this.module.HEAPU8.set(new Uint8Array(arrBuf), p_u8vec3);
                                    this.module.ccall("u8vec3_to_vec4", null, ["number", "number", "number", "number"], [p_u8vec3, p_vec4, num_pos, 1.0]);
                                    this.module.ccall("dealloc", null, ["number"], [p_u8vec3]);
                                    
                                }

                            }
                            primitive_out.color_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, GPUBufferUsage.VERTEX, p_vec4, num_pos*4*4);
                            this.module.ccall("dealloc", null, ["number"], [p_vec4]);

                        }                      

                        resolve(true);

                    };                    

                    xhr_color = new XMLHttpRequest(); 
                    xhr_color.open("GET", bin_uri);
                    xhr_color.responseType = "arraybuffer";            
                    xhr_color.setRequestHeader('Range', `bytes=${bin_offset + info.color_offset}-${bin_offset + info.color_offset + num_pos*chn*byte_per_ch - 1}`);
                    xhr_color.onload = load_color;
                    xhr_color.send();

                }));

            }

            if (info.uv_offset >=0)
            {
                pendings.push(new Promise((resolve, reject) => {

                    let xhr_uv;
                    const load_uv = ()=>
                    {
                        const arrBuf = xhr_uv.response;
                        primitive_out.cpu_uv = arrBuf;
                        primitive_out.uv_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.VERTEX, 0, num_pos*2*4);
                        resolve(true);
                    };

                    xhr_uv = new XMLHttpRequest(); 
                    xhr_uv.open("GET", bin_uri);
                    xhr_uv.responseType = "arraybuffer";            
                    xhr_uv.setRequestHeader('Range', `bytes=${bin_offset + info.uv_offset}-${bin_offset + info.uv_offset + num_pos*2*4 - 1}`);
                    xhr_uv.onload = load_uv;
                    xhr_uv.send();

                }));
            }

            if (info.joints_offset >= 0)
            {
                pendings.push(new Promise((resolve, reject) => {
                    let xhr_joints;
                    let byte_per_ch = 4;
                    if (info.joints_componentType == 5123)
                    {
                        byte_per_ch = 2;
                    }
                    else if (info.joints_componentType == 5121)
                    {
                        byte_per_ch = 1;
                    }

                    const load_joints = ()=>
                    {
                        const arrBuf = xhr_joints.response;  
                        if (byte_per_ch == 4)
                        {
                            primitive_out.joints_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.STORAGE, 0, num_pos*4*4);
                        }
                        else
                        {
                            let p_u32vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);

                            if (byte_per_ch == 2)
                            {
                                let p_u16vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*2*4]);
                                this.module.HEAPU8.set(new Uint8Array(arrBuf), p_u16vec4);
                                this.module.ccall("u16vec4_to_u32vec4", null, ["number", "number", "number"], [p_u16vec4, p_u32vec4, num_pos]);
                                this.module.ccall("dealloc", null, ["number"], [p_u16vec4]);

                            }
                            else
                            {
                                let p_u8vec4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4]);
                                this.module.HEAPU8.set(new Uint8Array(arrBuf), p_u8vec4);
                                this.module.ccall("u8vec4_to_u32vec4", null, ["number", "number", "number"], [p_u8vec4, p_u32vec4, num_pos]);
                                this.module.ccall("dealloc", null, ["number"], [p_u8vec4]);
                            }

                            primitive_out.joints_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, GPUBufferUsage.STORAGE, p_u32vec4, num_pos*4*4);
                            this.module.ccall("dealloc", null, ["number"], [p_u32vec4]);
                        }
                        resolve(true);
                    };

                    xhr_joints = new XMLHttpRequest(); 
                    xhr_joints.open("GET", bin_uri);
                    xhr_joints.responseType = "arraybuffer";            
                    xhr_joints.setRequestHeader('Range', `bytes=${bin_offset + info.joints_offset}-${bin_offset + info.joints_offset + num_pos*byte_per_ch*4 - 1}`);
                    xhr_joints.onload = load_joints;
                    xhr_joints.send();

                }));
            }

            if (info.weights_offset >= 0)
            {
                pendings.push(new Promise((resolve, reject) => {
                    let xhr_weights;

                    const load_weights = ()=>
                    {
                        const arrBuf = xhr_weights.response;  
                        primitive_out.weights_buf = engine_ctx.createBuffer(arrBuf, GPUBufferUsage.STORAGE, 0, num_pos*4*4);
                        resolve(true);
                    }

                    xhr_weights = new XMLHttpRequest(); 
                    xhr_weights.open("GET", bin_uri);
                    xhr_weights.responseType = "arraybuffer";            
                    xhr_weights.setRequestHeader('Range', `bytes=${bin_offset + info.weights_offset}-${bin_offset + info.weights_offset + num_pos*4*4 - 1}`);
                    xhr_weights.onload = load_weights;
                    xhr_weights.send();


                }));
            }

            await Promise.all(pendings);                      
            
            if (geo_set.normal_buf == null)
            {
                let p_pos3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_pos), p_pos3);

                let p_indices = 0;
                if (primitive_out.cpu_indices!=null)
                {
                    p_indices = this.module.ccall("alloc", "number", ["number"], [num_face*type_indices*3]);
                    this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_indices), p_indices);
                }

                let p_norm4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                this.module.ccall("zero", null, ["number", "number"], [p_norm4, num_pos*4*4]);                
                this.module.ccall("calc_normal", null, ["number", "number", "number", "number", "number", "number"], [num_face, num_pos, type_indices, p_indices, p_pos3, p_norm4]);
                geo_set.normal_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, usage0, p_norm4, num_pos*4*4);

                let p_norm3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                this.module.ccall("vec4_to_vec3", null, ["number", "number", "number"], [p_norm4, p_norm3, num_pos]);
                primitive_out.cpu_norm = new ArrayBuffer(num_pos*3*4);
                {
                    let view_src = new Uint8Array(this.module.HEAPU8.buffer, p_norm3, num_pos*3*4);
                    let view_dst = new Uint8Array(primitive_out.cpu_norm)
                    view_dst.set(view_src);
                }
                this.module.ccall("dealloc", null, ["number"], [p_norm3]);

                this.module.ccall("dealloc", null, ["number"], [p_pos3]);
                if (p_indices > 0)
                {
                    this.module.ccall("dealloc", null, ["number"], [p_indices]);
                }
                this.module.ccall("dealloc", null, ["number"], [p_norm4]);                
            }

            if (info.has_tangent)
            {                
                let p_pos3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_pos), p_pos3);

                let p_uv2 =  this.module.ccall("alloc", "number", ["number"], [num_pos*2*4]);
                this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_uv), p_uv2);

                let p_indices = 0;
                if (primitive_out.cpu_indices!=null)
                {
                    p_indices = this.module.ccall("alloc", "number", ["number"], [num_face*type_indices*3]);
                    this.module.HEAPU8.set(new Uint8Array(primitive_out.cpu_indices), p_indices);
                }

                let p_tangent4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                let p_bitangent4 = this.module.ccall("alloc", "number", ["number"], [num_pos*4*4]);
                this.module.ccall("zero", null, ["number", "number"], [p_tangent4, num_pos*4*4]);
                this.module.ccall("zero", null, ["number", "number"], [p_bitangent4, num_pos*4*4]);
                
                this.module.ccall("calc_tangent", null, ["number", "number", "number", "number", "number", "number", "number", "number"], [num_face, num_pos, type_indices, p_indices, p_pos3, p_uv2, p_tangent4, p_bitangent4]);                
                
                geo_set.tangent_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, usage0, p_tangent4, num_pos*4*4);
                geo_set.bitangent_buf = engine_ctx.createBuffer(this.module.HEAPU8.buffer, usage0, p_bitangent4, num_pos*4*4);

                let p_tangent3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                this.module.ccall("vec4_to_vec3", null, ["number", "number", "number"], [p_tangent4, p_tangent3, num_pos]);
                primitive_out.cpu_tangent = new ArrayBuffer(num_pos*3*4);
                {
                    let view_src = new Uint8Array(this.module.HEAPU8.buffer, p_tangent3, num_pos*3*4);
                    let view_dst = new Uint8Array(primitive_out.cpu_tangent)
                    view_dst.set(view_src);
                }
                this.module.ccall("dealloc", null, ["number"], [p_tangent3]);

                let p_bitangent3 = this.module.ccall("alloc", "number", ["number"], [num_pos*3*4]);
                this.module.ccall("vec4_to_vec3", null, ["number", "number", "number"], [p_bitangent4, p_bitangent3, num_pos]);
                primitive_out.cpu_bitangent = new ArrayBuffer(num_pos*3*4);
                {
                    let view_src = new Uint8Array(this.module.HEAPU8.buffer, p_bitangent3, num_pos*3*4);
                    let view_dst = new Uint8Array(primitive_out.cpu_bitangent)
                    view_dst.set(view_src);
                }
                this.module.ccall("dealloc", null, ["number"], [p_bitangent3]);

                this.module.ccall("dealloc", null, ["number"], [p_pos3]);
                this.module.ccall("dealloc", null, ["number"], [p_uv2]);
                if (p_indices > 0)
                {
                    this.module.ccall("dealloc", null, ["number"], [p_indices]);
                }
                this.module.ccall("dealloc", null, ["number"], [p_tangent4]);
                this.module.ccall("dealloc", null, ["number"], [p_bitangent4]);
                
            }           

            for (let k=1; k<num_geo_sets; k++)
            {
                let usage = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;
                if (k==num_geo_sets-1)
                {
                    usage|=GPUBufferUsage.VERTEX;
                }
                let commandEncoder = engine_ctx.device.createCommandEncoder();
                
                let geo_set2 = new GeometrySet();     
                primitive_out.geometry.push(geo_set2);
                
                geo_set2.pos_buf = engine_ctx.createBuffer0(geo_set.pos_buf.size, usage);                
                commandEncoder.copyBufferToBuffer(geo_set.pos_buf, 0, geo_set2.pos_buf, 0, geo_set.pos_buf.size);

                geo_set2.normal_buf = engine_ctx.createBuffer0(geo_set.normal_buf.size, usage);
                commandEncoder.copyBufferToBuffer(geo_set.normal_buf, 0, geo_set2.normal_buf, 0, geo_set.normal_buf.size);

                if (info.has_tangent)
                {
                    geo_set2.tangent_buf = engine_ctx.createBuffer0(geo_set.tangent_buf.size, usage);
                    commandEncoder.copyBufferToBuffer(geo_set.tangent_buf, 0, geo_set2.tangent_buf, 0, geo_set.tangent_buf.size);

                    geo_set2.bitangent_buf = engine_ctx.createBuffer0(geo_set.bitangent_buf.size, usage);
                    commandEncoder.copyBufferToBuffer(geo_set.bitangent_buf, 0, geo_set2.bitangent_buf, 0, geo_set.bitangent_buf.size);
                }                

                let cmdBuf = commandEncoder.finish();
                engine_ctx.queue.submit([cmdBuf]);
            }

            primitive_out.updateUUID();

            if (num_targets>0)
            {
                await load_targets(primitive_out, info);
            }            

        }

        const load_json = async ()=>
        {
            if (this.module == null) 
            {
                this.module = await get_module();
            }

            const json = xhr.response;  
            const buffer = json.buffers[0];
            if ('uri' in buffer)
            {
                bin_uri = buffer.uri;
                bin_offset = 0;
            }
            else
            {
                bin_offset = 20 + json_length + 8;
            }

            let num_meshes = json.meshes.length;
            let num_nodes = json.nodes.length;        
            let num_materials = json.materials.length;    
            let default_material_used = false;
            let skinned = new Array(num_meshes).fill(false);           
            for (let i=0; i<num_nodes; i++)
            {
                let node_in = json.nodes[i];
                if (node_in.mesh >= 0 && node_in.skin >= 0)
                {
                    skinned[node_in.mesh] = true;
                } 
            }

            if ("skins" in json)
            {
                let num_skins = json.skins.length;            
                for (let i=0; i<num_skins; i++)
                {
                    let skin_in = json.skins[i];
                    let skin_out = new Skin();
                    model.skins.push(skin_out);
                    let num_joints = skin_in.joints.length;                
                    skin_out.joints = skin_in.joints;
                    skin_out.buf_rela_mat = engine_ctx.createBuffer0(4*16*num_joints, GPUBufferUsage.STORAGE)
                    let acc_mats = json.accessors[skin_in.inverseBindMatrices];
                    let view_mats = json.bufferViews[acc_mats.bufferView];
                    
                    let xhr_skin;

                    const load_skin = ()=>{

                        let view_in = new Float32Array(xhr_skin.response);
                        for (let j=0; j<num_joints; j++)
                        {
                            let matrix = new Matrix4();
                            for (let k=0; k<16; k++)
                            {
                                matrix.elements[k] = view_in[j*16+k];
                            }
                            skin_out.inverseBindMatrices.push(matrix);
                        }                   
                    };
                
                    let offset = (view_mats.byteOffset||0) + (acc_mats.byteOffset||0);
                    xhr_skin = new XMLHttpRequest(); 
                    xhr_skin.open("GET", bin_uri);
                    xhr_skin.responseType = "arraybuffer";
                    xhr_skin.setRequestHeader('Range', `bytes=${bin_offset + offset}-${bin_offset + offset + view_mats.byteLength-1}`);
                    xhr_skin.onload = load_skin;
                    xhr_skin.send();
                }
            }

            material_affected_primitives = new Array(num_materials+1);
            for (let i=0; i<num_materials+1; i++)
            {
                material_affected_primitives[i] = new Set();
            }

            for (let i=0; i<num_meshes; i++)
            {
                let mesh_in = json.meshes[i];
                let mesh_out = new Mesh();
                model.meshes.push(mesh_out);
                let is_skinned = skinned[i];
                let num_primitives = mesh_in.primitives.length;

                let num_targets = 0;
                if (num_primitives>0 && ("targets" in mesh_in.primitives[0]))
                {
                    num_targets = mesh_in.primitives[0].targets.length;                    
                }         
                
                if (num_targets>0)
                {   
                    mesh_out.weights = new Array(num_targets).fill(0);
                    let init_weights = new Float32Array(num_targets);
                    mesh_out.buf_weights = engine_ctx.createBuffer(init_weights.buffer, GPUBufferUsage.STORAGE, 0, num_targets * 4);
                }

                for (let j=0; j<num_primitives; j++)
                {
                    let primitive_in = mesh_in.primitives[j];
                    let primitive_out = new Primitive();
                    mesh_out.primitives.push(primitive_out);
                    primitive_out.material_idx = primitive_in.material;
                    material_affected_primitives[primitive_in.material].add({i, j});
                    if (primitive_out.material_idx < 0)
                    {
                        primitive_out.material_idx = num_materials;
                        default_material_used = true;
                    }

                    let has_tangent = false;
                    {
                        let material = json.materials[primitive_in.material];
                        if ("normalTexture" in material)
                        {
                            if (material.normalTexture.index >=0)
                            {
                                has_tangent = true;
                            }
                        }
                    }                    
                    
                    primitive_out.num_targets = num_targets;

                    let info = {};
                    info.is_skinned = is_skinned;

                    if ("extensions" in primitive_in)
                    {
                        let extensions = primitive_in.extensions;
                        if ("KHR_draco_mesh_compression" in extensions)
                        {
                            let draco = extensions.KHR_draco_mesh_compression;                            
                            let draco_view = json.bufferViews[draco.bufferView];
                            info.draco_offset = draco_view.byteOffset||0;
                            info.draco_length = draco_view.byteLength;
                            info.draco_attributes = draco.attributes;
                        }
                    }
                    

                    let id_pos_in = primitive_in.attributes["POSITION"];                    
                    let acc_pos_in = json.accessors[id_pos_in];                    
                    primitive_out.num_pos = acc_pos_in.count;                    
                    primitive_out.min_pos.x = acc_pos_in.min[0];
                    primitive_out.min_pos.y = acc_pos_in.min[1];
                    primitive_out.min_pos.z = acc_pos_in.min[2];
                    primitive_out.max_pos.x = acc_pos_in.max[0];
                    primitive_out.max_pos.y = acc_pos_in.max[1];
                    primitive_out.max_pos.z = acc_pos_in.max[2];   
                    if ("bufferView" in acc_pos_in)
                    {                   
                        let view_pos_in = json.bufferViews[acc_pos_in.bufferView];                    
                        info.position_offset = (view_pos_in.byteOffset||0) + (acc_pos_in.byteOffset||0);
                    }
                    else
                    {
                        info.position_offset = -1;
                    }

                    let id_indices_in = primitive_in.indices;
                    if (id_indices_in>=0)
                    {
                        let acc_indices_in =  json.accessors[id_indices_in];
                        primitive_out.num_face = acc_indices_in.count/3;                        
                        if ("bufferView" in acc_indices_in)
                        {
                            let view_indices_in = json.bufferViews[acc_indices_in.bufferView];
                            info.indices_offset = (view_indices_in.byteOffset||0) + (acc_indices_in.byteOffset||0);
                        }
                        else
                        {
                            info.indices_offset = -1;
                        }

                        if(acc_indices_in.componentType == 5121)
                        {
                            primitive_out.type_indices = 1;
                        }
                        else if(acc_indices_in.componentType == 5123)
                        {
                            primitive_out.type_indices = 2;
                        }
                        else if(acc_indices_in.componentType == 5125)
                        {
                            primitive_out.type_indices = 4;
                        }
                    }
                    else
                    {
                        primitive_out.num_face = primitive_out.num_pos / 3;     
                        info.indices_offset = -1;
                    }
                    
                    if ("NORMAL" in primitive_in.attributes)
                    {
                        let id_norm_in = primitive_in.attributes["NORMAL"];
                        let acc_norm_in = json.accessors[id_norm_in];
                        if ("bufferView" in acc_norm_in)
                        {
                            let view_norm_in = json.bufferViews[acc_norm_in.bufferView];
                            info.normal_offset =  (view_norm_in.byteOffset||0) + (acc_norm_in.byteOffset||0);
                        }
                        else
                        {
                            info.normal_offset = -1;
                        }
                    }
                    else
                    {
                        info.normal_offset = -1;
                    }

                    if ("COLOR_0" in primitive_in.attributes)
                    {
                        let id_color_in = primitive_in.attributes["COLOR_0"];
                        let acc_color_in = json.accessors[id_color_in];                        
                        if ("bufferView" in acc_color_in)
                        {
                            let view_color_in = json.bufferViews[acc_color_in.bufferView];
                            info.color_offset = (view_color_in.byteOffset||0) + (acc_color_in.byteOffset||0);
                        }
                        else
                        {
                            info.color_offset = -1;
                        }
                        info.color_type = acc_color_in.type;
                        info.color_componentType = acc_color_in.componentType;
                    }
                    else
                    {
                        info.color_offset = -1;
                    }

                    if ("TEXCOORD_0" in primitive_in.attributes)
                    {
                        let id_uv_in = primitive_in.attributes["TEXCOORD_0"];
                        let acc_uv_in = json.accessors[id_uv_in];
                        if ("bufferView" in acc_uv_in)
                        {
                            let view_uv_in = json.bufferViews[acc_uv_in.bufferView];
                            info.uv_offset = (view_uv_in.byteOffset||0) + (acc_uv_in.byteOffset||0);
                        }
                        else
                        {
                            info.uv_offset = -1;                            
                        }
                    }
                    else
                    {
                        info.uv_offset = -1;
                    }       
                    
                    if ("JOINTS_0" in primitive_in.attributes)
                    {
                        let id_joints_in = primitive_in.attributes["JOINTS_0"];
                        let acc_joints_in = json.accessors[id_joints_in];
                        if ("bufferView" in acc_joints_in)
                        {
                            let view_joints_in = json.bufferViews[acc_joints_in.bufferView];
                            info.joints_offset = (view_joints_in.byteOffset||0) + (acc_joints_in.byteOffset||0);
                        }
                        else
                        {
                            info.joints_offset = -1;
                        }
                        info.joints_componentType = acc_joints_in.componentType;
                    }
                    else
                    {
                        info.joints_offset = -1;
                    }

                    if ("WEIGHTS_0" in primitive_in.attributes)
                    {
                        let id_weights_in = primitive_in.attributes["WEIGHTS_0"];
                        let acc_weights_in = json.accessors[id_weights_in];
                        if ("bufferView" in acc_weights_in)
                        {
                            let view_weights_in = json.bufferViews[acc_weights_in.bufferView];
                            info.weights_offset = (view_weights_in.byteOffset||0) + (acc_weights_in.byteOffset||0);
                        }
                        else
                        {
                            info.weights_offset = -1;
                        }
                    }
                    else
                    {
                        info.weights_offset = -1;
                    }

                    info.has_tangent = has_tangent;

                    if (num_targets > 0)
			        {
                        info.targets = new Array(num_targets);

                        let prim_is_sparse  = true;

                        for (let k=0; k<num_targets; k++)
                        {
                            let target_in = primitive_in.targets[k];
                            info.targets[k] = {};
                            let target_info =  info.targets[k];
                            
                            let id_t_pos_in =  target_in["POSITION"];
                            let acc_t_pos_in = json.accessors[id_t_pos_in];
                            let pos_is_sparse = false;
                            if ("sparse" in acc_t_pos_in)
                            {
                                pos_is_sparse = true;
                            }
                            target_info.pos_is_sparse = pos_is_sparse;

                            if (!pos_is_sparse)
                            {
                                prim_is_sparse = false;
                                let view_t_pos_in =  json.bufferViews[acc_t_pos_in.bufferView];
                                target_info.position_offset = (view_t_pos_in.byteOffset||0) + (acc_t_pos_in.byteOffset||0);
                            }
                            else
                            {
                                let count_idx = acc_t_pos_in.sparse.count;
                                target_info.position_count = count_idx;

                                let view_idx = json.bufferViews[acc_t_pos_in.sparse.indices.bufferView];
                                target_info.position_idx_offset = (view_idx.byteOffset||0) + (acc_t_pos_in.sparse.indices.byteOffset||0);
                                target_info.position_idx_type =  acc_t_pos_in.sparse.indices.componentType;
                                
                                let view_value = json.bufferViews[acc_t_pos_in.sparse.values.bufferView];
                                target_info.position_value_offset = (view_value.byteOffset||0) + (acc_t_pos_in.sparse.values.byteOffset||0);                                
                            }

                            if ("NORMAL" in target_in)
                            {
                                target_info.has_normal = true;
                                let id_t_norm_in = target_in["NORMAL"];
                                let acc_t_norm_in = json.accessors[id_t_norm_in];
                                let norm_is_sparse = false;
                                if ("sparse" in acc_t_norm_in)
                                {
                                    norm_is_sparse = true;
                                }
                                target_info.norm_is_sparse = norm_is_sparse;
                                if (!norm_is_sparse)
                                {
                                    prim_is_sparse = false;
                                    let view_t_norm_in =  json.bufferViews[acc_t_norm_in.bufferView];
                                    target_info.normal_offset = (view_t_norm_in.byteOffset||0) + (acc_t_norm_in.byteOffset||0);
                                }
                                else
                                {
                                    let count_idx = acc_t_norm_in.sparse.count;
                                    target_info.normal_count = count_idx;

                                    let view_idx = json.bufferViews[acc_t_norm_in.sparse.indices.bufferView];
                                    target_info.normal_idx_offset = (view_idx.byteOffset||0) + (acc_t_norm_in.sparse.indices.byteOffset||0);
                                    target_info.normal_idx_type =  acc_t_norm_in.sparse.indices.componentType;

                                    let view_value = json.bufferViews[acc_t_norm_in.sparse.values.bufferView];
                                    target_info.normal_value_offset = (view_value.byteOffset||0) + (acc_t_norm_in.sparse.values.byteOffset||0);
                                }
                            }
                            else
                            {
                                target_info.has_normal = false;
                            }                            
                        }
                        info.is_sparse = prim_is_sparse;
                    }                  
                    (async()=>{
                        await load_primitive(primitive_out, info);
                        if (num_targets>0)
                        {
                            primitive_out.create_bind_group_morph(mesh_out.buf_weights);
                        }
                    })();                     
                }
            }

            for (let i=0; i< num_nodes; i++)
            {
                let node_in = json.nodes[i];
                let node_out = new Node();
                model.nodes.push(node_out);
                if ("children" in node_in)
                {
                    node_out.children = node_in.children;
                }

                if ("matrix" in node_in)
                {
                    let matrix = new Matrix4();
                    for (let c =0; c<16; c++)
                    {
                        matrix.elements[c] =  node_in.matrix[c];
                    }
                    matrix.decompose(node_out.translation, node_out.rotation, node_out.scale);
                }
                else
                {
                    if ("translation" in node_in)
                    {
                        node_out.translation.set(node_in.translation[0],node_in.translation[1],node_in.translation[2]);
                    }                    

                    if ("rotation" in node_in)
                    {
                        node_out.rotation.set(node_in.rotation[0],node_in.rotation[1],node_in.rotation[2],node_in.rotation[3]);
                    }

                    if ("scale" in node_in)
                    {
                        node_out.scale.set(node_in.scale[0],node_in.scale[1],node_in.scale[2]);
                    }
                }

                let name = "";
                if ("name" in node_in)
                {
                    name =  node_in.name;
                }
                if (name == "")
                {
                    name =`node_${i}`;
                }
                model.node_dict[name] = i;
            }                         
            
            model.roots = json.scenes[0].nodes;            

            for (let i=0; i< num_nodes; i++)
            {
                let node_in = json.nodes[i];                
                let j = -1;
                if ("mesh" in node_in)
                {
                    j = node_in.mesh;
                }

                if (j>=0)
                {
                    let mesh_out = model.meshes[j];
                    mesh_out.node_id = i;
                    if ("skin" in node_in)
                    {
                        mesh_out.skin_id = node_in.skin;
                    }

                    let name = "";
                    if ("name" in node_in)
                    {
                        name =  node_in.name;
                    }
                    if (name == "")
                    {
                        name = `mesh_${j}`;                        
                    }
                    model.mesh_dict[name] = j;
                }
            }

            model.updateNodes();            
            
            let num_textures = 0;
            if ("textures" in json)
            {            
                num_textures = json.textures.length;
            }
            let tex_opts = new Array(num_textures);
            tex_affected_materials = new Array(num_textures);          
            for (let i=0; i< num_textures; i++)
            {
                tex_opts[i] = {
                    is_srgb: true,                    
                };

                tex_affected_materials[i] = new Set();                
            }            
            
            for (let i=0; i< num_materials; i++)
            {                
                let material_in = json.materials[i];
                let material_out = new MeshStandardMaterial();
                model.materials.push(material_out);
                if (material_in.alphaMode == "OPAQUE")
                {
                    material_out.alphaMode = "Opaque";
                }
                else if (material_in.alphaMode == "MASK")
                {
                    material_out.alphaMode = "Mask";
                }
                else if (material_in.alphaMode == "BLEND")
                {
                    material_out.alphaMode = "Blend";
                }
                if ("alphaCutoff" in material_in)
                {
                    material_out.alphaCutoff = material_in.alphaCutoff;
                }
                if ("doubleSided" in material_in)
                {
                    material_out.doubleSided = material_in.doubleSided;
                }

                if ("pbrMetallicRoughness" in material_in)
                {
                    let pbr = material_in.pbrMetallicRoughness;
                    if ("baseColorFactor" in pbr)
                    {                
                        let color_in = pbr.baseColorFactor;
                        material_out.color.r = color_in[0];
                        material_out.color.g = color_in[1];
                        material_out.color.b = color_in[2];
                        material_out.alpha = color_in[3];
                    }

                    if ("baseColorTexture" in pbr)
                    {
                        if (pbr.baseColorTexture.index >= 0)
                        {
                            material_out.tex_idx_map = pbr.baseColorTexture.index;
                            tex_affected_materials[material_out.tex_idx_map].add(i);
                        }
                    }
                   
                    if ("metallicFactor" in pbr)
                    {
                        material_out.metallicFactor = pbr.metallicFactor;
                    }
                    else
                    {
                        material_out.metallicFactor = 1.0;
                    }

                    if ("roughnessFactor" in pbr)
                    {
                        material_out.roughnessFactor = pbr.roughnessFactor;
                    }
                    else
                    {
                        material_out.roughnessFactor = 1.0;
                    }

                    if ("metallicRoughnessTexture" in pbr)
                    {
                        let id_mr = pbr.metallicRoughnessTexture.index;
                        if (id_mr>=0)
                        {
                            tex_opts[id_mr].is_srgb = false;
                            material_out.tex_idx_metalnessMap = id_mr;			              
                            tex_affected_materials[id_mr].add(i);
                        }
                    }
                }    
                
                if ("normalTexture" in material_in)
                {
                    if (material_in.normalTexture.index >=0)
                    {   
                        tex_opts[material_in.normalTexture.index].is_srgb = false;
                        material_out.tex_idx_normalMap = material_in.normalTexture.index;
                        tex_affected_materials[material_out.tex_idx_normalMap].add(i);
                        if ("scale" in material_in.normalTexture)
                        {
                            let scale = material_in.normalTexture.scale;
                            material_out.normalScale.x = scale;
                            material_out.normalScale.y = scale;                               
                        }
                    }
                }

                if ("emissiveFactor" in material_in)
                {
                    material_out.emissive.x = material_in.emissiveFactor[0];
                    material_out.emissive.y = material_in.emissiveFactor[1];
                    material_out.emissive.z = material_in.emissiveFactor[2];
                }

                if ("emissiveTexture" in material_in)
                {
                    if (material_in.emissiveTexture.index>=0)
                    {
                        material_out.tex_idx_emissiveMap = material_in.emissiveTexture.index;
                        tex_affected_materials[material_out.tex_idx_emissiveMap].add(i);
                    }
                }

                if ("extensions" in material_in)
                {
                    let extensions = material_in.extensions;
                    if ("KHR_materials_emissive_strength" in extensions)
                    {
                        let emissive_stength = extensions.KHR_materials_emissive_strength;
                        let strength = emissive_stength.emissiveStrength;
                        material_out.multiplyScalar(strength);
                    }
                    if ("KHR_materials_pbrSpecularGlossiness" in extensions)
                    {
                        material_out.specular_glossiness = true;
                        let sg = extensions.KHR_materials_pbrSpecularGlossiness;
                        if ("diffuseFactor" in sg)
                        {                         
                            let color_in = sg.diffuseFactor;
                            material_out.color.r = color_in[0];
                            material_out.color.g = color_in[1];
                            material_out.color.b = color_in[2];
                            material_out.alpha = color_in[3];
                        }

                        if ("diffuseTexture" in sg)
                        {
                            if (sg.diffuseTexture.index>=0)
                            {
                                material_out.tex_idx_map = sg.diffuseTexture.index;
                                tex_affected_materials[material_out.tex_idx_map].add(i);
                            }
                        }

                        if ("glossinessFactor" in sg)
                        {
                            material_out.glossinessFactor = sg.glossinessFactor;
                        }

                        if ("specularFactor" in sg)
                        {
                            let color_in = sg.specularFactor;
                            material_out.specular.r = color_in[0];
                            material_out.specular.g = color_in[1];
                            material_out.specular.b = color_in[2];
                        }

                        if ("specularGlossinessTexture" in sg)
                        {
                            let id_sg =  sg.specularGlossinessTexture.index;
                            if (id_sg >=0)
                            {
                                material_out.tex_idx_specularMap = id_sg;                                
                                tex_affected_materials[id_sg].add(i);
                            }
                        }
                    }
                }

                material_out.update_constant();

                let affected_primitives = material_affected_primitives[i];
                for (let {i,j} of affected_primitives)
                {
                    let mesh = model.meshes[i];
                    let prim = mesh.primitives[j];
                    prim.create_bind_group(mesh.constant, model.materials, model.textures);
                }
                
            }
            
            if (default_material_used)
            {
                let material_out = new MeshStandardMaterial();
                model.materials.push(material_out);
                material_out.update_constant();

                let affected_primitives = material_affected_primitives[num_materials];
                for (let {i,j} of affected_primitives)
                {
                    let mesh = model.meshes[i];
                    let prim = mesh.primitives[j];
                    prim.create_bind_group(mesh.constant, model.materials, model.textures);
                }
            }

            for (let i=0; i< num_textures; i++)
            {
                let tex_in = json.textures[i];
                let img_in = json.images[tex_in.source];
                model.tex_dict[img_in.name] = i;
                let opts = tex_opts[i];

                if ("bufferView" in img_in)
                {
                    let view = json.bufferViews[img_in.bufferView];
                    let offset = view.byteOffset || 0;                    
                    let xhr_img = new XMLHttpRequest(); 
                    xhr_img.open("GET", bin_uri);
                    xhr_img.responseType = "blob";
                    xhr_img.setRequestHeader('Range', `bytes=${bin_offset + offset}-${bin_offset + offset + view.byteLength-1}`);
                    xhr_img.onload = ()=> load_image(xhr_img.response, i, opts);
                    xhr_img.send();
                }
            }

            if ("pending_frame" in model)
            {
                model.setAnimationFrame(model.pending_frame, true);
                delete model.pending_frame;
            }
            
        };

        const load_header = ()=>
        {
            const arrBuf = xhr.response;  
            let offset = 0;
            let magic = new TextDecoder().decode(new Uint8Array(arrBuf,offset,4));
            offset+=4;
            if (magic!="glTF")
            {
                console.log(`${url} is not a GLTF file.`);
                return;
            }            

            let data_view = new DataView(arrBuf);            
            gltf_version = data_view.getInt32(offset, true);
            offset+=4;
            file_length = data_view.getInt32(offset, true);
            offset+=4;

            json_length = data_view.getInt32(offset, true);
            offset+=4;
            let chunk_type = new TextDecoder().decode(new Uint8Array(arrBuf,offset,4));
            offset+=4;

            xhr = new XMLHttpRequest(); 
            xhr.open("GET", url);
            xhr.responseType = "json";
            xhr.setRequestHeader('Range', `bytes=20-${20+json_length-1}`);
            xhr.onload = load_json;
            xhr.send();

           
        };


        let ext =  get_url_extension(url);
        if (ext=="glb")
        {
            xhr = new XMLHttpRequest(); 
            xhr.open("GET", url);
            xhr.responseType = "arraybuffer";
            xhr.setRequestHeader('Range', `bytes=0-19`);
            xhr.onload = load_header;
            xhr.send();
        }


        return model;
        
    }

}

