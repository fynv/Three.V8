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

class LightmapBaker
{
    constructor(doc, iterations)
    {
        this.doc = doc;
        this.lst = [];
        for (let obj of doc.bakables)
        {
            let xml_node = doc.internal_index[obj.uuid].xml_node;
            let attributes = xml_node.attributes;
            if ("lightmap" in attributes)
            {
                this.lst.push({ model: obj, lightmap: attributes.lightmap, count: -1 });
            }
        }
        if (this.lst.length<1)
        {
            print("No bakable object found!");
            return;
        }
        
        this.iterations = iterations;
        this.iter = 0;
        this.idx_model = 0;
        this.idx_texel = 0;
        this.check_time = now();
    }
    
    render(renderer)
    {
        if (this.lst.length<1)
        {
            this.doc.lightmap_bake = null;
            return;
        }
        let frame_time = now();
        if (frame_time - this.check_time>=500)
        {
            print(`Building LOD Probe-Grid, iteration: ${this.iter+1}/${this.iterations}, model: ${this.idx_model+1}/${this.lst.length}, texel: ${this.idx_texel +1}`);
            this.check_time = frame_time;
        }
        
        while(now()-frame_time<10)
        {
            if (this.doc.lightmap_bake == null) break;
            let item = this.lst[this.idx_model];
            if (item.count<0)
            {
                item.count = item.model.initializeLightmap(renderer);
            }
            let num_rays = (64 << this.iter);
            let count_texels = renderer.updateLightmap(this.doc.scene, item.model, this.idx_texel, num_rays, 1.0);
            this.idx_texel += count_texels;
            if (this.idx_texel>=item.count)
            {
                renderer.filterLightmap(item.model);
                this.idx_texel = 0;
                this.idx_model++;
                if (this.idx_model >= this.lst.length)
                {
                    this.idx_model = 0;
                    this.iter++;
                    if (this.iter >= this.iterations)
                    {
                        this.doc.lightmap_bake = null; 
                        print("Saving lightmaps.");
                        for (let item of this.lst)
                        {
                            let hdr_image = item.model.getLightmap();
                            let res = HDRImageSaver.saveFile(hdr_image, item.lightmap);
                            if (!res)
                            {
                                print(`Failed to save ${item.lightmap}`);
                            }
                        }
                        
                    }
                    
                }
                
            }
        }
        
    }
}

class EnvMapGen
{
    constructor(doc, proxy, xml_node, irradiance_only = false)
    {
        this.doc = doc;
        this.xml_node = xml_node;
        this.irradiance_only = irradiance_only;
        
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
        
        renderer.renderCube(this.doc.scene, this.cube_target, new Vector3(x, y,z));
        
        let envLight = this.envMapCreator.create(this.cube_target, this.irradiance_only);
        if (props.hasOwnProperty('dynamic_map'))
        {
            envLight.dynamicMap = string_to_boolean(props.dynamic_map);
        }
        this.doc.scene.indirectLight = envLight;
        
        this.iter++;
        if (this.iter > 5)
        {
            print("Saving environemnt map.");
            if (this.irradiance_only)
            {
                let path_sh = "assets/sh.json";
                if (props.hasOwnProperty('path_sh'))
                {
                    path_sh = props.path_sh;
                }
        
                let text = JSON.stringify(envLight.shCoefficients);
                let res = fileSaver.saveTextFile(path_sh, text);
                
                if (!res)
                {
                    print("Failed to save enviroment map.");
                }
            }
            else
            {
                let url = "assets/textures";
                let posx = "env_face0.jpg";
                let negx = "env_face1.jpg";
                let posy = "env_face2.jpg";
                let negy = "env_face3.jpg";
                let posz = "env_face4.jpg";
                let negz = "env_face5.jpg";
                
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
                
                if (posx.split('.').pop()=="hdr")
                {
                    let down_img = this.cube_target.getHDRCubeImage();
                    
                    let res = HDRImageSaver.saveCubeToFile(down_img, 
                        url+"/"+posx, url+"/"+negx, 
                        url+"/"+posy, url+"/"+negy, 
                        url+"/"+posz, url+"/"+negz);
                        
                    if (!res)
                    {
                        print("Failed to save enviroment map.");
                    }
                    
                }
                else
                {
                    let down_img = this.cube_target.getCubeImage();
                            
                    let res = imageSaver.saveCubeToFile(down_img, 
                        url+"/"+posx, url+"/"+negx, 
                        url+"/"+posy, url+"/"+negy, 
                        url+"/"+posz, url+"/"+negz);
                        
                    if (!res)
                    {
                        print("Failed to save enviroment map.");
                    }
                }
            }
            
            this.doc.env_gen = null;
        }
    }
    
}

class ProbeGridBaker
{
    constructor(doc, proxy, xml_node, iterations)
    {
        this.doc = doc;
        this.xml_node = xml_node;
        
        this.cube_target = new CubeRenderTarget(64,64);
        this.probe_grid = new ProbeGrid();
        this.probe_grid.setDivisions(proxy.divisions);
        this.probe_grid.setCoverageMin(proxy.coverageMin);
        this.probe_grid.setCoverageMax(proxy.coverageMax);
        this.probe_grid.ypower = proxy.ypower;
        
        print("Constructing visibility information...");
        this.probe_grid.constructVisibility(doc.scene);
        
        let props = this.xml_node.attributes;
        if (props.hasOwnProperty('dynamic_map'))
        {
            this.probe_grid.dynamicMap = string_to_boolean(props.dynamic_map);
        }
        if (props.hasOwnProperty('normal_bias'))
        {
            this.probe_grid.normalBias = parseFloat(props.normal_bias);
        }
        this.doc.scene.indirectLight = this.probe_grid;
        
        let divisions = this.probe_grid.divisions;
        this.probe_count = divisions.x*divisions.y*divisions.z;
        
        this.update_lst = [];
        this.diff_stack = [];
        
        for (let i=0; i< this.probe_count; i++)
        {
            this.update_lst.push(i);
        }
        this.update_count = this.update_lst.length;
        this.probe_idx = 0;
        
        this.iterations = iterations;
        this.iter = 0;
        this.check_time = now();
        
    }
    
    render(renderer)
    {
        let frame_time = now();
        
        if (frame_time -  this.check_time>=500)
        {
            print(`Building probe-grid, iteration: ${this.iter+1}/${this.iterations}, probe: ${this.probe_idx +1}/${this.update_count}`);
            this.check_time = frame_time;
        }
         
        let divisions = this.probe_grid.divisions;
        this.probe_grid.recordReferences = true;
        while(now()-frame_time<10)
        {
            if (this.doc.probe_grid_bake == null) break;
            
            let x = this.update_lst[this.probe_idx];
            let y = x / divisions.x;
            let z = y / divisions.y;
            y = y % divisions.y;
            x = x % divisions.x;
            let v_idx = new Vector3(x,y,z);
            
            renderer.updateProbe(this.doc.scene, this.cube_target, this.probe_grid, v_idx);
            this.probe_idx++;
            if (this.probe_idx>=this.update_count)
            {
                this.probe_idx = 0;
                this.iter++;
                
                if (this.iter < this.iterations)
                {
                    let ref_arr = this.probe_grid.getReferences();
                    let new_lst = [];
                    let diff_lst = [];
                    for (let i of this.update_lst)
                    {
                        if (ref_arr[i])
                        {
                            new_lst.push(i);
                        }
                        else
                        {
                            diff_lst.push(i);
                        }
                    }
                    
                    if (diff_lst.length>0)
                    {
                        this.diff_stack.push(diff_lst);
                    }
                    
                    if (this.iter == this.iterations -1)
                    {
                        for(let j = this.diff_stack.length-1; j>=0; j--)
                        {
                            new_lst = new_lst.concat(this.diff_stack[j]);
                        }
                    }
                    
                    this.update_lst = new_lst;
                    this.update_count = this.update_lst.length;
                }
                else
                {
                    print("Saving probe-grid.");
                    let props = this.xml_node.attributes;
                    let probe_data = "assets/probes.dat";
                    if (props.hasOwnProperty('probe_data')) 
                    {
                        probe_data = props.probe_data;
                    }
                    let res = probeGridSaver.saveFile(this.probe_grid, probe_data);
                    if (!res)
                    {
                        print("Failed to save probe-grid.");
                    }
                    this.doc.probe_grid_bake = null;
                }
            }
        }
        this.probe_grid.recordReferences = false;
    }
    
}

class LODProbeGridBaker
{
    constructor(doc, proxy, xml_node, iterations)
    {
        this.doc = doc;
        this.xml_node = xml_node;
        
        this.cube_target = new CubeRenderTarget(64,64);
        this.probe_grid = this.doc.scene.indirectLight;
        
        print("Constructing visibility information...");
        this.probe_grid.constructVisibility(doc.scene);
        
        this.probe_count = this.probe_grid.numberOfProbes;
        this.probe_idx = 0;
        
        this.iterations = iterations;
        this.iter = 0;
        this.check_time = now();
        
    }
    
    render(renderer)
    {
        let frame_time = now();
        
        if (frame_time -  this.check_time>=500)
        {
            print(`Building LOD Probe-Grid, iteration: ${this.iter+1}/${this.iterations}, probe: ${this.probe_idx +1}/${this.probe_count}`);
            this.check_time = frame_time;
        }
        while(now()-frame_time<10)
        {
            if (this.doc.lod_probe_grid_bake == null) break;
            renderer.updateProbe(this.doc.scene, this.cube_target, this.probe_grid, this.probe_idx);
            this.probe_idx++;
            if (this.probe_idx>=this.probe_count)
            {
                this.probe_idx = 0;
                this.iter++;
                
                if (this.iter >= this.iterations)
                {
                    print("Saving LOD Probe-Grid.");
                    let props = this.xml_node.attributes;
                    let probe_data = "assets/lod_probes.dat";
                    if (props.hasOwnProperty('probe_data')) 
                    {
                        probe_data = props.probe_data;
                    }
                    let res = LODProbeGridSaver.saveFile(this.probe_grid, probe_data);
                    if (!res)
                    {
                        print("Failed to save probe-grid.");
                    }
                    this.doc.lod_probe_grid_bake = null;
                }
            }
        }
    }
    
}

class GPUProbeGridBaker
{
    constructor(doc, proxy, xml_node, iterations)
    {
        this.doc = doc;
        this.xml_node = xml_node;
        
        this.probe_grid = new ProbeGrid();
        this.probe_grid.setDivisions(proxy.divisions);
        this.probe_grid.setCoverageMin(proxy.coverageMin);
        this.probe_grid.setCoverageMax(proxy.coverageMax);
        this.probe_grid.ypower = proxy.ypower;
        
        print("Constructing visibility information...");
        this.probe_grid.constructVisibility(doc.scene);
        
        let props = this.xml_node.attributes;
        if (props.hasOwnProperty('dynamic_map'))
        {
            this.probe_grid.dynamicMap = string_to_boolean(props.dynamic_map);
        }
        if (props.hasOwnProperty('normal_bias'))
        {
            this.probe_grid.normalBias = parseFloat(props.normal_bias);
        }
        this.doc.scene.indirectLight = this.probe_grid;
        
        let divisions = this.probe_grid.divisions;
        this.probe_count = divisions.x*divisions.y*divisions.z;
        
        this.probe_idx = 0;
        this.iterations = iterations;
        this.iter = 0;
        this.check_time = now();
    }
    
    render(renderer)
    {
        let frame_time = now();
        
        if (frame_time -  this.check_time>=500)
        {
            print(`Building probe-grid, iteration: ${this.iter+1}/${this.iterations}, probe: ${this.probe_idx +1}/${this.probe_count}`);
            this.check_time = frame_time;
        }
        
        while(now()-frame_time<10)
        {
            if (this.doc.probe_grid_bake == null) break;
            let num_rays = (256 << this.iter);
            let num_probes = renderer.updateProbes(this.doc.scene, this.probe_grid, this.probe_idx, num_rays, 0.5, 1.0);
            this.probe_idx += num_probes;
            if (this.probe_idx>=this.probe_count)
            {
                this.probe_idx = 0;
                this.iter++;
                
                if (this.iter >= this.iterations)
                {
                    print("Saving probe-grid.");
                    let props = this.xml_node.attributes;
                    let probe_data = "assets/probes.dat";
                    if (props.hasOwnProperty('probe_data')) 
                    {
                        probe_data = props.probe_data;
                    }
                    let res = probeGridSaver.saveFile(this.probe_grid, probe_data);
                    if (!res)
                    {
                        print("Failed to save probe-grid.");
                    }
                    this.doc.probe_grid_bake = null;
                }
            }
        }
    }
}

class GPULODProbeGridBaker
{
    constructor(doc, proxy, xml_node, iterations)
    {
        this.doc = doc;
        this.xml_node = xml_node;
        
        this.cube_target = new CubeRenderTarget(64,64);
        this.probe_grid = this.doc.scene.indirectLight;
        
        print("Constructing visibility information...");
        this.probe_grid.constructVisibility(doc.scene);
        
        this.probe_count = this.probe_grid.numberOfProbes;
        this.probe_idx = 0;
        
        this.iterations = iterations;
        this.iter = 0;
        this.check_time = now();
        
    }
    
    render(renderer)
    {
        let frame_time = now();
        
        if (frame_time -  this.check_time>=500)
        {
            print(`Building LOD Probe-Grid, iteration: ${this.iter+1}/${this.iterations}, probe: ${this.probe_idx +1}/${this.probe_count}`);
            this.check_time = frame_time;
        }
        
        while(now()-frame_time<10)
        {
            if (this.doc.lod_probe_grid_bake == null) break;
            let num_rays = (256 << this.iter);
            let num_probes = renderer.updateProbes(this.doc.scene, this.probe_grid, this.probe_idx, num_rays, 0.5, 1.0);
            this.probe_idx += num_probes;
            if (this.probe_idx>=this.probe_count)
            {
                this.probe_idx = 0;
                this.iter++;
                
                if (this.iter >= this.iterations)
                {
                    print("Saving probe-grid.");
                    let props = this.xml_node.attributes;
                    let probe_data = "assets/lod_probes.dat";
                    if (props.hasOwnProperty('probe_data')) 
                    {
                        probe_data = props.probe_data;
                    }
                    let res = LODProbeGridSaver.saveFile(this.probe_grid, probe_data);
                    if (!res)
                    {
                        print("Failed to save probe-grid.");
                    }
                    this.doc.lod_probe_grid_bake = null;
                }
            }
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
    create: (doc, props, mode, parent) => {
        doc.scene = new Scene();
        create_default_sky(doc);
        create_default_env_light(doc);
        return doc.scene;
    },
    generate: (doc, obj, input) =>{
        generate_lightmap(doc, input);
    }
}

const camera = {
    reset: (doc) => {
        doc.camera = new PerspectiveCamera(45, doc.width / doc.height, 0.1, 100);
        doc.camera.setPosition(0, 1.5, 5.0);
    },

    create: (doc, props, mode, parent) => {
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
    create: (doc, props, mode, parent) =>{
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
    create: (doc, props, mode, parent) =>{        
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
        return "";
    },
    remove: (doc, fog) => {
        doc.scene.fog = null;
    }
}

const create_uniform_sky = (doc, props) => {
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
    return bg;
}

const create_hemisphere_sky = (doc, props)=>{
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
    return bg;
}

const create_cube_sky = (doc, props)=>{
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
    
    return bg;
}

const create_background_scene = (doc, props)=>{
    let path_scene = "terrain.xml";
    if (props.hasOwnProperty('scene'))
    {
        path_scene = props.scene;
    }
    
    let near = 10.0;
    let far = 10000.0;
    if (props.hasOwnProperty('near'))
    {
        near = parseFloat(props.near);
    }
    if (props.hasOwnProperty('far'))
    {
        far = parseFloat(props.far);
    }
    
    let bg_doc = new BackgroundDocument(near, far);
    bg_doc.load_local_xml(path_scene);
    doc.scene.background = bg_doc;
    
    return bg_doc;
}

const tuning_uniform_sky = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("color" in input)
    {
        props.color = input.color;
        const color = input.color.split(',');
        const r = parseFloat(color[0]);
        const g = parseFloat(color[1]);
        const b = parseFloat(color[2]);
        obj.setColor(r,g,b);
    }
}

const tuning_hemisphere_sky = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("skyColor" in input)
    {
        props.skyColor = input.skyColor;
        const color = input.skyColor.split(',');
        const r = parseFloat(color[0]);
        const g = parseFloat(color[1]);
        const b = parseFloat(color[2]);
        obj.setSkyColor(r,g,b);
    }
    
    if ("groundColor" in input)
    {
        props.groundColor = input.groundColor;
        const color = input.groundColor.split(',');
        const r = parseFloat(color[0]);
        const g = parseFloat(color[1]);
        const b = parseFloat(color[2]);
        obj.setGroundColor(r,g,b);
    }
}

const tuning_cube_sky = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
   
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
    
        obj.setCubemap(cube_img);
    }
}

const tuning_background_scene = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    if ("scene" in input)
    {
        props.scene = input.scene;
        obj.load_local_xml(input.scene);
    }
    if ("near" in input)
    {
        props.near = input.near;
        obj.near = parseFloat(input.near);
    }
    if ("far" in input)
    {
        props.far = input.far;
        obj.far = parseFloat(input.far);
    }
}

const sky = {
    reset: (doc) => {
        create_default_sky(doc);
    },
    create: (doc, props, mode, parent) => {
        let type = "hemisphere"
        if (props.hasOwnProperty("type"))
        {
            type = props.type;
        }
        if (type == "uniform")
        {
            return create_uniform_sky(doc,props);
        }
        else if (type == "hemisphere")
        {
            return create_hemisphere_sky(doc,props);
        }
        else if (type == "cube")
        {
            return create_cube_sky(doc,props);
        }
        else if (type == "scene")
        {
            return create_background_scene(doc,props);
        }
    },
    remove: (doc, obj) => {
        create_default_sky(doc);
    },
    tuning: (doc, obj, input) => {
        let key = obj.uuid;
        let node = doc.internal_index[key].xml_node;
        if (input.hasOwnProperty('type'))
        {
            node.attributes = {};
            node.attributes.type = input.type;
            doc.external_index.index[key].attributes = node.attributes;
            let obj_new = sky.create(doc, node.attributes, "local", doc.scene);
            obj_new.uuid = key;
            obj_new.tag = "sky";
            doc.internal_index[key].obj = obj_new;
            return JSON.stringify(node.attributes);
        }
        else
        {
            let props = node.attributes;
            let type = "hemisphere";
            if (props.hasOwnProperty("type"))
            {
                type = props.type;
            }
            if (type == "uniform")
            {
                tuning_uniform_sky(doc, obj, input);
            }
            else if (type=="hemisphere")
            {
                tuning_hemisphere_sky(doc, obj, input);
            }
            else if (type=="cube")
            {
                tuning_cube_sky(doc, obj, input);
            }
            else if (type=="scene")
            {
                tuning_background_scene(doc, obj, input);
            }
        }
        return ""
    }
}

const create_uniform_env_light = (doc, props) => {
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
    return envLight;
}

const create_hemisphere_env_light = (doc, props) => {
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
    return envLight;
}

const create_cube_env_light = (doc, props) => {
    const proxy = new SimpleModel();
    proxy.createBox(0.3, 0.3, 0.3);
    
    if (props.hasOwnProperty('probe_position')) 
    {
        const position = props.probe_position.split(',');
        proxy.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
    }
    proxy.setColor(0.7,0.0,0.7);
    doc.scene.addWidget(proxy);
    
    let irradiance_only = false;
    if (props.hasOwnProperty('irradiance_only'))
    {
        irradiance_only = string_to_boolean(props.irradiance_only);
    }
    
    if (irradiance_only)
    {
        let path_sh = "assets/sh.json";
        if (props.hasOwnProperty('path_sh'))
        {
            path_sh = props.path_sh;
        }
        
        let envLight = new EnvironmentMap();
        let text = fileLoader.loadTextFile(path_sh);
        if (text!=null)
        {
            envLight.shCoefficients = JSON.parse(text);
        }
        doc.scene.indirectLight = envLight;
    }
    else
    {
        let url = "assets/textures";
        let posx = "env_face0.jpg";
        let negx = "env_face1.jpg";
        let posy = "env_face2.jpg";
        let negy = "env_face3.jpg";
        let posz = "env_face4.jpg";
        let negz = "env_face5.jpg";
        
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
        
        if (posx.split('.').pop()=="hdr")
        {
            let cube_img = HDRImageLoader.loadCubeFromFile(
                url+"/"+posx, url+"/"+negx, 
                url+"/"+posy, url+"/"+negy, 
                url+"/"+posz, url+"/"+negz);
                
            let envLight = null;
            if (cube_img!=null)
            {
                let envMapCreator = new EnvironmentMapCreator();
                envLight = envMapCreator.create(cube_img);
            }
            else
            {
                envLight = new EnvironmentMap();
            }
            doc.scene.indirectLight = envLight;
        }
        else
        {
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
            else
            {
                envLight = new EnvironmentMap();
            }
            doc.scene.indirectLight = envLight;
        }
    }
    
    return proxy;
}

const create_probe_grid = (doc, props) => {
    const proxy = new ProbeGridWidget();
    if (props.hasOwnProperty('divisions')) 
    {
        const divisions = props.divisions.split(',');
        proxy.setDivisions(parseInt(divisions[0]), parseInt(divisions[1]), parseInt(divisions[2]));
    }
    if (props.hasOwnProperty('coverage_min')) 
    {
        const coverage_min = props.coverage_min.split(',');
        proxy.setCoverageMin(parseFloat(coverage_min[0]), parseFloat(coverage_min[1]), parseFloat(coverage_min[2]));
    }
    if (props.hasOwnProperty('coverage_max')) 
    {
        const coverage_max = props.coverage_max.split(',');
        proxy.setCoverageMax(parseFloat(coverage_max[0]), parseFloat(coverage_max[1]), parseFloat(coverage_max[2]));
    }
    if (props.hasOwnProperty('ypower'))
    {
        proxy.ypower = parseFloat(props.ypower);
    }
    doc.scene.addWidget(proxy);
    
    let probe_data = "assets/probes.dat";
    if (props.hasOwnProperty('probe_data')) 
    {
        probe_data = props.probe_data;
    }
    
    let probe_grid = probeGridLoader.loadFile(probe_data);
    if (probe_grid == null)
    {
        probe_grid = new ProbeGrid();
        probe_grid.setDivisions(proxy.divisions);
        probe_grid.setCoverageMin(proxy.coverageMin);
        probe_grid.setCoverageMax(proxy.coverageMax);
        probe_grid.ypower = proxy.ypower;
    }
    else
    {
        proxy.setDivisions(probe_grid.divisions);
        proxy.setCoverageMin(probe_grid.coverageMin);
        proxy.setCoverageMax(probe_grid.coverageMax);
        proxy.ypower = probe_grid.ypower;
        
        props.divisions = `${probe_grid.divisions.x}, ${probe_grid.divisions.y}, ${probe_grid.divisions.z}`;
        props.coverage_min = `${probe_grid.coverageMin.x}, ${probe_grid.coverageMin.y}, ${probe_grid.coverageMin.z}`;
        props.coverage_max = `${probe_grid.coverageMax.x}, ${probe_grid.coverageMax.y}, ${probe_grid.coverageMax.z}`;
        props.ypower = `${probe_grid.ypower}`;
    }

    if (props.hasOwnProperty('normal_bias'))
    {
        probe_grid.normalBias = parseFloat(props.normal_bias);
    }
    
    doc.scene.indirectLight = probe_grid;
    
    return proxy;
}

const create_lod_probe_grid = (doc, props) => {
    const proxy = new LODProbeGridWidget();    
    if (props.hasOwnProperty('base_divisions')) 
    {
        const divisions = props.base_divisions.split(',');
        proxy.setBaseDivisions(parseInt(divisions[0]), parseInt(divisions[1]), parseInt(divisions[2]));
    }
    if (props.hasOwnProperty('coverage_min')) 
    {
        const coverage_min = props.coverage_min.split(',');
        proxy.setCoverageMin(parseFloat(coverage_min[0]), parseFloat(coverage_min[1]), parseFloat(coverage_min[2]));
    }
    if (props.hasOwnProperty('coverage_max')) 
    {
        const coverage_max = props.coverage_max.split(',');
        proxy.setCoverageMax(parseFloat(coverage_max[0]), parseFloat(coverage_max[1]), parseFloat(coverage_max[2]));
    }
    proxy.subDivisionLevel = 2;
    if (props.hasOwnProperty('sub_division_level'))
    {
        proxy.subDivisionLevel = parseInt(props.sub_division_level);
    }

    doc.scene.addWidget(proxy);
    
    let probe_data = "assets/lod_probes.dat";
    if (props.hasOwnProperty('probe_data')) 
    {
        probe_data = props.probe_data;
    }
    
    let probe_grid = LODProbeGridLoader.loadFile(probe_data);
    if (probe_grid == null)
    {
        probe_grid = new LODProbeGrid();
        probe_grid.setBaseDivisions(proxy.baseDivisions);
        probe_grid.setCoverageMin(proxy.coverageMin);
        probe_grid.setCoverageMax(proxy.coverageMax);
        probe_grid.subDivisionLevel = proxy.subDivisionLevel;
    }
    else
    {
        proxy.setBaseDivisions(probe_grid.baseDivisions);
        proxy.setCoverageMin(probe_grid.coverageMin);
        proxy.setCoverageMax(probe_grid.coverageMax);
        proxy.subDivisionLevel = probe_grid.subDivisionLevel;
        
        props.base_divisions = `${probe_grid.baseDivisions.x}, ${probe_grid.baseDivisions.y}, ${probe_grid.baseDivisions.z}`;
        props.coverage_min = `${probe_grid.coverageMin.x}, ${probe_grid.coverageMin.y}, ${probe_grid.coverageMin.z}`;
        props.coverage_max = `${probe_grid.coverageMax.x}, ${probe_grid.coverageMax.y}, ${probe_grid.coverageMax.z}`;
        props.sub_division_level = `${probe_grid.subDivisionLevel}`;
    }
    proxy.probeGrid = probe_grid;

    if (props.hasOwnProperty('normal_bias'))
    {
        probe_grid.normalBias = parseFloat(props.normal_bias);
    }
    
    doc.scene.indirectLight = probe_grid;
    
    return proxy;
}

const tuning_ambient_light = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("color" in input)
    {
        props.color = input.color;
        const color = input.color.split(',');
        const r = parseFloat(color[0]);
        const g = parseFloat(color[1]);
        const b = parseFloat(color[2]);
        obj.setColor(r,g,b);
    }
    
    return "";
}


const tuning_hemisphere_light = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("skyColor" in input)
    {
        props.skyColor = input.skyColor;
        const color = input.skyColor.split(',');
        const r = parseFloat(color[0]);
        const g = parseFloat(color[1]);
        const b = parseFloat(color[2]);
        obj.setSkyColor(r,g,b);
    }
    
    if ("groundColor" in input)
    {
        props.groundColor = input.groundColor;
        const color = input.groundColor.split(',');
        const r = parseFloat(color[0]);
        const g = parseFloat(color[1]);
        const b = parseFloat(color[2]);
        obj.setGroundColor(r,g,b);
    }
    
    return ""
}

const tuning_cube_env_light = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("probe_position" in input)
    {
        let probe_position = input.probe_position;
        props.probe_position = probe_position;
        let position = probe_position.split(',');
        obj.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
    }
    
    let reload = false;
    
    if ("irradiance_only" in input)
    {
        props.irradiance_only = input.irradiance_only;
        reload = true;
    }
    
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
    
    if ("path_sh" in input)
    {
        props.path_sh = input.path_sh;
        reload = true;
    }
    
    if (reload)
    {
        let irradiance_only = false;
        if (props.hasOwnProperty('irradiance_only'))
        {
            irradiance_only = string_to_boolean(props.irradiance_only);
        }
        
        if (irradiance_only )
        {
            let path_sh = "assets/sh.json";
            if (props.hasOwnProperty('path_sh'))
            {
                path_sh = props.path_sh;
            }
            
            let envLight = new EnvironmentMap();
            let text = fileLoader.loadTextFile(path_sh);
            if (text!=null)
            {
                envLight.shCoefficients = JSON.parse(text);
            }
            doc.scene.indirectLight = envLight;
        }
        else
        {
            let url = "assets/textures";
            let posx = "env_face0.jpg";
            let negx = "env_face1.jpg";
            let posy = "env_face2.jpg";
            let negy = "env_face3.jpg";
            let posz = "env_face4.jpg";
            let negz = "env_face5.jpg";
            
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
            
            if (posx.split('.').pop()=="hdr")
            {
                let cube_img = HDRImageLoader.loadCubeFromFile(
                    url+"/"+posx, url+"/"+negx, 
                    url+"/"+posy, url+"/"+negy, 
                    url+"/"+posz, url+"/"+negz);
                    
                let envLight = null;
                if (cube_img!=null)
                {
                    let envMapCreator = new EnvironmentMapCreator();
                    envLight = envMapCreator.create(cube_img);
                }
                else
                {
                    envLight = new EnvironmentMap();
                }
                
                if (props.hasOwnProperty('dynamic_map'))
                {
                    envLight.dynamicMap = string_to_boolean(props.dynamic_map);
                }
                
                doc.scene.indirectLight = envLight;
            }
            else
            {
            
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
                else
                {
                    envLight = new EnvironmentMap();
                }
                
                if (props.hasOwnProperty('dynamic_map'))
                {
                    envLight.dynamicMap = string_to_boolean(props.dynamic_map);
                }
                
                doc.scene.indirectLight = envLight;
            }
        }
    }
    return "";
}

const tuning_probe_grid =  (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("probe_data" in input)
    {
        props.probe_data = input.probe_data;
        
        let probe_grid = probeGridLoader.loadFile(input.probe_data);
        if (probe_grid == null)
        {
            probe_grid = new ProbeGrid();
            probe_grid.setDivisions(obj.divisions);
            probe_grid.setCoverageMin(obj.coverageMin);
            probe_grid.setCoverageMax(obj.coverageMax);
            probe_grid.ypower = obj.ypower;
        }
        else
        {
            obj.setDivisions(probe_grid.divisions);
            obj.setCoverageMin(probe_grid.coverageMin);
            obj.setCoverageMax(probe_grid.coverageMax);
            obj.ypower = probe_grid.ypower;
            
            props.divisions = `${probe_grid.divisions.x}, ${probe_grid.divisions.y}, ${probe_grid.divisions.z}`;
            props.coverage_min = `${probe_grid.coverageMin.x}, ${probe_grid.coverageMin.y}, ${probe_grid.coverageMin.z}`;
            props.coverage_max = `${probe_grid.coverageMax.x}, ${probe_grid.coverageMax.y}, ${probe_grid.coverageMax.z}`;
            props.ypower = `${probe_grid.ypower}`;
        }
        
        if (props.hasOwnProperty('dynamic_map'))
        {
            probe_grid.dynamicMap = string_to_boolean(props.dynamic_map);
        }
        
        doc.scene.indirectLight = probe_grid;
        
        return JSON.stringify(props);
    }
    
    if ("divisions" in input)
    {
        props.divisions = input.divisions;
        let divisions = input.divisions.split(',');
        obj.setDivisions(parseInt(divisions[0]), parseInt(divisions[1]), parseInt(divisions[2]));
    }
    
    if ("coverage_min" in input)
    {
        props.coverage_min = input.coverage_min;
        let coverage_min = input.coverage_min.split(',');
        obj.setCoverageMin(parseFloat(coverage_min[0]), parseFloat(coverage_min[1]), parseFloat(coverage_min[2]));
    }
    
    if ("coverage_max" in input)
    {
        props.coverage_max = input.coverage_max;
        let coverage_max = input.coverage_max.split(',');
        obj.setCoverageMax(parseFloat(coverage_max[0]), parseFloat(coverage_max[1]), parseFloat(coverage_max[2]));
    }

    if ("ypower" in input)
    {
        props.ypower = input.ypower;
        obj.ypower = parseFloat(input.ypower);
    }

    if ("normal_bias" in input)
    {
        props.normal_bias = input.normal_bias;
        doc.scene.indirectLight.normalBias = parseFloat(input.normal_bias);
    }
    
    if ("auto_area" in input)
    {
        let aabb = doc.scene.getBoundingBox();
        let minPos = aabb.minPos;
        let maxPos = aabb.maxPos;
        let size_x = maxPos.x - minPos.x;
        let size_y = maxPos.y - minPos.y;
        let size_z = maxPos.z - minPos.z;
        let div_x = Math.ceil(size_x); if (div_x<2) div_x = 2;
        let div_y = Math.ceil(size_y); if (div_y<2) div_y = 2;
        let div_z = Math.ceil(size_z); if (div_z<2) div_z = 2;
        obj.setDivisions(div_x, div_y, div_z);
        obj.setCoverageMin(minPos);
        obj.setCoverageMax(maxPos);
        props.divisions = `${div_x}, ${div_y}, ${div_z}`;
        props.coverage_min = `${minPos.x}, ${minPos.y}, ${minPos.z}`;
        props.coverage_max = `${maxPos.x}, ${maxPos.y}, ${maxPos.z}`;
        let ret = { divisions: props.divisions, coverage_min: props.coverage_min, coverage_max: props.coverage_max };
        return JSON.stringify(ret);
    }
    
    return "";
    
}

const tuning_lod_probe_grid =  (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("probe_data" in input)
    {
        props.probe_data = input.probe_data;
        
        let probe_grid = LODProbeGridLoader.loadFile(input.probe_data);
        if (probe_grid == null)
        {
            probe_grid = new LODProbeGrid();
            probe_grid.setBaseDivisions(obj.baseDivisions);
            probe_grid.setCoverageMin(obj.coverageMin);
            probe_grid.setCoverageMax(obj.coverageMax);
            probe_grid.subDivisionLevel = obj.subDivisionLevel;
        }
        else
        {
            obj.setBaseDivisions(probe_grid.baseDivisions);
            obj.setCoverageMin(probe_grid.coverageMin);
            obj.setCoverageMax(probe_grid.coverageMax);
            obj.subDivisionLevel = probe_grid.subDivisionLevel;
            
            props.base_divisions = `${probe_grid.baseDivisions.x}, ${probe_grid.baseDivisions.y}, ${probe_grid.baseDivisions.z}`;
            props.coverage_min = `${probe_grid.coverageMin.x}, ${probe_grid.coverageMin.y}, ${probe_grid.coverageMin.z}`;
            props.coverage_max = `${probe_grid.coverageMax.x}, ${probe_grid.coverageMax.y}, ${probe_grid.coverageMax.z}`;
            props.sub_division_level = `${probe_grid.subDivisionLevel}`;
        }
        obj.probeGrid = probe_grid;
        
        if (props.hasOwnProperty('dynamic_map'))
        {
            probe_grid.dynamicMap = string_to_boolean(props.dynamic_map);
        }
        
        doc.scene.indirectLight = probe_grid;
        
        return JSON.stringify(props);
    }
    
    if ("base_divisions" in input)
    {
        props.base_divisions = input.base_divisions;
        let divisions = input.base_divisions.split(',');
        obj.setBaseDivisions(parseInt(divisions[0]), parseInt(divisions[1]), parseInt(divisions[2]));
    }
    
    if ("coverage_min" in input)
    {
        props.coverage_min = input.coverage_min;
        let coverage_min = input.coverage_min.split(',');
        obj.setCoverageMin(parseFloat(coverage_min[0]), parseFloat(coverage_min[1]), parseFloat(coverage_min[2]));
    }
    
    if ("coverage_max" in input)
    {
        props.coverage_max = input.coverage_max;
        let coverage_max = input.coverage_max.split(',');
        obj.setCoverageMax(parseFloat(coverage_max[0]), parseFloat(coverage_max[1]), parseFloat(coverage_max[2]));
    }
    
    if ("sub_division_level" in input)
    {
        props.sub_division_level = input.sub_division_level;
        obj.subDivisionLevel = parseInt(input.sub_division_level);
    }

    if ("normal_bias" in input)
    {
        props.normal_bias = input.normal_bias;
        obj.probeGrid.normalBias = parseFloat(input.normal_bias);
    }
    
    if ("auto_area" in input)
    {
        let aabb = doc.scene.getBoundingBox();
        let minPos = aabb.minPos;
        let maxPos = aabb.maxPos;
        let size_x = maxPos.x - minPos.x;
        let size_y = maxPos.y - minPos.y;
        let size_z = maxPos.z - minPos.z;
        let div_x = Math.ceil(size_x / 4); if (div_x<2) div_x = 2;
        let div_y = Math.ceil(size_y / 4); if (div_y<2) div_y = 2;
        let div_z = Math.ceil(size_z / 4); if (div_z<2) div_z = 2;
        obj.setBaseDivisions(div_x, div_y, div_z);
        obj.setCoverageMin(minPos);
        obj.setCoverageMax(maxPos);
        obj.subDivisionLevel = 2;
        props.base_divisions = `${div_x}, ${div_y}, ${div_z}`;
        props.coverage_min = `${minPos.x}, ${minPos.y}, ${minPos.z}`;
        props.coverage_max = `${maxPos.x}, ${maxPos.y}, ${maxPos.z}`;
        props.sub_division_level = "2";
        let ret = { base_divisions: props.base_divisions, coverage_min: props.coverage_min, coverage_max: props.coverage_max, sub_division_level: "2" };
        return JSON.stringify(ret);
    }
    
    return "";
    
}

const initialize_lod_probe_grid = (doc, obj, input) =>{
    let probe_grid = obj.probeGrid;
    
    doc.lod_probe_grid_bake = null;
    probe_grid.setBaseDivisions(obj.baseDivisions);
    probe_grid.setCoverageMin(obj.coverageMin);
    probe_grid.setCoverageMax(obj.coverageMax);
    probe_grid.subDivisionLevel = obj.subDivisionLevel;
    probe_grid.initialize(doc.renderer, doc.scene);
    return probe_grid.numberOfProbes;
}


const generate_lightmap = (doc, input) =>{
    let iterations = parseInt(input.iterations);
    doc.lightmap_bake = new LightmapBaker(doc, iterations)
}

const generate_cube_env_light = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    let irradiance_only = false;
    if (props.hasOwnProperty('irradiance_only'))
    {
        irradiance_only = string_to_boolean(props.irradiance_only);
    }
    
    if (!irradiance_only)
    {
        props.path = input.path;
        props.posx = input.posx;
        props.negx = input.negx;
        props.posy = input.posy;
        props.negy = input.negy;
        props.posz = input.posz;
        props.negz = input.negz;
    }
    doc.env_gen = new EnvMapGen(doc, obj, node, irradiance_only);
}

const generate_probe_grid = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let iterations = parseInt(input.iterations);
    doc.probe_grid_bake = new GPUProbeGridBaker(doc, obj, node, iterations);
}


const generate_lod_probe_grid = (doc, obj, input) =>{
    initialize_lod_probe_grid(doc, obj, input);
    let node = doc.internal_index[obj.uuid].xml_node;
    let iterations = parseInt(input.iterations);
    doc.lod_probe_grid_bake = new GPULODProbeGridBaker(doc, obj, node, iterations);
}

const env_light = {
    reset: (doc) => {
        create_default_env_light(doc);
    },
    create: (doc, props, mode, parent) => {
        let type = "hemisphere"
        if (props.hasOwnProperty("type"))
        {
            type = props.type;
        }

        let ret = null;
        
        if (type == "uniform")
        {
            ret = create_uniform_env_light(doc,props);
        }
        else if (type == "hemisphere")
        {
            ret =  create_hemisphere_env_light(doc,props);
        }
        else if (type == "cube")
        {
            ret = create_cube_env_light(doc,props);
        }
        else if (type == "probe_grid")
        {
            ret = create_probe_grid(doc, props);
        }
        else if (type == "lod_probe_grid")
        {
            ret = create_lod_probe_grid(doc, props);
        }
        
        if (props.hasOwnProperty('dynamic_map'))
        {
            doc.scene.indirectLight.dynamicMap = string_to_boolean(props.dynamic_map);
        }
        return ret;
    },
    remove: (doc, obj) => {
        create_default_env_light(doc);
        
        let key = obj.uuid;
        let node = doc.internal_index[key].xml_node;
        let props = node.attributes;
        let type = "hemisphere"
        if (props.hasOwnProperty("type"))
        {
            type = props.type;
        }
        
        if (type == "cube" || type =="probe_grid" || type == "lod_probe_grid")
        {
            doc.scene.removeWidget(obj);
        }
        
    },
    
    tuning: (doc, obj, input) => {
        let key = obj.uuid;
        let node = doc.internal_index[key].xml_node;
        if (input.hasOwnProperty('type'))
        {
            doc.remove(obj);
            node.attributes = {};
            node.attributes.type = input.type;
            doc.external_index.index[key].attributes = node.attributes;
            let obj_new = env_light.create(doc, node.attributes, "local", doc.scene);
            obj_new.uuid = key;
            obj_new.tag = "env_light";
            doc.internal_index[key].obj = obj_new;
            return JSON.stringify(node.attributes);
        }
        else
        {
            let props = node.attributes;
            
            if (input.hasOwnProperty('dynamic_map'))
            {
                props.dynamic_map = input.dynamic_map;
                doc.scene.indirectLight.dynamicMap = string_to_boolean(input.dynamic_map);
                return ""
            }
            
            let type = "hemisphere";
            if (props.hasOwnProperty("type"))
            {
                type = props.type;
            }
            if (type == "uniform")
            {
                return tuning_ambient_light(doc, obj, input);
            }
            else if (type=="hemisphere")
            {
                return tuning_hemisphere_light(doc, obj, input);
            }
            else if (type=="cube")
            {
                return tuning_cube_env_light(doc,obj,input);
            }
            else if (type == "probe_grid")
            {
                return tuning_probe_grid(doc,obj,input);
            }
            else if (type == "lod_probe_grid")
            {
                return tuning_lod_probe_grid(doc,obj,input);
            }
        }
        
    },
    
    initialize: (doc, obj, input) =>{
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        if (props.type == "lod_probe_grid")
        {
            return initialize_lod_probe_grid(doc,obj,input);
        }
        return "";
    },
    
    generate: (doc, obj, input) =>{
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        if (props.type == "cube")
        {
            generate_cube_env_light(doc,obj,input);
        }
        else if (props.type == "probe_grid")
        {
            generate_probe_grid(doc,obj,input);
        }
        else if (props.type == "lod_probe_grid")
        {
            generate_lod_probe_grid(doc,obj,input);
        }
    }
    
}


const tuning_object3d = (doc, obj, input) => {
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    if ("name" in input)
    {
        props.name = input.name;
        obj.name = input.name;
    }
    if ("position" in input)
    {
        props.position = input.position;
        let position = input.position.split(',');
        obj.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
    }
    if ("rotation" in input)
    {
        props.rotation = input.rotation;
        let rotation = input.rotation.split(',');
        obj.setRotation(parseFloat(rotation[0])* Math.PI / 180.0, parseFloat(rotation[1])* Math.PI / 180.0, parseFloat(rotation[2])* Math.PI / 180.0);
    }
    if ("scale" in input)
    {
        props.scale = input.scale;
        let scale = input.scale.split(',');
        obj.setScale(parseFloat(scale[0]), parseFloat(scale[1]), parseFloat(scale[2]));
    }
    if ('is_building' in input)
    {
        props.is_building = input.is_building;
        obj.isBuilding = string_to_boolean(input.is_building);
    }
}

const tuning_material = (doc, obj, input) =>{
    let node = doc.internal_index[obj.uuid].xml_node;
    let props = node.attributes;
    
    if ("color" in input)
    {
        props.color = input.color;
        const color = input.color.split(',');
        const r = parseFloat(color[0]);
        const g = parseFloat(color[1]);
        const b = parseFloat(color[2]);
        obj.setColor(r,g,b);
    }
    
    if ("texture" in input)
    {
        props.texture = input.texture;
        let img = imageLoader.loadFile(input.texture);
        obj.setColorTexture(img);
    }
    
    if ("metalness" in input)
    {
        let metalness = input.metalness;
        props.metalness = metalness;
        obj.metalness = parseFloat(metalness);
    }
    
    if ("roughness" in input)
    {
        let roughness = input.roughness;
        props.roughness = roughness;
        obj.roughness = parseFloat(roughness);
    }
}

const group = {
    create: (doc, props, mode, parent) => {
        const group = new Object3D();
        if (parent != null) {
            parent.add(group);
        }
        else {
            doc.scene.add(group);
        }
        return group;
    },
    
    tuning: (doc, obj, input) => {
        tuning_object3d(doc, obj, input);
        return "";
    }
}

const plane = {
    create: (doc, props, mode, parent) => {
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
    },
    
    tuning: (doc, obj, input) => {
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        if ("size" in input)
        {
            props.size = input.size;
            let size = input.size.split(','); 
            let width = parseFloat(size[0]);
            let height = parseFloat(size[1]);
            obj.createPlane(width, height);
        }
        tuning_object3d(doc, obj, input);
        tuning_material(doc, obj, input);
        return "";
    }
}


const box = {
    create: (doc, props, mode, parent) => {
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
    },
    
    tuning: (doc, obj, input) => {
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        if ("size" in input)
        {
            props.size = input.size;
            let size = input.size.split(','); 
            let width = parseFloat(size[0]);
            let height = parseFloat(size[1]);
            let depth =  parseFloat(size[2]);
            obj.createBox(width, height, depth);
        }
        tuning_object3d(doc, obj, input);
        tuning_material(doc, obj, input);
        return "";
    }
}

const sphere = {
    create: (doc, props, mode, parent) => {
        let radius = 1.0;
        if (props.hasOwnProperty('radius'))
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
    },
    
    tuning: (doc, obj, input) => {
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        
        let to_create = false;
        
        let radius = 1.0;
        if ("radius" in input)
        {
            props.radius = input.radius;
            radius = parseFloat(input.radius);
            to_create = true;
        }
        
        let widthSegments = 32;
        if ("widthSegments" in input)
        {
            props.widthSegments = input.widthSegments;
            widthSegments = parseInt(input.widthSegments);
            to_create = true;
        }
        
        let heightSegments = 16;
        if ("heightSegments" in input)
        {
            props.heightSegments = input.heightSegments;
            heightSegments = parseInt(input.heightSegments);
            to_create = true;
        }
        
        if (to_create)
        {
            obj.createSphere(radius, widthSegments, heightSegments);
        }
        
        tuning_object3d(doc, obj, input);
        tuning_material(doc, obj, input);
        return "";
    }
}

const model = {
    create: (doc, props, mode, parent) => {
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
        else
        {
            if (props.hasOwnProperty('is_building') && string_to_boolean(props.is_building))
            {
                model.batchPrimitives();
            }
            
            if (model.isBakable)
            {
                doc.bakables.push(model);
                
                if (props.hasOwnProperty('lightmap'))
                {
                    let filename = props.lightmap;
                    let hdr_img = HDRImageLoader.loadFile(filename);
                    if (hdr_img!=null)
                    {
                        model.setLightmap(hdr_img);
                    }
                }
            }
        }
        
        
        if (parent != null) {
            parent.add(model);
        }
        else {
            doc.scene.add(model);
        }
        return model;
    },
    
    tuning: (doc, obj, input) => {
        let key = obj.uuid;
        let node = doc.internal_index[key].xml_node;
        let props = node.attributes;
        if (input.hasOwnProperty('src'))
        {
            doc.remove(obj);
            props.src = input.src;
            let obj_new = model.create(doc, props, "local", obj.parent);
            obj_new.uuid = key;
            obj_new.tag = "model";
            
            if (props.hasOwnProperty('name')) 
            {
                obj_new.name = props.name;
            }
            
            if (props.hasOwnProperty('position')) 
            {
                const position = props.position.split(',');
                obj_new.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
            }
    
            if (props.hasOwnProperty('rotation')) 
            {
                const rotation = props.rotation.split(',');
                obj_new.setRotation(parseFloat(rotation[0])* Math.PI / 180.0, parseFloat(rotation[1])* Math.PI / 180.0, parseFloat(rotation[2])* Math.PI / 180.0);      
            }
    
            if (props.hasOwnProperty('scale')) 
            {
                const scale = props.scale.split(',');
                obj_new.setScale(parseFloat(scale[0]), parseFloat(scale[1]), parseFloat(scale[2]));
            }
            
            obj_new.setToonShading(16, 5.0, new Vector3(1.0, 1.0, 0.2));
            
            doc.internal_index[key].obj = obj_new;
        }
        else
        {
            if ("is_building" in input)
            {
                props.is_building = input.is_building;
                if (string_to_boolean(props.is_building))
                {
                    obj.batchPrimitives();
                }
            }
            
            if ("lightmap" in input)
            {
                props.lightmap = input.lightmap;
                if (obj.isBakable)
                {
                    let filename = input.lightmap;
                    let hdr_img = HDRImageLoader.loadFile(filename);
                    if (hdr_img!=null)
                    {
                        obj.setLightmap(hdr_img);
                    }
                }
                
            }
            tuning_object3d(doc, obj, input);
        }
        return "";
    }
}

const avatar = {
    create: (doc, props, mode, parent) => {
        let avatar = model.create(doc, { ...props}, mode, parent);
        return avatar;
    },
    
    tuning: (doc, obj, input) =>  {
        model.tuning(doc,obj,input);
        return "";
    }
}


const directional_light = {
    create: (doc, props, mode, parent) => {
        const light = new DirectionalLight();           
        
        if (props.hasOwnProperty('intensity')) {
            light.intensity = parseFloat(props.intensity);
        }
        
        if (props.hasOwnProperty('target')){
            let target = doc.scene.getObjectByName(props.target);
            light.target = target;
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
                let bottom = parseFloat(area[2]);
                let top = parseFloat(area[3]);       
                let near = parseFloat(area[4]);
                let far = parseFloat(area[5]);
                light.setShadowProjection(left, right, bottom, top, near, far);
            }
            
            if (props.hasOwnProperty('radius'))
            {
                let radius = parseFloat(props.radius);
                light.setShadowRadius(radius);
            }
            
            if (props.hasOwnProperty('bias'))
            {
                light.bias = parseFloat(props.bias);
            }
            
            if (props.hasOwnProperty('force_cull'))
            {
                light.forceCull = string_to_boolean(props.force_cull);
            }
        }

        if (parent != null) {
            parent.add(light);
        }
        else {
            doc.scene.add(light);
        }
        doc.scene.addWidget(light);
        return light;
    },
    remove: (doc, obj) => {
        doc.scene.removeWidget(obj);
    },
    tuning: (doc, obj, input) => {
        let node = doc.internal_index[obj.uuid].xml_node;
        let props = node.attributes;
        
        if ("intensity" in input)
        {
            let intensity = input.intensity;
            props.intensity = intensity;
            obj.intensity = parseFloat(intensity);
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
        
        if ("target" in input)
        {
            props.target = input.target;
            let target = doc.scene.getObjectByName(input.target);
            obj.target = target;
        }
        
        if ("castShadow" in input)
        {
            props.castShadow = input.castShadow;
            
            let castShadow = string_to_boolean(input.castShadow);
            let width = 512;
            let height = 512;
            if (input.hasOwnProperty('size')) {
                props.size = input.size;
                let size = input.size.split(',');
                width = parseInt(size[0]);
                height = parseInt(size[1]);
            }
            obj.setShadow(castShadow, width, height);
        }
        
        if ("area" in input)
        {
            props.area = input.area;
            const area = input.area.split(',');
            let left = parseFloat(area[0]);
            let right = parseFloat(area[1]);
            let bottom = parseFloat(area[2]);
            let top = parseFloat(area[3]);       
            let near = parseFloat(area[4]);
            let far = parseFloat(area[5]);
            obj.setShadowProjection(left, right, bottom, top, near, far);
        }
        
        if ("radius" in input)
        {
            props.radius = input.radius;
            let radius = parseFloat(input.radius);
            obj.setShadowRadius(radius);
        }
        
        if ("bias" in input)
        {
            props.bias = input.bias;
            obj.bias = parseFloat(input.bias);
        }
        
        if ("force_cull" in input)
        {
            props.force_cull = input.force_cull;
            obj.forceCull = string_to_boolean(input.force_cull);
        }
        
        if ("auto_area" in input)
        {
            let aabb = obj.getBoundingBox(doc.scene);
            let minPos = aabb.minPos;
            let maxPos = aabb.maxPos;
            obj.setShadowProjection(minPos.x, maxPos.x, minPos.y, maxPos.y, -maxPos.z, -minPos.z);
            props.area = `${minPos.x}, ${maxPos.x}, ${minPos.y}, ${maxPos.y}, ${-maxPos.z}, ${-minPos.z}`;
            let ret = { area: props.area };
            return JSON.stringify(ret);
        }
        
        tuning_object3d(doc, obj, input);
        
        return "";
    }
}

class BackgroundDocument extends BackgroundScene
{
    constructor(near, far)
    {
        super(null, near, far);
        
        this.Tags = { scene, sky, env_light, group, plane, box, sphere, model, directional_light };
        this.reset();
    }
    
    reset() 
    {
        for (let tag in this.Tags) 
        {
            if (this.Tags[tag].hasOwnProperty('reset')) 
            {
                this.Tags[tag].reset(this);
            }
        }
    }
    
    create(tag, props, mode, parent = null) 
    {
        if (!(tag in this.Tags)) return null;
        
        const obj = this.Tags[tag].create(this, props, mode, parent);
        if (obj == null) return null;
        
        if (Object.isExtensible(obj)) 
        {
            obj.tag = tag;
        }
        
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
            if (img!=null)
            {
                obj.setColorTexture(img);
            }
        }
        
        if (props.hasOwnProperty('metalness'))
        {
            obj.metalness = parseFloat(props.metalness);
        }
        
        if (props.hasOwnProperty('roughness'))
        {
            obj.roughness = parseFloat(props.roughness);
        }
        
        if (props.hasOwnProperty('is_building'))
        {
            obj.isBuilding = string_to_boolean(props.is_building);
        }
        
        return obj;
    }
 
    load_xml_node(xmlNode, mode, parent = null)
    {
        if (parent == null) {
            parent = this;
        }
        for (let child of xmlNode.children) {           
            const obj = this.create(child.tagName, child.attributes, mode, parent);
            if (obj===null) continue;
            this.load_xml_node(child, mode, obj);
        }
        
    }
    
    load_xml(xmlText, mode)
    {
        const parsed = txml.parse(xmlText); 
        let root = null;
        for (let top of parsed)
        {
            if (top.tagName == 'document')
            {
                root = top;
                break;
            }
        }
        if (root)
        {
            this.load_xml_node(root, mode);
        }
    }
    
    load_local_xml(filename)
    {
        const xmlText = fileLoader.loadTextFile(filename);
        if (xmlText!=null)
        {
            this.load_xml(xmlText, "local");
        }
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
        this.picked_key = "";
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
        
        if (this.picked_key!="")
        {
            gamePlayer.message("object_picked", "");
            this.picked_key = "";
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
        this.bakables = [];
        
        this.lightmap_bake = null;
        this.env_gen = null;
        this.probe_grid_bake = null;
        this.lod_probe_grid_bake = null;
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
        this.renderer = renderer;
        
        if (this.lightmap_bake!=null)
        {
            this.lightmap_bake.render(renderer);
        }
        
        if (this.env_gen!=null)
        {
            this.env_gen.render(renderer);
        }
        
        if (this.probe_grid_bake!=null)
        {
            this.probe_grid_bake.render(renderer);
        }
        
        if (this.lod_probe_grid_bake!=null)
        {
            this.lod_probe_grid_bake.render(renderer);
        }
        
        if (this.scene && this.camera) 
        {
            renderer.render(this.scene, this.camera);
        }
    }
    
    create(tag, props, mode, parent = null) 
    {
        if (!(tag in this.Tags)) return null;
        
        const obj = this.Tags[tag].create(this, props, mode, parent);
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
            if (img!=null)
            {
                obj.setColorTexture(img);
            }
        }
        
        if (props.hasOwnProperty('metalness'))
        {
            obj.metalness = parseFloat(props.metalness);
        }
        
        if (props.hasOwnProperty('roughness'))
        {
            obj.roughness = parseFloat(props.roughness);
        }
        
        if (props.hasOwnProperty('is_building'))
        {
            obj.isBuilding = string_to_boolean(props.is_building);
        }
        
        return obj;
    }
    
    remove(obj)
    {
        if (obj.hasOwnProperty('tag')) {
            const tag = this.Tags[obj.tag];
            if (tag.hasOwnProperty('remove')) {
                tag.remove(this, obj);
            }
        }
        
        {
            let index = this.bakables.indexOf(obj);
            if (index>=0)
            {
                 this.bakables.splice(index, 1);
            }
        }
        
        if (obj.parent) 
        {
            obj.parent.remove(obj);
        }
    }
    
    load_xml_node(xmlNode, mode, parent = null)
    {
        for (let child of xmlNode.children) {           
            let obj = null;
            if (parent == null)
            {
                obj = this.create(child.tagName, child.attributes, mode, this);
            }
            else
            {
                obj = this.create(child.tagName, child.attributes, mode, parent);
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
            
            this.load_xml_node(child, mode, obj);
        }
    }
   
    
    load_xml(xmlText, mode)
    {
        this.xml_nodes = txml.parse(xmlText, {keepComments: true});
        this.saved_text = genXML(this.xml_nodes);
        
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
            this.load_xml_node(root, mode);
        }
        
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
        
        if (this.picked_key != "")
        {
            if (this.picked_key in this.internal_index)
            {
                let picked_obj = this.internal_index[this.picked_key].obj;
                if (picked_obj.hasOwnProperty("setToonShading"))
                {
                    picked_obj.setToonShading(0);
                }
            }
        }
        
        this.picked_key = key;
        
        if (obj!=null)
        {
            if (obj.hasOwnProperty("setToonShading"))
            {
                obj.setToonShading(16, 5.0, new Vector3(1.0, 1.0, 0.2));
            }
        }
        gamePlayer.message("object_picked", key);
    }
    
    tuning(input)
    {
        if (this.picked_key=="") return "";
        let picked_obj = this.internal_index[this.picked_key].obj;
        let node = this.internal_index[this.picked_key].xml_node;
        let tag = node.tagName;
        
        if (!(tag in this.Tags)) return "";
        return this.Tags[tag].tuning(this, picked_obj, input);
    }
    
    initialize(input)
    {
        if (this.picked_key=="") return "";
        let picked_obj = this.internal_index[this.picked_key].obj;
        let node = this.internal_index[this.picked_key].xml_node;
        let tag = node.tagName;
        
        if (!(tag in this.Tags)) return "";
        return this.Tags[tag].initialize(this, picked_obj, input);
    }

    generate(input)
    {
        if (this.picked_key=="") return;
        let picked_obj = this.internal_index[this.picked_key].obj;
        let node = this.internal_index[this.picked_key].xml_node;
        let tag = node.tagName;
        
        if (!(tag in this.Tags)) return;
        this.Tags[tag].generate(this, picked_obj, input);
    }
    
    req_create(base_key, tag)
    {
        let internal_node_base = this.internal_index[base_key];
        let external_node_base = this.external_index.index[base_key];
        
        let xmlNode = internal_node_base.xml_node;
        let parent = internal_node_base.obj;
        
        let child = {tagName:tag, attributes: {}, children: []};
        xmlNode.children.push(child);
        
        let obj = this.create(tag, {}, "local", parent);
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
    let x = event.clientX;
    let y = event.clientY;
    
    let intersect = gamePlayer.pickObject(x,y);
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



