[<--Home](index.html)

# class ProbeGrid

An uniform grid of light-probes.

`class ProbeGrid extends IndirectLight`

Inheritance [IndirectLight](IndirectLight.html) --> ProbeGrid

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [ProbeGrid()](#probegrid)                                     | Creates a new ProbeGrid.                                       |
| **Properties**                                                |                                                                |
| [coverageMin](#coveragemin)                                   | minimum position of the grid coverage                          |
| [coverageMax](#coveragemax)                                   | maximum position of the grid coverage                          |
| [divisions](#divisions)                                       | number of divisions along each axis                            |
| [ypower](#ypower)                                             | a distribution bending factor along y-axis                     |
| [normalBias](#normalbias)                                     | bias used for sampling visibility information                  |
| [perPrimitive](#perprimitive)                                 | whether using per-primitive interpolation for better performance |
| **Methods**                                                   |                                                                |
| [getCoverageMin](#getcoveragemin)                             | Get the minimum position of the grid coverage                  |
| [setCoverageMin](#setcoveragemin)                             | Set the minimum position of the grid coverage                  |
| [getCoverageMax](#getcoveragemax)                             | Get the maximum position of the grid coverage                  |
| [setCoverageMax](#setcoveragemax)                             | Set the maximum position of the grid coverage                  |
| [getDivisions](#getdivisions)                                 | Get the number of divisions along each axis                    |
| [setDivisions](#setdivisions)                                 | Set the number of divisions along each axis                    |

# Constructors

## ProbeGrid()

 `ProbeGrid`()

Creates a new ProbeGrid.

# Properties

## coverageMin

`.coverageMin`: Object

The minimum position of the grid coverage.

Read-only. Use the method [`.setCoverageMin`](#setcoveragemin) to modify this property.

## coverageMax

`.coverageMax`: Object

The maximum position of the grid coverage.

Read-only. Use the method [`.setCoverageMax`](#setcoveragemax) to modify this property.

## divisions

`.divisions`: Object

The number of divisions along each axis.

Read-only. Use the method [`.setDivisions`](#setdivisions) to modify this property.

## ypower

`.ypower`: Number

A distribution bending factor along y-axis.

Readable and writable. Default value is 1.0.

## normalBias

`.normalBias`: Number

The bias used for sampling visibility information.

Readable and writable. Default value is 0.2.

# perPrimitive

`.perPrimitive`: Boolean

Whether using per-primitive interpolation for better performance

Readable and writable. Default value is false.

# Methods

## getCoverageMin()

`.getCoverageMin`(`coverage_min`: Vector3) : Vector3

Copy the value of [`.coverageMin`](#coveragemin) into `coverage_min`.

## setCoverageMin()

`.setCoverageMin`(`coverage_min`: Vector3): undefined

Set the value of [`.coverageMin`](#coveragemin) according to `coverage_min`.

## getCoverageMax()

`.getCoverageMax`(`coverage_max`: Vector3) : Vector3

Copy the value of [`.coverageMax`](#coveragemax) into `coverage_max`.

## setCoverageMax()

`.setCoverageMax`(`coverage_max`: Vector3): undefined

Set the value of [`.coverageMax`](#coveragemax) according to `coverage_max`.

## getDivisions()

`.getDivisions`(`divisions`: Vector3) : Vector3

Copy the value of [`.divisions`](#divisions) into `divisions`.

## setDivisions()

`.setDivisions`(`divisions`: Vector3): undefined

Set the value of [`.divisions`](#divisions) according to `divisions`.
