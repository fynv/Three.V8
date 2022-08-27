[<--Home](index.html)

# class Image

Class that represents an image that resides in CPU memory. 

Usually not created directly. Use [ImageLoader](ImageLoader.html) class to create an image.

`class Image`

| Name                              | Description                                                    |
| ----------------------------------| -------------------------------------------------------------- |
| **Constructors**                  |                                                                |
| [Image()](#image)                 | Creates an image.                                              |
| **Properties**                    |                                                                |
| [width](#width)                   | Width of the image.                                            |
| [height](#height)                 | Height of the image.                                           |
| **Methods**                       |                                                                |
| [dispose()](#dispose)             | Dispose the unmanaged resource.                                |

# Constructors

## Image()

`Image`()

Creates an empty image.

`Image`(`width`: Number, `height`: Number)

Create an image of specified size.

### Parameters

`width`: width of image

`height`: height of image

# Properties

## width

 `.width`: Number

Width of the image.

Read-only.

## height

 `.height`: Number

Height of the image.

Read-only.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.