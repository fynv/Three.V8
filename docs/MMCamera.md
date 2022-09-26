[<--Home](index.html)

# class MMCamera

Class that represents an image source from a web-camera.

`class MMCamera`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [MMCamera()](#mmcamera)                 | Creates a web-camera source.                                   |
| **Properties**                          |                                                                |
| [width](#width)                         | Width of the image source.                                     |
| [height](#height)                       | Height of the image source.                                    |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |
| [updateTexture()](#updatetexture)       | Attempt to read new frames and update the texture data.        |


# Constructors

## MMCamera()

`MMCamera`(`idx`: Number)

Creates a web-camera source.

### Parameters

`idx` : index of the camera device.

# Properties

## width

 `.width`: Number

Width of the image source.

Read-only.

## height

 `.height`: Number

Height of the image source.

Read-only.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## updateTexture()

`.updateTexture`(): undefined

Attempt to read new frames and update the texture data.

