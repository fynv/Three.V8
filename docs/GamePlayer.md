[<--Home](index.html)

# class GamePlayer

Provides a few interfaces to access the host GamePlayer object.

No constructor, exposed as a global object [`gamePlayer`](Index.html#global-objects).

`class GamePlayer`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Properties**                                                |                                                                |
| [width](#width)                                               | Current video width                                            |
| [height](#height)                                             | Current video height                                           |
| **Methods**                                                   |                                                                |
| [setMouseCapture()](#setmousecapture)                         | Set the mouse capture state to True.                           |
| [releaseMouseCapture()](#releasemousecapture)                 | Set the mouse capture state to False.                          |
| [hasFont()](#hasfont)                                         | Check if the font of `name` has been loaded.                   |
| [createFontFromFile()](#createfontfromfile)                   | Load font from local file.                                     |
| [createFontFromMemory()](#createfontfrommemory)               | Load font from a memory buffer.                                |

# Properties

## width

`.width`: Number

Read-only value of current video width.

## height

`.height`: Number

Read-only value of current video height.

# Methods

## setMouseCapture()

`.setMouseCapture`(): undefined

Set the mouse capture state to True.

## releaseMouseCapture()

 `.releaseMouseCapture`(): undefined

Set the mouse capture state to False.

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






