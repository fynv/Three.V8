import { ColorBackground, HemisphereBackground } from "../backgrounds/Background.js"
import { Color } from "../math/Color.js"
import { DrawHemisphere } from "./routines/DrawHemisphere.js"

export class GPURenderer
{
    constructor()
    {

        
    }

    draw_hemisphere(passEncoder, target, camera, bg)
    {
        DrawHemisphere(passEncoder, target, camera, bg);
    }

    render(scene, camera, target)
    {
        camera.updateMatrixWorld(false);
    	camera.updateConstant();

        let msaa= target.msaa;
        let clearColor = new Color(0.0, 0.0, 0.0);        

        if(scene.background!=null &&
            scene.background instanceof ColorBackground)
        {
            clearColor = scene.background.color;
        }

        let colorAttachment =  {            
            clearValue: { r: clearColor.r, g: clearColor.g, b: clearColor.b, a: 1 },
            loadOp: 'clear',
            storeOp: 'store'
        };

        if (msaa)
        {
            colorAttachment.view = target.view_msaa;
            colorAttachment.resolveTarget = target.view_video;
        }
        else
        {
            colorAttachment.view = target.view_video;
        }

        let depthAttachment = {
            view: target.view_depth,
            depthClearValue: 1,
            depthLoadOp: 'clear',
            depthStoreOp: 'store',                  
        };

        let renderPassDesc = {
            colorAttachments: [colorAttachment],
            depthStencilAttachment: depthAttachment
        }; 

        let commandEncoder = engine_ctx.device.createCommandEncoder();

        let passEncoder = commandEncoder.beginRenderPass(renderPassDesc);

        while(scene.background!=null)
        {
            if (scene.background instanceof HemisphereBackground)
            {
                scene.background.updateConstant();
                this.draw_hemisphere(passEncoder, target, camera, scene.background);
                break;
            }

            break;
        }
        
        passEncoder.end();


        let cmdBuf = commandEncoder.finish();
        engine_ctx.queue.submit([cmdBuf]);

    }

}
