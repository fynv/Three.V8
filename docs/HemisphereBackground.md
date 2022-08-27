[<--Home](index.html)

# class HemisphereBackground

A background that has a sky color and a ground color.

`class HemisphereBackground extends Background`

Inheritance [Background](Background.html) --> HemisphereBackground

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [HemisphereBackground()](#hemispherebackground)               | Creates a new HemisphereBackground.                            |
| **Properties**                                                |                                                                |
| [setSkyColor](#setskycolor)                                   | Sky color of the background.                                   |
| [groundColor](#groundcolor)                                   | Ground color of the background.                                |
| **Methods**                                                   |                                                                |
| [getSkyColor](#getskycolor)                                   | Get the value of `.skyColor`                                   |
| [setSkyColor](#setskycolor)                                   | Set the value of `.skyColor`                                   |


# Constructors

## HemisphereBackground()

`HemisphereBackground`()

Creates a new HemisphereBackground.

# Properties

## skyColor

`.skyColor`: Object

The sky color of the background.

Read-only. Use the method [`.setSkyColor`](#setskycolor) to modify this property.

## groundColor

`.groundColor`: Object

The ground color of the background.

Read-only. Use the method [`.setGroundColor`](#setgroundcolor) to modify this property.

# Methods

## getSkyColor()

`.getSkyColor`(`color`: Vector3) : Vector3

Copy the value of [`.skyColor`](#skycolor) into `color`.

## setSkyColor()

`.setSkyColor`(`color`: Vector3): undefined

Set the value of [`.skyColor`](#skycolor) according to `color`.

`.setSkyColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

Set the value of [`.skyColor`](#skycolor) according to the `r`, `g`, `b` values.



