import { ColorBackground, HemisphereBackground, CubeBackground } from "../backgrounds/Background.js"
import { BackgroundScene } from "../backgrounds/BackgroundScene.js"
import { PerspectiveCameraEx } from "../cameras/PerspectiveCameraEx.js"
import { Color } from "../math/Color.js"
import { Vector3 } from "../math/Vector3.js"
import { Vector4 } from "../math/Vector4.js"
import { Matrix4 } from "../math/Matrix4.js"
import { RenderDepth, RenderDepthBundle} from "./routines/DepthOnly.js"
import { DrawHemisphere, DrawHemisphereBundle } from "./routines/DrawHemisphere.js"
import { DrawSkyBox, DrawSkyBoxBundle } from "./routines/DrawSkyBox.js"
import { SimpleModel } from "../models/SimpleModel.js"
import { GLTFModel } from "../models/GLTFModel.js"
import { RenderStandard, RenderStandardBundle } from "./routines/StandardRoutine.js"
import { DirectionalLight } from "../lights/DirectionalLight.js"
import { RenderDirectionalShadow, RenderDirectionalShadowBundle } from "./routines/DirectionalShadowCast.js"
import { MorphUpdate } from "./routines/MorphUpdate.js"
import { SkinUpdate } from "./routines/SkinUpdate.js"
import { ResolveWeightedOIT } from "./routines/WeightedOIT.js"
import { AmbientLight } from "../lights/AmbientLight.js"
import { HemisphereLight } from "../lights/HemisphereLight.js"

function toViewAABB(MV, min_pos, max_pos)
{
    let view_pos = [];
    {
        let pos = new Vector4(min_pos.x, min_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(max_pos.x, min_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(min_pos.x, max_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }
    
    {
        let pos = new Vector4(max_pos.x, max_pos.y, min_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(min_pos.x, min_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(max_pos.x, min_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    {
        let pos = new Vector4(min_pos.x, max_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }
    
    {
        let pos = new Vector4(max_pos.x, max_pos.y, max_pos.z, 1.0);
        pos.applyMatrix4(MV);
        view_pos.push(pos);
    }

    let min_pos_view = new Vector3(Infinity,Infinity,Infinity);
    let max_pos_view = new Vector3(-Infinity, -Infinity, -Infinity);

    for (let k=0; k<8; k++)
    {
        let pos = new Vector3(view_pos[k].x, view_pos[k].y, view_pos[k].z);
        min_pos_view.min(pos);
        max_pos_view.max(pos);
    }

    return { min_pos_view, max_pos_view };
}

function visible(MV, P, min_pos, max_pos)
{
    let { min_pos_view, max_pos_view} = toViewAABB(MV, min_pos, max_pos);

    let invP = P.clone();        
    invP.invert();
    let view_far = new Vector4(0.0, 0.0, 1.0, 1.0);
    view_far.applyMatrix4(invP);
    view_far.multiplyScalar(1.0/view_far.w);
    let view_near = new Vector4(0.0, 0.0, -1.0, 1.0);
    view_near.applyMatrix4(invP);
    view_near.multiplyScalar(1.0/view_near.w);

    if (min_pos_view.z > view_near.z) return false;
    if (max_pos_view.z < view_far.z) return false;
    if (min_pos_view.z < view_far.z)
    {
        min_pos_view.z = view_far.z;
    }

    let min_pos_proj = new Vector4(min_pos_view.x, min_pos_view.y, min_pos_view.z, 1.0);
    min_pos_proj.applyMatrix4(P);
    min_pos_proj.multiplyScalar(1.0/min_pos_proj.w);

    let max_pos_proj = new Vector4(max_pos_view.x, max_pos_view.y, min_pos_view.z, 1.0);
    max_pos_proj.applyMatrix4(P);
    max_pos_proj.multiplyScalar(1.0/max_pos_proj.w);

    return max_pos_proj.x >= -1.0 && min_pos_proj.x <= 1.0 && max_pos_proj.y >= -1.0 && min_pos_proj.y <= 1.0;

}


export class GPURenderer
{
    constructor()
    {
        this.bg_bundles = {};
        this.depth_bundles = {};
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

        if (model.needUpdateSkinnedMeshes)
        {            
            for (let mesh of model.meshes)
            {
                if (mesh.skin_id >=0)
                {
                    let ready = true;
                    for (let primitive of mesh.primitives)
                    {                    
                        if (primitive.bind_group_skin==null)
                        {
                            ready = false;
                            break;
                        }
                    }

                    if (ready)
                    {
                        const commandEncoder = engine_ctx.device.createCommandEncoder();
                        const passEncoder = commandEncoder.beginComputePass();
                        for (let primitive of mesh.primitives)
                        {
                            SkinUpdate(passEncoder, primitive);
                        }
                        mesh.needUpdateMorphTargets = false;
                        passEncoder.end();
                        engine_ctx.device.queue.submit([commandEncoder.finish()]);
                    }
                }
            }
        }

        model.updateMeshConstants();
    }

    _render_shadow_simple_model(passEncoder, model, shadow)
    {
        if (model.geometry.uuid == 0) return;

        let view_matrix = shadow.light.matrixWorld.clone();
        view_matrix.invert();
        let MV = new Matrix4();
        MV.multiplyMatrices(view_matrix, model.matrixWorld);
        if (!visible(MV, shadow.light_proj_matrix, model.geometry.min_pos, model.geometry.max_pos)) return;        
        
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
        let view_matrix = shadow.light.matrixWorld.clone();
        view_matrix.invert();
        for (let mesh of model.meshes)
        {
            let matrix = model.matrixWorld.clone();
            if (mesh.node_id >=0 && mesh.skin_id <0)
            {
                let node = model.nodes[mesh.node_id];
                matrix.multiply(node.g_trans);
            }
            let MV = new Matrix4();
            MV.multiplyMatrices(view_matrix, matrix);

            for (let primitive of mesh.primitives)
            {
                if (primitive.uuid == 0) continue;
                if (!visible(MV, shadow.light_proj_matrix, primitive.min_pos, primitive.max_pos)) continue;

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

        let lights = scene.lights;
        
        scene.traverse((obj)=>{
            if(obj!=null)
            {
                if (obj instanceof SimpleModel)
                {
                    scene.simple_models.push(obj);
                }         
                else if (obj instanceof GLTFModel)
                {
                    scene.gltf_models.push(obj);
                }
                else if (obj instanceof DirectionalLight)
                {                    
                    lights.directional_lights.push(obj);
                }
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
        for (let light of lights.directional_lights)
        {
            light.lookAtTarget();
            light.updateConstant();
            if (light.shadow!=null) has_shadow_map = true;
        }

        if (has_shadow_map)
        {
            let commandEncoder = engine_ctx.device.createCommandEncoder();

            for (let light of lights.directional_lights)
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

        if(scene.indirectLight!=null)
        {
            lights.reflection_map = scene.indirectLight.reflection;
            if (scene.indirectLight instanceof AmbientLight)
            {
                scene.indirectLight.updateConstant();
                lights.ambient_light = scene.indirectLight;               
            }
            else if (scene.indirectLight instanceof HemisphereLight)
            {
                scene.indirectLight.updateConstant();
                lights.hemisphere_light = scene.indirectLight;
            }

        }

        lights.update_bind_group();
    }

    _render_depth_simple_model(passEncoder, model, camera, target)
    {
        if (model.geometry.uuid == 0) return;
        let material = model.material;
        if (material.alphaMode != "Opaque") return;

        let params = {            
            target,
            material_list: [material],
            bind_group_camera: camera.bind_group,            
            primitive: model.geometry,
        };

        //RenderDepth(passEncoder, params);

        let signature = JSON.stringify({
            id_primitive: model.geometry.uuid,
            id_camera: camera.uuid,
            id_target: target.uuid            
        });

        if (!(signature in this.shadow_bundles))
        {    
            this.shadow_bundles[signature] = RenderDepthBundle(params);
        }
        passEncoder.executeBundles([this.shadow_bundles[signature]]);

    }

    _render_depth_gltf_model(passEncoder, model, camera, target)
    {
        for (let mesh of model.meshes)
        {
            let matrix = model.matrixWorld.clone();
            if (mesh.node_id >=0 && mesh.skin_id <0)
            {
                let node = model.nodes[mesh.node_id];
                matrix.multiply(node.g_trans);
            }
            let MV = new Matrix4();
            MV.multiplyMatrices(camera.matrixWorldInverse, matrix);

            for (let primitive of mesh.primitives)
            {
                if (primitive.uuid == 0) continue;
                if (!visible(MV, camera.projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

                let idx_material = primitive.material_idx;
                let material = model.materials[idx_material];

                if (material.alphaMode != "Opaque") continue;

                let params = {                    
                    target,
                    material_list: model.materials,
                    bind_group_camera: camera.bind_group,            
                    primitive: primitive,
                };

                let signature = JSON.stringify({
                    id_primitive: primitive.uuid,
                    id_camera: camera.uuid,
                    id_target: target.uuid            
                });
        
                if (!(signature in this.shadow_bundles))
                {    
                    this.shadow_bundles[signature] = RenderDepthBundle(params);
                }
                passEncoder.executeBundles([this.shadow_bundles[signature]]);        
            }
        }
    }

    _render_simple_model(passEncoder, model, camera, lights, target, pass)
    {
        if (model.geometry.uuid == 0) return;
        let material = model.material;

        if (pass == "Opaque")
        {
            if (material.alphaMode == "Blend") return;
        }
        else if (pass == "Alpha")
        {
            if (material.alphaMode != "Blend") return;
        }

        let params = {            
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
            let matrix = model.matrixWorld.clone();
            if (mesh.node_id >=0 && mesh.skin_id <0)
            {
                let node = model.nodes[mesh.node_id];
                matrix.multiply(node.g_trans);
            }
            let MV = new Matrix4();
            MV.multiplyMatrices(camera.matrixWorldInverse, matrix);

            for (let primitive of mesh.primitives)
            {
                if (primitive.uuid == 0) continue;
                if (!visible(MV, camera.projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

                let idx_material = primitive.material_idx;
                let material = model.materials[idx_material];

                if (pass == "Opaque")
                {
                    if (material.alphaMode == "Blend") continue;
                }
                else if (pass == "Alpha")
                {
                    if (material.alphaMode != "Blend") continue;
                }

                let params = {                    
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

    _render_scene(commandEncoder, scene, camera, target)
    {
        camera.updateMatrixWorld(false);
    	camera.updateConstant();

        let simple_models = [...scene.simple_models];
        let gltf_models =  [...scene.gltf_models];

        let has_alpha = false;
        let has_opaque = false;

        // model culling
        for (let i=0; i<simple_models.length; i++)
        {
            let model = simple_models[i];
            let MV = new Matrix4();
            MV.multiplyMatrices(camera.matrixWorldInverse, model.matrixWorld);
            if (!visible(MV, camera.projectionMatrix, model.geometry.min_pos, model.geometry.max_pos))
            {
                simple_models.splice(i,1);
                i--;
            }
            else
            {
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
        }

        for (let i=0; i<gltf_models.length; i++)
        {
            let model = gltf_models[i];
            let MV = new Matrix4();
            MV.multiplyMatrices(camera.matrixWorldInverse, model.matrixWorld);
            if (!visible(MV, camera.projectionMatrix, model.min_pos, model.max_pos))
            {
                gltf_models.splice(i,1);
                i--;
            }
            else
            {
                for (let material of model.materials)
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
        }

        let msaa= target.msaa;

        // background pass
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
            if (!has_alpha && !has_opaque)
            {
                colorAttachment.resolveTarget = target.view_video;
            }
        }
        else
        {
            colorAttachment.view = target.view_video;
        }
        
        if(scene.background!=null)
        {
            if (scene.background instanceof HemisphereBackground)
            {
                scene.background.updateConstant();

                let renderPassDesc_bg = {
                    colorAttachments: [colorAttachment],                    
                }; 
                let passEncoder = commandEncoder.beginRenderPass(renderPassDesc_bg);
    
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
                
                this._draw_hemisphere(passEncoder, target, camera, scene.background);        
                
                passEncoder.end();

                colorAttachment.loadOp = 'load';
            }
            else if (scene.background instanceof CubeBackground)
            {
                let renderPassDesc_bg = {
                    colorAttachments: [colorAttachment],                    
                }; 
                let passEncoder = commandEncoder.beginRenderPass(renderPassDesc_bg);

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

                this._draw_skybox(passEncoder, target, camera, scene.background);    
                
                passEncoder.end();

                colorAttachment.loadOp = 'load';
            }
            else if (scene.background instanceof BackgroundScene)
            {
                let bg = scene.background;
                let cam = new PerspectiveCameraEx(camera.fov, camera.aspect, bg.near, bg.far);
                cam.parent = camera.parent;
                cam.position.copy(camera.position);
                cam.quaternion.copy(camera.quaternion);
                this._render_scene(commandEncoder, bg.scene, cam, target);
            }
        }

        // depth-prepass
        let depthAttachment = {
            view: target.view_depth,
            depthClearValue: 1,
            depthLoadOp: 'clear',
            depthStoreOp: 'store',
        };

        if (has_opaque)
        {            
            let renderPassDesc_depth = {
                colorAttachments: [],
                depthStencilAttachment: depthAttachment
            }; 
            let passEncoder = commandEncoder.beginRenderPass(renderPassDesc_depth);

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

            for (let model of simple_models)
            {
                this._render_depth_simple_model(passEncoder, model, camera, target);
            }

            for (let model of gltf_models)
            {
                this._render_depth_gltf_model(passEncoder, model, camera, target);
            }

            passEncoder.end();

            depthAttachment.depthLoadOp = 'load';
        }

        // opaque pass
        let lights = scene.lights;

        if (has_opaque)
        {
            if (!has_alpha)
            {
                colorAttachment.resolveTarget = target.view_video;
            }
            let renderPassDesc_opaque = {
                colorAttachments: [colorAttachment],
                depthStencilAttachment: depthAttachment
            }; 
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
            
            for (let model of simple_models)
            {
                this._render_simple_model(passEncoder, model, camera, lights, target, "Opaque");
            }

            for (let model of gltf_models)
            {
                this._render_gltf_model(passEncoder, model, camera, lights, target, "Opaque");
            }
            
            passEncoder.end();
            
            colorAttachment.loadOp = 'load';
            depthAttachment.depthLoadOp = 'load';
        }

        // alpha pass
        if (has_alpha)
        {
            target.update_oit_buffers();

            let oitAttchment0 = {
                view: target.oit_view0,
                clearValue: { r: 0.0, g: 0.0, b: 0.0, a: 0.0 },
                loadOp: 'clear',
                storeOp: 'store'
            }
    
            let oitAttchment1 = {
                view: target.oit_view1,
                clearValue: { r: 1.0, g: 1.0, b: 1.0, a: 1.0 },
                loadOp: 'clear',
                storeOp: 'store'
            }

            let renderPassDesc_alpha = {
                colorAttachments: [colorAttachment, oitAttchment0, oitAttchment1],
                depthStencilAttachment: depthAttachment
            }; 

            let passEncoder = commandEncoder.beginRenderPass(renderPassDesc_alpha);

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

            for (let model of simple_models)
            {
                this._render_simple_model(passEncoder, model, camera, lights, target, "Alpha");
            }

            for (let model of gltf_models)
            {
                this._render_gltf_model(passEncoder, model, camera, lights, target, "Alpha");
            }

            passEncoder.end();

            {
                colorAttachment.resolveTarget = target.view_video;

                let renderPassDesc_oit = {
                    colorAttachments: [colorAttachment],            
                }; 

                let passEncoder = commandEncoder.beginRenderPass(renderPassDesc_oit);

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

                ResolveWeightedOIT(passEncoder, target);

                passEncoder.end();
            }
        }

    }

    _render(scene, camera, target)
    {        
        let commandEncoder = engine_ctx.device.createCommandEncoder();
        this._render_scene(commandEncoder, scene, camera, target);

        let cmdBuf = commandEncoder.finish();
        engine_ctx.queue.submit([cmdBuf]);

    }
    

    render(scene, camera, target)
    {
        this._pre_render(scene);
        this._render(scene, camera, target);       

    }

}
