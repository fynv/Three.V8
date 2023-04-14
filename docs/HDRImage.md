[<--Home](index.html)

# class HDRImage

Class that represents a HDR image that resides in CPU memory. 

Usually not created directly. Use [HDRImageLoader](HDRImageLoader.html) class to create an HDR image.

`class HDRImage`

| Name                              | Description                                                    |
| ----------------------------------| -------------------------------------------------------------- |
| **Constructors**                  |                                                                |
| [HDRImage()](#hdrimage)           | Creates a HDR image.                                           |
| **Properties**                    |                                                                |
| [width](#width)                   | Width of the image.                                            |
| [height](#height)                 | Height of the image.                                           |
| **Methods**                       |                                                                |
| [dispose()](#dispose)             | Dispose the unmanaged resource.                                |

# Constructors

## HDRImage()

`HDRImage`()

Creates an empty HDR image.

`HDRImage`(`width`: Number, `height`: Number)

Create an HDR image of specified size.

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