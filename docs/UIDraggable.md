[<--Home](index.html)

# class UIDraggable

A draggable ui-panel.

`class UIDraggable extends UIPanel`

Inheritance [UIElement](UIElement.html) --> [UIBlock](UIBlock.html) --> [UIPanel](UIPanel.html) --> UIDraggable

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [UIDraggable()](#uidraggable)                                 | Creates a new UIDraggable.                                     |
| **Properties**                                                |                                                                |
| [draggableHorizontal](#draggablehorizontal)                   | If the panel is draggable in horizontal direction.             |
| [draggableVertical](#draggablevertical)                       | If the panel is draggable in vertical direction.               |
| [originMin](#originmin)                                       | Minimum position for origin.                                   |
| [originMax](#originmax)                                       | Maximum position for origin.                                   |
| [value](#value)                                               | Value corresponding to current position.                       |
| [onDrag](#ondrag)                                             | Callback functions called when the element is dragged.         |
| **Methods**                                                   |                                                                |
| [getOriginMin()](#getoriginmin)                               | Get the value of `.originMin`                                  |
| [setOriginMin()](#setoriginmin)                               | Set the value of `.originMin`                                  |
| [getOriginMax()](#getoriginmax)                               | Get the value of `.originMax`                                  |
| [setOriginMax()](#setoriginmax)                               | Set the value of `.originMax`                                  |
| [getValue()](#getvalue)                                       | Get the value of `.value`                                      |
| [setValue()](#setvalue)                                       | Set the value of `.value`                                      |

# Constructors

## UIDraggable()

`UIDraggable`()

Creates a new UIDraggable. 

# Properties

See the base [UIPanel](UIPanel.html#properties) class for common properties.

## draggableHorizontal

`.draggableHorizontal`: Boolean

Indicating if the panel is draggable in horizontal direction

Readable and writable.

Dafault is true.

## draggableVertical

`.draggableVertical`: Boolean

Indicating if the panel is draggable in vertical direction.

Readable and writable.

Dafault is false.

## originMin

`.originMin`: Object

A Vector2 representing the minimum position for origin.

Read-only. Use method [`setOriginMin()`](#setoriginmin) to modify this property.

Default is {x: 0, y: 0}.

## originMax

`.originMax`: Object

A Vector2 representing the maximum position for origin.

Read-only. Use method [`setOriginMax()`](#setoriginmax) to modify this property.

Default is {x: 0, y: 0}.

## value

`.value`: Object

A Vector2 representing the value corresponding to current position.

Read-only. Use method [`setValue()`](#setvalue) to modify this property.

Default is {x: 0, y: 0}.

## onDrag

 `.onDrag`: Function
 `.onDrag`(`x`: Number, `y`: Number): undefined

Callback functions called when the element is dragged.

# Methods

See the base [UIPanel](UIPanel.html#methods) class for common methods.

## getOriginMin()

`.getOriginMin`(`vector`: Vector2): Vector2

Copy the value of [`.originMin`](#originmin) into `vector`.

## setOriginMin()

`.setOriginMin`(`vector`: Vector2): undefined

Set the value of [`.originMin`](#originmin) according to `vector`.

`.setOriginMin`(`x`: Number, `y`: Number ): undefined

Set the value of [`.originMin`](#originmin) according to the x, y coordinates.

## getOriginMax()

`.getOriginMax`(`vector`: Vector2): Vector2

Copy the value of [`.originMax`](#originmax) into `vector`.

## setOriginMax()

`.setOriginMax`(`vector`: Vector2): undefined

Set the value of [`.originMax`](#originmax) according to `vector`.

`.setOriginMax`(`x`: Number, `y`: Number ): undefined

Set the value of [`.originMax`](#originmax) according to the x, y coordinates.

## getValue()

`.getValue`(`vector`: Vector2): Vector2

Copy the value of [`.value`](#value) into `vector`.

## setValue()

`.setValue`(`vector`: Vector2): undefined

Set the value of [`.value`](#value) according to `vector`.

`.setValue`(`x`: Number, `y`: Number ): undefined

Set the value of [`.value`](#value) according to the x, y coordinates.




