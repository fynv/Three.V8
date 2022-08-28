[<--Home](index.html)

# class UILineEdit

Editable text box in a UI.

`class UILineEdit extends UIElement`

Inheritance [UIElement](UIElement.html) --> UILineEdit

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UILineEdit()](#uilineedit)                                   | Creates a new UILineEdit.                                      |
| **Properties**                                                |                                                                |
| [size](#size)                                                 | A Vector2 representing the ui-lineedit's size.                 |
| [text](#text)                                                 | Current text content.                                          |
| **Methods**                                                   |                                                                |
| [getSize()](#getsize)                                         | Get the value of `.size`                                       |
| [setSize()](#setsize)                                         | Set the value of `.size`                                       |
| [setStyle()](#setstyle)                                       | Set the displaying style of the ui-text.                       |

# Constructors

## UILineEdit()

`UILineEdit`()

Creates a new UILineEdit. 

# Properties

See the base [UIElement](UIElement.html#properties) class for common properties.

## size

`.size`: Object

A Vector2 representing the ui-lineedit's size.

Read-only. Use method [`setSize()`](#setsize) to modify this property.

Default is {x: 100, y: 40}.

## text

`.text`: String

Current text content.

Readable and writable.

Default is "".

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

## setStyle()

`.setStyle`(`style`: Object): undefined

Set the displaying style of the ui-text.

### `style` may have the following properties:

`style.fontSize`: Number

Font size.

`style.fontFace`: String

Name of TypeTrue font face.

`style.colorBg`: String

Background color.

`style.colorFg`: String

Text color.



