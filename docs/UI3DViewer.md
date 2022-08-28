[<--Home](index.html)

# class UI3DViewer

Object for embedding a 3D view in a UI.

`class UI3DViewer`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UI3DViewer()](#ui3dviewer)                                   | Creates a new UI3DViewer.                                      |
| **Properties**                                                |                                                                |
| [block](#block)                                               | The block element to which this ui-3dviewer belongs.           |
| [origin](#origin)                                             | A Vector2 representing the ui-3dviewer's position.             |
| [size](#size)                                                 | A Vector2 representing the ui-3dviewer's size.                 |
| [onRender](#onrender)                                         | Callback function triggered by UIManager during UI rendering.  |
| [onMouseDown](#onmousedown)                                   | Callback function passed by UIManager during UI input handling.|
| [onMouseUp](#onmouseup)                                       | Callback function passed by UIManager during UI input handling.|
| [onMouseMove](#onmousemove)                                   | Callback function passed by UIManager during UI input handling.|
| [onMouseWheel](#onmousewheel)                                 | Callback function passed by UIManager during UI input handling.|
| [onTouchDown](#ontouchdown)                                   | Callback function passed by UIManager during UI input handling.|
| [onTouchUp](#ontouchup)                                       | Callback function passed by UIManager during UI input handling.|
| [onTouchMove](#ontouchmove)                                   | Callback function passed by UIManager during UI input handling.|
| **Methods**                                                   |                                                                |
| [dispose()](#dispose)                                         | Dispose the unmanaged resource.                                |
| [getOrigin()](#getorigin)                                     | Get the value of `.origin`                                     |
| [setOrigin()](#setorigin)                                     | Set the value of `.origin`                                     |
| [getSize()](#getsize)                                         | Get the value of `.size`                                       |
| [setSize()](#setsize)                                         | Set the value of `.size`                                       |

# Constructors

## UI3DViewer()

`UI3DViewer`()

Creates a new UI3DViewer. 

# Properties

## block

`.block`: [UIBlock](UIBlock.html)

The block element to which this ui-3dviewer belongs.

When there is a valid block element, this ui-3dviewer is translated and clipped according the its block element.

Readable and writable.

Default is null.

## origin

`.origin`: Object

A Vector2 representing the ui-3dviewer's position.

Read-only. Use method [`setOrigin()`](#setorigin) to modify this property.

Default is {x: 0, y: 0}.

## onRender

`.onRender`: Function

Callback function triggered by [UIManager](UIManager.html) during UI rendering.

Signature: `onRender`(`width`: Number, `height`: Number, `size_changed`: Boolean):undefined

Readable and writable.

Default is null.

## onMouseDown

`.onMouseDown`: Function

Callback function passed by [UIManager](UIManager.html) during UI input handling.

Signature: `onMouseDown`(`e`: Object): undefined

The parameter `e` has the same structure as in the global callback [`OnMouseDown()`](index.html#onmousedown)

Readable and writable.

Default is null.

## onMouseUp

`.onMouseUp`: Function

Callback function passed by [UIManager](UIManager.html) during UI input handling.

Signature: `onMouseUp`(`e`: Object): undefined

The parameter `e` has the same structure as in the global callback [`OnMouseDown()`](index.html#onmousedown)

Readable and writable.

Default is null.

## onMouseMove

`.onMouseMove`: Function

Callback function passed by [UIManager](UIManager.html) during UI input handling.

Signature: `onMouseMove`(`e`: Object): undefined

The parameter `e` has the same structure as in the global callback [`OnMouseDown()`](index.html#onmousedown)

Readable and writable.

Default is null.

## onMouseWheel

`.onMouseWheel`: Function

Callback function passed by [UIManager](UIManager.html) during UI input handling.

Signature: `onMouseWheel`(`e`: Object): undefined

The parameter `e` has the same structure as in the global callback [`OnMouseDown()`](index.html#onmousedown)

Readable and writable.

Default is null.

## onTouchDown

`.onTouchDown`: Function

Callback function passed by [UIManager](UIManager.html) during UI input handling.

Signature: `onTouchDown`(`e`: Object): undefined

The parameter `e` has the same structure as in the global callback [`OnTouchDown()`](index.html#ontouchdown)

Readable and writable.

Default is null.

## onTouchUp

`.onTouchUp`: Function

Callback function passed by [UIManager](UIManager.html) during UI input handling.

Signature: `onTouchUp`(`e`: Object): undefined

The parameter `e` has the same structure as in the global callback [`OnTouchDown()`](index.html#ontouchdown)

Readable and writable.

Default is null.

## onTouchMove

`.onTouchMove`: Function

Callback function passed by [UIManager](UIManager.html) during UI input handling.

Signature: `onTouchMove`(`e`: Object): undefined

The parameter `e` has the same structure as in the global callback [`OnTouchDown()`](index.html#ontouchdown)

Readable and writable.

Default is null.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## getOrigin()

`.getOrigin`(`vector`: Vector2): Vector2

Copy the value of [`.origin`](#origin) into `vector`.

## setOrigin()

`.setOrigin`(`vector`: Vector2): undefined

Set the value of [`.origin`](#origin) according to `vector`.

`.setOrigin`(`x`: Number, `y`: Number ): undefined

Set the value of [`.origin`](#origin) according to the x, y coordinates.

## getSize()

`.getSize`(`vector`: Vector2): Vector2

Copy the value of [`.size`](#size) into `vector`.

## setSize()

`.setSize`(`vector`: Vector2): undefined

Set the value of [`.size`](#size) according to `vector`.

`.setSize`(`x`: Number, `y`: Number ): undefined

Set the value of [`.size`](#size) according to the x, y coordinates.


