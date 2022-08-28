[<--Home](index.html)

# class UITextBlock

Non-editable, mutli-lined text element in a UI.

`class UITextBlock extends UIElement`

Inheritance [UIElement](UIElement.html) --> UITextBlock

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UITextBlock()](#uitextblock)                                 | Creates a new UITextBlock.                                     |
| **Properties**                                                |                                                                |
| [text](#text)                                                 | Text content.                                                  |
| **Methods**                                                   |                                                                |
| [setStyle()](#setstyle)                                       | Set the displaying style of the ui-text.                       |

# Constructors

## UITextBlock()

`UITextBlock`()

Creates a new UITextBlock. 

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

`style.lineWidth`: Number

Number of characters each line.

`style.lineHeight`: Number

Height of each line.

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



