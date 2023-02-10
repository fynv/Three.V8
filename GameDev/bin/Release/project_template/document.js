import { OrbitControls } from "./controls/OrbitControls.js";
import { Vector3 } from "./math/Vector3.js";
import { view, UIViewDispatcher } from "./view.js";

import * as txml from "./txml.js";

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


class JoyStick
{
    constructor(options)
    {
        let width = options.width;
        let height = options.height;
        this.center_x = width * 0.5;
        this.center_y = height - 75.0;
                
        this.ui_area = new UIArea();
        this.ui_area.setOrigin(this.center_x-60.0, this.center_y - 60.0);
        this.ui_area.setSize(120.0, 120.0);
        UIManager.add(this.ui_area);
        
        this.circle = new UIPanel();
        this.circle.setOrigin(20.0, 20.0);
        this.circle.setSize(80.0, 80.0);
        this.circle.setStyle({cornerRadius:40.0, colorBg: "#7E7E7E7E"});        
        this.ui_area.add(this.circle); 
        
        this.thumb = new UIPanel();
        this.thumb.setOrigin(40.0, 40.0);
        this.thumb.setSize(40.0, 40.0);
        this.thumb.setStyle({cornerRadius:20.0, colorBg: "#FFFFFFFF", shadowOffset:0.0});
        this.ui_area.add(this.thumb); 
        
        this.maxRadius = options.maxRadius || 40;
        this.maxRadiusSquared = this.maxRadius * this.maxRadius;
        this.onMove = options.onMove;
        
        const move = (x, y)=>
        {            
            let delta_x = x - this.offset.x;
            let delta_y = y - this.offset.y;
            
            const run_status = Math.abs(delta_x) - 50 >= 0 || Math.abs(delta_y) - 50 >= 0;
            const sqMag = delta_x * delta_x + delta_y * delta_y;
            if (sqMag > this.maxRadiusSquared) {
                const magnitude = Math.sqrt(sqMag);
                delta_x /= magnitude;
                delta_y /= magnitude;
                delta_x *= this.maxRadius;
                delta_y *= this.maxRadius;
            }
            
            this.thumb.setOrigin(40.0 + delta_x, 40.0 + delta_y);
            
            const forward = -delta_y / this.maxRadius;
            const turn = delta_x / this.maxRadius;
            
            if (this.onMove != undefined) this.onMove(forward, turn, run_status);        
        }
        
        const up = (x, y) =>
        {
            this.thumb.onPointerMove = null;
            this.thumb.onPointerUp = null;   
            this.thumb.setOrigin(40.0, 40.0);
            if (this.onMove != undefined) this.onMove(0,0,false);
            gamePlayer.message("releasePointerCapture", "");
        }
        
        const tap = (x, y) =>
        {         
            this.offset = { x, y };
            this.thumb.onPointerMove = move;
            this.thumb.onPointerUp = up;
            gamePlayer.message("setPointerCapture", "");
        };
        
        this.thumb.onPointerDown = tap;
        
        
    }
    
    onResize(width, height)
    {
        let center_x =  width * 0.5;
        let center_y =  height - 75.0;
        if (center_x!=this.center_x || center_y!= this.center_y)
        {
            this.center_x = center_x;
            this.center_y = center_y;
            this.ui_area.setOrigin(this.center_x-60.0, this.center_y - 60.0);
        }
    }

    
}

class AnimCrossFader
{
    constructor(fade_time)
    {
        this.mixer = new AnimationMixer();
        this.fade_time = fade_time || 0.5;        
    }
    
    add_clips(clips)
    {
        this.mixer.addAnimations(clips);
    }
    
    set_current(clip_name)
    {
        this.start_time = now();
        this.mixer.startAnimation(clip_name);
    }
    
    update_weights(weight_cur)
    {
        let lst_active = this.mixer.currentPlaying;
        let weight_last = lst_active[lst_active.length - 1].weight;        
        let k = (1.0 - weight_cur)/(1.0-weight_last);
        let weights = [];
        for (let i=0; i< lst_active.length-1; i++)
        {
            let w = lst_active[i].weight * k;
            weights.push(w);
        }
        weights.push(weight_cur);
        this.mixer.setWeights(weights);
    }
    
    get_frame()
    {        
        let lst_active = this.mixer.currentPlaying;        
        if (lst_active.length<1) return;
        if (lst_active.length>1)
        {
            let cur_time = now();
            let delta = cur_time - this.start_time;
            let weight = 1.0;
            if (delta<this.fade_time)
            {
                weight = delta/this.fade_time;
            }
            this.update_weights(weight);
        }
        return this.mixer.getFrame();
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
        else if (type == "scene")
        {
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
            await bg_doc.load_local_xml(path_scene);
            doc.scene.background = bg_doc;
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
                
                let cube_img = imageLoader.loadCubeFromFile(
                    url+"/"+posx, url+"/"+negx, 
                    url+"/"+posy, url+"/"+negy, 
                    url+"/"+posz, url+"/"+negz);
                
                let envLight = null;
                if (cube_img!=null)
                {
                    let envMapCreator = new EnvironmentMapCreator();
                    envLight = envMapCreator.create(cube_img);
                    print(JSON.stringify(envLight.shCoefficients));
                }
                else
                {
                    envLight = new EnvironmentMap();
                }
                
                doc.scene.indirectLight = envLight;
            }
        }
        else if (type == "probe_grid")
        {
            let probe_data = "assets/probes.dat";
            if (props.hasOwnProperty('probe_data')) 
            {
                probe_data = props.probe_data;
            }
            let probe_grid = probeGridLoader.loadFile(probe_data);
            if (probe_grid == null)
            {
                probe_grid = new ProbeGrid();                
            }
            doc.scene.indirectLight = probe_grid;
        }
        
        if (props.hasOwnProperty('dynamic_map'))
        {
            doc.scene.indirectLight.dynamicMap = string_to_boolean(props.dynamic_map);
        }
        
        return doc.scene.indirectLight;
    },
    remove: (doc, obj) => {
        create_default_env_light(doc);
    },
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
        let avatar = await model.create(doc, props, mode, parent);
        doc.avatar = avatar;
        if (!props.hasOwnProperty('lookat_height')) {
            props.lookat_height = '1';
        }
        
        let lookat_height = parseFloat(props.lookat_height);
        avatar.state = "idle";
        avatar.move = null;
        
        const name_idle = props.name_idle;
        const name_forward = props.name_forward;
        const name_backward = props.name_backward;
                
        cam = new Object3D();
        const camera_position = props.camera_position.split(',');
        const [camx, camy, camz] = camera_position;
        cam.setPosition(+camx, +camy, +camz);
        avatar.add(cam);
        
        const anims = gltfLoader.loadAnimationsFromFile(props.url_anim);
        if (props.hasOwnProperty('fix_anims')) {
            doc.loaded_module[props.fix_anims](anims);
        } 
        
        let mixer = new AnimCrossFader();
        mixer.add_clips(anims);
        mixer.set_current(name_idle);
        avatar.cur_action = name_idle;
        avatar.mixer = mixer;       
        
        const onMove = (forward, turn, run_status)=>
        {
            avatar.move = { forward, turn};
            let action_changed = false;
            let new_action = null;
            
            if (forward > 0)
            {
                if (avatar.state != "forward")
                {
                    new_action = name_forward;
                    action_changed = true;
                    avatar.state = "forward";
                }            
            }
            else if (forward < 0)
            {
                if (avatar.state != "backward")
                {
                    new_action = name_backward;
                    action_changed = true;
                    avatar.state = "backward";
                }
            }
            else
            {
                if (avatar.state != "idle")
                {
                    new_action = name_idle;
                    action_changed = true;
                    avatar.state = "idle";
                }
            }
                   
            if (action_changed)
            {
                mixer.set_current(new_action);
                avatar.cur_action = new_action;
            }
        }
        
        const joystick = new JoyStick({ width: doc.width, height: doc.height, onMove: onMove});   
        
        if (doc.controls != null) {
            doc.controls.dispose();
            doc.controls = null;
        }
        
        const update_avatar = (doc, mixer, delta) => {
            let frame = mixer.get_frame();
            avatar.setAnimationFrame(frame);
            
            joystick.onResize(doc.width, doc.height);
            
            if (avatar.move)
            {
                if (avatar.move.forward !=0)
                {
                    let threshold = 0.4;
                    let movement = delta * avatar.move.forward * 2.0;
                    let pos = avatar.getPosition(new Vector3());
                    pos.y += threshold;
                    
                    if (doc.building.length > 0 && movement !=0)
                    {
                        // cast front
                        {
                            let negated = false;
                            let dir = avatar.getWorldDirection(new Vector3());
                            if (avatar.move.forward < 0) 
                            {
                                dir.negate();
                                negated = true;
                            }
                                
                            const intersect = doc.bvh.intersect({origin: pos, direction: dir});
                            if (intersect!==null)
                            {                    
                                if (!negated)
                                {
                                    if (movement > intersect.distance - 0.3) 
                                    {
                                        movement = intersect.distance - 0.3;
                                    }
                                }
                                else
                                {
                                    if (-movement > intersect.distance - 0.3) 
                                    {
                                        movement = 0.3 - intersect.distance;
                                    }
                                }
                            }
                        }
                    }
                    
                    avatar.translateZ(movement);
                    pos = avatar.getPosition(new Vector3());
                    pos.y += threshold;
                    
                    if (doc.building.length > 0)
                    {
                        if (movement !=0)
                        {
                            // cast up
                            {
                                let dir = new Vector3(0,1,0);
                                const intersect = doc.bvh.intersect({origin: pos, direction: dir});
                                if (intersect!==null)
                                {
                                    const targetY = pos.y + intersect.distance;
                                    if (targetY < 2.0)
                                    {
                                        avatar.translateZ(-movement);
                                        pos = avatar.getPosition(new Vector3());
                                        pos.y += threshold;
                                    }
                                }
                            }
                            
                            // cast left
                            {
                                let cast_from =  avatar.localToWorld(new Vector3(0,0,0));
                                let cast_to = avatar.localToWorld(new Vector3(1,0,0));
                                let dir = new Vector3();
                                dir.subVectors(cast_to, cast_from);
                                const intersect = doc.bvh.intersect({origin: pos, direction: dir});
                                if (intersect!==null)
                                {
                                    if (intersect.distance < 0.3) 
                                    {
                                        avatar.translateX(intersect.distance - 0.3);
                                        pos = avatar.getPosition(new Vector3());
                                        pos.y += threshold;
                                    }
                                }
                            }
                            
                            // cast right
                            {
                                let cast_from =  avatar.localToWorld(new Vector3(0,0,0));
                                let cast_to = avatar.localToWorld(new Vector3(-1,0,0));
                                let dir = new Vector3();
                                dir.subVectors(cast_to, cast_from);
                                const intersect = doc.bvh.intersect({origin: pos, direction: dir});
                                if (intersect!==null)
                                {
                                    if (intersect.distance < 0.3) 
                                    {
                                        avatar.translateX(0.3 - intersect.distance);
                                        pos = avatar.getPosition(new Vector3());
                                        pos.y += threshold;
                                    }
                                }
                            }
                        }
                        
                        
                        // cast down
                        {
                            let dir = new Vector3(0, -1, 0);
                            pos.y += 0.2;
                            const intersect = doc.bvh.intersect({origin: pos, direction: dir});
                            
                            const gravity = 0.3;
                            if (intersect!==null)
                            {
                                const targetY = pos.y - intersect.distance;
                                if (targetY > avatar.position.y) {
                                    //Going up                            
                                    avatar.translateY(0.2 * (targetY - avatar.position.y));
                                    avatar.velocityY = 0;
                                }
                                else if (targetY < avatar.position.y){
                                    // Falling
                                    if (avatar.velocityY == undefined) avatar.velocityY = 0;
                                    avatar.velocityY += delta * gravity;
                                    avatar.translateY(-avatar.velocityY);
                                    if (avatar.position.y < targetY) {
                                        avatar.velocityY = 0;
                                        avatar.translateY(targetY - avatar.position.y);
                                    }
                                }
                            }
                        }
                    }
                }
                if (avatar.move.turn != 0)
                {
                    avatar.rotateY(-avatar.move.turn * delta);
                }
            }
            let look_from = cam.getWorldPosition(new Vector3());
            let look_at = avatar.getWorldPosition(new Vector3());
            look_at.y += lookat_height;
            
            let dir = new Vector3();
            dir.subVectors(look_from, look_at);
            let dis = dir.length();
            dir.normalize();
            const intersect = doc.bvh.intersect({origin: look_at, direction: dir});
            if (intersect!==null)
            {
                let max_dis = intersect.distance * 0.9 + doc.camera.near;
                if (dis > max_dis)
                {
                    dir.multiplyScalar(max_dis);
                    look_from.addVectors(look_at, dir);
                }        
            }
            let cam_pos = doc.camera.getPosition(new Vector3());    
            cam_pos.lerp(look_from, 0.1);
            doc.camera.setPosition(cam_pos);
            doc.camera.lookAt(look_at);
            
        }
        doc.set_tick(mixer, update_avatar);
        
        return avatar;

    },
    
    remove: (doc, avatar) =>
    {
        doc.remove_tick(avatar.mixer);
        doc.avatar = null;
    }
}

const character = {
    create: async (doc, props, mode, parent) => {
        let character = await model.create(doc, props, mode, parent);
        character.state = "idle";
        character.move = null;
        character.name_idle = props.name_idle;
        character.name_forward = props.name_forward;
        character.name_backward = props.name_backward;
        
        const anims = gltfLoader.loadAnimationsFromFile(props.url_anim);        
        if (props.hasOwnProperty('fix_anims')) {
            doc.loaded_module[props.fix_anims](anims);
        }       
        
        let mixer = new AnimCrossFader();
        mixer.add_clips(anims);
        mixer.set_current(character.name_idle);
        character.cur_action = character.name_idle;
        character.mixer = mixer;
        
        const update_character = (doc, mixer, delta) => {
            let frame = mixer.get_frame();
            character.setAnimationFrame(frame);
        };      
        doc.set_tick(mixer, update_character);
                
        return character;
    },
    
    remove: (doc, character) =>
    {
        doc.remove_tick(character.mixer);
    },
    
    set_state: (doc, character, state) => {
        let new_action = null;
        if (state == 'idle') {
            new_action = character.name_idle;
        }
        else if (state == 'forward') {
            new_action = character.name_forward;
        }
        else if (state == 'backward') {
            new_action = character.name_backward;
        }       
        character.mixer.set_current(new_action);
        character.state = state;
    }
}

const directional_light = {
    create: async (doc, props, mode, parent) => {
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
                let top = parseFloat(area[2]);
                let bottom = parseFloat(area[3]);       
                let near = parseFloat(area[4]);
                let far = parseFloat(area[5]);
                light.setShadowProjection(left, right, top, bottom, near, far);
            }
            
            if (props.hasOwnProperty('radius'))
            {
                let radius = parseFloat(props.radius);
                light.setShadowRadius(radius);
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
    
    async create(tag, props, mode, parent = null) 
    {
        if (!(tag in this.Tags)) return null;
        
        const obj = await this.Tags[tag].create(this, props, mode, parent);
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
        
        return obj;
    }
 
    async load_xml_node(xmlNode, mode, parent = null)
    {
        if (parent == null) {
            parent = this;
        }
        for (let child of xmlNode.children) {           
            const obj = await this.create(child.tagName, child.attributes, mode, parent);
            if (obj===null) continue;
            await this.load_xml_node(child, mode, obj);
        }
        
    }
    
    async load_xml(xmlText, mode)
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
            await this.load_xml_node(root, mode);
        }
    }
    
    async load_local_xml(filename)
    {
        const xmlText = fileLoader.loadTextFile(filename);
        if (xmlText!=null)
        {
            await this.load_xml(xmlText, "local");
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
        this.Tags = { scene, camera, fog, sky, env_light, control, group, plane, box, sphere, model, avatar, character, directional_light };
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
        if (this.unloadables) 
        {
            for (const object of this.unloadables) 
            {
                object.unload(this, object);
            }
        }
        
        this.unloadables = [];
        this.updatables = [];
        this.building = [];
        
        for (let tag in this.Tags) 
        {
            if (this.Tags[tag].hasOwnProperty('reset')) 
            {
                this.Tags[tag].reset(this);
            }
        }
        
        this.loaded_module = null;
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
        
        for (const object of this.updatables) 
        {
            object.tick(this, object, delta);
        }
    }
    
    render(renderer)
    {
        
        if (this.scene && this.camera) 
        {
            renderer.render(this.scene, this.camera);
        }
    }
    
    set_unload(obj, func) 
    {
        obj.unload = func;
        this.unloadables.push(obj);
    }

    remove_unload(obj) 
    {
        for (let i = 0; i < this.unloadables.length; i++) 
        {
            if (this.unloadables[i] == obj) 
            {
                this.unloadables.splice(i, 1);
                i--;
            }
        }
    }

    set_tick(obj, func) 
    {
        obj.tick = func;
        this.updatables.push(obj);
    }
    
    remove_tick(obj) 
    {
        for (let i = 0; i < this.updatables.length; i++) 
        {
            if (this.updatables[i] == obj) 
            {
                this.updatables.splice(i, 1);
                i--;
            }
        }
    }
    
    add_building_object(obj) {
        this.building.push(obj);
    }

    remove_building_object(obj) {
        for (let i = 0; i < this.building.length; i++) {
            if (this.building[i] == obj) {
                this.building.splice(i, 1);
                i--;
            }
        }
    }
    
    async create(tag, props, mode, parent = null) 
    {
        if (!(tag in this.Tags)) return null;
        
        const obj = await this.Tags[tag].create(this, props, mode, parent);
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
        
        if (props.hasOwnProperty('is_building') && string_to_boolean(props.is_building)) {
            this.add_building_object(obj);
        }
        
        if (props.hasOwnProperty('ontick')) 
        {           
            this.set_tick(obj, this.loaded_module[props.ontick]);
        }

        if (props.hasOwnProperty('onload')) 
        {
            obj.load = this.loaded_module[props.onload];
        }

        if (props.hasOwnProperty('onunload')) 
        {
            this.set_unload(obj, this.loaded_module[props.onunload]);
        }
        
        return obj;
    }
    
    remove(obj)
    {
        const doc = this;
        obj.traverse((child) => {
            this.remove_unload(child);
            this.remove_tick(child);
            this.remove_building_object(child);
            if (child.hasOwnProperty('tag')) {
                const tag = this.Tags[child.tag];
                if (tag.hasOwnProperty('remove')) {
                    tag.remove(this, obj);
                }
            }
        });
        if (obj.parent) 
        {
            obj.parent.remove(obj);
        }
    }
    
    async load_xml_node(xmlNode, mode, parent = null)
    {
        if (parent == null) {
            parent = this;
        }
        for (let child of xmlNode.children) {           
            const obj = await this.create(child.tagName, child.attributes, mode, parent);
            if (obj===null) continue;
            await this.load_xml_node(child, mode, obj);
            if (obj.load) {
                obj.load(this, obj);
            }
        }
        
    }
    
    async load_xml(xmlText, mode)
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
            await this.load_xml_node(root, mode);
        }
        
        if (this.building.length>0)
        { 
            this.bvh = new BoundingVolumeHierarchy(this.building);
        }
        else
        {
            this.bvh = null;
        }
    }
    
    async load_local_xml(filename)
    {
        const xmlText = fileLoader.loadTextFile(filename);  
        await this.load_xml(xmlText, "local");
    }
    
    async load(module)
    {
        this.reset();
        this.loaded_module = module;
        await module.load(this);
    }
    
}
