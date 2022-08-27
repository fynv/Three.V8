[<--Home](index.html)

# class HemisphereLight

A light source positioned directly above the scene, with color fading from the sky color to the ground color.

This light cannot be used to cast shadows.

`class HemisphereLight extends IndirectLight`

Inheritance [IndirectLight](IndirectLight.html) --> HemisphereLight

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [HemisphereLight()](#hemispherelight)                         | Creates a new HemisphereLight.                                 |
| **Properties**                                                |                                                                |
| [skyColor](#skyColor)                                         | The sky color of the hemisphere light.                         |
| [groundColor](#groundcolor)                                   | The ground color of the hemisphere light.                      |
| [intensity](#intensity)                                       | The intensity of the hemisphere light.                         |
| **Methods**                                                   |                                                                |
| [getSkyColor](#getskycolor)                                   | Get the value of `.skyColor`                                   |
| [setSkyColor](#setskycolor)                                   | Set the value of `.skyColor`                                   |
| [getGroundColor](#getgroundcolor)                             | Get the value of `.groundColor`                                |
| [setGroundColor](#setgroundcolor)                             | Set the value of `.groundColor`                                |


# Constructors

## HemisphereLight()

`HemisphereLight`()

Creates a new HemisphereLight.

# Properties

## skyColor

`.skyColor`: Object

The sky color of the hemisphere light.

Read-only. Use the method [`.setSkyColor`](#setskycolor) to modify this property.

## groundColor

`.groundColor`: Object

The ground color of the hemisphere light.

Read-only. Use the method [`.setGroundColor`](#setgroundcolor) to modify this property.

## intensity

`.intensity`: Number

The intensity of the hemisphere light.

Readable and writable.

# Methods

## getSkyColor()

`.getSkyColor`(`color`: Vector3) : Vector3

Copy the value of [`.skyColor`](#skycolor) into `color`.

## setSkyColor()

`.setSkyColor`(`color`: Vector3): undefined

Set the value of [`.skyColor`](#skycolor) according to `color`.

`.setSkyColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of [`.skyColor`](#skycolor) according to the `r`, `g`, `b` values.

## getGroundColor()

`.getGroundColor`(`color`: Vector3) : Vector3

Copy the value of [`.groundColor`](#groundcolor) into `color`.

## setGroundColor();

`.setGroundColor`(`color`: Vector3): undefined

Set the value of [`.groundColor`](#groundcolor) according to `color`.

`.setGroundColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of [`.groundColor`](#groundcolor) according to the `r`, `g`, `b` values.


