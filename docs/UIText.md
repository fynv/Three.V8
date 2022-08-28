[<--Home](index.html)

# class UIText

Non-editable text element in a UI.

`class UIText extends UIElement`

Inheritance [UIElement](UIElement.html) --> UIText

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UIText()](#uitext)                                           | Creates a new UIText.                                          |
| **Properties**                                                |                                                                |
| [text](#text)                                                 | Text content.                                                  |
| **Methods**                                                   |                                                                |
| [setStyle()](#setstyle)                                       | Set the displaying style of the ui-text.                       |

# Constructors

## UIText()

`UIText`()

Creates a new UIText. 

# Properties

See the base [UIElement](UIElement.html#properties) class for common properties.

## text

`.text`: String

Text content.

Readable and writable.

Default is "".

# Methods

## setStyle()

`.setStyle`(`style`: Object): undefined

Set the displaying style of the ui-text.

### `style` may have the following properties:

`style.fontSize`: Number

Font size.

`style.fontFace`: String

Name of TypeTrue font face.

`style.alignmentHorizontal`:  Number

Horizontal alignment mode. 

1: left

2: center

3: right

`style.alignmentVertical`: Number

Vertial alignment mode.

1: top

2: center

3: bottom

4: baseline

`style.colorFg`: String

Text color.
