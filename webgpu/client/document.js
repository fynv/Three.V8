import { Vector3 } from "./engine/math/Vector3.js"
import { ImageLoader } from "./engine/loaders/ImageLoader.js"
import { HDRImageLoader } from "./engine/loaders/HDRImageLoader.js"
import { GPURenderTarget } from "./engine/renderers/GPURenderTarget.js"
import { Object3D } from "./engine/core/Object3D.js"
import { Scene } from "./engine/scenes/Scene.js"
import { PerspectiveCameraEx } from "./engine/cameras/PerspectiveCameraEx.js"
import { OrbitControls } from "./engine/controls/OrbitControls.js"
import { ColorBackground, HemisphereBackground, CubeBackground } from "./engine/backgrounds/Background.js"
import { BackgroundScene } from "./engine/backgrounds/BackgroundScene.js"
import { AmbientLight} from "./engine/lights/AmbientLight.js"
import { HemisphereLight } from "./engine/lights/HemisphereLight.js"
import { EnvironmentMap } from "./engine/lights/EnvironmentMap.js"
import { EnvironmentMapCreator} from "./engine/lights/EnvironmentMapCreator.js"
import { SimpleModel } from "./engine/models/SimpleModel.js"
import { GLTFLoader } from "./engine/loaders/GLTFLoader.js"
import { DirectionalLight } from "./engine/lights/DirectionalLight.js"
import { LightmapLoader } from "./engine/loaders/LightmapLoader.js"
import { BoundingVolumeHierarchy } from "./engine/core/BoundingVolumeHierarchy.js"
import { AnimationMixer } from "./engine/models/AnimationMixer.js"

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

class JoyStick {
    constructor(options) {
        const circleOut = document.createElement("div")
        circleOut.style.cssText = "position:absolute; bottom:35px; left:50%; transform:translateX(-50%) translateY(40px); border-radius:50%; padding:80px";
        const circle = document.createElement("div");
        circle.style.cssText = "position:relative; width:80px; height:80px; background:rgba(126, 126, 126, 0.5); border:#444 solid medium; border-radius:50%;";
        const thumb = document.createElement("div");
        thumb.style.cssText = "position: absolute; left: 20px; top: 20px; width: 40px; height: 40px; border-radius: 50%; background: #fff;";
        circleOut.appendChild(circle)
        circle.appendChild(thumb);
        document.body.appendChild(circleOut);
        
        this.domElement = thumb;
        this.domCircle = circleOut
        this.maxRadius = options.maxRadius || 40;
        this.maxRadiusSquared = this.maxRadius * this.maxRadius;
        this.onMove = options.onMove;
        this.doc = options.doc;
        
        const move = (evt) => {
            
            evt = evt || window.event;
            const mouse = this.getMousePosition(evt);
            let delta_x = mouse.x - this.offset.x;
            let delta_y = mouse.y - this.offset.y;
            
            const run_status = Math.abs(delta_x) - 50 >= 0 || Math.abs(delta_y) - 50 >= 0;
            const sqMag = delta_x * delta_x + delta_y * delta_y;
            if (sqMag > this.maxRadiusSquared) {
                const magnitude = Math.sqrt(sqMag);
                delta_x /= magnitude;
                delta_y /= magnitude;
                delta_x *= this.maxRadius;
                delta_y *= this.maxRadius;
            }
            
            this.domElement.style.left = `${20.0 + delta_x}px`;
            this.domElement.style.top = `${20.0 + delta_y}px`;            
            
            const forward = -delta_y / this.maxRadius;
            const turn = delta_x / this.maxRadius;
            
            if (this.onMove != undefined) this.onMove(forward, turn, run_status);
            
            evt.stopPropagation();
        };
        
        const up = (evt) => {            
            if ('ontouchstart' in window) {
                this.domCircle.ontouchmove = null;
                this.domCircle.ontouchend = null;
            }
            else {                
                document.onmousemove = null;
                document.onmouseup = null;
            }     
            this.domElement.style.top = `20.0px`;
            this.domElement.style.left = `20.0px`;
            if (this.onMove != undefined) this.onMove(0, 0, false);                        
            
            evt.stopPropagation();
        };
        
        const tap = (evt) =>
        {
            evt = evt || window.event;
            this.offset = this.getMousePosition(evt);
            
            if ('ontouchstart' in window) {
                this.domCircle.ontouchmove = move;
                this.domCircle.ontouchend = up;
            }
            else {                
                document.onmousemove = move;
                document.onmouseup = up;
            }
            
            evt.stopPropagation();
        };
        
        if (this.domElement != undefined) {         
            if ('ontouchstart' in window) {
                this.domElement.addEventListener('touchstart', tap);
            }
            else {
                this.domElement.addEventListener('mousedown', tap);
            }
        }
        
    }
    
    getMousePosition(evt) {
        let clientX = evt.targetTouches ? evt.targetTouches[0].pageX : evt.clientX;
        let clientY = evt.targetTouches ? evt.targetTouches[0].pageY : evt.clientY;
        return { x: clientX, y: clientY };
    }
}

class AnimCrossFader
{
    constructor(fade_time)
    {
        this.mixer = new AnimationMixer();
        this.fade_time = fade_time || 500.0;        
    } 

    add_clips(clips)
    {
        this.mixer.animations = clips.animations;
        this.mixer.animation_dict = clips.animation_dict;
    }

    set_current(clip_name)
    {        
        this.start_time = Date.now();
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
            let cur_time = Date.now();            
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
    doc.controls = new OrbitControls(doc.camera, doc.canvas);
    doc.controls.enableDamping = true;
    doc.controls.target.set(0, 1.5, 0);
}

const create_default_sky = (doc)=>{
    let bg = new HemisphereBackground();   
    bg.skyColor.setRGB(0.318, 0.318, 0.318);
    bg.groundColor.setRGB(0.01, 0.025, 0.025);
    doc.scene.background = bg;
}

const create_default_env_light = (doc) =>{
    let envLight = new HemisphereLight();
    envLight.skyColor.setRGB(0.318, 0.318, 0.318);
    envLight.groundColor.setRGB(0.01, 0.025, 0.025);
    doc.scene.indirectLight = envLight;    
}

const scene = {
    reset: (doc) => {
        doc.scene = new Scene();
    },
    create: (doc, props, parent) => {
        doc.scene = new Scene();
        create_default_sky(doc);
        create_default_env_light(doc);
        return doc.scene;
    }
}

const camera = {
    reset: (doc) => {
        doc.camera = new PerspectiveCameraEx(45, doc.width / doc.height, 0.1, 100);
        doc.camera.position.set(0, 1.5, 5.0);
    },

    create: (doc, props, parent) => {
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
        doc.camera = new PerspectiveCameraEx(fov, doc.width / doc.height, near, far);
        create_default_controls(doc);
        return doc.camera;
    }
};

const control = {
    reset: (doc) => {
        create_default_controls(doc);
    },
    create: (doc, props, parent) =>{
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
            
            doc.camera.position.set(from_x, from_y, from_z);
            if (doc.controls != null)
                doc.controls.dispose();
            doc.controls = new OrbitControls(doc.camera, doc.canvas);
            doc.controls.enableDamping = true;
            doc.controls.target.set(to_x, to_y, to_z);
        }
    }
};

const sky = {
    reset: (doc) => {
        create_default_sky(doc);
    },
    create: (doc, props, parent) => {        
        let type = "hemisphere"
        if (props.hasOwnProperty("type"))
        {
            type = props.type;
        }        
        if (type == "uniform")
        {
            let bg = new ColorBackground();            
            
            if (props.hasOwnProperty('color'))
            {
                const color = props.color.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.color.setRGB(r,g,b);                
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
                bg.skyColor.setRGB(r,g,b);                
            }
            
            if (props.hasOwnProperty('groundColor'))
            {
                const color = props.groundColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                bg.groundColor.setRGB(r,g,b);
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

            (async() =>{                
                let cube_img = await doc.imageLoader.loadCubeFromFile([
                    url+"/"+posx, url+"/"+negx, 
                    url+"/"+posy, url+"/"+negy, 
                    url+"/"+posz, url+"/"+negz]);                
                bg.setCubemap(cube_img);                
            })();
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

            let bg_doc = new BackgroundDocument(doc, near, far);
            bg_doc.load_local_xml(path_scene);
            doc.scene.background = bg_doc;

        }
        return doc.scene.background;
    },
    remove: (doc, obj) => {
        create_default_sky(doc);
    }
};

const env_light = {
    reset: (doc) => {
        create_default_env_light(doc);
    },
    create: (doc, props, parent) => {
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
                envLight.color.setRGB(r,g,b);
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
                envLight.skyColor.setRGB(r,g,b);
            }
            
            if (props.hasOwnProperty('groundColor'))
            {
                const color = props.groundColor.split(',');
                const r = parseFloat(color[0]);
                const g = parseFloat(color[1]);
                const b = parseFloat(color[2]);
                envLight.groundColor.setRGB(r,g,b);
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
                (async()=>{
                    const response = await fetch(path_sh);
                    const text = await response.text();
                    envLight.shCoefficients = JSON.parse(text);

                })();
                
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
                    let envMapCreator = new EnvironmentMapCreator();
                    (async()=>
                    { 
                        let cube_img = await doc.hdrImageLoader.loadCubeFromFile([
                            url+"/"+posx, url+"/"+negx, 
                            url+"/"+posy, url+"/"+negy, 
                            url+"/"+posz, url+"/"+negz]);
                        let envLight = envMapCreator.create(cube_img);
                        doc.scene.indirectLight = envLight;
                    })();
                }
                else
                {
                    let envMapCreator = new EnvironmentMapCreator(); 
                    (async()=>
                    {                       
                        let cube_img = await doc.imageLoader.loadCubeFromFile([
                            url+"/"+posx, url+"/"+negx, 
                            url+"/"+posy, url+"/"+negy, 
                            url+"/"+posz, url+"/"+negz]);                        
                        let envLight = envMapCreator.create(cube_img);
                        doc.scene.indirectLight = envLight;
                    })();                    
                }
            }
        }

        return doc.scene.indirectLight;
    },
    remove: (doc, obj) => {
        create_default_env_light(doc);        
    },
};


const group = {
    create: (doc, props, parent) => {
        const group = new Object3D();
        if (parent != null) {
            parent.add(group);
        }
        else {
            doc.scene.add(group);
        }
        return group;
    }
};


const plane = {
    create: (doc, props, parent) => {
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
};


const box = {
    create: (doc, props, parent) => {
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
};

const sphere = {
    create: (doc, props, parent) => {
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
};

const model = {
    create: (doc, props, parent) => {
        let url = "assets/models/model.glb";
        if (props.hasOwnProperty('src'))
        {
            url = props.src;
        }
        let model = doc.model_loader.loadModelFromFile(url);
        
        if (props.hasOwnProperty('lightmap'))
        {
            let lightmap_loader = new LightmapLoader();

            let filename = props.lightmap;
            let ext = filename.split('.').pop().toLowerCase();
            if (ext=="hdr")
            {                
                (async()=>{
                    let lightmap = await lightmap_loader.fromHDR(filename);
                    model.setLightmap(lightmap);
                })();
            }
            else if (ext=="png" || ext == "webp")
            {                
                (async()=>{
                    let lightmap = await lightmap_loader.fromRGBM(filename);
                    model.setLightmap(lightmap);
                })();
            }
            if (ext=="csv")
            {                
                (async()=>{
                    let lightmap = await lightmap_loader.fromImages(filename);
                    model.setLightmap(lightmap);
                })();
            }
        }

        if (parent != null) {
            parent.add(model);
        }
        else {
            doc.scene.add(model);
        }
        return model;
    }
};

const avatar = {
    create: (doc, props, parent) => {
        let avatar = model.create(doc, props, parent);
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

        let cam = new Object3D();
        const camera_position = props.camera_position.split(',');
        const [camx, camy, camz] = camera_position;
        cam.position.set(+camx, +camy, +camz);
        avatar.add(cam);

        const anims = doc.model_loader.loadAnimationsFromFile(props.url_anim, ()=>{
            if (props.hasOwnProperty('fix_anims')) 
            {            
                doc.loaded_module[props.fix_anims](anims.animations);
            }
        });

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
        };

        const joystick = new JoyStick({ width: doc.width, height: doc.height, onMove: onMove});   

        if (doc.controls != null) {
            doc.controls.dispose();
            doc.controls = null;
        }
        

        const update_avatar = (doc, mixer, delta) => {            
            let frame = mixer.get_frame();
            avatar.setAnimationFrame(frame);

            if (avatar.move)
            {
                if (avatar.move.forward !=0)
                {
                    let threshold = 0.4;
                    let movement = delta * avatar.move.forward * 2.0;
                    let pos = avatar.position.clone();
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
                                    if (movement > intersect.t - 0.3) 
                                    {
                                        movement = intersect.t - 0.3;
                                    }
                                }
                                else
                                {
                                    if (-movement > intersect.t - 0.3) 
                                    {
                                        movement = 0.3 - intersect.t;
                                    }
                                }
                            }
                        }
                    }

                    avatar.translateZ(movement);
                    pos = avatar.position.clone();
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
                                    const targetY = threshold + intersect.t;
                                    if (targetY < 2.0)
                                    {
                                        avatar.translateZ(-movement);
                                        pos = avatar.position.clone();
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
                                    if (intersect.t < 0.3) 
                                    {
                                        avatar.translateX(intersect.t - 0.3);
                                        pos = avatar.position.clone();
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
                                    if (intersect.t < 0.3) 
                                    {
                                        avatar.translateX(0.3 - intersect.t);
                                        pos = avatar.position.clone();
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
                                const targetY = pos.y - intersect.t;
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
                let max_dis = intersect.t * 0.9 + doc.camera.near;
                if (dis > max_dis)
                {
                    dir.multiplyScalar(max_dis);
                    look_from.addVectors(look_at, dir);
                }        
            }            
            doc.camera.position.lerp(look_from, 0.1);            
            doc.camera.lookAt(look_at);

            
        };

        doc.set_tick(mixer, update_avatar);

        return avatar;

    },

    remove: (doc, avatar) =>
    {
        doc.remove_tick(avatar.mixer);
        doc.avatar = null;
    }


};

const directional_light = {
    create: (doc, props, parent) => {
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
        
        return light;
    }

};


class BackgroundDocument extends BackgroundScene
{
    constructor(doc, near, far)
    {
        super(null, near, far);

        if ("main_doc" in doc)
        {
            this.main_doc = doc.main_doc;
        }
        else
        {
            this.main_doc = doc;
        }
        this.imageLoader = this.main_doc.imageLoader;
        this.hdrImageLoader = this.main_doc.hdrImageLoader;
        this.model_loader = this.main_doc.model_loader;    
        
        this.Tags = { scene, sky, env_light, group, plane, box, sphere, model, directional_light};
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

    create(tag, props, parent = null) 
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
            obj.position.set(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
        }

        if (props.hasOwnProperty('rotation')) 
        {
            const rotation = props.rotation.split(',');
            obj.rotation.set(parseFloat(rotation[0])* Math.PI / 180.0, parseFloat(rotation[1])* Math.PI / 180.0, parseFloat(rotation[2])* Math.PI / 180.0);      
        }

        if (props.hasOwnProperty('scale')) 
        {
            const scale = props.scale.split(',');
            obj.scale.set(parseFloat(scale[0]), parseFloat(scale[1]), parseFloat(scale[2]));
        }

        if (props.hasOwnProperty('color')) 
        {
            const color = props.color.split(',');
            const r = parseFloat(color[0]);
            const g = parseFloat(color[1]);
            const b = parseFloat(color[2]);
            obj.color.setRGB(r,g,b);
        }

        if (props.hasOwnProperty('texture'))
        {
            (async()=>{
                let img = await this.imageLoader.loadFile(props.texture);
                obj.setColorTexture(img);
            })();
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

    load_xml_node(xmlNode, parent = null)
    {
        if (parent == null) 
        {
            parent = this;
        }

        for (let child of xmlNode.children) 
        {
            let props = {};
            for (let i = 0; i < child.attributes.length; i++) 
            {
                let attrib = child.attributes[i];
                props[attrib.name] = attrib.value;
            }
            const obj = this.create(child.tagName, props, parent);
            if (obj===null) continue;

            this.load_xml_node(child, obj);            
        }
    }

    load_xml(xmlText)
    {
        const parser = new DOMParser();
        const xmlDoc = parser.parseFromString(xmlText, "text/xml");        
        let root = xmlDoc.documentElement;
        this.load_xml_node(root);
    }

    async load_xml_url(url)
    {
        const response = await fetch(url);
        const text = await response.text();
        this.load_xml(text);       

    }

}

export class Document
{
    constructor(canvas_ctx)
    {
        this.canvas_ctx = canvas_ctx;
        this.canvas = canvas_ctx.canvas;        
        this.resized = false;

        const size_changed = ()=>{
            this.canvas.width = this.canvas.clientWidth;
            this.canvas.height = this.canvas.clientHeight;        
            this.resized = true;
        };

        let observer = new ResizeObserver(size_changed);
        observer.observe(this.canvas);        

        this.imageLoader = new ImageLoader();  
        this.hdrImageLoader = new HDRImageLoader();      
        this.model_loader = new GLTFLoader();

        this.render_target = new GPURenderTarget(canvas_ctx, true);
        this.Tags = { scene, camera, control, sky, env_light, group, plane, box, sphere, model, avatar, directional_light};
        this.reset();
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
            if (object.tick)
            {
                object.tick(this, object, delta);
            }
        }

    }

    render(renderer)
    {
        if (this.resized)
        {            
            if (this.camera)
            {
                let width = this.canvas.width;
                let height =  this.canvas.height;
                this.camera.aspect = width / height;
                this.camera.updateProjectionMatrix();
                this.resized = false;
            }           
        }

        if (this.scene && this.camera)
        {            
            this.render_target.update();
            renderer.render(this.scene, this.camera, this.render_target);
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

    add_building_object(obj) 
    {
        this.building.push(obj);
    }

    remove_building_object(obj) 
    {
        for (let i = 0; i < this.building.length; i++) 
        {
            if (this.building[i] == obj) 
            {
                this.building.splice(i, 1);
                i--;
            }
        }
    }

    create(tag, props, parent = null)
    {   
        if (!(tag in this.Tags)) return null;

        const obj = this.Tags[tag].create(this, props, parent);
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
            obj.position.set(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
        }

        if (props.hasOwnProperty('rotation')) 
        {
            const rotation = props.rotation.split(',');
            obj.rotation.set(parseFloat(rotation[0])* Math.PI / 180.0, parseFloat(rotation[1])* Math.PI / 180.0, parseFloat(rotation[2])* Math.PI / 180.0);      
        }

        if (props.hasOwnProperty('scale')) 
        {
            const scale = props.scale.split(',');
            obj.scale.set(parseFloat(scale[0]), parseFloat(scale[1]), parseFloat(scale[2]));
        }

        if (props.hasOwnProperty('color')) 
        {
            const color = props.color.split(',');
            const r = parseFloat(color[0]);
            const g = parseFloat(color[1]);
            const b = parseFloat(color[2]);
            obj.color.setRGB(r,g,b);
        }

        if (props.hasOwnProperty('texture'))
        {
            (async()=>{
                let img = await this.imageLoader.loadFile(props.texture);
                obj.setColorTexture(img);
            })();
        }

        if (props.hasOwnProperty('metalness'))
        {
            obj.metalness = parseFloat(props.metalness);
        }
        
        if (props.hasOwnProperty('roughness'))
        {
            obj.roughness = parseFloat(props.roughness);
        }

        if (props.hasOwnProperty('is_building') && string_to_boolean(props.is_building)) 
        {            
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

    load_xml_node(xmlNode, parent = null)
    {
        if (parent == null) 
        {
            parent = this;
        }

        for (let child of xmlNode.children) 
        {
            let props = {};
            for (let i = 0; i < child.attributes.length; i++) 
            {
                let attrib = child.attributes[i];
                props[attrib.name] = attrib.value;
            }
            const obj = this.create(child.tagName, props, parent);
            if (obj===null) continue;

            this.load_xml_node(child, obj);
            if (obj.load) {
                obj.load(this, obj);
            }
        }
    }

    load_xml(xmlText)
    {
        const parser = new DOMParser();
        const xmlDoc = parser.parseFromString(xmlText, "text/xml");        
        let root = xmlDoc.documentElement;
        this.load_xml_node(root);

        if (this.building.length>0)
        { 
            this.bvh = new BoundingVolumeHierarchy(this.building);
        }
        else
        {
            this.bvh = null;
        }
    }

    async load_xml_url(url)
    {
        const response = await fetch(url);
        const text = await response.text();
        this.load_xml(text);
    }

    load(module)
    {
        this.reset();
        this.loaded_module = module;
        module.load(this);
    }

}

