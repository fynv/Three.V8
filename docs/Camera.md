[<--Home](index.html)

# class Camera

Base class for cameras. 

`class Camera extends Object3D`

Inheritance [Object3D](Object3D.html) --> Camera

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [Camera()](#camera)                                           | Creates a new Camera.                                          |
| **Properties**                                                |                                                                |
| [matrixWorldInverse](#matrixworldinverse)                     | Inverse of `.matrixWorld`                                      |
| [projectionMatrix](#projectionmatrix)                         | Matrix which contains the projection.                          |
| [projectionMatrixInverse](#projectionmatrixinverse)           | The inverse of `.projectionMatrix`                             |
| **Methods**                                                   |                                                                |
| [getMatrixWorldInverse](#getmatrixworldinverse)               | Get the value of `.matrixWorldInverse`                         |
| [getProjectionMatrix](#getprojectionmatrix)                   | Get the value of `.projectionMatrix`                           |
| [getProjectionMatrixInverse](#getprojectionmatrixinverse)     | Get the value of `.projectionMatrixInverse`                    |

# Constructors

## Camera()

`Camera`()

Creates a new Camera. Note that this class is not intended to be created directly.

# Properties

See the base [Object3D](Object3D.html#properties) class for common properties.

## matrixWorldInverse

`.matrixWorldInverse`: Object

This is the inverse of [`.matrixWorld`](Object3D.html#matrixworld). `.matrixWorld` contains the Matrix which has the world transform of the Camera.

Read-only.

## projectionMatrix

`.projectionMatrix`: Object

This is the matrix which contains the projection.

Read-only.

## projectionMatrixInverse

`.projectionMatrixInverse`: Object

The inverse of [`.projectionMatrix`](#projectionmatrix).

Read-only.

# Methods

See the base [Object3D](Object3D.html#methods) class for common methods.

## getMatrixWorldInverse()

`.getMatrixWorldInverse`(`matrix`: Matrix4) : Matrix4

Copy the value of [`.matrixWorldInverse`](#matrixworldinverse) into `matrix`.

## getProjectionMatrix()

`.getProjectionMatrix`(`matrix`: Matrix4) : Matrix4

Copy the value of [`.projectionMatrix`](#projectionmatrix) into `matrix`.

## getProjectionMatrixInverse()

`.getProjectionMatrixInverse`(`matrix`: Matrix4) : Matrix4

Copy the value of [`.projectionMatrixInverse`](#projectionmatrixinverse) into `matrix`.

