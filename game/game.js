import { Vector3 } from "./math/Vector3.js";
import { OrbitControls } from "./controls/OrbitControls.js";
import { view } from "./view.js";

function init(width, height) {
    renderer = new GLRenderer();
    scene = new Scene();
    
    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    camera.setPosition(0.0, 0.0, 7.0);
    
    directional_light = new DirectionalLight();
    directional_light.intensity = 4.0;
    directional_light.setPosition(5.0, 10.0, 5.0);
    directional_light.setShadow(true, 4096, 4096);
    directional_light.setShadowProjection(-10.0, 10.0, -10.0, 10.0, 0.0, 50.0);
    scene.add(directional_light);
    
    bg = new HemisphereBackground();   
    bg.setSkyColor(1.0, 1.0, 1.0);
    bg.setGroundColor(0.02843, 0.07819, 0.07819);
    scene.background = bg;
    
    envLight = new HemisphereLight();
    envLight.setSkyColor(1.0, 1.0, 1.0);
    envLight.setGroundColor(0.02843, 0.07819, 0.07819);
    scene.indirectLight = envLight;

    box = new SimpleModel();
    box.name = "box";
    box.createBox(2.0, 2.0, 2.0);
    box.translateX(-1.5);

    let axis = new Vector3(1.0, 1.0, 0.0);
    axis.normalize();
    box.rotateOnAxis(axis, 1.0);
    {
        let img = imageLoader.loadFile("assets/textures/uv-test-bw.png");
        box.setColorTexture(img);
        img.dispose();
    }
    scene.add(box);

    sphere = new SimpleModel();
    sphere.name = "sphere";
    sphere.createSphere(1.0);
    sphere.translateX(1.5);
    {
        let img = imageLoader.loadFile("assets/textures/uv-test-col.png");
        sphere.setColorTexture(img);
        img.dispose();
    }
    sphere.metalness = 0.5;
    sphere.roughness = 0.5;
    scene.add(sphere);
    
    ground = new SimpleModel();
    ground.createPlane(10.0, 10.0);    
    ground.translateY(-1.7);
    ground.rotateX(-3.1416*0.5);
    scene.add(ground);  

    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;
}

function dispose() {
    ground.dispose();
    sphere.dispose();
    box.dispose();
    envLight.dispose();
    bg.dispose();
    directional_light.dispose();
    camera.dispose();
    scene.dispose();
    renderer.dispose();
}

function render(width, height, size_changed) {    
    if (size_changed) 
    {
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    }

    if (controls.hasOwnProperty('update'))
    {
        controls.update();
    }
    
    renderer.render(scene, camera);
}

setCallback('init', init);
setCallback('dispose', dispose);
setCallback('render', render);
