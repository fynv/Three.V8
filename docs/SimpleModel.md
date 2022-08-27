[<--Home](index.html)

# class SimpleModel

A Model containing a single simple geometry.

`class SimpleModel extends Object3D`

Inheritance [Object3D](Object3D.html) --> SimpleModel

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [SimpleModel()](#simplemodel)                                 | Creates a new SimpleModel.                                     |
| **Properties**                                                |                                                                |
| [color](#color)                                               | Base-color of the material of the model.                       |
| [metalness](#metalness)                                       | Metalness factor of the material of the model.                 |
| [roughness](#roughness)                                       | Roughness factor of the material of the model.                 |
| **Methods**                                                   |                                                                |
| [createBox](#createbox)                                       | Create a Box shaped geometry for the model.                    |
| [createSphere](#createsphere)                                 | Create a Sphere shaped geometry for the model.                 |
| [createPlane](#createplane)                                   | Create a Plane shaped geometry for the model.                  |
| [getColor](#getcolor)                                         | Get the value of `.color`.                                     |
| [setColor](#setcolor)                                         | Set the value of `.color`.                                     |
| [setColorTexture](#setcolortexture)                           | Set a texture image as the based color map of the model.       |


# Constructors

## SimpleModel()

Creates a new SimpleModel.

# Properties

See the base [Object3D](Object3D.html#properties) class for common properties.

## color

`.color`: Object

Base-color of the material of the model.

Read-only. Use the method [`.setColor`](setcolor) to modify this property.

## metalness

`.metalness`: Number

Metalness factor of the material of the model.

Readable and writable.

## roughness

`.roughness`: Number

Roughness factor of the material of the model.

Readable and writable.

# Methods

See the base [Object3D](Object3D.html#methods) class for common methods.

## createBox()

`.createBox`(`width`: Number, `height`: Number, `depth`: Number): undefined

Create a Box shaped geometry for the model.

### Parameters

`width`: width of the Box

`height`: height of the Box

`depth`: depth of the Box

## createSphere()

`.createSphere`(`radius`: Number, `widthSegments`: Number, `heightSegments`: Number): undefined

Create a Sphere shaped geometry for the model.

### Parameters

`radius`: radius of the Sphere

`widthSegments`: number of width segments of the triangulated Sphere.

`heightSegments`: number of height segments of the triangulated Sphere.

## createPlane()

`.createPlane`(`width`: Number, `height`: Number): undefined

Create a Plane shaped geometry for the model.

### Parameters

`width`: width of the Plane

`height`: height of the Plane

## getColor()

`.getColor`(`color`: Vector3) : Vector3

Copy the value of [`.color`](#color) into `color`.

## setColor()

`.setColor`(`color`: Vector3): undefined

Set the value of [`.color`](#color) according to `color`.

`.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

set the value of [`.color`](#color) according to the `r`, `g`, `b` values.

## setColorTexture()

`.setColorTexture`(`image`: [Image](Image.html)): undefined

Set a texture image as the based color map of the model.

