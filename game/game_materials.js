import { Vector3 } from "./math/Vector3.js";
import { OrbitControls } from "./controls/OrbitControls.js";
import { view } from "./view.js";

let spheres = new Array(25);

function init(width, height) {
    renderer = new GLRenderer();
    scene = new Scene();
    
    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    camera.setPosition(0.0, 0.0, 4.0);
    
    directional_light = new DirectionalLight();
    directional_light.intensity = 2.0;    
    directional_light.setPosition(6.0, 6.0*0.51, 6.0*0.88); 
    scene.add(directional_light);     
    
    background = new CubeBackground();
    {
        let cube_img = new imageLoader.loadCubeFromFile(
        "assets/textures/sky_cube_face0.jpg", "assets/textures/sky_cube_face1.jpg",
        "assets/textures/sky_cube_face2.jpg", "assets/textures/sky_cube_face3.jpg",
        "assets/textures/sky_cube_face4.jpg", "assets/textures/sky_cube_face5.jpg");        
        background.setCubemap(cube_img);                
        
        let envMapCreator = new EnvironmentMapCreator();
        envMap = envMapCreator.create(background);
        
        envMapCreator.dispose();
        cube_img.dispose();     
    }
    scene.background = background;
    
    envMap.diffuseThresh = 0.2*0.4;
    envMap.diffuseHigh = 0.8 *0.4;
    envMap.diffuseLow = 0.2 *0.4;
    scene.indirectLight = envMap;
    
    for (let y=0; y<5; y++)
    {
        for (let x=0; x<5; x++)
        {
            let i = x + y*5;
        
            spheres[i] = new SimpleModel();
            spheres[i].createSphere(0.2);
            spheres[i].metalness = y * 0.25;
            spheres[i].roughness = 1.0 - x*0.25;            
            spheres[i].setPosition(-1.2 + x*0.6, 1.2 - y*0.6, 0.0);
            // spheres[i].setToonShading(1);
            scene.add(spheres[i]);
        
        }
    }
  
 
    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;

}

function dispose() {
    for (let i=0; i<25; i++)
    {
        if (spheres[i])
        {
            spheres[i].dispose();
        }
    }
    envMap.dispose();
    background.dispose();
    directional_light.dispose();
    camera.dispose();
    scene.dispose();
    renderer.dispose();
}


function render(width, height, size_changed) {    
    if (size_changed) {
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
