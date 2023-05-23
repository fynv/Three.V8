import { EngineContext } from "./engine/EngineContext.js"
import { CanvasContext } from "./engine/CanvasContext.js"
import { GPURenderer } from "./engine/renderers/GPURenderer.js"
import { GPURenderTarget } from "./engine/renderers/GPURenderTarget.js"
import { Scene } from "./engine/scenes/Scene.js"
import { ColorBackground } from "./engine/backgrounds/Background.js"
import { PerspectiveCameraEx } from "./engine/cameras/PerspectiveCameraEx.js"

export async function test()
{

    const canvas = document.getElementById('gfx');
    canvas.style.cssText = "position:absolute; width: 100%; height: 100%;";      

    const engine_ctx = new EngineContext();
    const canvas_ctx = new CanvasContext(canvas);
    await canvas_ctx.initialize();

    const size_changed = ()=>{
        canvas.width = canvas.clientWidth/devicePixelRatio;
        canvas.height = canvas.clientHeight/devicePixelRatio;
    };
    
    let observer = new ResizeObserver(size_changed);
    observer.observe(canvas);

    let msaa = true;
    let render_target = new GPURenderTarget(canvas_ctx, msaa);

    let scene = new Scene();
    let bg = new ColorBackground();
    bg.color.setRGB(0.2, 0.6, 0.8)
    scene.background = bg;
    let camera = new PerspectiveCameraEx();
    let renderer = new GPURenderer();

    const render = () =>
    {      
        render_target.update();
        renderer.render(scene, camera, render_target);
        requestAnimationFrame(render);
    };

    render();
    
}

