import { ColorBackground, HemisphereBackground, CubeBackground } from "../backgrounds/Background.js"
import { Color } from "../math/Color.js"
import { DrawHemisphere, DrawHemisphereBundle } from "./routines/DrawHemisphere.js"
import { DrawSkyBox } from "./routines/DrawSkyBox.js"

export class GPURenderer
{
    constructor()
    {
        this.bg_bundles = {};
    }

    draw_hemisphere(passEncoder, target, camera, bg)
    {
        passEncoder.setViewport(
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
            target.height,
        );
        
        //DrawHemisphere(passEncoder, target, camera, bg);

        let signature = JSON.stringify({
            id_target: target.uuid,
            id_camera: camera.uuid,
            id_bg: bg.uuid
        });
        
        if (!(signature in this.bg_bundles))
        {            
            this.bg_bundles[signature] = DrawHemisphereBundle(passEncoder, target, camera, bg);
        }
        passEncoder.executeBundles([this.bg_bundles[signature]]);

    }

    draw_skybox(passEncoder, target, camera, bg)
    {
        passEncoder.setViewport(
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
            target.height,
        );
        DrawSkyBox(passEncoder, target, camera, bg);

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

            if (scene.background instanceof CubeBackground)
            {
                this.draw_skybox(passEncoder, target, camera, scene.background);
                break;
            }
            break;
        }
        
        passEncoder.end();


        let cmdBuf = commandEncoder.finish();
        engine_ctx.queue.submit([cmdBuf]);

    }

}
