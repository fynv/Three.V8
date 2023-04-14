[<--Home](index.html)

# class DDSImageLoader

Provides a few interfaces to load dds-images from local files or from memory.

No constructor, exposed as a global object [`DDSImageLoader`](index.html#global-objects).


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

`class DDSImageLoader`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [loadFile()](#loadfile)                                       | Load a dds-image from local file.                              |
| [loadMemory()](#loadmemory)                                   | Load a dds-image from a memory buffer.                         |


# Methods

## loadFile()

`.loadFile`(`name`: String): [DDSImage](DDSImage.html)

Load a dds-image from local file.

## loadMemory()

`.loadMemory`(`buf`: ArrayBuffer): [DDSImage](DDSImage.html)

Load a dds-image from a memory buffer.
