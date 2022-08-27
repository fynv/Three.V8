[<--Home](index.html)

# class Light

Abstract class for all direct lights.

No contructor, never used directly.

`class Light extends Object3D`

Inheritance [Object3D](Object3D.html) --> Light

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Properties**                                                |                                                                |
| [color](#color)                                               | Color of the light object.                                     |
| [intensity](#intensity)                                       | Intensity of the light object.                                 |
| **Methods**                                                   |                                                                |
| [getColor](#getcolor)                                         | Get the value of `.color`                                      |
| [setColor](#setcolor)                                         | Set the value of `.color`                                      |


# Properties

See the base [Object3D](Object3D.html#properties) class for common properties.

## color

`.color`: Object

Color of the light object.

Read-only. Use the method [`.setColor`](#setcolor) to modify this property.

## intensity

Intensity of the light object.

Readable and writable.

# Methods

See the base [Object3D](Object3D.html#methods) class for common methods.

## getColor()

`.getColor`(`color`: Vector3) : Vector3

Copy the value of [`.color`](#color) into `color`.

## setColor()

`.setColor`(`color`: Vector3): undefined

Set the value of [`.color`](#color) according to `color`.

`.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of [`.color`](#color) according to the `r`, `g`, `b` values.


