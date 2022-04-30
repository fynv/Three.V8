# Global Functions

These are functions exposed through the "global object", so they can be called directly in user script.

### `print`(`text1`: String, `text2`: String, ...): undefined

Print strings to `stdout` separated by spaces. Objects are not stringified automatically. To do that you need `JSON.stringify()`.

### `setCallback`(`name`: String, `callback`: Function): undefined

Register a callback function.

### `now`(): Number

Current time in milliseconds.

### `getGLError`(): Number

Get the last OpenGL error code for debugging.

# Callback Functions

User scripts are event driven programs. Callback functions need to be registered in the global scope by calling:

`setCallback`(`name`: String, `callback`: Function): undefined

The host program calls these functions at specific events according to their names. 

The following callback functions are called by the default "GamePlayer":

### `init`(`width`: Number, `height`: Number): undefined

Called immediately after loading the script.

### `dispose`(): undefined

Called before unloading the script.

### `render`(`width`: Number, `height`: Number, `sizeChanged`: Boolean): undefined

Called when rendering a video frame.

### `OnMouseDown`(`e`: Object): undefined

Called when mouse button is pressed down.

The parameter `e` has the following properties:

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

### `OnMouseUp`(`e`: Object): undefined

Called when mouse button is up.

The parameter `e` has the same structure as in `OnMouseDown`

### `OnMouseMove`(`e`: Object): undefined

Called when mouse pointer is moved.

The parameter `e` has the same structure as in `OnMouseDown`

### `OnMouseWheel`(`e`: Object): undefined

Called when mouse wheel is moved.

The parameter `e` has the same structure as in `OnMouseDown`

# Image

`class Image`

Class that represents an image that resides in CPU memory. Usually not created directly. Use ImageLoader class to create an image.

## Constructor 

### `Image`()
### `Image`(`width`: Number, `height`: Number, `hasAlpha`: Boolean)

`hasAlpha` -- When set to True, the image has 4 channels, otherwise it has 3 channels.

Note that this class is not intended to be called directly.

## Properties

### `.hasAlpha`: Boolean

When set to True, the image has 4 channels, otherwise it has 3 channels.

Read-only.

### `.width`: Number

Width of the image.

Read-only.

### `.height`: Number

Height of the image.

Read-only.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.

# CubeImage

Class that represents an cubemap image that resides in CPU memory.

An CubeImage contains 6 images.

Usually not created directly. Use ImageLoader class to create an cubemap image.

## Constructor 

### `CubeImage`()

Note that this class is not intended to be called directly.

## Properties

### `.width`: Number

Width of the image.

Read-only.

### `.height`: Number

Height of the image.

Read-only.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.


# Object3D

`class Object3D`

Base class of all 3D objects visible to user script.

## Constructor

### `Object3D`()

## Properties

### `.name`: String

Readable and writtable. Default is an empty string.

### `.parent`: Object3D

Object's parent in the scene graph.

Readable and writtable. 

### `.children`: Array

Array with object's children. 

Read-only. Use methods like `add()`, `remove()` to modify this property.

### `.up`: Object

This is used by the lookAt method, for example, to determine the orientation of the result.

Read-only. Use method `setUp()` to modify this property.

Default is {x: 0, y: 1, z: 0}.

### `.position`: Object

A Vector3 representing the object's local position

Read-only. Use method `setPosition()` to modify this property.

Default is {x: 0, y: 0, z: 0}.

### `.rotation`: Object

Object's local rotation, Euler angles in radians.

Read-only. Use method `setRotation()` to modify this property.

### `.quaternion`: Object

Object's local rotation as a Quaternion.

Read-only. Use method `setQuaternion()` to modify this property.

### `.scale`: Object

The object's local scale. 

Default is {x: 1, y: 1, z: 1}.

Read-only. Use method `setScale()` to modify this property.

### `.matrix`: Object

The local transform matrix.

Read-only.

### `.matrixWorld`: Object

The global transform of the object. If the Object3D has no parent, then it's identical to the local transform .matrix.

Read-only.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.

### `.getUp`(`vector`: Vector3): Vector3

Copy the value of `.up` into `vector`.

### `.setUp`(`vector`: Vector3): undefined
### `.setUp`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of `.up` according to `vector`.

Or, set the value of `.up` according to the x, y, z coordinates.

### `.getPosition`(`vector`: Vector3): Vector3

Copy the value of `.position` into `vector`.

### `.setPosition`(`vector`: Vector3): undefined
### `.setPosition`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of `.position` according to `vector`.

Or, set the value of `.position` according to the x, y, z coordinates.

### `.getRotation`(`vector`: Vector3): Vector3

Copy the value of `.rotation` into `vector`.

### `.setRotation`(`vector`: Vector3): undefined
### `.setRotation`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of `.rotation` according to `vector`.

Or, set the value of `.rotation` according to the x, y, z coordinates.

### `.getQuaternion`(`quaternion`: Quaternion): Quaternion

Copy the value of `.quaternion` into `quaternion`.

### `.setQuaternion`(`quaternion`: Quaternion): undefined
### `.setQuaternion`(`x`: Number, `y`: Number, `z`: Number, `w`: Number ): undefined

Set the value of `.quaternion` according to `quaternion`.

Or, set the value of `.quaternion` according to the x, y, z, w coordinates.

### `.getScale`(`vector`: Vector3): Vector3

Copy the value of `.scale` into `vector`.

### `.setScale`(`vector`: Vector3): undefined
### `.setScale`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of `.scale` according to `vector`.

Or, set the value of `.scale` according to the x, y, z coordinates.

### `.getMatrix`(`matrix`: Matrix4): Matrix4

Copy the value of `.matrix` into `matrix`.

### `.getMatrixWorld`(`matrix`: Matrix4): Matrix4

Copy the value of `.matrixWorld` into `matrix`.

### `.updateMatrix`(): undefined

Updates the local transform.

### `.updateMatrixWorld`(`force` : Boolean): undefined

Updates the global transform of the object and its descendants.

### `.updateWorldMatrix`(`updateParents`: Boolean, `updateChildren`: Boolean): undefined

`updateParents` -- recursively updates global transform of ancestors.

`updateChildren` -- recursively updates global transform of descendants.

### `.applyMatrix4`(`matrix`: Matrix4): this

Applies the matrix transform to the object and updates the object's position, rotation and scale.

### `.applyQuaternion`(`quaternion`: Quaternion): this

Applies the rotation represented by the quaternion to the object.

### `.setRotationFromAxisAngle`(`axis`: Vector3, `angle`: Number ): undefined

`axis` -- A normalized vector in object space.

`angle` -- angle in radians

### `.setRotationFromMatrix`(`m`: Matrix4): undefined

`m` -- rotate the quaternion by the rotation component of the matrix.

Note that this assumes that the upper 3x3 of m is a pure rotation matrix (i.e, unscaled).

### `.rotateOnAxis`(`axis`: Vector3, `angle`: Number ) : this

`axis` -- A normalized vector in object space.

`angle` -- The angle in radians.

Rotate an object along an axis in object space. The axis is assumed to be normalized.

### `.rotateOnWorldAxis`(`axis`: Vector3, `angle`: Number ) : this

`axis` -- A normalized vector in world space.
`angle` -- The angle in radians.

Rotate an object along an axis in world space. The axis is assumed to be normalized. Method Assumes no rotated parent.

### `.rotateX`(`rad`: Number ) : this
`rad` -- the angle to rotate in radians.

Rotates the object around x axis in local space.

### `.rotateY`(`rad`: Number ) : this

`rad` -- the angle to rotate in radians.

Rotates the object around y axis in local space.

### `.rotateZ`(`rad`: Number ) : this

`rad` -- the angle to rotate in radians.

Rotates the object around z axis in local space.

### `.translateOnAxis` `axis`: Vector3, `distance`: Number ) : this

`axis` -- A normalized vector in object space.
`distance` -- The distance to translate.

Translate an object by distance along an axis in object space. The axis is assumed to be normalized.

### `.translateX`(`distance`: Number ) : this

Translates object along x axis in object space by distance units.

### `.translateY`(`distance`: Number ) : this

Translates object along y axis in object space by distance units.

### `.translateZ`(`distance`: Number ) : this

Translates object along z axis in object space by distance units.

### `.localToWorld`(`vector`: Vector3 ) : Vector3

`vector` -- A vector representing a position in this object's local space.

Converts the vector from this object's local space to world space.

### `.worldToLocal`(`vector`: Vector3 ) : Vector3

`vector` -- A vector representing a position in world space.

Converts the vector from world space to this object's local space.

### `.getWorldPosition`(`target`: Vector3 ) : Vector3

`target` -- the result will be copied into this Vector3.

Returns a vector representing the position of the object in world space.

### `.getWorldQuaternion`(`target`: Quaternion ) : Quaternion

`target` -- the result will be copied into this Quaternion.

Returns a quaternion representing the rotation of the object in world space.

### `.getWorldScale`(`target`: Vector3 ) : Vector3

`target` -- the result will be copied into this Vector3.

Returns a vector of the scaling factors applied to the object for each axis in world space.

### `.getWorldDirection`(`target`: Vector3 ) : Vector3

`target` -- the result will be copied into this Vector3.

Returns a vector representing the direction of object's positive z-axis in world space.

### `.lookAt`(`vector` : Vector3 ) : undefined
### `.lookAt`(`x`: Number, `y`: Number, `z`: Number ) : undefined

`vector` -- A vector representing a position in world space.

Optionally, the `x`, `y` and `z` components of the world space position.

Rotates the object to face a point in world space.

This method does not support objects having non-uniformly-scaled parent(s).

### `.add`(`object`: Object3D) : this

Adds an object as child of this object. Any current parent on the object passed in here will be removed, since an object can have at most one parent.

### `.remove`(`object`: Object3D) : this

Removes an object as child of this object.

### `.removeFromParent`() : this

Removes this object from its current parent.

### `.clear`() : this

Removes all child objects.

### `.getObjectByName`(`name`: String ) : Object3D

`name` -- String to match to the children's Object3D.name property.

Searches through an object and its children, starting with the object itself, and returns the first with a matching name.
Note that for most objects the name is an empty string by default. You will have to set it manually to make use of this method.

### `.traverse`(`callback`: Function ) : undefined

`callback` -- A function with as first argument an object3D object.

Executes the callback on this object and all descendants.
Note: Modifying the scene graph inside the callback is discouraged.

# Camera

`class Camera extends Object3D`

Base class for cameras. 

## Constructor

### `Camera`()

Creates a new Camera. Note that this class is not intended to be called directly.

## Properties

See the base Object3D class for common properties.

`.matrixWorldInverse`: Object

This is the inverse of `.matrixWorld`. `.matrixWorld` contains the Matrix which has the world transform of the Camera.

Read-only.

`.projectionMatrix`: Object

This is the matrix which contains the projection.

Read-only.

`.projectionMatrixInverse`: Object

The inverse of projectionMatrix.

Read-only.

## Methods

See the base Object3D class for common methods.

`.getMatrixWorldInverse`(`matrix`: Matrix4) : Matrix4

Copy the value of `.matrixWorldInverse` into `matrix`.

`.getProjectionMatrix`(`matrix`: Matrix4) : Matrix4

Copy the value of `.projectionMatrix` into `matrix`.

`.getProjectionMatrixInverse`(`matrix`: Matrix4) : Matrix4

Copy the value of `.projectionMatrixInverse` into `matrix`.

# PerspectiveCamera

`class PerspectiveCamera extends Camera`

## Constructor

### `PerspectiveCamera`(`fov`: Number, `aspect`: Number, `near`: Number, `far`: Number)

`fov` -- Camera frustum vertical field of view.
`aspect` -- Camera frustum aspect ratio.
`near` -- Camera frustum near plane.
`far` -- Camera frustum far plane.

Together these define the camera's viewing frustum.

## Properties

See the base Camera class for common properties.
Note that after making changes to most of these properties you will have to call `.updateProjectionMatrix` for the changes to take effect.

`.isPerspectiveCamera`: Boolean 

Read-only flag to check if a given object is of type PerspectiveCamera.

`.fov`: Number

Camera frustum vertical field of view, from bottom to top of view, in degrees. Default is 50.

Readable and writable.

`.aspect`: Number

Camera frustum aspect ratio, usually the canvas width / canvas height. Default is 1 (square canvas).

Readable and writable.

`.near`: Number

Camera frustum near plane. Default is 0.1.

The valid range is greater than 0 and less than the current value of the far plane.

Readable and writable.

`.far`: Number

Camera frustum far plane. Default is 200.

Must be greater than the current value of near plane.

Readable and writable.

## Methods

See the base Camera class for common methods.

`.updateProjectionMatrix`(): undefined

Updates the camera projection matrix. Must be called after any change of parameters.

# Backround

`class Background`

Abstract class for all backgrounds

No contructor, never used directly.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.

# ColorBackground

`class ColorBackground extends Backround`

A background that has a monotone color.

## Constructor

### `ColorBackground`()

Creates a new ColorBackground. 

## Properties

### `.color`: Object

The color of the background.

Read-only. Use the method `.setColor` to modify this property.

## Methods

### `.getColor`(`color`: Vector3) : Vector3

Copy the value of `.color` into `color`.

### `.setColor`(`color`: Vector3): undefined
### `.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.color` according to `color`.

Or, set the value of `.color` according to the `r`, `g`, `b` values.

# CubeBackground

`class CubeBackground extends Backround`

A background using a CubeMap.

## `CubeBackground`()

Creates a new CubeBackground. 

## Methods

### `.setCubemap`(`cubeMap`: CubeMap): undefined

Set the cube-map data.

# HemisphereBackground

`class HemisphereBackground extends Backround`

A background that has a sky color and a ground color.

## Constructor

### `HemisphereBackground`()

Creates a new HemisphereBackground. 

## Properties

### `.skyColor`: Object

The sky color of the background.

Read-only. Use the method `.setSkyColor` to modify this property.

### `.groundColor`: Object

The ground color of the background.

Read-only. Use the method `.setGroundColor` to modify this property.

## Methods

### `.getSkyColor`(`color`: Vector3) : Vector3

Copy the value of `.skyColor` into `color`.

### `.setSkyColor`(`color`: Vector3): undefined
### `.setSkyColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.skyColor` according to `color`.

Or, set the value of `.skyColor` according to the `r`, `g`, `b` values.

### `.getGroundColor`(`color`: Vector3) : Vector3

Copy the value of `.groundColor` into `color`.

### `.setGroundColor`(`color`: Vector3): undefined
### `.setGroundColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.groundColor` according to `color`.

Or, set the value of `.groundColor` according to the `r`, `g`, `b` values.

# Light

`class Light extends Object3D`

Abstract class for all direct lights.

No contructor, never used directly.

## Properties

See the base Object3D class for common properties.

### `.color`: Object

Color of the light object.

Read-only. Use the method `.setColor` to modify this property.

### `.intensity`: Number

Intensity of the light object.

Readable and writable.

## Methods

See the base Object3D class for common methods.

### `.getColor`(`color`: Vector3) : Vector3

Copy the value of `.color` into `color`.

### `.setColor`(`color`: Vector3): undefined
### `.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.color` according to `color`.

Or, set the value of `.color` according to the `r`, `g`, `b` values.

# DirectionalLight

`class Light extends Light`

A light that gets emitted in a specific direction. This light will behave as though it is infinitely far away and the rays produced from it are all parallel. The common use case for this is to simulate daylight; the sun is far enough away that its position can be considered to be infinite, and all light rays coming from it are parallel. 

This light can cast shadows.

## Constructor

### `DirectionalLight`()

Creates a new DirectionalLight.

## Properties

See the base Light class for common properties.

### `.target`: Object3D

The DirectionalLight points from its position to target.position. The default position of the target is (0, 0, 0).

## Methods

See the base Light class for common methods.

### `.setShadow`(`enable` : Boolean, `width`: Number, `height`: Number): undefined

`enable` -- If set to true light will cast dynamic shadows.

`width` -- width of the shadow map.

`height` -- height of the shadow map.

### `.setShadowProjection`(`left`: Number, `right`: Number, `bottom`: Number, `top`: Number, `zNear`: Number, `zFar`: Number): undefined

Set the orthographic frustum parameters.

`left` -- Frustum left plane.

`right` -- Frustum right plane.

`top` -- Frustum top plane.

`bottom` -- Frustum bottom plane.

`near` -- Frustum near plane.

`far` -- Frustum far plane.

# IndirectLight

`class IndirectLight`

Abstract class for all indirect lights

No contructor, never used directly.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.

# EnvironmentMap

`class EnvironmentMap extends IndirectLight`

## Constructor 

### `EnvironmentMap`()

Usually not created directly. Use EnvironmentMapCreator class to create an EnvironmentMap object.

# EnvironmentMapCreator

`class EnvironmentMapCreator`

## Constructor

### `EnvironmentMapCreator`()

Create an EnvironmentMapCreator object, which can be used to create EnvironmentMap objects.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.

### `.create`(`image`: CubeImage): EnvironmentMap
### `.create`(`background`: CubeBackground): EnvironmentMap

Create an EnvironmentMap object using a cubemap image or a cubemap background.

# AmbientLight

`class AmbientLight extends IndirectLight`

Create an ambient light object.

This light globally illuminates all objects in the scene equally.

This light cannot be used to cast shadows as it does not have a direction.

## Constructor

### `AmbientLight`()

Creates a new AmbientLight.

## Properties

### `.color`: Object

The color of the ambient light.

Read-only. Use the method `.setColor` to modify this property.

### `.intensity`: Number

The intensity of the ambient light.

Readable and writable.

## Methods

### `.getColor`(`color`: Vector3) : Vector3

Copy the value of `.color` into `color`.

### `.setColor`(`color`: Vector3): undefined
### `.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.color` according to `color`.

Or, set the value of `.color` according to the `r`, `g`, `b` values.

# HemisphereLight

`class HemisphereLight extends IndirectLight`

A light source positioned directly above the scene, with color fading from the sky color to the ground color.

This light cannot be used to cast shadows.

## Constructor

### `HemisphereLight`()

Creates a new HemisphereLight.

## Properties

### `.skyColor`: Object

The sky color of the hemisphere light.

Read-only. Use the method `.setSkyColor` to modify this property.

### `.groundColor`: Object

The ground color of the hemisphere light.

Read-only. Use the method `.setGroundColor` to modify this property.

### `.intensity`: Number

The intensity of the hemisphere light.

Readable and writable.

## Methods

### `.getSkyColor`(`color`: Vector3) : Vector3

Copy the value of `.skyColor` into `color`.

### `.setSkyColor`(`color`: Vector3): undefined
### `.setSkyColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.skyColor` according to `color`.

Or, set the value of `.skyColor` according to the `r`, `g`, `b` values.

### `.getGroundColor`(`color`: Vector3) : Vector3

Copy the value of `.groundColor` into `color`.

### `.setGroundColor`(`color`: Vector3): undefined
### `.setGroundColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.groundColor` according to `color`.

Or, set the value of `.groundColor` according to the `r`, `g`, `b` values.

# SimpleModel

`class SimpleModel extends Object3D`

A Model containing a single simple geometry.

## Constructor

### `SimpleModel`()

Creates a new SimpleModel.

## Properties

See the base Object3D class for common properties.

### `.color`: Object

Base-color of the material of the model.

Read-only. Use the method `.setColor` to modify this property.

### `.metalness`: Number

Metalness factor of the material of the model.

Readable and writable.

### `.roughness`: Number

Roughness factor of the material of the model.

Readable and writable.

## Methods

See the base Object3D class for common methods.

### `.createBox`(`width`: Number, `height`: Number, `depth`: Number): undefined

Create a Box shaped geometry for the model.

`width` -- width of the Box

`height` -- height of the Box

`depth` -- depth of the Box

### `.createSphere`(`radius`: Number, `widthSegments`: Number, `heightSegments`: Number): undefined

Create a Sphere shaped geometry for the model.

`radius` -- radius of the Sphere

`widthSegments` -- number of width segments of the triangulated Sphere.

`heightSegments` -- number of height segments of the triangulated Sphere.

### `.createPlane`(`width`: Number, `height`: Number): undefined

Create a Plane shaped geometry for the model.

`width` -- width of the Plane

`height` -- height of the Plane

### `.getColor`(`color`: Vector3) : Vector3

Copy the value of `.color` into `color`.

### `.setColor`(`color`: Vector3): undefined
### `.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.color` according to `color`.

Or, set the value of `.color` according to the `r`, `g`, `b` values.

### `.setColorTexture`(`image`: Image): undefined

Set a texture image as the based color map of the model.

# GLTFModel

`class GLTFModel extends Object3D`

A Model that has a GLTF style internal structure.

## Constructor

### `GLTFModel`()

Creates a new GLTFModel. Usually not created directly. Use GLTFLoader class to create an GLTFModel.

## Properties

See the base Object3D class for common properties.

### `.meshes`: Object

Read-only property for displaying the info of internal meshes.

### `.animations`: Object

Read-only property for displaying the info of internal animation clips.

## Methods

See the base Object3D class for common methods.

### `.setAnimationFrame`(`frame`: Object): undefined

Assign the current stage of movable parts using a JS object.

The object should have the following properties:

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

### `.getAnimation`(`name`: String): Object

Get an loaded animation clip by name. 

The returned object has the following properties:

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

### `.getAnimations`(): Array

Get all loaded animation clips.

Each element of the returned array has the same structure as the return value of `.getAnimation`.

### `.addAnimation`(`animation`: Object): undefined

Add an animation clip to the model.

The animation object should have the same structure as the return value of `.getAnimation`.

### `.AddAnimations`(`animations`: Array): undefined

Add multiple animation clips to the model.

Each element of the array should have the same structure as the return value of `.getAnimation`.

### `.playAnimation`(`name`: String): undefined

Play the animation clip of the given name.

### `.stopAnimation`(`name`: String): undefined

Stop the animation clip of the given name.

### `.updateAnimation`(): undefined

Update the movable parts according to the current frame. This function should be called from the `render` callback function.

# Scene

`class Scene extends Object3D`

Scenes allow you to set up what and where is to be rendered by Three.V8. This is where you place objects, lights and cameras.

## Constructor

### `Scene`()

Create a new scene object.

## Properties

See the base Object3D class for common properties.

### `.background`: Background

Object used as background.

Readable and writable.

### `.indirectLight`: IndirectLight

Object used as the indirect light-source.

Readable and writable.

## Methods

See the base Object3D class for common methods.

# GLRenderer

The OpenGL renderer displays your beautifully crafted scenes using OpenGL.

## Constructor

### `GLRenderer`()

Create a GLRenderer.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.

### `.render`(`width`: Number, `height`: Number, `scene`: Scene, `camera`: Camera): undefined

Should be called from the `render` callback function.

`width` -- width of the current video

`height` -- height of the current video

`scene` -- scene object to be rendered

`camera` -- camera object from where the scene is rendered

# BoundingVolumeHierarchy

Acceleration structure for ray-casting.

## Constructor

### `BoundingVolumeHierarchy`(`objects`: Array)

Create a BoundingVolumeHierarchy from a list of Object3D objects.

## Methods

### `.dispose`(): undefined

Dispose the unmanaged resource.

### `.intersect`(`ray`: Object): Object

Intersect the given ray with the acceleration structure.

The input `ray` object should have the following properties:

`ray.origin`: Vector3

Origin of the ray.

`ray.direction`: Vector3

Direction of the ray.

`ray.near`: Number

Optional. Nearest distance of search.

`ray.far`: Number

Optional. Furthest distance of search.

The returned object has the following properties:

`.name`: String 

Name of the first intersected object.

`.distance`: Number

Distance of the first intersection point.

At the event of missing intersection, it will return null;

# GamePlayer

Provides a few interfaces to access the host GamePlayer object.

No constructor, exposed as a global object `gamePlayer`.

## Properties

### `.width`: Number

Read-only value of current video width.

### `.height`: Number

Read-only value of current video height.

## Methods

### `.setMouseCapture`(): undefined

Set the mouse capture state to True.

### `.releaseMouseCapture`(): undefined

Set the mouse capture state to False.

# FileLoader

Provides a few interfaces to loading local files into memory.

No constructor, exposed as a global object `fileLoader`.

## Methods

### `.loadBinaryFile`(`name`: String): ArrayBuffer

Load a binary file into memory.

### `.loadTextFile`(`name`: String): String

Load a text file (utf8 encoding assumed) into memory.

# ImageLoader

Provides a few interfaces to load images from local files or from memory.

No constructor, exposed as a global object `imageLoader`.

## Methods

### `.loadFile`(`name`: String): Image

Load an image from local file.

### `.loadMemory`(`buf`: ArrayBuffer): Image

Load an image from a memory buffer.

### `.loadCubeFromFile`(`name0`: String, `name1`: String, `name2`: String, `name3`: String, `name4`: String, `name5`: String) : CubeImage 

Load 6 images from local files to form a cubemap image.

### `.loadCubeFromMemory`(`buf0`: ArrayBuffer, `buf1`: ArrayBuffer, `buf2`: ArrayBuffer, `buf3`: ArrayBuffer, `buf4`: ArrayBuffer, `buf5`: ArrayBuffer) : CubeImage 

Load 6 images from memory buffers to form a cubemap image.

# GLTFLoader

Provides a few interfaces to load GLTF models from local files or from memory.

No constructor, exposed as a global object `gltfLoader`.

## Methods

### `.loadModelFromFile`(`name`: String): GLTFModel

Load a GLTF model from a local GLTF file.

### `.loadAnimationsFromFile`(`name`: String): Object

Load only animation data from a local GLTF file.

The returned object has the same structure as the return value of `GLTFModel.getAnimations`.

### `.loadModelFromMemory`(`buf`: ArrayBuffer): GLTFModel

Load a GLTF model from a memory buffer.

### `.loadAnimationsFromMemory`(`buf`: ArrayBuffer): Object

Load only animation data from a memory buffer.

The returned object has the same structure as the return value of `GLTFModel.getAnimations`.

