import { Vector3 } from "./math/Vector3.js";
import { Matrix4 } from "./math/Matrix4.js";
import { Clock } from "./utils/Clock.js";
import { OrbitControls } from "./controls/OrbitControls.js";

import { view } from "./view.js";

/*const getCircularReplacer = () => {
  const seen = new WeakSet();
  return (key, value) => {
    if (typeof value === "object" && value !== null) {
      if (seen.has(value)) {
        return;
      }
      seen.add(value);
    }
    return value;
  };
};*/


let renderer, scene, camera, directional_light, bg, box, sphere, ground, clock, controls;

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
    
    bg = new ColorBackground();
    scene.background = bg;
    bg.setColor(0.0, 0.52, 1.0);

    box = new SimpleModel();
    box.name = "box";
    box.createBox(2.0, 2.0, 2.0);
    box.translateX(-1.5);

    let axis = new Vector3(1.0, 1.0, 0.0);
    axis.normalize();
    box.rotateOnAxis(axis, 1.0);
    //box.setColor(0.8, 0.4, 0.4);
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
    //sphere.setColor(0.4, 0.8, 0.4);
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

    clock = new Clock();

    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;

}

function dispose() {
    ground.dispose();
    sphere.dispose();
    box.dispose();
    bg.dispose();
    directional_light.dispose();
    camera.dispose();
    scene.dispose();
    renderer.dispose();
}

const axis = new Vector3(0, 1, 0);
const rotation = new Matrix4();

function render(width, height, size_changed) {    
    if (size_changed) {
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    }
    let delta = clock.getDelta();
    /*rotation.makeRotationAxis(axis, delta * 0.5);
    camera.applyMatrix4(rotation);*/

    if (controls.hasOwnProperty('update'))
    {
        controls.update();
    }
    renderer.render(width, height, scene, camera);
}

setCallback('init', init);
setCallback('dispose', dispose);
setCallback('render', render);
