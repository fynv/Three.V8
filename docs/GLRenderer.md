[<--Home](index.html)

# class GLRenderer

The OpenGL renderer displays your beautifully crafted scenes using OpenGL.

`class GLRenderer`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [GLRenderer()](#glrenderer)                                   | Creates a new GLRenderer.                                      |
| **Methods**                                                   |                                                                |
| [dispose](#dispose)                                           | Dispose the unmanaged resource.                                |
| [render](#render)                                             | Renders a scene.                                               |


# Constructors

## GLRenderer()

`GLRenderer`()

Create a GLRenderer.

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




