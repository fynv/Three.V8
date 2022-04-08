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


let renderer, scene, camera, bg, model, clock, controls;

function init(width, height) {
    renderer = new GLRenderer();
    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    camera.setPosition(0.0, 0.0, 4.0);
    scene = new Scene();
    bg = new ColorBackground();
    scene.background = bg;
    bg.setColor(0.0, 0.52, 1.0);

    model = gltfLoader.loadModelFromFile("../game/assets/models/toy_freddy.glb");
    model.setScale(0.01, 0.01, 0.01);
    model.rotateX(-3.1416 / 2);

    //model = gltfLoader.loadModelFromFile("../game/assets/models/Parrot.glb");
    scene.add(model);  

    clock = new Clock();

    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;

}

function dispose() {
    model.dispose();
    bg.dispose();
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
