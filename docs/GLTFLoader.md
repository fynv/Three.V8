[<--Home](index.html)

# class GLTFLoader

Provides a few interfaces to load GLTF models from local files or from memory.

No constructor, exposed as a global object [`gltfLoader`](Index.html#global-objects).

`class GLTFLoader`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [loadModelFromFile()](#loadmodelfromfile)                     | Load a GLTF model from a local GLTF file.                      |
| [loadAnimationsFromFile()](#loadanimationsfromfile)           | Load only animation data from a local GLTF file.               |
| [loadModelFromMemory()](#loadmodelfrommemory)                 | Load a GLTF model from a memory buffer.                        |
| [loadAnimationsFromMemory()](#loadanimationsfrommemory)       | Load only animation data from a memory buffer.                 |


# Methods

## loadModelFromFile()

`.loadModelFromFile`(`name`: String): GLTFModel

Load a GLTF model from a local GLTF file.

## loadAnimationsFromFile()

`.loadAnimationsFromFile`(`name`: String): Object

Load only animation data from a local GLTF file.

The returned object has the same structure as the return value of [`GLTFModel.getAnimation`](GLTFModel.html#getanimation).

## loadModelFromMemory()

`.loadModelFromMemory`(`buf`: ArrayBuffer): GLTFModel

Load a GLTF model from a memory buffer.

## loadAnimationsFromMemory()

`.loadAnimationsFromMemory`(`buf`: ArrayBuffer): Object

Load only animation data from a memory buffer.

The returned object has the same structure as the return value of [`GLTFModel.getAnimation`](GLTFModel.html#getanimation).
