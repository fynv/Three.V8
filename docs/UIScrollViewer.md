[<--Home](index.html)

# class UIScrollViewer

A scrollable ui-block.

`class UIScrollViewer extends UIBlock`

Inheritance [UIElement](UIElement.html) --> [UIBlock](UIBlock.html) --> UIScrollViewer

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UIScrollViewer()](#uiscrollviewer)                           | Creates a new UIScrollViewer.                                  |
| **Properties**                                                |                                                                |
| [scrollableVertical](#scrollablevertical)                     | Indicates whether the ui-scrollviewer is vertical-scrollable.  |
| [scrollableHorizontal](#scrollablehorizontal)                 | Indicates whether the ui-scrollviewer is horizontal-scrollable.|
| [scrollPosition](#scrollposition)                             | Current scroll position.                                       |
| [contentSize](#contentsize)                                   | Content size.                                                  |
| **Methods**                                                   |                                                                |
| [setStyle()](#setstyle)                                       | Set the displaying style of the ui-scrollviewer.               |
| [getScrollPosition()](#getscrollposition)                     | Get the value of `.scrollPosition`                             |
| [setScrollPosition()](#setscrollposition)                     | Set the value of `.scrollPosition`                             |
| [getContentSize()](#getcontentsize)                           | Get the value of `.contentSize`                                |
| [setContentSize()](#setcontentsize)                           | Set the value of `.contentSize`                                |


# Constructors

## UIScrollViewer()

`UIScrollViewer`()

Creates a new UIScrollViewer. 

# Properties

See the base [UIBlock](UIBlock.html#properties) class for common properties.

## scrollableVertical

`.scrollableVertical`: Boolean 

Indicates whether the ui-scrollviewer is vertical-scrollable.

Readble and writable.

Default is true.

## scrollableHorizontal

`.scrollableHorizontal`: Boolean

Indicates whether the ui-scrollviewer is horizontal-scrollable.

Readble and writable.

Default is false.

## scrollPosition

`.scrollPosition`: Object

A Vector2 representing the ui-scrollviewer's current scroll position.

Read-only. Use method [`setScrollPosition()`](#setscrollposition) to modify this property.

Default is {x: 0, y: 0}.

## contentSize

`.contentSize`: Object

A Vector2 representing the ui-scrollviewer's content size.

Read-only. Use method [`setContentSize()`](#setcontentsize) to modify this property.

Default is {x: 100, y: 100}.

# Methods

See the base [UIBlock](UIBlock.html#methods) class for common methods.

## setStyle()

`.setStyle`(`style`: Object): undefined

Set the displaying style of the ui-panel.

### `style` may have the following properties:

`style.cornerRadius`: Number

Corner radius of the rounded rectangle.

`style.strokeWidth`: Number

Line-width of the stroke.

`style.colorBg`: String

Background color of the panel.

`style.colorStroke`: String

Stroke color of the panel.

## getScrollPosition()

`.getScrollPosition`(`vector`: Vector2): Vector2

Copy the value of [`.scrollPosition`](#scrollposition) into `vector`.

## setScrollPosition()

`.setScrollPosition`(`vector`: Vector2): undefined

Set the value of [`.scrollPosition`](#scrollposition) according to `vector`.

`.setScrollPosition`(`x`: Number, `y`: Number): undefined

Set the value of [`.scrollPosition`](#scrollposition) according to the x, y coordinates.

## getContentSize()

`.getContentSize`(`vector`: Vector2): Vector2

Copy the value of [`.contentSize`](#contentsize) into `vector`.

## setContentSize()

`.setContentSize`(`vector`: Vector2): undefined

Set the value of [`.contentSize`](#contentsize) according to `vector`.

`.setContentSize`(`x`: Number, `y`: Number): undefined

Set the value of [`.contentSize`](#contentsize) according to the x, y coordinates.