[<--Home](index.html)

# class Fog

Class that represents the particle substance in the air.

Setting a fog object to a scene would trigger a ray-marching calculation when some kind of light source is present.

`class Fog`

| Name                              | Description                                                    |
| ----------------------------------| -------------------------------------------------------------- |
| **Constructors**                  |                                                                |
| [Fog()](#fog)                     | Creates a Fog object.                                          |
| **Properties**                    |                                                                |
| [color](#color)                   | The color of the Fog.                                          |
| [density](#density)               | The density of the Fog.                                        |
| [maxNumSteps](#maxNumSteps)       | Maximum number of steps for each ray-marching.                 |
| [minStep](#minStep)               | Minimal distance to march for each ray-marching step.          |
| **Methods**                       |                                                                |
| [dispose()](#dispose)             | Dispose the unmanaged resource.                                |
| [getColor](#getcolor)             | Get the value of `.color`                                      |
| [setColor](#setcolor)             | Set the value of `.color`                                      |

# Constructors

## Fog()

`Fog`()

Creates a Fog object.

# Properties

## color

`.color`: Object

The color of the fog.

Read-only. Use the method [`.setColor`](#setcolor) to modify this property.

## density

`.density`: Number

Density of the fog, which is the opacity of thickness 1 of the fog.

Valid range: 0.0 ~ 1.0

Defalut value: 0.1

Readable and writable.

## maxNumSteps

`.maxNumSteps`: Number

Maximum number of steps for each ray-marching.

Valid range: > 0

Default value: 50

Readable and writable.

## minStep

`.minStep`: Number

Minimal distance to march for each ray-marching step.

Valid range: > 0.0

Default value: 0.15

Readable and writable.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## getColor()

`.getColor`(`color`: Vector3) : Vector3

Copy the value of [`.color`](#color) into `color`.

## setColor()

 `.setColor`(`color`: Vector3): undefined

 Set the value of [`.color`](#color) according to `color`.

 `.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

 Set the value of [`.color`](#color) according to the `r`, `g`, `b` values.

