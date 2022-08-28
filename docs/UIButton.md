[<--Home](index.html)

# class UIButton

A clickable ui-block

`class UIButton extends UIBlock`

Inheritance [UIElement](UIElement.html) --> [UIBlock](UIBlock.html) --> UIButton

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UIButton()](#uibutton)                                       | Creates a new UIButton.                                        |
| **Properties**                                                |                                                                |
| [onClick](#onclick)                                           | Callback function triggered when the button is clicked.        |
| [onLongPress](#onlongpress)                                   | Callback function triggered when the button is long-pressed.   |
| **Methods**                                                   |                                                                |
| [setStyle()](#setstyle)                                       | Set the displaying style of the ui-button.                     |

# Constructors

## UIButton()

`UIButton`()

Creates a new UIButton. 

# Properties

See the base [UIBlock](UIBlock.html#properties) class for common properties.

## onClick

`.onClick`: Function

Callback function triggered when the button is clicked.

Signature: `onClick`():undefined

Readable and writable.

Default is null.

## onLongPress

`.onLongPress`: Function

Callback function triggered when the button is long-pressed.

Signature: `onLongPress`():undefined

Readable and writable.

Default is null.

# Methods

See the base [UIBlock](UIBlock.html#methods) class for common methods.

## setStyle()

`.setStyle`(`style`: Object): undefined

Set the displaying style of the ui-button.

### `style` may have the following properties:

`style.cornerRadius`: Number

Corner radius of the rounded rectangle.

`style.strokeWidth`: Number

Line-width of the stroke.

`style.colorBg`: String

Background color of the panel.

`style.colorStroke`: String

Stroke color of the panel.



