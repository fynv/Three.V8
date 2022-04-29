# Features and Code Snippets

## Basic Routine

```js
import { Vector3 } from "./math/Vector3.js";
import { Matrix4 } from "./math/Matrix4.js";
import { OrbitControls } from "./controls/OrbitControls.js";
import { view } from "./view.js";

let renderer, scene, camera, controls;

function init(width, height)
{
    renderer = new GLRenderer();
    scene = new Scene();

    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    camera.setPosition(0.0, 0.0, 7.0);

    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;

}

function dispose()
{    
    camera.dispose();
    scene.dispose();
    renderer.dispose();
}

function render(width, height, size_changed)
{
    if (size_changed) 
    {
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    }

    if (controls.hasOwnProperty('update'))
    {
        controls.update();
    }
    renderer.render(width, height, scene, camera);

}

setCallback('init', init);
setCallback('dispose', dispose);
setCallback('render', render);
```

In the basic routine above, we register 3 callback functions for `init`, `dispose`, `render` events. In addition, mouse events are recieved and dispatched by the imported `view` object. The `view` object serves as a event dispatcher. The OrbitControls object uses `view` to listen to the mouse events.

Wrappers of engine objects are created in the `init` function. All of these objects needs a `dispose` call in the `dispose` callback function. Three.V8 has such a design because of the concern that all graphics resources need to be released while the OpenGL context is still present, so there's no way to rely on the garbage collector.

## Backgrounds

The Scene class has a [`.background`](https://github.com/fynv/Three.V8/blob/main/docs/UserScriptAPIs.md#background-background) property.

A background is optional in a Three.V8 Scene. It is possible to integrate background/foreground layers from outside the engine. 

When user choose to use a background Three.V8, now we have the following options:

[ColorBackground](https://github.com/fynv/Three.V8/blob/main/docs/UserScriptAPIs.md#colorbackground): Use a monotone color.

[CubeBackground](https://github.com/fynv/Three.V8/blob/main/docs/UserScriptAPIs.md#cubebackground): Use a cubemap.

[HemisphereBackground](https://github.com/fynv/Three.V8/blob/main/docs/UserScriptAPIs.md#hemispherebackground): Use a gradient change from sky-color to ground-color.

ColorBackground:
```js
bg = new ColorBackground();
bg.setColor(0.0, 0.52, 1.0);
scene.background = bg;
```

HemisphereBackground:
```js
bg = new HemisphereBackground();   
bg.setSkyColor(1.0, 1.0, 1.0);
bg.setGroundColor(0.02843, 0.07819, 0.07819);
scene.background = bg;
```

CubeBackground:
```js
bg = new CubeBackground();
let cube_img = new imageLoader.loadCubeFromFile(
"assets/textures/sky_cube_face0.jpg", "assets/textures/sky_cube_face1.jpg",
"assets/textures/sky_cube_face2.jpg", "assets/textures/sky_cube_face3.jpg",
"assets/textures/sky_cube_face4.jpg", "assets/textures/sky_cube_face5.jpg");        
bg.setCubemap(cube_img);      
cube_img.dispose();
scene.background = bg;
```


