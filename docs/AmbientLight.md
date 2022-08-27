[<--Home](index.html)

# class AmbientLight

This light globally illuminates all objects in the scene equally.

This light cannot be used to cast shadows as it does not have a direction.

`class AmbientLight extends IndirectLight`

Inheritance [IndirectLight](IndirectLight.html) --> AmbientLight

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [AmbientLight()](#ambientlight)                               | Creates a new AmbientLight.                                    |
| **Properties**                                                |                                                                |
| [color](#color)                                               | The color of the ambient light.                                |
| [intensity](#intensity)                                       | The intensity of the ambient light.                            |
| **Methods**                                                   |                                                                |
| [getColor](#getcolor)                                         | Get the value of `.color`                                      |
| [setColor](#setcolor)                                         | Set the value of `.color`                                      |

# Constructors

## AmbientLight()

Creates a new AmbientLight.

# Properties

## color()

`.color`: Object

The color of the ambient light.

Read-only. Use the method [`.setColor`](#setcolor) to modify this property.

## intensity()

`.intensity`: Number

The intensity of the ambient light.

Readable and writable.

# Methods

## getColor()

`.getColor`(`color`: Vector3) : Vector3

Copy the value of `.color` into `color`.

## setColor()

`.setColor`(`color`: Vector3): undefined

Set the value of `.color` according to `color`.

 `.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of `.color` according to the `r`, `g`, `b` values.

