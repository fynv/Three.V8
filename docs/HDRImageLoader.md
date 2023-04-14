[<--Home](index.html)

# class HDRImageLoader

Provides a few interfaces to load HDR images from local files or from memory.

No constructor, exposed as a global object [`HDRImageLoader`](index.html#global-objects).

`class HDRImageLoader`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [loadFile()](#loadfile)                                       | Load a HDR image from local file.                             |
| [loadMemory()](#loadmemory)                                   | Load a HDR image from a memory buffer.                        |
| [loadCubeFromFile()](#loadcubefromfile)                       | Load 6 HDR images from local files to form a HDR cubemap image.    |
| [loadCubeFromMemory()](#loadcubefrommemory)                   | Load 6 HDR images from memory buffers to form a HDR cubemap image. |
| [fromImages()](#fromimages)                                   | Load a HDR image from a series of LDR images.                   |
| [fromRGBM()](#fromrgbm)                                       | Load a HDR image from a RGBM image(LDR).                        |


# Methods

## loadFile()

`.loadFile`(`name`: String): [HDRImage](HDRImage.html)

Load a HDR (RGBE) image from local file.

## loadMemory()

`.loadMemory`(`buf`: ArrayBuffer): [HDRImage](HDRImage.html)

Load a HDR (RGBE) image from a memory buffer.

## loadCubeFromFile()

`.loadCubeFromFile`(`name0`: String, `name1`: String, `name2`: String, `name3`: String, `name4`: String, `name5`: String) : [HDRCubeImage](HDRCubeImage.html)

Load 6 HDR (RGBE) images from local files to form a HDR cubemap image.

## loadCubeFromMemory()

`.loadCubeFromMemory`(`buf0`: ArrayBuffer, `buf1`: ArrayBuffer, `buf2`: ArrayBuffer, `buf3`: ArrayBuffer, `buf4`: ArrayBuffer, `buf5`: ArrayBuffer) : [HDRCubeImage](HDRCubeImage.html)

Load 6 HDR (RGBE) images from memory buffers to form a HDR cubemap image.

## fromImages()

`.fromImages`(`lst_images`: Array, `lst_ranges`: Array) : [HDRImage](HDRImage.html)

Load a HDR image from a series of LDR images. 

This is a decoding method for a HDR image compressed using [cascaded residual encoding](https://fynv.github.io/Cascaded-Residual-Encoding-for-HDR-Lightmap-Compression).

### Parameters

`lst_images`: an array of [Image](Image.html) objects.

`lst_ranges`: an array of objects containing a 'low' property and a 'high' property, each of which is a Vector3.

## fromRGBM()

`.fromRGBM`(`img`: [Image](Image.html), `rate`: Number)

Load a HDR image from a RGBM image(LDR).


