[<--Home](index.html)

# class DDSImage

Class that represents an image loaded from a DDS file that resides in CPU memory. 

Usually not created directly. Use [DDSImageLoader](DDSImageLoader.html) class to create a dds-image.

Note that the implementation is incomplete. Supported formats include:

```
BGRA,
BC1,
BC2,
BC3,
BC4,
BC5,
BC6H,
BC7,
```
Faces and mipmaps are not implemented.

Currently this class is mainly used as an option for loading lightmaps stored in BC6H.

`class DDSImage`

| Name                    | Description                     |
| ----------------------- | ------------------------------- |
| **Constructors**        |                                 |
| [DDSImage()](#ddsimage) | Creates a dds-image.            |
| **Properties**          |                                 |
| [width](#width)         | Width of the dds-image.         |
| [height](#height)       | Height of the dds-image.        |
| **Methods**             |                                 |
| [dispose()](#dispose)   | Dispose the unmanaged resource. |

# Constructors

## DDSImage()

`DDSImage`()

Creates an empty dds-image.

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
