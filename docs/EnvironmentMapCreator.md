[<--Home](index.html)

# class EnvironmentMapCreator

`class EnvironmentMapCreator`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [EnvironmentMapCreator()](#environmentmapcreator)             | Creates a new EnvironmentMapCreator.                           |
| **Methods**                                                   |                                                                |
| [dispose()](#dispose)                                         | Dispose the unmanaged resource.                                |
| [create()](#create)                                           | Create an EnvironmentMap object.                               |

# Constructors

## EnvironmentMapCreator()

`EnvironmentMapCreator`()

Create an EnvironmentMapCreator object, which can be used to create [EnvironmentMap](EnvironmentMap.html) objects.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## create()

`.create`(`image`: [CubeImage](CubeImage.html)): EnvironmentMap

Create an EnvironmentMap object using a cubemap image.

`.create`(`image`: [HDRCubeImage](HDRCubeImage.html)): EnvironmentMap

Create an EnvironmentMap object using a HDR cubemap image.

 `.create`(`background`: [CubeBackground](CubeBackground.html)): EnvironmentMap

Create an EnvironmentMap object using a cubemap background.

`.create`(`target`: [CubeRenderTarget](CubeRenderTarget.html), `irradiance_only = false`: Boolean): EnvironmentMap

Create an EnvironmentMap object using a cubemap render target.

