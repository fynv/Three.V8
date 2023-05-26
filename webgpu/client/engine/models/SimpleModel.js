import { Object3D } from "../core/Object3D.js"
import { MeshStandardMaterial } from "../materials/MeshStandardMaterial.js"
import { CreateBindGroup_Model, UpdateConstant_Model, GeometrySet, Primitive } from "./ModelComponents.js"
import * as MathUtils from '../math/MathUtils.js';
import get_module from './GeoGen.js'
import { CreateTexture } from "../renderers/GPUUtils.js"

export class SimpleModel extends Object3D
{
    constructor()
    {
        super();
        this.textures = [];
        this.material = new MeshStandardMaterial();
        this.geometry = new Primitive();
        this.constant = engine_ctx.createBuffer0(128, GPUBufferUsage.UNIFORM|GPUBufferUsage.COPY_DST);
        this.bind_group = CreateBindGroup_Model(this.constant);       
        this.setColor(0.8, 0.8, 0.8);
        this.geometry.material_idx = 0;

    }

    updateConstant()
    {
        UpdateConstant_Model(this.constant, this.matrixWorld);
    }

    _create(geoGen, p_geo)
    {
        this.geometry.num_pos = geoGen.ccall("GeoGetNumPos", "number", ["number"], [p_geo]);
        this.geometry.num_face = geoGen.ccall("GeoGetNumFace", "number", ["number"], [p_geo]);
        this.geometry.type_indices = 4;        

        let geo_set = new GeometrySet();

        let p_pos = geoGen.ccall("GeoGetPosition", "number", ["number"], [p_geo]);
        geo_set.pos_buf = engine_ctx.createBuffer(geoGen.HEAPU8.buffer, GPUBufferUsage.VERTEX, p_pos, this.geometry.num_pos*4*4);

        let p_norm = geoGen.ccall("GeoGetNormal", "number", ["number"], [p_geo]);
        geo_set.normal_buf = engine_ctx.createBuffer(geoGen.HEAPU8.buffer, GPUBufferUsage.VERTEX, p_norm, this.geometry.num_pos*4*4);        

        this.geometry.geometry[0] = geo_set;

        let p_uv = geoGen.ccall("GeoGetUV", "number", ["number"], [p_geo]);
        this.geometry.uv_buf = engine_ctx.createBuffer(geoGen.HEAPU8.buffer, GPUBufferUsage.VERTEX, p_uv, this.geometry.num_pos*2*4);

        let p_faces = geoGen.ccall("GeoGetFaces", "number", ["number"], [p_geo]);
        this.geometry.index_buf = engine_ctx.createBuffer(geoGen.HEAPU8.buffer, GPUBufferUsage.INDEX, p_faces, this.geometry.num_face*3*4);

        let data_view = new DataView(geoGen.HEAPU8.buffer);

        let p_min_pos = geoGen.ccall("GeoGetMinPos", "number", ["number"], [p_geo]);
        this.geometry.min_pos.x =  data_view.getFloat32(p_min_pos, true);
        this.geometry.min_pos.y =  data_view.getFloat32(p_min_pos + 4, true);
        this.geometry.min_pos.z =  data_view.getFloat32(p_min_pos + 8, true);

        let p_max_pos = geoGen.ccall("GeoGetMaxPos", "number", ["number"], [p_geo]);
        this.geometry.max_pos.x =  data_view.getFloat32(p_max_pos, true);
        this.geometry.max_pos.y =  data_view.getFloat32(p_max_pos + 4, true);
        this.geometry.max_pos.z =  data_view.getFloat32(p_max_pos + 8, true);

        geoGen.ccall("GeoDelete", null, ["number"], [p_geo]);

        this.uuid = MathUtils.generateUUID();

    }

    async createBox(width, height, depth)
    {
        await engine_ctx.initialize();
        let geoGen = await get_module();
        let p_geo = geoGen.ccall("CreateBox", "number", ["number", "number", "number"], [width, height, depth]);
        this._create(geoGen, p_geo);
    }
    
    async createSphere(radius, widthSegments = 32, heightSegments = 16)
    {
        await engine_ctx.initialize();
        let geoGen = await get_module();
        let p_geo = geoGen.ccall("CreateSphere", "number", ["number", "number", "number"], [radius, widthSegments, heightSegments]);
        this._create(geoGen, p_geo);
        
    }

    async createPlane(width, height)
    {
        await engine_ctx.initialize();
        let geoGen = await get_module();
        let p_geo = geoGen.ccall("CreatePlane", "number", ["number", "number"], [width, height]);
        this._create(geoGen, p_geo);

    }

    get color()
    {
        return this.material.color;
    }

    setColor( r, g, b )
    {        
        if ( g === undefined && b === undefined ) 
        {
            this.material.color.copy(r);
        }
        else
        {
            this.material.color.setRGB(r,g,b);
        }        
        this.material.update_constant();
    }

    setColorTexture(image)
    {
        let texture = CreateTexture(image, true);
        this.textures[0] = texture;
        this.material.tex_idx_map = 0;
        this.material.create_binding_group(this.textures);
    }

}

