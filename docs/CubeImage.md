[<--Home](index.html)

# class CubeImage

Class that represents an cubemap image that resides in CPU memory.

An CubeImage contains 6 images.

Usually not created directly. Use [ImageLoader](ImageLoader.html) class to create an cubemap image.

`class CubeImage`

| Name                              | Description                                                    |
| ----------------------------------| -------------------------------------------------------------- |
| **Constructors**                  |                                                                |
| [CubeImage()](#cubeimage)         | Creates a cube image.                                          |
| **Properties**                    |                                                                |
| [width](#width)                   | Width of the image.                                            |
| [height](#height)                 | Height of the image.                                           |
| **Methods**                       |                                                                |
| [dispose()](#dispose)             | Dispose the unmanaged resource.                                |

# Constructors

## CubeImage()

`CubeImage`()

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
