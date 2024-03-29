[<--Home](index.html)

# class GLTFModel

A Model that has a GLTF style internal structure.

`class GLTFModel extends Object3D`

Inheritance [Object3D](Object3D.html) --> GLTFModel

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [GLTFModel()](#gltfmodel)                                     | Creates a new GLTFModel.                                       |
| **Properties**                                                |                                                                |
| [minPos](#minpos)                                             | Bounding box min-position.                                     |
| [maxPos](#maxPos)                                             | Bounding box max-position.                                     |
| [meshes](#meshes)                                             | Info of internal meshes.                                       |
| [animations](#animations)                                     | Info of internal animation clips.                              |
| **Methods**                                                   |                                                                |
| [setTexture](#settexture)                                     | Replace a texture using texture name.                          |
| [batchPrimitives](#batchprimitives)                           | Batch together primitives of the same materials.               |
| [setAnimationFrame](#setanimationframe)                       | Assign the current stage of movable parts.                     |
| [getAnimation](#getanimation)                                 | Get a loaded animation clip by name.                           |
| [getAnimations](#getanimations)                               | Get all loaded animation clips.                                |
| [addAnimation](#addanimation)                                 | Add an animation clip to the model.                            |
| [addAnimations](#addanimations)                               | Add multiple animation clips to the model.                     |
| [playAnimation](#playanimation)                               | Play the animation clip of the given name.                     |
| [stopAnimation](#stopanimation)                               | Stop the animation clip of the given name.                     |
| [updateAnimation](#updateanimation)                           | Update the movable parts according to the current frame.       |
| [setLightmap](#setlightmap)                                   | Set a lightmap image for the model.                            |

# Constructors

## GLTFModel()

 `GLTFModel`()

Creates a new GLTFModel. Usually not created directly. Use [GLTFLoader](GLTFLoader.html) class to create an GLTFModel.

# Properties

See the base [Object3D](Object3D.html#properties) class for common properties.

## minPos

`.minPos`: Object

Read-only property for bounding box min-position.

## maxPos

`.maxPos`: Object

Read-only property for bounding box max-position.

## meshes

`.meshes`: Object

Read-only property for displaying the info of internal meshes.

## animations

`.animations`: Array

Read-only property for displaying the info of internal animation clips.

# Methods

See the base [Object3D](Object3D.html#methods) class for common methods.

## setTexture()

`.setTexture`(`name`: String, `image`: [Image](Image.html)): undefined

Replace the texture of `name` with `image`.

`.setTexture`(`name`: String, `target`: [GLRenderTarget](GLRenderTarget.html)): undefined

Use the rendering result of `target` to replace the texture of `name`.

`.setTexture`(`name`: String, `mmcamera`: [MMCamera](MMCamera.html)): undefined

Usa a web-camera image source `mmcamera` to replace the texture of `name`.

`.setTexture`(`name`: String, `mmlazyvideo`: [MMLazyVideo](MMLazyVideo.html)): undefined

Usa a video-file image source `mmlazyvideo` to replace the texture of `name`.

`.setTexture`(`name`: String, `mmvideo`: [MMVideo](MMVideo.html)): undefined

Usa a video-file player `mmvideo` to replace the texture of `name`.

## batchPrimitives()

`.batchPrimitives`(): undefined

Batch together primitives of the same materials.

## setAnimationFrame()

`.setAnimationFrame`(`frame`: Object): undefined

Assign the current stage of movable parts using a JS object.

### The `frame` object should have the following properties:

`frame.morphs`: Array

Optional. Morph state for morphable meshes.

`frame.morphs[i].name`: String

Name of a morphable mesh.

`frame.morphs[i].weights`: Array

Weight for each morph target of the mesh.

`frame.translations`: Array

Optional. Translation states for nodes.

`frame.translations[i].name`: String

Name of the targeted node.

`frame.translations[i].translation`: Vector3

Translation state of the node.

`frame.rotations`: Array

Optional. Rotation states for nodes.

`frame.rotations[i].name`: String

Name of the targeted node.

`frame.rotations[i].rotation`: Quaternion

Rotation state of the node.

`frame.scales`: Array

Optional. Scale states for nodes.

`frame.scales[i].name`: String

Name of the targeted node.

`frame.translations[i].scale`: Vector3

Scale state of the node.

## getAnimation()

`.getAnimation`(`name`: String): Object

Get a loaded animation clip by name. 

### The returned object has the following properties:

`.name`: String

Name of the animation clip.

`.duration`: Number

Duration of the animation clip in seconds.

`.morphs`: Array

Optional. Morph tracks.

`.morphs[i].name`: String

Name of the morphable mesh.

`.morphs[i].targets`: Number

Number of morph targets of the morphable mesh.

`.morphs[i].interpolation`: String

Interpolation method: "STEP", "LINEAR" or "CUBICSPLINE"

`.morphs[i].times`: Float32Array

Time stamp of each frame.

`.morphs[i].values`: Float32Array

Weight values of each frame. 

`.translations`: Array

Optional. Translation tracks.

`.translations[i].name`: String

Name of the targeted node.

`.translations[i].interpolation`: String

Interpolation method: "STEP", "LINEAR" or "CUBICSPLINE"

`.translations[i].times`: Float32Array

Time stamp of each frame.

`.translations[i].values`: Float32Array

Translation values of each frame. 

`.rotations`: Array

Optional. Rotation tracks.

`.rotations[i].name`: String

Name of the targeted node.

`.rotations[i].interpolation`: String

Interpolation method: "STEP", "LINEAR"

`.rotations[i].times`: Float32Array

Time stamp of each frame.

`.rotations[i].values`: Float32Array

Rotation values of each frame. 

`.scales`: Array

Optional. Scale tracks.

`.scales[i].name`: String

Name of the targeted node.

`.scales[i].interpolation`: String

Interpolation method: "STEP", "LINEAR" or "CUBICSPLINE"

`.scales[i].times`: Float32Array

Time stamp of each frame.

`.scales[i].values`: Float32Array

Scale values of each frame. 

## getAnimations()

`.getAnimations`(): Array

Get all loaded animation clips.

Each element of the returned array has the same structure as the return value of [`.getAnimation()`](#getanimation).

## addAnimation()

`.addAnimation`(`animation`: Object): undefined

Add an animation clip to the model.

The animation object should have the same structure as the return value of [`.getAnimation()`](#getanimation).

## addAnimations()

`.addAnimations`(`animations`: Array): undefined

Add multiple animation clips to the model.

Each element of the array should have the same structure as the return value of [`.getAnimation()`](#getanimation).

## playAnimation()

`.playAnimation`(`name`: String): undefined

Play the animation clip of the given name.

## stopAnimation()

`.stopAnimation`(`name`: String): undefined

Stop the animation clip of the given name.

## updateAnimation()

`.updateAnimation`(): undefined

Update the movable parts according to the current frame. This function should be called from the `render` callback function.

## setLightmap()

`.setLightmap`(`img`: [HDRImage](HDRImage.html)): undefined

Set a lightmap image for the model.

