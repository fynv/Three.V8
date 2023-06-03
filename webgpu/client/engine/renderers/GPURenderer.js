import { ColorBackground, HemisphereBackground, CubeBackground } from "../backgrounds/Background.js"
import { Color } from "../math/Color.js"
import { DrawHemisphere, DrawHemisphereBundle } from "./routines/DrawHemisphere.js"
import { DrawSkyBox, DrawSkyBoxBundle } from "./routines/DrawSkyBox.js"
import { SimpleModel } from "../models/SimpleModel.js"
import { GLTFModel } from "../models/GLTFModel.js"
import { RenderStandard, RenderStandardBundle } from "./routines/StandardRoutine.js"
import { DirectionalLight } from "../lights/DirectionalLight.js"
import { RenderDirectionalShadow, RenderDirectionalShadowBundle } from "./routines/DirectionalShadowCast.js"
import { MorphUpdate } from "./routines/MorphUpdate.js"

export class GPURenderer
{
    constructor()
    {
        this.bg_bundles = {};
        this.render_bundles = {};
        this.shadow_bundles = {};
    }

    _draw_hemisphere(passEncoder, target, camera, bg)
    {       
        
        //DrawHemisphere(passEncoder, target, camera, bg);

        let signature = JSON.stringify({
            id_target: target.uuid,
            id_camera: camera.uuid,
            id_bg: bg.uuid
        });
        
        if (!(signature in this.bg_bundles))
        {            
            this.bg_bundles[signature] = DrawHemisphereBundle(target, camera, bg);
        }
        passEncoder.executeBundles([this.bg_bundles[signature]]);

    }

    _draw_skybox(passEncoder, target, camera, bg)
    {
        // DrawSkyBox(passEncoder, target, camera, bg);

        let signature = JSON.stringify({
            id_target: target.uuid,
            id_camera: camera.uuid,
            id_bg: bg.uuid
        });
        if (!(signature in this.bg_bundles))
        {    
            this.bg_bundles[signature] = DrawSkyBoxBundle(target, camera, bg);
        }
        passEncoder.executeBundles([this.bg_bundles[signature]]);

    }

    _update_simple_model(model)
    {
        model.updateConstant();
    }

    _update_gltf_model(model)
    {        
        for (let mesh of model.meshes)
        {
            if (mesh.needUpdateMorphTargets)
            {
                let ready = true;
                for (let primitive of mesh.primitives)
                {                    
                    if (primitive.bind_group_morph==null)
                    {
                        ready = false;
                        break;
                    }
                }

                if (ready)
                {
                    mesh.update_weights();

                    const commandEncoder = engine_ctx.device.createCommandEncoder();
                    const passEncoder = commandEncoder.beginComputePass();
                    for (let primitive of mesh.primitives)
                    {
                        MorphUpdate(passEncoder, primitive);
                    }
                    mesh.needUpdateMorphTargets = false;
                    passEncoder.end();
                    engine_ctx.device.queue.submit([commandEncoder.finish()]);
                }
                
            }
        }
        model.updateMeshConstants();
    }

    _render_shadow_simple_model(passEncoder, model, shadow)
    {
        if (model.geometry.uuid == 0) return;
        
        let material = model.material;
        
        let params = {                        
            material_list: [material],
            bind_group_shadow: shadow.bind_group,            
            primitive: model.geometry,
            force_cull: shadow.force_cull
        };

        //RenderDirectionalShadow(passEncoder, params);

        let signature = JSON.stringify({
            id_primitive: model.geometry.uuid,
            id_shadow: shadow.uuid,                        
        });

        if (!(signature in this.shadow_bundles))
        {    
            this.shadow_bundles[signature] = RenderDirectionalShadowBundle(params);
        }
        passEncoder.executeBundles([this.shadow_bundles[signature]]);
    }

    _render_shadow_gltf_model(passEncoder, model, shadow)
    {
        for (let mesh of model.meshes)
        {
            for (let primitive of mesh.primitives)
            {
                if (primitive.uuid == 0) continue;

                let params = {                        
                    material_list: model.materials,
                    bind_group_shadow: shadow.bind_group,            
                    primitive: primitive,
                    force_cull: shadow.force_cull
                };

                let signature = JSON.stringify({
                    id_primitive: primitive.uuid,
                    id_shadow: shadow.uuid,                        
                });

                if (!(signature in this.shadow_bundles))
                {    
                    this.shadow_bundles[signature] = RenderDirectionalShadowBundle(params);
                }
                passEncoder.executeBundles([this.shadow_bundles[signature]]);
            }
        }

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

                if (obj instanceof GLTFModel)
                {
                    scene.gltf_models.push(obj);
                    break;
                }

                if (obj instanceof DirectionalLight)
                {                    
                    scene.lights.directional_lights.push(obj);
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

        for (let model of scene.gltf_models)
        {
            this._update_gltf_model(model);
        }

        let has_shadow_map = false;
        for (let light of scene.lights.directional_lights)
        {
            light.lookAtTarget();
            light.updateConstant();
            if (light.shadow!=null) has_shadow_map = true;
        }

        if (has_shadow_map)
        {
            let commandEncoder = engine_ctx.device.createCommandEncoder();

            for (let light of scene.lights.directional_lights)
            {
                let shadow = light.shadow;
                if (shadow == null) continue;

                shadow.updateConstant();

                let depthAttachment = {
                    view: shadow.lightTexView,
                    depthClearValue: 1,
                    depthLoadOp: 'clear',
                    depthStoreOp: 'store',
                };
        
                let renderPassDesc = {          
                    colorAttachments: [],          
                    depthStencilAttachment: depthAttachment
                }; 

                let passEncoder = commandEncoder.beginRenderPass(renderPassDesc);
                
                for (let model of scene.simple_models)
                {
                    this._render_shadow_simple_model(passEncoder, model, shadow);                    
                }

                for (let model of scene.gltf_models)
                {
                    this._render_shadow_gltf_model(passEncoder, model, shadow); 
                }

                passEncoder.end();
            }


            let cmdBuf = commandEncoder.finish();
            engine_ctx.queue.submit([cmdBuf]);            
        }

        scene.lights.update_bind_group();
    }

    _render_simple_model(passEncoder, model, camera, lights, target, pass)
    {
        if (model.geometry.uuid == 0) return;

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
            primitive: model.geometry,
            lights
        };

        //RenderStandard(passEncoder, params);

        let signature = JSON.stringify({
            id_primitive: model.geometry.uuid,
            id_camera: camera.uuid,
            id_target: target.uuid,
            signature_lights: lights.signature,
            pass            
        });

        if (!(signature in this.render_bundles))
        {    
            this.render_bundles[signature] = RenderStandardBundle(params);
        }
        passEncoder.executeBundles([this.render_bundles[signature]]);

    }

    _render_gltf_model(passEncoder, model, camera, lights, target, pass)
    {        
        for (let mesh of model.meshes)
        {
            for (let primitive of mesh.primitives)
            {
                if (primitive.uuid == 0) continue;

                let idx_material = primitive.material_idx;
                let material = model.materials[idx_material];

                if (pass == "Opaque")
                {
                    if (material.alphaMode == "Blend") continue;
                }
                else if (pass == "Alpha" || pass == "Highlight")
                {
                    if (material.alphaMode != "Blend") continue;
                }

                let params = {
                    is_highlight_pass: pass == "Highlight",
                    target,
                    material_list: model.materials,
                    bind_group_camera: camera.bind_group,            
                    primitive: primitive,
                    lights
                };

                let signature = JSON.stringify({
                    id_primitive:  primitive.uuid,
                    id_camera: camera.uuid,
                    id_target: target.uuid,
                    signature_lights: lights.signature,
                    pass            
                });

                if (!(signature in this.render_bundles))
                {    
                    this.render_bundles[signature] = RenderStandardBundle(params);
                }
                passEncoder.executeBundles([this.render_bundles[signature]]);
            }
        }

    }

    _render(scene, camera, target)
    {
        camera.updateMatrixWorld(false);
    	camera.updateConstant();

        let simple_models = [...scene.simple_models];
        let gltf_models =  [...scene.gltf_models];

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

        for (let i=0; i<gltf_models.length; i++)
        {
            let model = gltf_models[i];
            for (let material in model.materials)
            {
                if (material.alphaMode == "Blend")
                {
                    has_alpha = true;
                }
                else
                {
                    has_opaque= true;
                }
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

        let lights = scene.lights;

        let commandEncoder = engine_ctx.device.createCommandEncoder();

        {
            let passEncoder = commandEncoder.beginRenderPass(renderPassDesc_opaque);

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

                for (let model of gltf_models)
                {
                    this._render_gltf_model(passEncoder, model, camera, lights, target, "Opaque");
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
