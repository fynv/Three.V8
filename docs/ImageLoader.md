[<--Home](index.html)

# class ImageLoader

Provides a few interfaces to load images from local files or from memory.

No constructor, exposed as a global object [`imageLoader`](Index.html#global-objects).

`class ImageLoader`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [loadFile()](#loadfile)                                       | Load an image from local file.                                 |
| [loadMemory()](#loadmemory)                                   | Load an image from a memory buffer.                            |
| [loadCubeFromFile()](#loadcubefromfile)                       | Load 6 images from local files to form a cubemap image.        |
| [loadCubeFromMemory()](#loadcubefrommemory)                   | Load 6 images from memory buffers to form a cubemap image.     |


# Methods

## loadFile()

`.loadFile`(`name`: String): [Image](Image.html)

Load an image from local file.

## loadMemory()

`.loadMemory`(`buf`: ArrayBuffer): [Image](Image.html)

Load an image from a memory buffer.

## loadCubeFromFile()

`.loadCubeFromFile`(`name0`: String, `name1`: String, `name2`: String, `name3`: String, `name4`: String, `name5`: String) : [CubeImage](CubeImage.html)

Load 6 images from local files to form a cubemap image.

## loadCubeFromMemory()

`.loadCubeFromMemory`(`buf0`: ArrayBuffer, `buf1`: ArrayBuffer, `buf2`: ArrayBuffer, `buf3`: ArrayBuffer, `buf4`: ArrayBuffer, `buf5`: ArrayBuffer) : CubeImage 

Load 6 images from memory buffers to form a cubemap image.
