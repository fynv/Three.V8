import { ColorBackground, HemisphereBackground, CubeBackground } from "../backgrounds/Background.js"
import { Color } from "../math/Color.js"
import { DrawHemisphere, DrawHemisphereBundle } from "./routines/DrawHemisphere.js"
import { DrawSkyBox, DrawSkyBoxBundle } from "./routines/DrawSkyBox.js"
import { SimpleModel } from "../models/SimpleModel.js"
import { RenderStandard, RenderStandardBundle } from "./routines/StandardRoutine.js"

export class GPURenderer
{
    constructor()
    {
        this.bg_bundles = {};
        this.render_bundles = {};
    }

    _draw_hemisphere(passEncoder, target, camera, bg)
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

    _draw_skybox(passEncoder, target, camera, bg)
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

        // DrawSkyBox(passEncoder, target, camera, bg);

        let signature = JSON.stringify({
            id_target: target.uuid,
            id_camera: camera.uuid,
            id_bg: bg.uuid
        });
        if (!(signature in this.bg_bundles))
        {    
            this.bg_bundles[signature] = DrawSkyBoxBundle(passEncoder, target, camera, bg);
        }
        passEncoder.executeBundles([this.bg_bundles[signature]]);

    }

    _update_simple_model(model)
    {
        model.updateConstant();
    }

    _pre_render(scene)
    {
        scene.clear_lists();
        
        scene.traverse((obj)=>{
            while(obj!=null)
            {
                if (obj instanceof SimpleModel)
                {
                    scene.simple_models.push(obj);
                    break;
                }         

                break;
            }
            obj.updateWorldMatrix(false, false);
        });

        for (let model of scene.simple_models)
        {
            this._update_simple_model(model);
        }
    }

    _render_simple_model(passEncoder, model, camera, lights, target, pass)
    {
        let material = model.material;

        if (pass == "Opaque")
        {
            if (material.alphaMode == "Blend") return;
        }
        else if (pass == "Alpha" || pass == "Highlight")
        {
            if (material.alphaMode != "Blend") return;
        }


        let params = {
            is_highlight_pass: pass == "Highlight",
            target,
            material_list: [material],
            bind_group_camera: camera.bind_group,            
            primitive: model.geometry
        };

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

        //RenderStandard(passEncoder, params);

        let signature = JSON.stringify({
            id_primitive: model.geometry.uuid,
            id_camera: camera.uuid,
            id_target: target.uuid,
            pass            
        });

        if (!(signature in this.render_bundles))
        {    
            this.render_bundles[signature] = RenderStandardBundle(passEncoder, params);
        }
        passEncoder.executeBundles([this.render_bundles[signature]]);


    }

    _render(scene, camera, target)
    {
        camera.updateMatrixWorld(false);
    	camera.updateConstant();

        let simple_models = [...scene.simple_models];

        let has_alpha = false;
        let has_opaque = false;

        for (let i=0; i<simple_models.length; i++)
        {
            let model = simple_models[i];
            let material = model.material;
            if (material.alphaMode == "Blend")
            {
                has_alpha = true;
            }
            else
            {
                has_opaque= true;
            }
        }

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

        let renderPassDesc_opaque = {
            colorAttachments: [colorAttachment],
            depthStencilAttachment: depthAttachment
        }; 

        let lights = null;

        let commandEncoder = engine_ctx.device.createCommandEncoder();

        {
            let passEncoder = commandEncoder.beginRenderPass(renderPassDesc_opaque);

            while(scene.background!=null)
            {
                if (scene.background instanceof HemisphereBackground)
                {
                    scene.background.updateConstant();
                    this._draw_hemisphere(passEncoder, target, camera, scene.background);
                    break;
                }

                if (scene.background instanceof CubeBackground)
                {
                    this._draw_skybox(passEncoder, target, camera, scene.background);
                    break;
                }
                break;
            }

            if (has_opaque)
            {
                for (let model of simple_models)
                {
                    this._render_simple_model(passEncoder, model, camera, lights, target, "Opaque");
                }
            }
            
            passEncoder.end();
        }


        let cmdBuf = commandEncoder.finish();
        engine_ctx.queue.submit([cmdBuf]);

    }
    

    render(scene, camera, target)
    {
        this._pre_render(scene);
        this._render(scene, camera, target);       

    }

}
