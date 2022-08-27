For a quick walkthrough of what can be done in a Three.V8 user script, see [Features](Features.html).

For details of the APIs that enable the features, see below.

# Three.V8 User Script APIs

The user script APIs consist of 

* [Engine Classes](#engine-classes-3d): have properties and methods to communicate with engine objects.
* [Global Functions](#global-functions): can be called directly from user script.
* [Global Objects](#global-objects): are special engine objects which are preexisting in the context rather than created by the user script.
* [Callback Functions](#callback-functions): are used by user script to recieve the event calls from the host program.

# Engine Classes (3D)

| Class Name                                                | Description                                                  |
| --------------------------------------------------------  | ------------------------------------------------------------ |
| [Image](Image.html)                                       | Represents an image that resides in CPU memory.              |
| [CubeImage](CubeImage.html)                               | Represents an an cubemap image that resides in CPU memory.   |
| [Object3D](Object3D.html)                                 | Base class of all 3D objects visible to user script.         |
| [Camera](Camera.html)                                     | Base class for cameras.                                      |
| [PerspectiveCamera](PerspectiveCamera.html)               | Perspective camera                                           |
| [Backround](Background.html)                              | Abstract class for all backgrounds                           |
| [ColorBackground](ColorBackground.html)                   | A background that has a monotone color.                      |
| [CubeBackground](CubeBackground.html)                     | A background using a CubeMap.                                |
| [HemisphereBackground](HemisphereBackground.html)         | A background that has a sky color and a ground color.        |
| [Light](Light.html)                                       | Abstract class for all direct lights.                        |
| [DirectionalLight](DirectionalLight.html)                 | A light that gets emitted in a specific direction.           |
| [IndirectLight](IndirectLight.html)                       | Abstract class for all indirect lights                       |
| [EnvironmentMap](EnvironmentMap.html)                     | Image based indirect light                                   |
| [EnvironmentMapCreator](EnvironmentMapCreator.html)       | Cube-map filter that generates environmaps                   |
| [AmbientLight](AmbientLight.html)                         | Uniform indirect light                                       |
| [HemisphereLight](HemisphereLight.html)                   | Gradient indirect light                                      |
| [SimpleModel](SimpleModel.html)                           | A Model containing a single simple geometry                  |
| [GLTFModel](GLTFModel.html)                               | A Model that has a GLTF style internal structure.            |
| [Scene](Scene.html)                                       | 3D Object collection for rendering                           |
| [GLRenderer](GLRenderer.html)                             | Manages rendering routines, including shaders                |
| [BoundingVolumeHierarchy](BoundingVolumeHierarchy.html)   | Acceleration structure for ray-casting.                      | 
| [GamePlayer](GamePlayer.html)                             | Wrapper for the host GamePlayer object.                      |
| [FileLoader](FileLoader.html)                             | Provides a few interfaces to loading local files.            |
| [ImageLoader](ImageLoader.html)                           | Provides a few interfaces to load images.                    |
| [GLTFLoader](GLTFLoader.html)                             | Provides a few interfaces to load GLTF models.               |

# Engine Classes (Network)

# Engine Classes (GUI)

# Global Functions

These are functions that can be called directly in user script.

| Function Name                                             | Description                                                  |
| --------------------------------------------------------  | ------------------------------------------------------------ |
| [print()](#print)                                         | Print strings to `stdout`                                    |
| [setCallback()](#setcallback)                             | Register a callback function.                                |
| [now()](#now)                                             | Current time in milliseconds.                                |
| [getGLError()](#getglerror)                               | Get the last OpenGL error code for debugging.                |

## print()

`print`(`text1`: String, `text2`: String, ...): undefined

Print strings to `stdout` separated by spaces. Objects are not stringified automatically. To do that you need `JSON.stringify()`.

## setCallback()

`setCallback`(`name`: String, `callback`: Function): undefined

Register a callback function.

### Parameters

`name`: one of the names listed in [Callback Functions](#callback-functions).

`callback`: function accessable from script, or lambda.

## now()

`now`(): Number

Current time in milliseconds.

## getGLError()

`getGLError`(): Number

Get the last OpenGL error code for debugging.

# Global Objects

These are engine class singletons that can be used directly in user script.

| Object Name                                               | Description                                                  |
| --------------------------------------------------------  | ------------------------------------------------------------ |
| gamePlayer                                                | Instance of [GamePlayer](GamePlayer.html).                   |
| fileLoader                                                | Instance of [FileLoader](FileLoader.html).                   |
| imageLoader                                               | Instance of [ImageLoader](ImageLoader.html).                 |
| gltfLoader                                                | Instance of [GLTFLoader](GLTFLoader.html).                   |

# Callback Functions

User scripts are event driven programs. Callback functions need to be registered in the global scope by calling [setCallback](#setcallback).

The host program calls these functions at specific events according to their names. 

The following callback functions are called by the default "GamePlayer".

Mouse callbacks are specific to desktop while touch callbacks are specific to mobile devices.

| Callback name                     | Description                                                  |
| --------------------------------  | ------------------------------------------------------------ |
| [init()](#init)                   | Called immediately after loading the script.                 |
| [dispose()](#dispose)             | Called before unloading the script.                          |
| [render()](#render)               | Called when rendering a video frame.                         |
| [OnMouseDown()](#onmousedown)     | Called when mouse button is pressed down.                    |
| [OnMouseUp()](#onmouseup)         | Called when mouse button is up.                              |
| [OnMouseMove()](#onmousemove)     | Called when mouse pointer is moved.                          |
| [OnMouseWheel()](#onmousewheel)   | Called when mouse wheel is moved.                            |
| [OnTouchDown()](#ontouchdown)     | Called when user touches screen.                             |
| [OnTouchUp()](#ontouchup)         | Called when user stops touching screen.                      |
| [OnTouchMove()](#ontouchmove)     | Called when user moves on touch screen.                      |

## init()

`init`(`width`: Number, `height`: Number): undefined

Called immediately after loading the script.

### Parameters

`width`: width of the container window.

`height`: height of the container window.

## dispose()

`dispose`(): undefined

Called before unloading the script.

## render()

`render`(`width`: Number, `height`: Number, `sizeChanged`: Boolean): undefined

Called when rendering a video frame.

### Parameters

`width`: width of the container window.

`height`: height of the container window.

`sizeChanged`: if size of windows has changed.

## OnMouseDown()

`OnMouseDown`(`e`: Object): undefined

Called when mouse button is pressed down.

### Parameters

`e`: has the following properties:

`e.x`: Number

x coordinate of mouse pointer

`e.y`: Number

y coordinate of mouse pointer

`e.delta`: Number

wheel delta

`e.button`: Number

0 = Left Button

1 = Middle Button

2 = Right Button

3 = XButton1

4 = XButton2

## OnMouseUp()

`OnMouseUp`(`e`: Object): undefined

Called when mouse button is up.

The parameter `e` has the same structure as in [`OnMouseDown`](#onmousedown)

## OnMouseMove()

`OnMouseMove`(`e`: Object): undefined

Called when mouse pointer is moved.

The parameter `e` has the same structure as in [`OnMouseDown`](#onmousedown)

## OnMouseWheel()

`OnMouseWheel`(`e`: Object): undefined

Called when mouse wheel is moved.

The parameter `e` has the same structure as in [`OnMouseDown`](#onmousedown)

## OnTouchDown()

`OnTouchDown`(`e`: Object): undefined

Called when user touches screen.

### Parameters

`e`: has the following properties:

`pointerId`: Number

Identifier for distinguishing multiple touch points.

`x`: number 

x coordinate of touch point.

`y`: number 

y coordinate of touch point.

## OnTouchUp()

`OnTouchUp`(`e`: Object): undefined

Called when user stops touching screen.

The parameter `e` has the same structure as in [`OnTouchDown`](#ontouchdown)

## OnTouchMove()

`OnTouchMove`(`e`: Object): undefined

Called when user moves on touch screen.

The parameter `e` has the same structure as in [`OnTouchDown`](#ontouchdown)
