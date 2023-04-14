[<--Home](index.html)

# class AnimationMixer

Utility for linear blending of animation clips.

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [AnimationMixer()](#animationmixer)                           | Creates a new AnimationMixer.                                  |
| **Properties**                                                |                                                                |
| [animations](#animations)                                     | list of current added animation clips                          |
| [currentPlaying](#currentplaying)                             | list of current playing animation clips                        |
| **Methods**                                                   |                                                                |
| [dispose()](#dispose)                                         | Dispose the unmanaged resource.                                |
| [getAnimation](#getanimation)                                 | Get an added animation clip by name.                           |
| [getAnimations](#getanimations)                               | Get all added animation clips.                                 |
| [addAnimation](#addanimation)                                 | Add an animation clip to the mixer.                            |
| [addAnimations](#addanimations)                               | Add multiple animation clips to the mixer.                     |
| [startAnimation](#startanimation)                             | Start an animation by name.                                    |
| [stopAnimation](#stopanimation)                               | Stop an animation by name.                                     |
| [setWeights](#setweights)                                     | Set blending weigths for each animation currently being played.|
| [getFrame](#getframe)                                         | Get the mixed animation frame of current time point.           |

# Constructors

## AnimationMixer()

 `AnimationMixer`()

Creates a new AnimationMixer.

# Properties

## animations

`.animations`: Array

Read-only property for displaying the info of added animation clips.

`.currentPlaying`: Array 

Read-only property for displaying the info of currently playing animation clips.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## getAnimation()

`.getAnimation`(`name`: String): Object

Get an added animation clip by name. 

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

Get all added animation clips.

Each element of the returned array has the same structure as the return value of [`.getAnimation()`](#getanimation).

## addAnimation()

`.addAnimation`(`animation`: Object): undefined

Add an animation clip to the mixer.

The animation object should have the same structure as the return value of [`.getAnimation()`](#getanimation).

## addAnimations()

`.addAnimations`(`animations`: Array): undefined

Add multiple animation clips to the mixer.

Each element of the array should have the same structure as the return value of [`.getAnimation()`](#getanimation).

## startAnimation()

`.startAnimation`(`name`: String) : undefined

Start an animation by name.

## stopAnimation()

`.stopAnimation`(`name`: String) : undefined

Stop an animation by name.

## setWeights()

`.setWeights`(`weights`: Array) : undefined

Set blending weigths for each animation currently being played.

A weight of 0 will cause the corresponding animation clip being removed from the current playing list.

## getFrame()

`.getFrame`() : Object

Get the mixed animation frame of current time point.

The returned frame object has the following properties:

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

