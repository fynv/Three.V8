import { EngineContext } from "./engine/EngineContext.js"
import { CanvasContext } from "./engine/CanvasContext.js"
import { GPURenderer } from "./engine/renderers/GPURenderer.js"
import { GPURenderTarget } from "./engine/renderers/GPURenderTarget.js"
import { Scene } from "./engine/scenes/Scene.js"
import { ColorBackground, HemisphereBackground } from "./engine/backgrounds/Background.js"
import { PerspectiveCameraEx } from "./engine/cameras/PerspectiveCameraEx.js"
import { OrbitControls } from "./engine/controls/OrbitControls.js"

export async function test()
{

    const canvas = document.getElementById('gfx');
    canvas.style.cssText = "position:absolute; width: 100%; height: 100%;";      

    const engine_ctx = new EngineContext();
    const canvas_ctx = new CanvasContext(canvas);
    await canvas_ctx.initialize();

    let resized = false;
    const size_changed = ()=>{
        canvas.width = canvas.clientWidth/devicePixelRatio;
        canvas.height = canvas.clientHeight/devicePixelRatio;
        resized = true;
    };
    
    let observer = new ResizeObserver(size_changed);
    observer.observe(canvas);

    let msaa = true;
    let render_target = new GPURenderTarget(canvas_ctx, msaa);

    let scene = new Scene();
    
    //let bg = new ColorBackground();
    //bg.color.setRGB(0.2, 0.6, 0.8)

    let bg = new HemisphereBackground();    

    scene.background = bg;
    let camera = new PerspectiveCameraEx();
    camera.position.set(0.0, 0.0, 5.0);    

    let controls = new OrbitControls(camera, canvas);    
    controls.target.set(0.0, 0.0, 0.0); 
    controls.enableDamping = true; 

    let renderer = new GPURenderer();

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

