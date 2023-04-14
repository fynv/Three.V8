[<--Home](index.html)

# class OpusRecorder

Record from an audio device and encode as a raw opus stream.

`class OpusRecorder`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [OpusRecorder()](#opusRecorder)         | Creates an opus-recorder.                                      |
| **Properties**                          |                                                                |
| [callback](#callback)                   | Callback function for recieving opus packets.                  |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |

# Constructors

## OpusRecorder()

`OpusRecorder`(`id_device`: Number)

Creates an opus-recorder.

### Parameters

`id_device` : index of the audio input device

# Properties

## callback

 `.callback`: Function
 `.callback`(`data`: ArrayBuffer): undefined

Callback function for recieving opus packets.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

