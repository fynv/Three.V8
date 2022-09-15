[<--Home](index.html)

# class GLRenderTarget

An off-screen render target.

Can be used as an extra parameter to [GLRenderer.render()](GLRenderer.html#render).

`class GLRenderTarget`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [GLRenderTarget()](#glrendertarget)                           | Creates a new GLRenderTarget.                                  |
| **Methods**                                                   |                                                                |
| [dispose](#dispose)                                           | Dispose the unmanaged resource.                                |
| [getImage](#getimage)                                         | Get the rendering result as an [Image](Image.html)             |

# Constructors

## GLRenderTarget()

`GLRenderTarget`(`width`: Number, `height`: Number, `msaa`= true: Boolean)

Create a new GLRenderTarget.

### Parameters

`width`: width of the image data

`height`: height of the image data

`msaa`: should MSAA be used

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## getImage()

`.getImage`(): [Image](Image.html)

Returns the rendering result as an [Image](Image.html) object.


