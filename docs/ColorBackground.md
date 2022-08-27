[<--Home](index.html)

# class ColorBackground

A background that has a monotone color.

`class ColorBackground extends Background`

Inheritance [Background](Background.html) --> ColorBackground

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [ColorBackground()](#colorbackground)                         | Creates a new ColorBackground.                                 |
| **Properties**                                                |                                                                |
| [color](#color)                                               | Color of the background.                                       |
| **Methods**                                                   |                                                                |
| [getColor](#getcolor)                                         | Get the value of `.color`                                      |
| [setColor](#setcolor)                                         | Set the value of `.color`                                      |


# Constructors

## ColorBackground()

`ColorBackground`()

Creates a new ColorBackground. 

# Properties

## color

`.color`: Object

The color of the background.

Read-only. Use the method [`.setColor`](#setcolor) to modify this property.

# Methods

## getColor()

`.getColor`(`color`: Vector3) : Vector3

Copy the value of [`.color`](#color) into `color`.

## setColor()

 `.setColor`(`color`: Vector3): undefined

 Set the value of [`.color`](#color) according to `color`.

 `.setColor`(`r`: Number, `g`: Number, `b`: Number ): undefined

 Set the value of [`.color`](#color) according to the `r`, `g`, `b` values.

