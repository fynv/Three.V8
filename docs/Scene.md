[<--Home](index.html)

# class Scene

Scenes allow you to set up what and where is to be rendered by Three.V8. This is where you place objects, lights and cameras.

`class Scene extends Object3D`

Inheritance [Object3D](Object3D.html) --> Scene

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [Scene()](#scene)                                             | Creates a new Scene.                                           |
| **Properties**                                                |                                                                |
| [background](#background)                                     | Object used as background                                      |
| [indirectLight](#indirectlight)                               | Object used as the indirect light-source.                      |
| [fog](#fog)                                                   | Object representing the particle substance in the air.         |
| **Methods**                                                   |                                                                |
| [getBoundingBox](#getboundingbox)                             | Get the bounding-box of the whole scene                        |

# Constructors

## Scene()

`Scene`()

Create a new scene object.

# Properties

See the base [Object3D](Object3D.html#properties) class for common properties.

## background

`.background`: [Background](Background.html)

Object used as background.

Readable and writable.

## indirectLight

`.indirectLight`: [IndirectLight](IndirectLight.html)

Object used as the indirect light-source.

Readable and writable.

## fog

`.fog`: [Fog](Fog.html)

Object representing the particle substance in the air.

Readable and writable.

# Methods

See the base [Object3D](Object3D.html#methods) class for common methods.

## getBoundingBox()

`.getBoundingBox`() : Object

Get the bounding-box of the whole scene.

The returned object contains a 'minPos' property and a 'maxPos' property, each of which is a Vector3.





