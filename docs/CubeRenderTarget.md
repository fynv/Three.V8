[<--Home](index.html)

# class CubeRenderTarget

A cubemap render target.

Can be used in [GLRenderer.renderCube()](GLRenderer.html#rendercube).

`class CubeRenderTarget`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [CubeRenderTarget()](#cuberendertarget)                       | Creates a new CubeRenderTarget.                                |
| **Methods**                                                   |                                                                |
| [dispose](#dispose)                                           | Dispose the unmanaged resource.                                |
| [getCubeImage](#getcubeimage)                                 | Get the rendering result as an [CubeImage](CubeImage.html)     |

# Constructors

## CubeRenderTarget()

`CubeRenderTarget`(`width`: Number, `height`: Number)

Create a new CubeRenderTarget.

### Parameters

`width`: width of the image data

`height`: height of the image data

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## getCubeImage()

`.getCubeImage`(): [CubeImage](CubeImage.html)

Returns the rendering result as an [CubeImage](CubeImage.html) object.



