[<--Home](index.html)

# class GamePlayer

Provides a few interfaces to access the host GamePlayer object.

No constructor, exposed as a global object [`gamePlayer`](index.html#global-objects).

`class GamePlayer`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Properties**                                                |                                                                |
| [width](#width)                                               | Current video width                                            |
| [height](#height)                                             | Current video height                                           |
| [picking](#picking)                                           | Whether picking is enabled                                     |
| **Methods**                                                   |                                                                |
| [message()](#message)                                         | Send a general message to the game player.                     |
| [hasFont()](#hasfont)                                         | Check if the font of `name` has been loaded.                   |
| [createFontFromFile()](#createfontfromfile)                   | Load font from local file.                                     |
| [createFontFromMemory()](#createfontfrommemory)               | Load font from a memory buffer.                                |
| [pickObject()](#pickobject)                                   | Pick the object visible at screen location x,y.                |

# Properties

## width

`.width`: Number

Read-only value of current video width.

## height

`.height`: Number

Read-only value of current video height.

## picking

`.picking`: Boolean

Whether picking is enabled.

# Methods

## message()

`.message`(`name`: String, `msg`: String) : String

Send a general message to the game player.

Pointer capture/release messages are sent through this interface, using names "setPointerCapture" and "releasePointerCapture".

## hasFont()

`.hasFont`(`name`: String) : Boolean

Check if the font of `name` has been loaded.

## createFontFromFile()

`.createFontFromFile`(`name`: String, `filename`: String): undefined

Load font from local file.

### Parameters

`name`: name of the font being loaded.

`filename`: file name of the TrueType font


## createFontFromMemory()

`.createFontFromMemory`(`name`: String, `data`: ArrayBuffer): undefined

Load font from a memory buffer.

### Parameters

`name`: name of the font being loaded.

`data`: memory buffer containing the binary data of the TrueType font.


## pickObject()

`.pickObject`(`x`:Number, `y`:Number): Object

Pick the object visible at screen location x,y.

The returned object contains a "name" property.



