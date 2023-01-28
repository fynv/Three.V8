import { OrbitControls } from "./controls/OrbitControls.js";
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
const create_default_controls = (doc)=>{
    if (doc.controls)
        doc.controls.dispose();
    doc.controls = new OrbitControls(doc.camera, doc.view);
    doc.controls.enableDamping = true;
    doc.controls.target.set(0, 1.5, 0);
}

const create_default_sky = (doc)=>{
    let bg = new HemisphereBackground();   
    bg.setSkyColor(0.318, 0.318, 0.318);
    bg.setGroundColor(0.01, 0.025, 0.025);
    doc.scene.background = bg;
}

const create_default_env_light = (doc) =>{
    let envLight = new HemisphereLight();
    envLight.setSkyColor(0.318, 0.318, 0.318);
    envLight.setGroundColor(0.01, 0.025, 0.025);
    doc.scene.indirectLight = envLight;    
}

const scene = {
    reset: (doc) => {
        doc.scene = new Scene();
    },
    create: async (doc, props, mode, parent) => {
        doc.scene = new Scene();
        create_default_sky(doc);
        create_default_env_light(doc);
        return doc.scene;
    }
}

const camera = {
    reset: (doc) => {
        doc.camera = new PerspectiveCamera(45, doc.width / doc.height, 0.1, 100);
        doc.camera.setPosition(0, 1.5, 5.0);
    },

    create: async (doc, props, mode, parent) => {
        let fov = 50.0;
        let near = 0.1;
        let far = 200.0;
        if (props.hasOwnProperty("fov"))
        {
            fov = parseFloat(props.fov);
        }
        if (props.hasOwnProperty("near"))
        {
            near = parseFloat(props.near);
        }
        if (props.hasOwnProperty("far"))
        {
            far = parseFloat(props.far);
        }
        doc.camera = new PerspectiveCamera(fov, doc.width / doc.height, near, far);
        create_default_controls(doc);
        return doc.camera;
    },
    
    remove: (doc, obj) => {
        camera.reset(doc);
        create_default_controls(doc);
    }
}

const control = {
    reset: (doc) => {
        create_default_controls(doc);
    },
    create: async (doc, props, mode, parent) =>{
        let type = 'orbit';
        if (props.hasOwnProperty("type"))
        {
            type = props.type;
        }
        if (type == 'orbit') 
        {
            let from_x = 0.0;
            let from_y = 1.5;
            let from_z = 5.0;
            if (props.hasOwnProperty("look_from"))
            {
                let look_from = props.look_from.split(',');
                from_x = parseFloat(look_from[0]);
                from_y = parseFloat(look_from[1]);
                from_z = parseFloat(look_from[2]);
            }
            
            let to_x = 0.0;
            let to_y = 1.5;
            let to_z = 0.0;
            if (props.hasOwnProperty("look_at"))
            {
                let look_at = props.look_at.split(',');
                to_x = parseFloat(look_at[0]);
                to_y = parseFloat(look_at[1]);
                to_z = parseFloat(look_at[2]);    
            }
            
            doc.camera.setPosition(from_x, from_y, from_z);
            if (doc.controls != null)
                doc.controls.dispose();
            doc.controls = new OrbitControls(doc.camera, doc.view);
            doc.controls.enableDamping = true;
            doc.controls.target.set(to_x, to_y, to_z);
        }
        return doc.controls;
    },
    remove: (doc, obj) => {
        create_default_controls(doc);
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
    },
    tuning: (doc, obj, input) =>{
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        if ("density" in input)
        {
            let density = input.density;
            props.density = density;
            obj.density = parseFloat(density);
        }
        if ("color" in input)
        {
            props.color = input.color;
            const color = input.color.split(',');
            const r = parseFloat(color[0]);
            const g = parseFloat(color[1]);
            const b = parseFloat(color[2]);
            obj.setColor(r,g,b);
        }
    },
    remove: (doc, fog) => {
        doc.scene.fog = null;
    }
}

const sky = {
    reset: (doc) => {
        create_default_sky(doc);
    },
    create: async (doc, props, mode, parent) => {
        let type = "hemisphere"
        if (props.hasOwnProperty("type"))
        {
            type = props.type;
        }
        if (type == "uniform")
        {
            let bg = new ColorBackground();
            let envLight = null;
            
            if (props.hasOwnProperty('color'))
            {
                const color = props.color.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.setColor(r,g,b);
            }
            doc.scene.background = bg;
        }
        else if (type == "hemisphere")
        {
            let bg = new HemisphereBackground();
            
            if (props.hasOwnProperty('skyColor'))
            {
                const color = props.skyColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.setSkyColor(r,g,b);
            }
            
            if (props.hasOwnProperty('groundColor'))
            {
                const color = props.groundColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.setGroundColor(r,g,b);               
                
            }
            
            doc.scene.background = bg;
        }
        else if (type == "cube")
        {
            let bg = new CubeBackground();
            
            let url = "assets/textures";
            let posx = "face0.jpg";
            let negx = "face1.jpg";
            let posy = "face2.jpg";
            let negy = "face3.jpg";
            let posz = "face4.jpg";
            let negz = "face5.jpg";
            
            if (props.hasOwnProperty('path'))
            {
                url = props.path;
            }
            if (props.hasOwnProperty('posx'))
            {
                posx = props.posx;
            }
            if (props.hasOwnProperty('negx'))
            {
                negx = props.negx;
            }
            if (props.hasOwnProperty('posy'))
            {
                posy = props.posy;
            }
            if (props.hasOwnProperty('negy'))
            {
                negy = props.negy;
            }
            if (props.hasOwnProperty('posz'))
            {
                posz = props.posz;
            }
            if (props.hasOwnProperty('negz'))
            {
                negz = props.negz;
            }
            
            let cube_img = imageLoader.loadCubeFromFile(
                url+"/"+posx, url+"/"+negx, 
                url+"/"+posy, url+"/"+negy, 
                url+"/"+posz, url+"/"+negz);
            
            if (cube_img!=null)
            {
                bg.setCubemap(cube_img);
            }
            doc.scene.background = bg;
        }
        
        return doc.scene.background;
    },
    remove: (doc, obj) => {
        create_default_sky(doc);
    }
}

const env_light = {
    reset: (doc) => {
        create_default_env_light(doc);
    },
    create: async (doc, props, mode, parent) => {
        let type = "hemisphere"
        if (props.hasOwnProperty("type"))
        {
            type = props.type;
        }
        
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
            
            let url = "assets/textures";
            let posx = "face0.jpg";
            let negx = "face1.jpg";
            let posy = "face2.jpg";
            let negy = "face3.jpg";
            let posz = "face4.jpg";
            let negz = "face5.jpg";
            
            if (props.hasOwnProperty('path'))
            {
                url = props.path;
            }
            if (props.hasOwnProperty('posx'))
            {
                posx = props.posx;
            }
            if (props.hasOwnProperty('negx'))
            {
                negx = props.negx;
            }
            if (props.hasOwnProperty('posy'))
            {
                posy = props.posy;
            }
            if (props.hasOwnProperty('negy'))
            {
                negy = props.negy;
            }
            if (props.hasOwnProperty('posz'))
            {
                posz = props.posz;
            }
            if (props.hasOwnProperty('negz'))
            {
                negz = props.negz;
            }
            
            let cube_img = imageLoader.loadCubeFromFile(
                url+"/"+posx, url+"/"+negx, 
                url+"/"+posy, url+"/"+negy, 
                url+"/"+posz, url+"/"+negz);
                
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
    remove: (doc, obj) => {
        create_default_env_light(doc);
    },
    
    tuning: (doc, obj, input) => {
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
        let width = 1.0;
        let height = 1.0;
        if (props.hasOwnProperty('size'))
        {
            let size = props.size.split(',');
            width = parseFloat(size[0]);
            height = parseFloat(size[1]);
        }
                
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
        let width = 1.0;
        let height = 1.0;
        let depth = 1.0;
        if (props.hasOwnProperty('size'))
        {
            let size = props.size.split(',');
            width = parseFloat(size[0]);
            height = parseFloat(size[1]);
            depth = parseFloat(size[2]);
        }
        
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
        let radius = 1.0;
        if (props.hasOwnProperty('size'))
        {
            radius = parseFloat(props.radius);
        }
        let widthSegments = 32;
        if (props.hasOwnProperty('widthSegments'))
        {
            widthSegments = parseInt(props.widthSegments);
        }
        let heightSegments = 16;
        if (props.hasOwnProperty('heightSegments'))
        {
            heightSegments = parseInt(props.heightSegments);
        }
        
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
        let url = "assets/models/model.glb";
        if (props.hasOwnProperty('src'))
        {
            url = props.src;
        }
        let model = gltfLoader.loadModelFromFile(url);
        if (model == null)
        {
            model= new SimpleModel();
            model.createBox(0.5, 1.5, 0.5);
            model.setColor(0.7,0.0,0.7);
        }
        
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
        obj.tag = tag;
        
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
        if (obj.hasOwnProperty("traverse"))
        {
            obj.traverse((child) => {
                this.remove_hitable_object(child);
            });
        }
        
        if (obj.hasOwnProperty('tag')) {
            const tag = this.Tags[obj.tag];
            if (tag.hasOwnProperty('remove')) {
                tag.remove(this, obj);
            }
        }
        
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
    
    tuning(input)
    {
        if (this.picked_obj==null) return;
        
        let node = this.internal_index[this.picked_obj.uuid].xml_node;
        let tag = node.tagName;
        
        if (!(tag in this.Tags)) return;
        this.Tags[tag].tuning(this, this.picked_obj, input);
    }

    generate()
    {
        if (this.picked_obj==null) return;
        let node = this.internal_index[this.picked_obj.uuid].xml_node;
        let tag = node.tagName;
        
        if (!(tag in this.Tags)) return;
        this.Tags[tag].generate(this, this.picked_obj);
    }
    
    async req_create(base_key, tag)
    {
        let internal_node_base = this.internal_index[base_key];
        let external_node_base = this.external_index.index[base_key];
        
        let xmlNode = internal_node_base.xml_node;
        let parent = internal_node_base.obj;
        
        let child = {tagName:tag, attributes: {}, children: []};
        xmlNode.children.push(child);
        
        let obj = await this.create(tag, {}, "local", parent);
        let key = obj.uuid;
        
        let internal_node = {};
        internal_node.obj = obj;
        internal_node.xml_node = child;
        this.internal_index[key] = internal_node;
        
        let external_node = {tagName:tag, attributes: {}, children: []};
        external_node.parent = base_key;
        external_node_base.children.push(key);
        this.external_index.index[key] = external_node;
        
        let msg = {};
        msg[key] = external_node;
        
        gamePlayer.message("object_created", JSON.stringify(msg));
        
        this.pick_obj(key);
    }
    
    req_remove(key)
    {
        let internal_node = this.internal_index[key];
        let external_node = this.external_index.index[key];
        
        let base_key = external_node.parent;
        let internal_node_base = this.internal_index[base_key];
        let external_node_base = this.external_index.index[base_key];
        
        let xmlNode = internal_node.xml_node;
        let xmlNode_parent = internal_node_base.xml_node;
        {
            let idx = xmlNode_parent.children.indexOf(xmlNode);
            if (idx>-1)
            {
                xmlNode_parent.children.splice(idx, 1);
            }
        }
        
        let obj = internal_node.obj;
        this.remove(obj);
        
        {
            let idx = external_node_base.children.indexOf(key);
            if (idx>-1)
            {
                external_node_base.children.splice(idx,1);
            }
        }
        
        delete this.internal_index[key];
        delete this.external_index.index[key];
        
        gamePlayer.message("object_removed", key);
        
        this.pick_obj("");
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



