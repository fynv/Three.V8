import { EngineContext } from "./engine/EngineContext.js"
import { CanvasContext } from "./engine/CanvasContext.js"
import { GPURenderer } from "./engine/renderers/GPURenderer.js"
import { GPURenderTarget } from "./engine/renderers/GPURenderTarget.js"
import { Scene } from "./engine/scenes/Scene.js"
import { ColorBackground, HemisphereBackground, CubeBackground } from "./engine/backgrounds/Background.js"
import { PerspectiveCameraEx } from "./engine/cameras/PerspectiveCameraEx.js"
import { OrbitControls } from "./engine/controls/OrbitControls.js"
import { ImageLoader } from "./engine/loaders/ImageLoader.js"
import { SimpleModel } from "./engine/models/SimpleModel.js"
import { Vector3 } from "./engine/math/Vector3.js"
import { DirectionalLight } from "./engine/lights/DirectionalLight.js"

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
    
    //let bg = new ColorBackground();
    //bg.color.setRGB(0.2, 0.6, 0.8)

    //let bg = new HemisphereBackground();    

    let bg = new CubeBackground();

    let imageLoader = new ImageLoader();
    let cubeImg = await imageLoader.loadCubeFromFile([
        "./assets/textures/sky_cube_face0.jpg",
        "./assets/textures/sky_cube_face1.jpg",
        "./assets/textures/sky_cube_face2.jpg",
        "./assets/textures/sky_cube_face3.jpg",
        "./assets/textures/sky_cube_face4.jpg",
        "./assets/textures/sky_cube_face5.jpg"
    ]);    

    bg.setCubemap(cubeImg);    

    scene.background = bg;
    let camera = new PerspectiveCameraEx();
    camera.position.set(0.0, 0.0, 7.0);    

    let controls = new OrbitControls(camera, canvas);    
    controls.target.set(0.0, 0.0, 0.0); 
    controls.enableDamping = true; 

    let renderer = new GPURenderer();

    let box = new SimpleModel();
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
    sphere.createSphere(1.0);
    sphere.translateX(1.5);
    sphere.metalness = 0.5;
    sphere.roughness = 0.5;
    scene.add(sphere);

    (async ()=>{
        let img = await imageLoader.loadFile("./assets/textures/uv-test-col.png");
        sphere.setColorTexture(img);
    })();

    let ground = new SimpleModel();
    ground.createPlane(10.0, 10.0);    
    ground.translateY(-1.7);
    ground.rotateX(-3.1416*0.5);
    scene.add(ground);  

    let directional_light = new DirectionalLight();
    directional_light.intensity = 4.0;
    directional_light.position.set(5.0, 10.0, 5.0);
    scene.add(directional_light);
    
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

