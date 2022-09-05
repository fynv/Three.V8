function init(width, height)
{
    renderer = new GLRenderer();
    scene = new Scene();

    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    camera.setPosition(2.0, 5.0, 5.0);
    camera.lookAt(0.0, 0.0, 0.0);
    
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
    scene.add(box);
}

function render(width, height, size_changed)
{
    if (size_changed) 
    {
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    }
    
    renderer.render(scene, camera);

}

setCallback('init', init);
setCallback('render', render);
