import { EngineContext } from "./engine/EngineContext.js"
import { CanvasContext } from "./engine/CanvasContext.js"
import { GPURenderer } from "./engine/renderers/GPURenderer.js"
import { Clock } from "./engine/core/Clock.js";
import { Document } from "./document.js"

import * as module from "./doc_module.js";

export async function test()
{
    const canvas = document.getElementById('gfx');
    canvas.style.cssText = "position:absolute; width: 100%; height: 100%;";      

    const engine_ctx = new EngineContext();
    const canvas_ctx = new CanvasContext(canvas);
    await canvas_ctx.initialize();

    let renderer = new GPURenderer();
    let doc = new Document(canvas_ctx);
    doc.load(module);	

    let clock = new Clock();

    const render = () =>
    {
        let delta = clock.getDelta();
        doc.tick(delta);
        doc.render(renderer);
        requestAnimationFrame(render);
    }

    render();
}

