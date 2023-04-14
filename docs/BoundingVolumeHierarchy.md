[<--Home](index.html)

# class BoundingVolumeHierarchy

Acceleration structure for ray-casting.

`class BoundingVolumeHierarchy`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [BoundingVolumeHierarchy()](#boundingvolumehierarchy)         | Creates a new BoundingVolumeHierarchy.                         |
| **Methods**                                                   |                                                                |
| [dispose](#dispose)                                           | Dispose the unmanaged resource.                                |
| [update](#update)                                             | Update the BVH with a new model.                               |
| [remove](#remove)                                             | Remove a model from the BVH.                                   |
| [intersect](#intersect)                                       | Intersect the given ray with the acceleration structure.       |

# Constructors

## BoundingVolumeHierarchy()

`BoundingVolumeHierarchy`(`objects`: Array)

Create a BoundingVolumeHierarchy from a list of [Object3D](Object3D.html) objects.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## update()

`.update`(`obj`: [Object3D](Object3D.html)): undefined

Update the BVH with a new model.

## remove()

`.remove`(`obj`: [Object3D](Object3D.html)): undefined

Remove a model from the BVH.

## intersect()

 `.intersect`(`ray`: Object): Object

Intersect the given ray with the acceleration structure.

### The input `ray` object should have the following properties:

`ray.origin`: Vector3

Origin of the ray.

`ray.direction`: Vector3

Direction of the ray.

`ray.near`: Number

Optional. Nearest distance of search.

`ray.far`: Number

Optional. Furthest distance of search.

### The returned object has the following properties:

`.name`: String 

Name of the first intersected object.

`.distance`: Number

Distance of the first intersection point.

At the event of missing intersection, it will return null;

