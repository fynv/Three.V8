[<--Home](index.html)

# class UIElement

Abstract base class of all ui-elements.

No contructor, never used directly.

`class UIElement`

| Name                                                        | Description                                          |
| ------------------------------------------------------------| ---------------------------------------------------- |
| **Properties**                                              |                                                      |
| [block](#block)                                             | The block element to which this ui-element belongs.  |
| [origin](#origin)                                           | A Vector2 representing the ui-element's position.    |
| [onPointerDown](#onpointerdown)                             | Callback functions called when pointer is down on the element |
| [onPointerUp](#onpointerup)                                 | Callback functions called when pointer is up on the element |
| [onPointerMove](#onpointermove)                             | Callback functions called when pointer is moved on the element |
| **Methods**                                                 |                                                      |
| [dispose()](#dispose)                                       | Dispose the unmanaged resource.                      |
| [getOrigin()](#getorigin)                                   | Get the value of `.origin`                           |
| [setOrigin()](#setorigin)                                   | Set the value of `.origin`                           |

# Properties

## block

`.block`: [UIBlock](UIBlock.html)

The block element to which this ui-element belongs.

When there is a valid block element, this element is translated and clipped according the its block element.

Readable and writable.

Default is null.

## origin

`.origin`: Object

A Vector2 representing the ui-element's position.

Read-only. Use method [`setOrigin()`](#setorigin) to modify this property.

Default is {x: 0, y: 0}.

## onPointerDown

 `.onPointerDown`: Function
 `.onPointerDown`(`x`: Number, `y`: Number): undefined

Callback functions called when pointer is down on the element.

## onPointerUp

 `.onPointerUp`: Function
 `.onPointerUp`(`x`: Number, `y`: Number): undefined

Callback functions called when pointer is up on the element.

## onPointerMove

 `.onPointerMove`: Function
 `.onPointerMove`(`x`: Number, `y`: Number): undefined

Callback functions called when pointer is moved on the element.

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


