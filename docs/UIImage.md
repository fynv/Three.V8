[<--Home](index.html)

# class UIImage

Image element in a UI.

`class UIImage extends UIElement`

Inheritance [UIElement](UIElement.html) --> UIImage

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UIImage()](#uiimage)                                         | Creates a new UIImage.                                         |
| **Properties**                                                |                                                                |
| [size](#size)                                                 | A Vector2 representing the ui-image's size.                    |
| **Methods**                                                   |                                                                |
| [getSize()](#getsize)                                         | Get the value of `.size`                                       |
| [setSize()](#setsize)                                         | Set the value of `.size`                                       |
| [setImage()](#setimage)                                       | Set the content of the ui-image.                               |

# Constructors

## UIImage()

`UIImage`()

Creates a new UIImage. 

# Properties

See the base [UIElement](UIElement.html#properties) class for common properties.

## size

`.size`: Object

A Vector2 representing the ui-image's size.

Read-only. Use method [`setSize()`](#setsize) to modify this property.

Default is {x: 100, y: 100}.

# Methods

See the base [UIElement](UIElement.html#methods) class for common methods.

## getSize()

`.getSize`(`vector`: Vector2): Vector2

Copy the value of [`.size`](#size) into `vector`.

## setSize()

`.setSize`(`vector`: Vector2): undefined

Set the value of [`.size`](#size) according to `vector`.

`.setSize`(`x`: Number, `y`: Number ): undefined

Set the value of [`.size`](#size) according to the x, y coordinates.

## setImage()

`.setImage`(`image`: [Image](Image.html)): undefined

Set the content of the ui-image using a [Image](Image.html) object.

[`.size`](#size) will be set according to the image contetnt.


