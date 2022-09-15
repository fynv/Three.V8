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
| [renderCube](#rendercube)                                     | Renders a scene to a cubemap                                   |
| [renderLayers](#renderlayers)                                 | Renders each layer successively                                |
| [renderLayersToCube](#renderlayerstocube)                     | Renders each layer successively to a cubemap                   |
| [renderCelluloid](#rendercelluloid)                           | Renders a scene to 3 separate render targets. PC Only.         |


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

`.render`(`scene`: [Scene](Scene.html), `camera`: [Camera](Camera.html), `target`: [GLRenderTarget](GLRenderTarget.html)): undefined

Renders `scene` using `camera` to `target`.

## renderCube()

`.renderCube`(`scene`: [Scene](Scene.html), `target`: [CubeRenderTarget](CubeRenderTarget.html), `position`: Object, `near`=0.1: Number, `far`=100.0: Number): undefined

Renders `scene` to a cubemap `target` centered at `position`.

## renderLayers()

`.renderLayers`(`layers`: Array): undefined

Renders each layer successively to the main view of the game-player.

Should be called from the [`render()`](index.html#render) callback function.

Each item of `layers` should have the properties:

`.scene`: [Scene](Scene.html) 

The scene to be rendered

`.camera`: [Camera](Camera.html)

The camera to be used to render the scene.

`.renderLayers`(`layers`: Array,  `ui3dviewer`: [UI3DViewer](UI3DViewer.html)): undefined

Renders each layer successively to `ui3dviewer`.

Should be called from the [`onRender()`](UI3DViewer.html#onrender) callback function of the `ui3dviewer`.

`.renderLayers`(`layers`: Array, `target`: [GLRenderTarget](GLRenderTarget.html)): undefined

Renders each layer successively to `target`.

## renderLayersToCube()

`.renderLayersToCube`(`layers`: Array, `target`:  [CubeRenderTarget](CubeRenderTarget.html)): undefined

Renders each layer successively to a cubemap `target`.

Each item of `layers` should have the properties:

`.scene`: [Scene](Scene.html) 

The scene to be rendered

`.position`: Object

The center of the cubemap in the scene

`.near`: Number

Near z value (positive)

`.far`: Number

Far z value (positive)

## renderCelluloid()

PC Only.

`renderCelluloid`(`scene`: [Scene](Scene.html), `camera`: [Camera](Camera.html), `base`: [GLRenderTarget](GLRenderTarget.html), `lighting`: [GLRenderTarget](GLRenderTarget.html), `alpha`: [GLRenderTarget](GLRenderTarget.html)): undefined

Renders `scene` using `camera` to 3 separate render targets, each containing: the base-color, the lighting, the tranparent layer.


