[<--Home](index.html)

# class ImageSaver

Provides a few interfaces to save images to disk files (PC only).

No constructor, exposed as a global object [`imageSaver`](index.html#global-objects).

`class ImageSaver`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [saveFile()](#savefile)                                       | Save an image to disk file.                                    |
| [saveCubeToFile()](#savecubetofile)                           | Save a cubemap image to disk as 6 image files.                 |

# Methods

## saveFile()

`.saveFile`(`image`: [Image](Image.html), `name`: String): undefined

Save an image to disk file. 

The file is saved in PNG format.

## saveCubeToFile()

`.saveCubeToFile`(`cubeImage`: [CubeImage](CubeImage.html), `name0`: String, `name1`: String, `name2`: String, `name3`: String, `name4`: String, `name5`: String) : undefined

Save a cubemap image to disk as 6 image files.

The file are saved in PNG format.


