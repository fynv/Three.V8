﻿import { OrbitControls } from "./controls/OrbitControls.js";
import { Vector3 } from "./math/Vector3.js";
import { Vector4 } from "./math/Vector4.js";
import { Matrix4 } from "./math/Matrix4.js";
import { view } from "./view.js";

import * as txml from "./txml.js";
import { genXML } from "./genXML.js";

function uuid(len, radix) {
    var chars = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'.split('');
    var uuid = [], i;
    radix = radix || chars.length;
 
    if (len) {
      // Compact form
      for (i = 0; i < len; i++) uuid[i] = chars[0 | Math.random()*radix];
    } else {
      // rfc4122, version 4 form
      var r;
 
      // rfc4122 requires these characters
      uuid[8] = uuid[13] = uuid[18] = uuid[23] = '-';
      uuid[14] = '4';
 
      // Fill in random data.  At i==19 set the high bits of clock sequence as
      // per rfc4122, sec. 4.1.5
      for (i = 0; i < 36; i++) {
        if (!uuid[i]) {
          r = 0 | Math.random()*16;
          uuid[i] = chars[(i == 19) ? (r & 0x3) | 0x8 : r];
        }
      }
    }
 
    return uuid.join('');
}

function string_to_boolean(string) {
    switch (string.toLowerCase().trim()) {
        case "true":
        case "yes":
        case "1":
            return true;

        case "false":
        case "no":
        case "0":
        case null:
            return false;

        default:
            return Boolean(string);
    }
}

class EnvMapGen
{
    constructor(doc, proxy, xml_node)
    {
        this.doc = doc;
        this.proxy = proxy;
        this.xml_node = xml_node;
        
        this.cube_target = new CubeRenderTarget(128,128);
        this.envMapCreator = new EnvironmentMapCreator();
        this.iter = 0;
    }
    
    render(renderer)
    {
        print(`Building environemnt map, iteration: ${this.iter}`);
        let props = this.xml_node.attributes;
        
        let x = 0.0;
        let y = 0.0;
        let z = 0.0;
        if ("probe_position" in props)
        {
            let probe_position = props.probe_position;
            let position = probe_position.split(',');
            x = parseFloat(position[0]);
            y = parseFloat(position[1]);
            z = parseFloat(position[2]);
        }
        
        let in_scene = this.proxy.parent == this.doc.scene;
        if (in_scene) this.doc.scene.remove(this.proxy);
        renderer.renderCube(this.doc.scene, this.cube_target, new Vector3(x, y,z));
        if (in_scene) this.doc.scene.add(this.proxy);
        
        let envLight = this.envMapCreator.create(this.cube_target);
        this.doc.scene.indirectLight = envLight;
        
        this.iter++;
        if (this.iter > 5)
        {
            print("Saving environemnt map.");
            let down_img = this.cube_target.getCubeImage();
            
            const url = props.path;
                    
            imageSaver.saveCubeToFile(down_img, 
                url+"/"+props.posx, url+"/"+props.negx, 
                url+"/"+props.posy, url+"/"+props.negy, 
                url+"/"+props.posz, url+"/"+props.negz);
            
            this.doc.env_gen = null;
        }
    }
    
}

// Tags
const scene = {
    reset: (doc) => {
        doc.scene = new Scene();
    },
    create: async (doc, props, mode, parent) => {
        doc.scene = new Scene();
        return doc.scene;
    }
}

const create_default_controls = (doc)=>{
    if (doc.controls)
        doc.controls.dispose();
    doc.controls = new OrbitControls(doc.camera, doc.view);
    doc.controls.enableDamping = true;
    doc.controls.target.set(0, 1.5, 0);
}


const camera = {
    reset: (doc) => {
        doc.camera = new PerspectiveCamera(45, doc.width / doc.height, 0.1, 100);
        doc.camera.setPosition(0, 1.5, 5.0);
    },

    create: async (doc, props, mode, parent) => {
        const fov = parseFloat(props.fov);
        const near = parseFloat(props.near);
        const far = parseFloat(props.far);
        doc.camera = new PerspectiveCamera(fov, doc.width / doc.height, near, far);
        create_default_controls(doc);
        return doc.camera;
    }
}

const control = {
    reset: (doc) => {
        create_default_controls(doc);
    },
    create: async (doc, props, mode, parent) =>{
        const type = props.type;
        if (type == 'orbit') {
            const look_from = props.look_from.split(',');
            const look_at = props.look_at.split(',');
            const from_x = parseFloat(look_from[0]);
            const from_y = parseFloat(look_from[1]);
            const from_z = parseFloat(look_from[2]);
            const to_x = parseFloat(look_at[0]);
            const to_y = parseFloat(look_at[1]);
            const to_z = parseFloat(look_at[2]);
            doc.camera.setPosition(from_x, from_y, from_z);
            if (doc.controls != null)
                doc.controls.dispose();
            doc.controls = new OrbitControls(doc.camera, doc.view);
            doc.controls.enableDamping = true;
            doc.controls.target.set(to_x, to_y, to_z);
        }
    }
}

const fog = {
    create: async (doc, props, mode, parent) =>{        
        doc.scene.fog = new Fog();
        if (props.hasOwnProperty("density"))
        {
            doc.scene.fog.density = parseFloat(props.density);
        }
        return doc.scene.fog;
    }
}


const sky = {
    reset: (doc) => {
        let bg = new HemisphereBackground();   
        bg.setSkyColor(0.318, 0.318, 0.318);
        bg.setGroundColor(0.01, 0.025, 0.025);
        doc.scene.background = bg;
    },
    create: async (doc, props, mode, parent) => {
        const type = props.type;
        let create_env_light = true;
        if (props.hasOwnProperty("create_env_light"))
        {
            create_env_light = string_to_boolean(props.create_env_light);
        }
        
        if (type == "uniform")
        {
            let bg = new ColorBackground();
            let envLight = null;
            if (create_env_light) envLight = new AmbientLight();
            
            if (props.hasOwnProperty('color'))
            {
                const color = props.color.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.setColor(r,g,b);
                if (create_env_light)
                {
                    envLight.setColor(r,g,b);
                }
            }
            doc.scene.background = bg;
            if (create_env_light)
            {
                doc.scene.indirectLight = envLight;
            }
        }
        else if (type == "hemisphere")
        {
            let bg = new HemisphereBackground();   
            let envLight = null;
            if (create_env_light) envLight = new HemisphereLight();
            
            if (props.hasOwnProperty('skyColor'))
            {
                const color = props.skyColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.setSkyColor(r,g,b);
                if (create_env_light)
                {
                    envLight.setSkyColor(r,g,b);
                }
            }
            
            if (props.hasOwnProperty('groundColor'))
            {
                const color = props.groundColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.setGroundColor(r,g,b);               
                envLight.setGroundColor(r,g,b);
            }
            
            doc.scene.background = bg;
            if (create_env_light)
            {
                doc.scene.indirectLight = envLight;
            }
        }
        else if (type == "cube")
        {
            const url = props.path;

            let cube_img = imageLoader.loadCubeFromFile(
                url+"/"+props.posx, url+"/"+props.negx, 
                url+"/"+props.posy, url+"/"+props.negy, 
                url+"/"+props.posz, url+"/"+props.negz);

            let bg = new CubeBackground();
            bg.setCubemap(cube_img);
            doc.scene.background = bg;
            
            if(create_env_light)
            {
                let envMapCreator = new EnvironmentMapCreator();
                let envLight = envMapCreator.create(cube_img);
                doc.scene.indirectLight = envLight;
            }
        }
        
        return doc.scene.background;
    }
}

const env_light = {
    create: async (doc, props, mode, parent) => {
        const type = props.type;
        
        if (type == "uniform")
        {
            let envLight = new AmbientLight();          
            if (props.hasOwnProperty('color'))
            {
                const color = props.color.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                envLight.setColor(r,g,b);
            }
            doc.scene.indirectLight = envLight;
        }
        else if (type == "hemisphere")
        {
            let envLight = new HemisphereLight();
            
            if (props.hasOwnProperty('skyColor'))
            {
                const color = props.skyColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                envLight.setSkyColor(r,g,b);
            }
            
            if (props.hasOwnProperty('groundColor'))
            {
                const color = props.groundColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                envLight.setGroundColor(r,g,b);
            }           
            doc.scene.indirectLight = envLight;
        }
        else if (type == "cube")
        {
            const proxy = new SimpleModel();
            proxy.createBox(0.3, 0.3, 0.3);
            
            if (props.hasOwnProperty('probe_position')) 
            {
                const position = props.probe_position.split(',');
                proxy.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
            }
            proxy.setColor(0.7,0.0,0.7);
            doc.scene.add(proxy);
            doc.add_hitable_object(proxy);
            
            const url = props.path;

            let cube_img = imageLoader.loadCubeFromFile(
                url+"/"+props.posx, url+"/"+props.negx, 
                url+"/"+props.posy, url+"/"+props.negy, 
                url+"/"+props.posz, url+"/"+props.negz);
                
            let envLight = null;
            if (cube_img!=null)
            {
                let envMapCreator = new EnvironmentMapCreator();
                envLight = envMapCreator.create(cube_img);
            }
            doc.scene.indirectLight = envLight;
            
            return proxy;
        }
        
        return doc.scene.indirectLight;
    },
    
    tuning: (doc, obj, args) => {
        let input = JSON.parse(args);
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        if (props.type == "cube")
        {
            if ("probe_position" in input)
            {
                let probe_position = input.probe_position;
                props.probe_position = probe_position;
                let position = probe_position.split(',');
                obj.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
                doc.generate_bvh();
            }
            
            let reload = false;
            if ("path" in input)
            {
                props.path = input.path;
                reload = true;
            }
            if ("posx" in input)
            {
                props.posx = input.posx;
                reload = true;
            }
            if ("negx" in input)
            {
                props.negx = input.negx;
                reload = true;
            }
            if ("posy" in input)
            {
                props.posy = input.posy;
                reload = true;
            }
            if ("negy" in input)
            {
                props.negy = input.negy;
                reload = true;
            }
            if ("posz" in input)
            {
                props.posz = input.posz;
                reload = true;
            }
            if ("negz" in input)
            {
                props.negz = input.negz;
                reload = true;
            }
            if (reload)
            {
                const url = props.path;
    
                let cube_img = imageLoader.loadCubeFromFile(
                    url+"/"+props.posx, url+"/"+props.negx, 
                    url+"/"+props.posy, url+"/"+props.negy, 
                    url+"/"+props.posz, url+"/"+props.negz);
                    
                let envLight = null;
                if (cube_img!=null)
                {
                    let envMapCreator = new EnvironmentMapCreator();
                    envLight = envMapCreator.create(cube_img);
                }
                doc.scene.indirectLight = envLight;
            }
        }
        
    },
    
    generate: (doc, obj) =>{
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        if (props.type == "cube")
        {
            doc.env_gen = new EnvMapGen(doc, obj, node);
        }
    }
}

const group = {
    create: async (doc, props, mode, parent) => {
        const group = new Object3D();
        if (parent != null) {
            parent.add(group);
        }
        else {
            doc.scene.add(group);
        }
        return group;
    }
}

const plane = {
    create: async (doc, props, mode, parent) => {
        const size = props.size.split(',');
        const width = parseFloat(size[0]);
        const height = parseFloat(size[1]);
                
        const plane = new SimpleModel();
        plane.createPlane(width, height);

        if (parent != null) {
            parent.add(plane);
        }
        else {
            doc.scene.add(plane);
        }
        return plane;
    }
}


const box = {
    create: async (doc, props, mode, parent) => {
        const size = props.size.split(',');
        const width = parseFloat(size[0]);
        const height = parseFloat(size[1]);
        const depth = parseFloat(size[2]);
        
        const box = new SimpleModel();
        box.createBox(width, height, depth);

        if (parent != null) {
            parent.add(box);
        }
        else {
            doc.scene.add(box);
        }
        return box;
    }
}

const sphere = {
    create: async (doc, props, mode, parent) => {
        const radius = parseFloat(props.radius);
        const widthSegments = parseInt(props.widthSegments);
        const heightSegments = parseInt(props.heightSegments);
        
        const sphere = new SimpleModel();
        sphere.createSphere(radius, widthSegments, heightSegments);
        
        if (parent != null) {
            parent.add(sphere);
        }
        else {
            doc.scene.add(sphere);
        }
        return sphere;
    }
}

const model = {
    create: async (doc, props, mode, parent) => {
        const url = props.src;
        const model = gltfLoader.loadModelFromFile(url);
        if (parent != null) {
            parent.add(model);
        }
        else {
            doc.scene.add(model);
        }
        return model;
    }
}

const avatar = {
    create: async (doc, props, mode, parent) => {
        let avatar = await model.create(doc, { ...props}, mode, parent);
        return avatar;
    }
}


const directional_light = {
    create: async (doc, props, mode, parent) => {
        const light = new DirectionalLight();           
        
        if (props.hasOwnProperty('intensity')) {
            light.intensity = parseFloat(props.intensity);
        }       
        
        if (props.hasOwnProperty('castShadow') && string_to_boolean(props.castShadow))
        {
            let width = 512;
            let height = 512;
            if (props.hasOwnProperty('size')) {
                const size = props.size.split(',');
                width = parseInt(size[0]);
                height = parseInt(size[1]);
            }
            light.setShadow(true, width, height);
            
            if (props.hasOwnProperty('area')) {
                const area = props.area.split(',');
                let left = parseFloat(area[0]);
                let right = parseFloat(area[1]);
                let top = parseFloat(area[2]);
                let bottom = parseFloat(area[3]);       
                let near = parseFloat(area[4]);
                let far = parseFloat(area[5]);
                light.setShadowProjection(left, right, top, bottom, near, far);
            }
        }

        if (parent != null) {
            parent.add(light);
        }
        else {
            doc.scene.add(light);
        }
        return light;
    }
}


export class Document
{
    constructor(view)
    {
        this.view = view;
        this.width = view.clientWidth;
        this.height = view.clientHeight;
        this.Tags = { scene, camera, fog, sky, env_light, control, group, plane, box, sphere, model, avatar, directional_light };
        this.hitable_tags = { plane, box, sphere, model, avatar };
        this.reset();
    }
    
    setSize(width, height)
    {
        this.width = width;
        this.height = height;
        
        if (this.camera)
        {
            this.camera.aspect = width / height;
            this.camera.updateProjectionMatrix();
        }
    }

    reset() 
    {
        this.saved_text = "";
        this.hitables = [];
        
        if (this.picked_obj!=null)
        {
            gamePlayer.message("object_picked", "");
            this.picked_obj = null;
        }
        
        for (let tag in this.Tags) 
        {
            if (this.Tags[tag].hasOwnProperty('reset')) 
            {
                this.Tags[tag].reset(this);
            }
        }
        
        this.internal_index = {};
        this.external_index = {};
        this.external_index.index = {};
        
        this.env_gen = null;
    }
    
    tick(delta)
    {
        if (this.controls)
        {
            if (this.controls.hasOwnProperty('update'))
            {
                this.controls.update();
            }
        }
    }
    
    render(renderer)
    {
        if (this.env_gen!=null)
        {
            this.env_gen.render(renderer);
        }
        
        if (this.scene && this.camera) 
        {
            renderer.render(this.scene, this.camera);
        }
    }
    
    add_hitable_object(obj) {
        this.hitables.push(obj);
    }

    remove_hitable_object(obj) {
        for (let i = 0; i < this.hitables.length; i++) {
            if (this.hitables[i] == obj) {
                this.hitables.splice(i, 1);
                i--;
            }
        }
    }
    
    
    async create(tag, props, mode, parent = null) 
    {
        if (!(tag in this.Tags)) return null;
        
        const obj = await this.Tags[tag].create(this, props, mode, parent);
        if (obj == null) return null;
        
        obj.uuid = uuid();
        
        if (props.hasOwnProperty('name')) 
        {
            obj.name = props.name;
        }
        
        if (props.hasOwnProperty('position')) 
        {
            const position = props.position.split(',');
            obj.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
        }

        if (props.hasOwnProperty('rotation')) 
        {
            const rotation = props.rotation.split(',');
            obj.setRotation(parseFloat(rotation[0])* Math.PI / 180.0, parseFloat(rotation[1])* Math.PI / 180.0, parseFloat(rotation[2])* Math.PI / 180.0);      
        }

        if (props.hasOwnProperty('scale')) 
        {
            const scale = props.scale.split(',');
            obj.setScale(parseFloat(scale[0]), parseFloat(scale[1]), parseFloat(scale[2]));
        }

        if (props.hasOwnProperty('color')) 
        {
            const color = props.color.split(',');
            const r = parseFloat(color[0]);
            const g = parseFloat(color[1]);
            const b = parseFloat(color[2]);
            obj.setColor(r,g,b);
        }
        
        if (props.hasOwnProperty('texture'))
        {
            let img = imageLoader.loadFile(props.texture);
            obj.setColorTexture(img);
        }
        
        if (tag in this.hitable_tags)
        {
            this.add_hitable_object(obj);
        }
        
        return obj;
    }
    
    remove(obj)
    {
        obj.traverse((child) => {
            this.remove_hitable_object(child);
        });
        if (obj.parent) 
        {
            obj.parent.remove(obj);
        }
    }
    
    async load_xml_node(xmlNode, mode, parent = null)
    {
        for (let child of xmlNode.children) {           
            let obj = null;
            if (parent == null)
            {
                obj = await this.create(child.tagName, child.attributes, mode, this);
            }
            else
            {
                obj = await this.create(child.tagName, child.attributes, mode, parent);
            }
            if (obj===null) continue;
            
            let key = obj.uuid;
            
            let internal_node = {};
            internal_node.obj = obj;
            internal_node.xml_node = child;
            this.internal_index[key] = internal_node;
            
            let external_node = {};
            external_node.tagName = child.tagName;
            external_node.attributes = child.attributes;
            external_node.children = [];
            
            if(child.tagName == "scene")
            {
                this.external_index.root = key;
            }
            else if (parent!=null)
            {
                let parent_key = parent.uuid;
                let external_node_parent = this.external_index.index[parent.uuid];
                external_node.parent = parent_key;
                external_node_parent.children.push(key);
            }
            this.external_index.index[key] = external_node;
            
            await this.load_xml_node(child, mode, obj);
        }
    }
    
    generate_bvh()
    {
        if (this.hitables.length>0)
        { 
            this.bvh = new BoundingVolumeHierarchy(this.hitables);
        }
        else
        {
            this.bvh = null;
        }
    }
    
    async load_xml(xmlText, mode)
    {
        this.xml_nodes = txml.parse(xmlText, {keepComments: true});
        let root = null;
        for (let top of this.xml_nodes)
        {
            if (top.tagName == 'document')
            {
                root = top;
                break;
            }
        }
        if (root)
        {
            await this.load_xml_node(root, mode);
        }
        
        this.generate_bvh();
        
        this.saved_text = genXML(this.xml_nodes);
        
        gamePlayer.message("index_loaded", JSON.stringify(this.external_index));
    }
    
    is_modified()
    {
        let gen_xml = genXML(this.xml_nodes);
        return gen_xml != this.saved_text;
    }
    
    get_xml()
    {
        this.saved_text = genXML(this.xml_nodes);
        return this.saved_text;
    }
    
    picking(state)
    {
        if (state)
        {
            this.controls.enabled = false;
            view.addEventListener("pointerdown", picking_pointerdown);
        }
        else
        {
            this.controls.enabled = true;
            view.removeEventListener("pointerdown", picking_pointerdown);
        }
    }
    
    pick_obj(key)
    {
        let obj = null;
        if (key!="") 
        {
            obj = this.internal_index[key].obj;
        }
        if (this.picked_obj != null && this.picked_obj.hasOwnProperty("setToonShading"))
        {
            this.picked_obj.setToonShading(0);
        }
        
        this.picked_obj = obj;
        
        if (obj!=null)
        {
            if (obj.hasOwnProperty("setToonShading"))
            {
                obj.setToonShading(16, 5.0, new Vector3(1.0, 1.0, 0.2));
            }
            gamePlayer.message("object_picked", obj.uuid);
        }
        else
        {
            gamePlayer.message("object_picked", "");
        }
    }
    
    tuning(args)
    {
        if (this.picked_obj==null) return;
        
        let input = JSON.parse(args);
        let node = this.internal_index[this.picked_obj.uuid].xml_node;
        let tag = node.tagName;
        
        if (!(tag in this.Tags)) return;
        this.Tags[tag].tuning(this, this.picked_obj, args);
    }

    generate()
    {
        if (this.picked_obj==null) return;
        let node = this.internal_index[this.picked_obj.uuid].xml_node;
        let tag = node.tagName;
        
        if (!(tag in this.Tags)) return;
        this.Tags[tag].generate(this, this.picked_obj);
    }
}

function picking_pointerdown(event)
{
    if (doc.bvh == null) return;
    
    let origin = doc.camera.getWorldPosition(new Vector3());
    
    let x = event.clientX;
    let y = event.clientY;
    
    let clipX = (x/doc.width)*2.0 -1.0;
    let clipY = 1.0 - (y/doc.height)*2.0;
    
    let pos = new Vector4(clipX, clipY, 0.0, 1.0);
    
    let matProjInv = doc.camera.getProjectionMatrixInverse(new Matrix4());
    let matViewInv = doc.camera.getMatrixWorld(new Matrix4());
    pos.applyMatrix4(matProjInv);
    pos.applyMatrix4(matViewInv);
    
    let dir = new Vector3(pos.x/pos.w, pos.y/pos.w, pos.z/pos.w);
    dir.sub(origin);
    dir.normalize();

    let intersect = doc.bvh.intersect({origin: origin, direction: dir});
    if (intersect!=null)
    {
        doc.pick_obj(intersect.uuid);
    }
    else if (doc.scene.background!=null)
    {
        doc.pick_obj(doc.scene.background.uuid);
    }
    else
    {
        doc.pick_obj(doc.scene.uuid);
    }
    
}



