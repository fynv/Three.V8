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

### `OnMouseUp`(`e`: Object): undefined

Called when mouse button is up.

### `OnMouseMove`(`e`: Object): undefined

Called when mouse pointer is moved.

### `OnMouseWheel`(`e`: Object): undefined

Called when mouse wheel is moved.

# Object3D

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


# `GamePlayer`
No constructor, exposed as global object `gamePlayer`.

# `FileLoader`
No constructor, exposed as global object `fileLoader`.

# `ImageLoader`
No constructor, exposed as global object `imageLoader`.

# `GLTFLoader`
No constructor, exposed as global object `gltfLoader`.
