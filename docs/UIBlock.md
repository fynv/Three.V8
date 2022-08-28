[<--Home](index.html)

# class UIBlock

Base class for all ui-elements containing other ui-elements.

`class UIBlock extends UIElement`

Inheritance [UIElement](UIElement.html) --> UIBlock

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UIBlock()](#uiblock)                                         | Creates a new UIBlock.                                         |
| **Properties**                                                |                                                                |
| [size](#size)                                                 | A Vector2 representing the ui-block's size.                    |
| [scissorEnabled](#scissorenabled)                             | Indicating whether the containing ui-elements are clipped .    |
| **Methods**                                                   |                                                                |
| [getSize()](#getsize)                                         | Get the value of `.size`                                       |
| [setSize()](#setsize)                                         | Set the value of `.size`                                       |

# Constructors

## UIBlock()

`UIBlock`()

Creates a new UIBlock. 

A UIBlock is not rendered by default. It only applies transformations and clipping to the ui-elements it contains.

# Properties

See the base [UIElement](UIElement.html#properties) class for common properties.

## size

`.size`: Object

A Vector2 representing the ui-block's size.

Read-only. Use method [`setSize()`](#setsize) to modify this property.

Default is {x: 100, y: 40}.

## scissorEnabled

`.scissorEnabled`: Boolean

Indicating whether the containing ui-elements are clipped by this ui-block.

Readable and writable.

Dafault is true.

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

