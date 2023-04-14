[<--Home](index.html)

# class GLRenderer

The OpenGL renderer displays your beautifully crafted scenes using OpenGL.

`class GLRenderer`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [GLRenderer()](#glrenderer)                                   | Creates a new GLRenderer.                                      |
| **Properties**                                                |                                                                |
| [useSSAO](#usessao)                                           | Whether enable SSAO.                                           |
| **Methods**                                                   |                                                                |
| [dispose](#dispose)                                           | Dispose the unmanaged resource.                                |
| [render](#render)                                             | Renders a scene.                                               |
| [renderCube](#rendercube)                                     | Renders a scene to a cubemap.                                  |
| [renderTexture](#rendertexture)                               | Render a 2d media to screen.                                   |

# Constructors

## GLRenderer()

`GLRenderer`()

Create a GLRenderer.

# Properties

## useSSAO

`.useSSAO`: Boolean

Whether enable SSAO. 

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## render()

`.render`(`scene`: [Scene](Scene.html), `camera`: [Camera](Camera.html)): undefined

Renders `scene` using `camera` to the main view of the game-player.

Should be called from the [`render()`](index.html#render) callback function.

`.render`(`scene`: [Scene](Scene.html), `camera`: [Camera](Camera.html), `ui3dviewer`: [UI3DViewer](UI3DViewer.html)): undefined

Renders `scene` using `camera` to `ui3dviewer`.

Should be called from the [`onRender()`](UI3DViewer.html#onrender) callback function of the `ui3dviewer`.

`.render`(`scene`: [Scene](Scene.html), `camera`: [Camera](Camera.html), `target`: [GLRenderTarget](GLRenderTarget.html)): undefined

Renders `scene` using `camera` to `target`.

## renderCube()

`.renderCube`(`scene`: [Scene](Scene.html), `target`: [CubeRenderTarget](CubeRenderTarget.html), `position`: Object, `near`=0.1: Number, `far`=100.0: Number): undefined

Renders `scene` to a cubemap `target` centered at `position`.

## renderTexture()

`.renderTexure`(`img`: [GLRenderTarget](GLRenderTarget.html)/[MMCamera](MMCamera.html)/[MMLazyVideo](MMLazyVideo.html)/[MMVideo](MMVideo.html)/[AVCPlayer](AVCPlayer.html), `x`:Number, `y`:Number, `width`:Number, `height`:Number, `alpha = 1.0`:Number, `target=undefined`: [UI3DViewer](UI3DViewer.html)/[GLRenderTarget](GLRenderTarget.html)): undefined

Render a 2D media to screen.

When `target` is not specified, the media is rendered to the main view of the game-player by default.

