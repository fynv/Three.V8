[<--Home](index.html)

# class Object3D

Class that represents an image that resides in CPU memory. 

`class Object3D`

| Name                                                        | Description                                          |
| ------------------------------------------------------------| ---------------------------------------------------- |
| **Constructors**                                            |                                                      |
| [Object3D()](#object3d)                                     | Base class of all 3D objects.                        |
| **Properties**                                              |                                                      |
| [name](#name)                                               |                                                      |
| [parent](#parent)                                           |                                                      |
| [children](#children)                                       |                                                      |
| [up](#up)                                                   | Up direction of object                               |
| [position](#position)                                       | Position                                             |
| [rotation](#rotation)                                       | Euler angles in radians                              |
| [quaternion](#quaternion)                                   | Rotation in quaternion                               |
| [scale](#scale)                                             | Scale                                                |
| [matrix](#matrix)                                           | The local transform matrix.                          |
| [matrixWorld](#matrixworld)                                 | The global transform matrix.                         |
| **Methods**                                                 |                                                      |
| [dispose()](#dispose)                                       | Dispose the unmanaged resource.                      |
| [getUp()](#getup)                                           | Get the value of `.up`                               |
| [setUp()](#setup)                                           | Set the value of `.up`                               |
| [getPosition()](#getposition)                               | Get the value of `.position`                         |
| [setPosition()](#setposition)                               | Set the value of `.position`                         |
| [getRotation()](#getrotation)                               | Get the value of `.rotation`                         |
| [setRotation()](#setrotation)                               | Set the value of `.rotation`                         |
| [getQuaternion()](#getquaternion)                           | Get the value of `.quaternion`                       |
| [setQuaternion()](#setquaternion)                           | Set the value of `.quaternion`                       |
| [getScale()](#getscale)                                     | Get the value of `.scale`                            |
| [setScale()](#setscale)                                     | Set the value of `.scale`                            |
| [getMatrix()](#getmatrix)                                   | Get the value of `.matrix`                           |
| [getMatrixWorld()](#getmatrixworld)                         | Get the value of `.matrixWorld`                      |
| [updateMatrix()](#updatematrix)                             | Updates the local transform.                         |
| [updateWorldMatrix()](#updateworldmatrix)                   | Updates the global transform.                        |
| [updateWorldMatrix()](#updateworldmatrix)                   | Updates the global transform.                        |
| [applyMatrix4()](#applymatrix4)                             | Applies the matrix transform to the object.          |
| [applyQuaternion()](#applyquaternion)                       | Applies the rotation to the object.                  |
| [setRotationFromAxisAngle()](#setrotationfromaxisangle)     | Set the rotation from axis and angle.                |
| [setRotationFromMatrix()](#setrotationfrommatrix)           | Set the rotation from rotation matrix.               |
| [rotateOnAxis()](#rotateonaxis)                             | Rotate an object along an axis.                      |
| [rotateOnWorldAxis()](#rotateonworldaxis)                   | Rotate an object along a world axis.                 |
| [rotateX()](#rotatex)                                       | Rotates the object around x axis.                    |
| [rotateY()](#rotatey)                                       | Rotates the object around y axis.                    |
| [rotateZ()](#rotatez)                                       | Rotates the object around z axis.                    |
| [translateOnAxis()](#translateonaxis)                       | Translate an object by distance along an axis        |
| [translateX()](#translatex)                                 | Translates object along x axis                       |
| [translateY()](#translatey)                                 | Translates object along y axis                       |
| [translateZ()](#translatez)                                 | Translates object along z axis                       |
| [localToWorld()](#localtoworld)                             | Converts vector from local space to world space.     |
| [worldToLocal()](#worldtolocal)                             | Converts vector from world space to local space.     |
| [getWorldPosition()](#getworldposition)                     | Get position in world space.                         |
| [getWorldQuaternion()](#getworldquaternion)                 | Get rotation in world space.                         |
| [getWorldScale()](#getworldscale)                           | Get scaling factors in world space.                  |
| [getWorldDirection()](#getworlddirection)                   | Get positive z-axis in world space.                  |
| [lookAt()](#lookat)                                         | Rotates the object to face a point in world space.   |
| [add()](#add)                                               | Adds an object as child of this object.              |
| [remove()](#remove)                                         | Removes an object as child of this object.           |
| [removeFromParent()](#removefromparent)                     | Removes this object from its current parent.         |
| [clear()](#clear)                                           | Removes all child objects.                           |
| [getObjectByName()](#getobjectbyname)                       | Get the first child with a matching name.            |
| [traverse()](#traverse)                                     | Executes the callback on this object and all descendants. |


# Constructors

## Object3D()

`Object3D`()

Can be created to group 3D objects together.

# Properties

## name

`.name`: String

Readable and writtable. Default is an empty string.

## parent

`.parent`: Object3D

Object's parent in the scene graph.

Readable and writtable. 

## children

`.children`: Array

Array with object's children. 

Read-only. Use methods like [`add()`](#add), [`remove()`](#remove) to modify this property.

## up

`.up`: Object

This is used by the lookAt method, for example, to determine the orientation of the result.

Read-only. Use method [`setUp()`](#setup) to modify this property.

Default is {x: 0, y: 1, z: 0}.

## position

`.position`: Object

A Vector3 representing the object's local position

Read-only. Use method [`setPosition()`](#setposition) to modify this property.

Default is {x: 0, y: 0, z: 0}.

## rotation

`.rotation`: Object

Object's local rotation, Euler angles in radians.

Read-only. Use method [`setRotation()`](#setrotation) to modify this property.

## quaternion

`.quaternion`: Object

Object's local rotation as a Quaternion.

Read-only. Use method [`setQuaternion()`](#setquaternion) to modify this property.

## scale

`.scale`: Object

The object's local scale. 

Default is {x: 1, y: 1, z: 1}.

Read-only. Use method [`setScale()`](#setscale) to modify this property.

## matrix

`.matrix`: Object

The local transform matrix.

Read-only.

## matrixWorld

`.matrixWorld`: Object

The global transform of the object. If the Object3D has no parent, then it's identical to the local transform .matrix.

Read-only.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## getUp()

`.getUp`(`vector`: Vector3): Vector3

Copy the value of [`.up`](#up) into `vector`.

## setUp()

`.setUp`(`vector`: Vector3): undefined

Set the value of [`.up`](#up) according to `vector`.

`.setUp`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of [`.up`](#up) according to the x, y, z coordinates.

## getPosition()

`.getPosition`(`vector`: Vector3): Vector3

Copy the value of `.position` into `vector`.

## setPosition()

`.setPosition`(`vector`: Vector3): undefined

Set the value of [`.position`](#position) according to `vector`.

`.setPosition`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of [`.position`](#position) according to the x, y, z coordinates.

## getRotation()

`.getRotation`(`vector`: Vector3): Vector3

Copy the value of [`.rotation`](#rotation) into `vector`.

## setRotation()

`.setRotation`(`vector`: Vector3): undefined

Set the value of [`.rotation`](#rotation) according to `vector`.

`.setRotation`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of [`.rotation`](#rotation) according to the x, y, z coordinates.

## getQuaternion()

`.getQuaternion`(`quaternion`: Quaternion): Quaternion

Copy the value of [`.quaternion`](#quaternion) into `quaternion`.

## setQuaternion()

`.setQuaternion`(`quaternion`: Quaternion): undefined

Set the value of[`.quaternion`](#quaternion) according to `quaternion`.

`.setQuaternion`(`x`: Number, `y`: Number, `z`: Number, `w`: Number ): undefined

Set the value of [`.quaternion`](#quaternion) according to the x, y, z, w coordinates.

## getScale()

`.getScale`(`vector`: Vector3): Vector3

Copy the value of [`.scale`](#scale) into `vector`.

## setScale()

`.setScale`(`vector`: Vector3): undefined

Set the value of [`.scale`](#scale) according to `vector`.

`.setScale`(`x`: Number, `y`: Number, `z`: Number ): undefined

Set the value of [`.scale`](#scale) according to the x, y, z coordinates.

## getMatrix()

`.getMatrix`(`matrix`: Matrix4): Matrix4

Copy the value of [`.matrix`](#matrix) into `matrix`.

## getMatrixWorld()

`.getMatrixWorld`(`matrix`: Matrix4): Matrix4

Copy the value of [`.matrixWorld`](#matrixworld) into `matrix`.

## updateMatrix()

`.updateMatrix`(): undefined

Updates the local transform.

## updateMatrixWorld()

`.updateMatrixWorld`(`force` : Boolean): undefined

Updates the global transform of the object and its descendants.

## updateWorldMatrix()

`.updateWorldMatrix`(`updateParents`: Boolean, `updateChildren`: Boolean): undefined

Updates the global transform of the object and its descendants.

### Parameters

`updateParents`: recursively updates global transform of ancestors.

`updateChildren`: recursively updates global transform of descendants.

## applyMatrix4()

`.applyMatrix4`(`matrix`: Matrix4): this

Applies the matrix transform to the object and updates the object's position, rotation and scale.

## applyQuaternion()

`.applyQuaternion`(`quaternion`: Quaternion): this

Applies the rotation represented by the quaternion to the object.

## setRotationFromAxisAngle()

`.setRotationFromAxisAngle`(`axis`: Vector3, `angle`: Number ): undefined

Set the rotation from axis and angle.

### Parameters

`axis`: A normalized vector in object space.

`angle`: angle in radians

## setRotationFromMatrix()

`.setRotationFromMatrix`(`m`: Matrix4): undefined

Set the rotation from rotation matrix.

### Parameters

`m`: rotate the quaternion by the rotation component of the matrix.

Note that this assumes that the upper 3x3 of m is a pure rotation matrix (i.e, unscaled).

## rotateOnAxis()

`.rotateOnAxis`(`axis`: Vector3, `angle`: Number ) : this

Rotate an object along an axis in object space. The axis is assumed to be normalized.

### Parameters

`axis`: A normalized vector in object space.

`angle`: The angle in radians.

## rotateOnWorldAxis()

`.rotateOnWorldAxis`(`axis`: Vector3, `angle`: Number ) : this

Rotate an object along an axis in world space. The axis is assumed to be normalized. Method Assumes no rotated parent.

### Parameters

`axis`: A normalized vector in object space.

`angle`: The angle in radians.

## rotateX()

`.rotateX`(`rad`: Number ) : this

Rotates the object around x axis in local space.

### Parameters

`rad`: the angle to rotate in radians.

## rotateY()

`.rotateY`(`rad`: Number ) : this

Rotates the object around y axis in local space.

### Parameters

`rad`: the angle to rotate in radians.

## rotateZ()

`.rotateZ`(`rad`: Number ) : this

Rotates the object around z axis in local space.

### Parameters

`rad`: the angle to rotate in radians.

## translateOnAxis()

`.translateOnAxis` `axis`: Vector3, `distance`: Number ) : this

Translate an object by distance along an axis in object space. The axis is assumed to be normalized.

### Parameters

`axis`: A normalized vector in object space.

`distance`: The distance to translate.

## translateX()

`.translateX`(`distance`: Number ) : this

Translates object along x axis in object space by distance units.

## translateY()

`.translateY`(`distance`: Number ) : this

Translates object along y axis in object space by distance units.

## translateZ()

`.translateZ`(`distance`: Number ) : this

Translates object along z axis in object space by distance units.

## localToWorld()

`.localToWorld`(`vector`: Vector3 ) : Vector3

Converts the vector from this object's local space to world space.

### Parameters

`vector`: A vector representing a position in this object's local space.

## worldToLocal()

`.worldToLocal`(`vector`: Vector3 ) : Vector3

Converts the vector from world space to this object's local space.

### Parameters

`vector`: A vector representing a position in world space.

## getWorldPosition

`.getWorldPosition`(`target`: Vector3 ) : Vector3

Returns a vector representing the position of the object in world space.

### Parameters

`target`: the result will be copied into this Vector3.

## getWorldQuaternion()

`.getWorldQuaternion`(`target`: Quaternion ) : Quaternion

Returns a quaternion representing the rotation of the object in world space.

### Parameters

`target`: the result will be copied into this Quaternion.

## getWorldScale()

`.getWorldScale`(`target`: Vector3 ) : Vector3

Returns a vector of the scaling factors applied to the object for each axis in world space.

### Parameters

`target`: the result will be copied into this Vector3.

## getWorldDirection()

 `.getWorldDirection`(`target`: Vector3 ) : Vector3

 Returns a vector representing the direction of object's positive z-axis in world space.

### Parameters

 `target`: the result will be copied into this Vector3.

## lookAt()

Rotates the object to face a point in world space.

This method does not support objects having non-uniformly-scaled parent(s).

`.lookAt`(`vector` : Vector3 ) : undefined

World space position as a Vector3.

`.lookAt`(`x`: Number, `y`: Number, `z`: Number ) : undefined

World space position as `x`, `y` and `z` components.

## add()

`.add`(`object`: Object3D) : this

Adds an object as child of this object. Any current parent on the object passed in here will be removed, since an object can have at most one parent.

## remove()

`.remove`(`object`: Object3D) : this

Removes an object as child of this object.

## removeFromParent()

`.removeFromParent`() : this

Removes this object from its current parent.

## clear()

`.clear`() : this

Removes all child objects.

## getObjectByName()

`.getObjectByName`(`name`: String ) : Object3D

Searches through an object and its children, starting with the object itself, and returns the first with a matching name.
Note that for most objects the name is an empty string by default. You will have to set it manually to make use of this method.
### Parameters

`name`: String to match to the children's Object3D.name property.

## traverse()

`.traverse`(`callback`: Function ) : undefined

Executes the callback on this object and all descendants.
Note: Modifying the scene graph inside the callback is discouraged.

