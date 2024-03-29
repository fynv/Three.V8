import { Vector3 } from "./math/Vector3.js";
import { Matrix4 } from "./math/Matrix4.js";
import { Clock } from "./utils/Clock.js";
import { OrbitControls } from "./controls/OrbitControls.js";

import { view } from "./view.js";

const getCircularReplacer = () => {
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
};

function init(width, height) {
    renderer = new GLRenderer();
    scene = new Scene();
    
    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    
    directional_light = new DirectionalLight();
    directional_light.intensity = 3.0;
    directional_light.setColor(0.5, 1.0, 0.5);
    directional_light.setPosition(15.0, 30.0, 15.0);
    directional_light.setShadow(true, 4096, 4096);
    directional_light.setShadowProjection(-20.0, 20.0, -20.0, 20.0, 0.0, 60.0);
    scene.add(directional_light);   
    
    directional_light2 = new DirectionalLight();
    directional_light2.intensity = 3.0;
    directional_light2.setColor(1.0, 0.5, 0.5);
    directional_light2.setPosition(-15.0, 30.0, 15.0);
    directional_light2.setShadow(true, 4096, 4096);
    directional_light2.setShadowProjection(-20.0, 20.0, -20.0, 20.0, 0.0, 60.0);
    scene.add(directional_light2);
    
    bg = new HemisphereBackground();   
    bg.setSkyColor(0.318, 0.318, 0.318);
    bg.setGroundColor(0.01, 0.025, 0.025);
    scene.background = bg;
    
    envLight = new HemisphereLight();
    envLight.setSkyColor(0.318, 0.318, 0.318);
    envLight.setGroundColor(0.01, 0.025, 0.025);
    scene.indirectLight = envLight;

    camera.setPosition(0.0, 0.0, 20.0);
    model = gltfLoader.loadModelFromFile("../game/assets/models/RZYAS.glb");
    model.setPosition(0, -8, 0);  
 
    scene.add(model);  
       
    ground = new SimpleModel();
    ground.createPlane(25.0, 25.0);    
    ground.translateY(-8.0);
    ground.rotateX(-3.1416*0.5);
    
    scene.add(ground);  
 
    clock = new Clock();

    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;

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

    model.updateAnimation();

    renderer.render(scene, camera);
}

setCallback('init', init);
setCallback('render', render);
