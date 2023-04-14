[<--Home](index.html)

# class BackgroundScene

Use another scene as background.

`class BackgroundScene extends Background`

Inheritance [Background](Background.html) --> BackgroundScene

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [BackgroundScene()](#backgroundscene)                         | Creates a new BackgroundScene.                                 |
| **Properties**                                                |                                                                |
| [scene](#scene)                                               | Reference to the scene.                                        |
| [near](#near)                                                 | near clipping distance for rendering the background            |
| [far](#far)                                                   | far clipping distance for rendering the background             |

# Constructors

## BackgroundScene()

`BackgroundScene`(`scene`: [Scene](Scene.html), near: Number, far: Number)

Creates a new BackgroundScene. 

### Parameters

`scene`: reference to the scene used as background.

`near`: near clipping distance for rendering the background. Default value 10.0.

`far`: far clipping distance for rendering the background. Default value 10000.0.

# Properties

## scene

`.scene`: [Scene](Scene.html)

Reference to the scene.

Readable & writable.

## near

`.near`: Number

Near clipping distance for rendering the background

## far

`.far`: Number

Far clipping distance for rendering the background

