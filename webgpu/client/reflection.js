import { EngineContext } from "./engine/EngineContext.js"
import { CanvasContext } from "./engine/CanvasContext.js"
import { GPURenderer } from "./engine/renderers/GPURenderer.js"
import { GPURenderTarget } from "./engine/renderers/GPURenderTarget.js"
import { Scene } from "./engine/scenes/Scene.js"
import { ColorBackground, HemisphereBackground, CubeBackground } from "./engine/backgrounds/Background.js"
import { PerspectiveCameraEx } from "./engine/cameras/PerspectiveCameraEx.js"
import { OrbitControls } from "./engine/controls/OrbitControls.js"
import { ImageLoader } from "./engine/loaders/ImageLoader.js"
import { HDRImageLoader } from "./engine/loaders/HDRImageLoader.js"
import { SimpleModel } from "./engine/models/SimpleModel.js"
import { Vector3 } from "./engine/math/Vector3.js"
import { DirectionalLight } from "./engine/lights/DirectionalLight.js"
import { EnvironmentMapCreator} from "./engine/lights/EnvironmentMapCreator.js"
import { Reflector } from "./engine/cameras/Reflector.js"
import { Fog } from "./engine/scenes/Fog.js"
import { GLTFLoader } from "./engine/loaders/GLTFLoader.js"

export async function test()
{
    const canvas = document.getElementById('gfx');
    canvas.style.cssText = "position:absolute; width: 100%; height: 100%;";      

    const engine_ctx = new EngineContext();
    const canvas_ctx = new CanvasContext(canvas);
    await canvas_ctx.initialize();

    let resized = false;
    const size_changed = ()=>{
        canvas.width = canvas.clientWidth;
        canvas.height = canvas.clientHeight;        
        resized = true;
    };
    
    let observer = new ResizeObserver(size_changed);
    observer.observe(canvas);

    let msaa = true;
    let render_target = new GPURenderTarget(canvas_ctx, msaa);

    let scene = new Scene();
    
    let bg = new CubeBackground();
    let imageLoader = new ImageLoader();
    let hdrImageLoader = new HDRImageLoader();

    (async ()=>{        
        let cubeImg = await imageLoader.loadCubeFromFile([
            "./assets/textures/sky_cube_face0.jpg",
            "./assets/textures/sky_cube_face1.jpg",
            "./assets/textures/sky_cube_face2.jpg",
            "./assets/textures/sky_cube_face3.jpg",
            "./assets/textures/sky_cube_face4.jpg",
            "./assets/textures/sky_cube_face5.jpg"
        ]);            
        bg.setCubemap(cubeImg);    
    })();

    scene.background = bg;

    let envMapCreator = new EnvironmentMapCreator(); 

    (async ()=>{        
        let cubeImg = await hdrImageLoader.loadCubeFromFile([
            "./assets/textures/env_face0.hdr",
            "./assets/textures/env_face1.hdr",
            "./assets/textures/env_face2.hdr",
            "./assets/textures/env_face3.hdr",
            "./assets/textures/env_face4.hdr",
            "./assets/textures/env_face5.hdr"
        ]);
        let envMap = envMapCreator.create(cubeImg);
        scene.indirectLight = envMap;
    })();


    let camera = new PerspectiveCameraEx();
    camera.position.set(0.0, 0.0, 7.0);    

    let controls = new OrbitControls(camera, canvas);    
    controls.target.set(0.0, 0.0, 0.0); 
    controls.enableDamping = true; 

    let renderer = new GPURenderer();

    let box = new SimpleModel();
    box.name = "box";
    box.createBox(2,2,2);
    box.translateX(-1.5);
    let axis = new Vector3(1.0, 1.0, 0.0);
    axis.normalize();
    box.rotateOnAxis(axis, 1.0);
    scene.add(box);   

    (async ()=>{
        let img = await imageLoader.loadFile("./assets/textures/uv-test-bw.png");
        box.setColorTexture(img);
    })();

    let sphere = new SimpleModel();
    sphere.name = "sphere";
    sphere.createSphere(1.0);
    sphere.translateX(1.5);
    sphere.metalness = 0.5;
    sphere.roughness = 0.5;
    scene.add(sphere);

    (async ()=>{
        let img = await imageLoader.loadFile("./assets/textures/uv-test-col.png");
        sphere.setColorTexture(img);
    })();

    let reflector = new Reflector();
    reflector.width = 11;
    reflector.height = 11;
    reflector.translateY(-1.7);
    reflector.rotateX(-3.1416*0.5);    
    scene.add(reflector);

    let ground = new SimpleModel();
    ground.name = "ground";
    ground.createPlane(10.0, 10.0);    
    ground.metalness = 0.9;
    ground.roughness = 0.3;

    /*let model_loader = new GLTFLoader();
    let ground = model_loader.loadModelFromFile("./assets/models/surface_0.3.glb");*/
    
    reflector.add(ground);

    let directional_light = new DirectionalLight();
    directional_light.intensity = 4.0;
    directional_light.position.set(5.0, 10.0, 5.0);
    directional_light.setShadow(true, 4096, 4096);
    directional_light.setShadowProjection(-10.0, 10.0, -10.0, 10.0, 0.0, 50.0);
    directional_light.setShadowRadius(0.1);
    scene.add(directional_light);

    /*let fog = new Fog();
    fog.color.setRGB(0.8,0.8,0.5);
    fog.density = 0.04;
    scene.fog = fog;*/

    const render = () =>
    {      
        controls.update();
        if (resized)
        {
            camera.aspect = canvas.width/canvas.height;
            camera.updateProjectionMatrix();
            resized = false;
        }

        render_target.update();
        renderer.render(scene, camera, render_target);
        requestAnimationFrame(render);
    };

    render();

}



