[<--Home](index.html)

# class UIManager

Manages [UIArea](UIArea.html) objects.

Handles UI related Mouse/Touch/Keyboard inputs.

No constructor, exposed as a global object [`UIManager`](index.html#global-objects).

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Properties**                                                |                                                                |
| [areas](#areas)                                               | Array of UIArea objects managed by the ui-manager.             |
| **Methods**                                                   |                                                                |
| [add()](#add)                                                 | Adds an ui-area to the ui-manager.                             |
| [remove()](#remove)                                           | Removes an ui-area from the ui-manager.                        |
| [clear()](#clear)                                             | Removes all ui-areas from the ui-manager.                      |


# Properties

## areas

`.areas`: Array

Array of [UIArea](UIArea.html) objects managed by the ui-manager.

Read-only.

# Methods

## add()

`.add`(`area`: [UIArea](UIArea.html)): this

Adds an ui-area to the ui-manager.

## remove()

`.remove`(`area`: [UIArea](UIArea.html)): this

Removes an ui-area from the ui-manager.

## clear()

`.clear`(): this

Removes all ui-areas from the ui-manager.
