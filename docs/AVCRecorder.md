[<--Home](index.html)

# class AVCRecorder

Record from a video device and encode as a raw AVC stream.

`class AVCRecorder`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [AVCRecorder()](#avcrecorder)           | Creates an AVC-recorder.                                       |
| **Properties**                          |                                                                |
| [callback](#callback)                   | Callback function for recieving AVC packets.                   |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |

# Constructors

## AVCRecorder()

`AVCRecorder`(`id_device`: Number)

Creates an AVC-recorder.

### Parameters

`id_device` : index of the camera device.

# Properties

## callback

 `.callback`: Function
 `.callback`(`data`: ArrayBuffer): undefined

Callback function for recieving AVC packets.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.




