[<--Home](index.html)

# class PerspectiveCamera

`class PerspectiveCamera extends Camera`

Inheritance [Object3D](Object3D.html) --> [Camera](Camera.html) --> PerspectiveCamera

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [PerspectiveCamera()](#perspectivecamera)                     | Creates a perspective Camera.                                  |
| **Properties**                                                |                                                                |
| [isPerspectiveCamera](#isperspectivecamera)                   | Checks if the object is PerspectiveCamera                      |
| [fov](#fov)                                                   | Camera frustum vertical field of view                          |
| [aspect](#aspect)                                             | Camera frustum aspect ratio                                    |
| [near](#near)                                                 | Camera frustum near plane                                      |
| [far](#far)                                                   | Camera frustum far plane                                       |
| **Methods**                                                   |                                                                |
| [updateProjectionMatrix](#updateprojectionmatrix)             | Updates the camera projection matrix.                          |


# Constructors

## PerspectiveCamera()

`PerspectiveCamera`(`fov`: Number, `aspect`: Number, `near`: Number, `far`: Number)

Creates a perspective camera.

### Parameters

`fov`: Camera frustum vertical field of view.

`aspect`: Camera frustum aspect ratio.

`near`: Camera frustum near plane.

`far`: Camera frustum far plane.

Together these define the camera's viewing frustum.

# Properties

See the base [Camera](Camera.html#properties) class for common properties.
Note that after making changes to most of these properties you will have to call [`.updateProjectionMatrix`](#updateprojectionmatrix) for the changes to take effect.

## isPerspectiveCamera()

`.isPerspectiveCamera`: Boolean 

Read-only flag to check if a given object is of type PerspectiveCamera.

## fov()

`.fov`: Number

Camera frustum vertical field of view, from bottom to top of view, in degrees. Default is 50.

Readable and writable.

## aspect()

`.aspect`: Number

Camera frustum aspect ratio, usually the canvas width / canvas height. Default is 1 (square canvas).

Readable and writable.

## near()

`.near`: Number

Camera frustum near plane. Default is 0.1.

The valid range is greater than 0 and less than the current value of the far plane.

Readable and writable.

## far()

`.far`: Numbers

Camera frustum far plane. Default is 200.

Must be greater than the current value of near plane.

Readable and writable.

# Methods

See the base [Camera](Camera.html#methods) class for common methods.

## updateProjectionMatrix()

`.updateProjectionMatrix`(): undefined

Updates the camera projection matrix. Must be called after any change of parameters.

