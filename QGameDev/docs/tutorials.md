# Tutorials

## New Project

![](new_project.jpg)

![](new_project2.jpg)

![](new_project3.jpg)

## New XML Scene

![](new_file.jpg)

![](new_file_xml.jpg)

![](new_file_xml2.jpg)

## Set-up sky

![](new_sky.jpg)

### Deploy sky-box images

![](sky_box_images.jpg)

![](set_sky_box.jpg)

![](set_sky_box2.jpg)

## Delploy models

Here we just load a single GLB model that contains all the static objects.

![](deploy_models.jpg)

![](deploy_models2.jpg)

![](deploy_models3.jpg)

![](deploy_models4.jpg)

## Setup directional light

![](set_directional_light.jpg)

![](set_directional_light2.jpg)

## Global illumination - Deploy a probe-grid

![](set_probe_grid.jpg)

![](set_probe_grid2.jpg)


## Scripting

Copy code from "project_template" to the project directory. (Skip project.json and scene.xml).

![](scripting.jpg)

Create a target:

![](scripting2.jpg)

Double click the created target:

![](run_target.jpg)

You can then do whatever change you want to the JavaScript code.

It is programmed against the [User Script APIs](api/index.html).


## Camera and Orbit Control settings

These are not part of the Scene editor UI.

Simply type into the XML code:

![](camera_control.jpg)


## Placing an avatar

Deploy the necessary models:

![](avatar_models.jpg)

Insert the following code to the scene.xml:

```xml
<avatar src="assets/models/CyberbotGold.glb" name_idle="idle" name_forward="walk_forward" name_backward="walk_backward" camera_position="0.0, 1.5, -2.0" url_anim="assets/models/Animations.glb" fix_anims="fix_anims" position="0, 1.8, 2.5" rotation="0, -180, 0"/>
```

and the following code to scene.js

```js
// The code is necessary only when the animation track names doesn't match
// the joint names in the character model.
export function fix_anims(anims)
{
    for (let anim of anims)
    {
        if ("translations" in anim)
        {
            let translations = anim.translations;
            for (let trans of translations)
            {
                trans.name = "mixamorig:"+trans.name;
            }
        }
        
        if ("rotations" in anim)
        {
            let rotations = anim.rotations;
            for (let rot of rotations)
            {
                rot.name = "mixamorig:"+rot.name;
            }
        }
    }
}

```

## Using lightmaps

Using Lightmapper.exe, you can insert a 2nd UV set into villa.glb. In this way, we get villa_atlas.glb:

![](atlas_model.jpg)

Create a new XML scene (scene2.xml) and copy \<sky\>, \<model\> and \<directional_light\> settings from scene.xml:

![](scene2.jpg)

Modify villa.glb into villa_atlas.glb, the go to "Scene" tab:

![](scene2_2.jpg)

Name the lightmap, then select "scene" in the Scene Graph, and click "Bake":

![](scene2_3.jpg)

Modify scene.js so that scene2.xml gets load instead of scene.xml, when you double click "Target":

![](scene2_4.jpg)

![](scene2_5.jpg)
