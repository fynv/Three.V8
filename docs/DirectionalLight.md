[<--Home](index.html)

# class DirectionalLight

A light that gets emitted in a specific direction. This light will behave as though it is infinitely far away and the rays produced from it are all parallel. The common use case for this is to simulate daylight; the sun is far enough away that its position can be considered to be infinite, and all light rays coming from it are parallel. 

This light can cast shadows.

`class DirectionalLight extends Light`

Inheritance [Light](Light.html) --> DirectionalLight

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [DirectionalLight()](#directionallight)                       | Creates a new DirectionalLight.                                |
| **Properties**                                                |                                                                |
| [target](#target)                                             | Target object                                                  |
| [bias](#bias)                                                 | bias for double-sided materials                                |
| [forceCull](#forcecull)                                       | front-face cull even for double-sided materials                |
| **Methods**                                                   |                                                                |
| [setShadow](#setshadow)                                       | Set the basic shadow-map options.                              |
| [setShadowProjection](#setshadowprojection)                   | Set the orthographic frustum parameters.                       |
| [setShadowRadius](#setshadowradius)                           | Set the soft-shadow radius.                                    |
| [getBoundingBox](#getboundingbox)                             | Get the bounding box of a scene in the shadow coordinate.      |


# Constructors

## DirectionalLight()

Creates a new DirectionalLight.

# Properties

See the base [Light](Light.html#properties) class for common properties.

## target

`.target`: [Object3D](Object3D.html)

The DirectionalLight points from its position to target.position. The default position of the target is (0, 0, 0).

## bias

`.bias`: Number

Bias for double-sided materials.

Readable & Writable. Default value is 0.001.

## forceCull

`.forceCull`: Boolean

Whether apply front-face culling even for double-sided materials.

# Methods

See the base [Light](Light.html#methods) class for common methods.

## setShadow

`.setShadow`(`enable` : Boolean, `width`: Number, `height`: Number): undefined

Set basic shadow-map options.

### Parameters

`enable`: If set to true light will cast dynamic shadows.

`width`: width of the shadow map.

`height`: height of the shadow map.

## setShadowProjection

`.setShadowProjection`(`left`: Number, `right`: Number, `bottom`: Number, `top`: Number, `zNear`: Number, `zFar`: Number): undefined

Set the orthographic frustum parameters.

### Parameters

`left`: Frustum left plane.

`right`: Frustum right plane.

`top`: Frustum top plane.

`bottom`: Frustum bottom plane.

`near`: Frustum near plane.

`far`: Frustum far plane.


## setShadowRadius

`.setShadowRadius`(`radius`: Number): undefined

Set the soft-shadow radius at distance 1.0.

`radius`>0 would activate the PCSS rendering path.

## getBoundingBox

`.getBoundingBox`(`scene`: [Scene](Scene.html)): Object

Get the bounding box of a scene in the shadow coordinate.

The returned object contains a 'minPos' property and a 'maxPos' property, each of which is a Vector3.

