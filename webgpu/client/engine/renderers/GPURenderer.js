import { ColorBackground } from "../backgrounds/Background.js"
import { Color } from "../math/Color.js"

export class GPURenderer
{
    constructor()
    {

        
    }

    render(scene, camera, target)
    {
        let msaa= target.msaa;
        let clearColor = new Color(0.0, 0.0, 0.0);        

        while(scene.background!=null)
        {
            if(scene.background instanceof ColorBackground)
            {
                clearColor = scene.background.color;
                break;
            }

            break;
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
        
        /*passEncoder.setViewport(
            0,
            0,
            target.width,
            target.height,
            0,
            1
        );
        passEncoder.setScissorRect(
            0,
            0,
            target.width,
            target.height
        );*/

        passEncoder.end();


        let cmdBuf = commandEncoder.finish();
        engine_ctx.queue.submit([cmdBuf]);

    }

}
