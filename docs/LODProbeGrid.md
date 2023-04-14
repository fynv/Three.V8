[<--Home](index.html)

# class LODProbeGrid

A [mixed resolution grid of light-probes](https://fynv.github.io/MixedResolutionGridOfLightProbes/).

`class LODProbeGrid extends IndirectLight`

Inheritance [IndirectLight](IndirectLight.html) --> LODProbeGrid

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Constructors**                                              |                                                                |
| [LODProbeGrid()](#lodprobegrid)                               | Creates a new LODProbeGrid.                                    |
| **Properties**                                                |                                                                |
| [coverageMin](#coveragemin)                                   | minimum position of the grid coverage                          |
| [coverageMax](#coveragemax)                                   | maximum position of the grid coverage                          |
| [baseDivisions](#basedivisions)                               | number of divisions of the base-level of the grid              |
| [subDivisionLevel](#subdivisionlevel)                         | number of sub-division levels                                  |
| [numberOfProbes](#numberofprobes)                             | number of probes in the grid                                   |
| [normalBias](#normalbias)                                     | bias used for sampling visibility information                  |
| [perPrimitive](#perprimitive)                                 | whether using per-primitive interpolation for better performance |
| **Methods**                                                   |                                                                |
| [getCoverageMin](#getcoveragemin)                             | Get the minimum position of the grid coverage                  |
| [setCoverageMin](#setcoveragemin)                             | Set the minimum position of the grid coverage                  |
| [getCoverageMax](#getcoveragemax)                             | Get the maximum position of the grid coverage                  |
| [setCoverageMax](#setcoveragemax)                             | Set the maximum position of the grid coverage                  |
| [getBaseDivisions](#getbasedivisions)                         | Get the number of divisions of the base-level of the grid      |
| [setBaseDivisions](#setbasedivisions)                         | Set the number of divisions of the base-level of the grid      |
| [toProbeGrid](#toprobegrid)                                   | Convert to uniform grid                                        |

# Constructors

## LODProbeGrid()

 `LODProbeGrid`()

Creates a new LODProbeGrid.

# Properties

## coverageMin

`.coverageMin`: Object

The minimum position of the grid coverage.

Read-only. Use the method [`.setCoverageMin`](#setcoveragemin) to modify this property.

## coverageMax

`.coverageMax`: Object

The maximum position of the grid coverage.

Read-only. Use the method [`.setCoverageMax`](#setcoveragemax) to modify this property.

## baseDivisions

`.baseDivisions`: Object

The number of divisions of the base-level of the grid.

Read-only. Use the method [`.setBaseDivisions`](#setbasedivisions) to modify this property.

## subDivisionLevel

`.subDivisionLevel`: Number

Number of sub-division levels.

Readable and writable. Default value is 2.

## numberOfProbes

`.numberOfProbes`: Number

Number of probes in the grid.

ReadOnly.

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


## getBaseDivisions()

`.getBaseDivisions`(`divisions`: Vector3) : Vector3

Copy the value of [`.baseDivisions`](#basedivisions) into `divisions`.

## setBaseDivisions()

`.setBaseDivisions`(`divisions`: Vector3): undefined

Set the value of [`.baseDivisions`](#basedivisions) according to `divisions`.

## toProbeGrid()

`.toProbeGrid`(`scene`: [Scene](Scene.html)) : [ProbeGrid](ProbeGrid.html)

Convert the mixed resoluion grid of light probes to an uniform grid of light probes.

Needs the `scene` paramter for reconstructing missing visibility information.


