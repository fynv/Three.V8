[<--Home](index.html)

# class HDRCubeImage

Class that represents a HDR cubemap image that resides in CPU memory.

A HDRCubeImage contains 6 images.

Usually not created directly. Use [HDRImageLoader](HDRImageLoader.html) class to create a HDR cubemap image.

`class HDRCubeImage`

| Name                              | Description                                                    |
| ----------------------------------| -------------------------------------------------------------- |
| **Constructors**                  |                                                                |
| [HDRCubeImage()](#hdrcubeimage)   | Creates a HDR cube image.                                      |
| **Properties**                    |                                                                |
| [width](#width)                   | Width of the image.                                            |
| [height](#height)                 | Height of the image.                                           |
| **Methods**                       |                                                                |
| [dispose()](#dispose)             | Dispose the unmanaged resource.                                |

# Constructors

## HDRCubeImage()

`HDRCubeImage`()

Note that this constructor is not intended to be called directly.

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
