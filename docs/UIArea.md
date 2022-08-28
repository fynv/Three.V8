[<--Home](index.html)

# class UIArea

Manages [UIElement](UIElement.html) objects and [UI3DViewer](UI3DViewer.html) objects.

Provides a framebuffer for rendering ui-elements. Also keeps a scaling record.

UIElement objects are rendered first, then UI3DViewer objects are rendered above all ui-elements at a higher-frequency.

`class UIArea`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UIArea()](#uiarea)                                           | Creates an UIArea.                                             |
| **Properties**                                                |                                                                |
| [elements](#elements)                                         | Array of UIElement objects managed by this ui-area.            |
| [viewers](#viewers)                                           | Array of UI3DViewer objects managed by this ui-area.           |
| [origin](#origin)                                             | A Vector2 representing the ui-area's position.                 |
| [size](#size)                                                 | A Vector2 representing the ui-area's size.                     |
| [scale](#scale)                                               | Scale factor of the ui-area.                                   |
| **Methods**                                                   |                                                                |
| [dispose()](#dispose)                                         | Dispose the unmanaged resource.                                |
| [add()](#add)                                                 | Adds an ui-element to this ui-area.                            |
| [remove()](#remove)                                           | Removes an ui-element from this ui-area.                       |
| [clear()](#clear)                                             | Removes all ui-elements from this ui-area.                     |
| [addViewer()](#addviewer)                                     | Adds an ui-3dviewer to this ui-area.                           |
| [removeViewer()](#removeviewer)                               | Removes an ui-3dviewer from this ui-area.                      |
| [clearViewer()](#clearviewer)                                 | Removes all ui-3dviewers from this ui-area.                    |
| [getOrigin()](#getorigin)                                     | Get the value of `.origin`                                     |
| [setOrigin()](#setorigin)                                     | Set the value of `.origin`                                     |
| [getSize()](#getsize)                                         | Get the value of `.size`                                       |
| [setSize()](#setsize)                                         | Set the value of `.size`                                       |

# Constructors

## UIArea()

`UIArea`()

Creates an UIArea.

# Properties

## elements

`.elements`: Array

Array of [UIElement](UIElement.html) objects managed by the ui-area.

Read-only.

## viewers

`.viewers`: Array

Array of [UI3DViewer](UI3DViewer.html) objects managed by the ui-area.

Read-only.

## origin

`.origin`: Object

A Vector2 representing the ui-area's position.

Scaled by [`.scale`](#scale).

Read-only. Use method [`setOrigin()`](#setorigin) to modify this property.

Default is {x: 0, y: 0}.

## size

`.size`: Object

A Vector2 representing the ui-area's size.

Scaled by [`.scale`](#scale).

Read-only. Use method [`setSize()`](#setsize) to modify this property.

Default is {x: 0, y: 0}.

## scale

`.scale`: Number

Scale factor of the ui-area.

Readable and writable. 

Default is 1.0.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## add()

`.add`(`elem`: [UIElement](UIElement.html)): this

Adds an ui-element to this ui-area.

## remove()

`.remove`(`elem`: [UIElement](UIElement.html)): this

Removes an ui-element from the ui-area.

## clear()

`.clear`(): this

Removes all ui-elements from the ui-area.

## addViewer()

`.addViewer`(`viewer`: [UI3DViewer](UI3DViewer.html)): this

Adds an ui-3dviewer to this ui-area.

## removeViewer()

`.removeViewer`(`viewer`: [UI3DViewer](UI3DViewer.html)): this

Removes an ui-3dviewer from the ui-area.

## clearViewer()

`.clearViewer`(): this

Removes all ui-3dviewers from the ui-area.

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

