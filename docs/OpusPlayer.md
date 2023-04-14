[<--Home](index.html)

# class OpusPlayer

Play back a raw opus stream.

`class OpusPlayer`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [OpusPlayer()](#opusplayer)             | Creates an opus-player.                                        |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |
| [addPacket()](#addpacket)               | Add an opus packet to the play queue.                          |

# Constructors

## OpusRecorder()

`OpusPlayer`(`id_device`: Number)

Creates an opus-player.

### Parameters

`id_device` : index of the audio output device

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## addPacket()

`.addPacket`(`data`: ArrayBuffer): undefined

Add an opus packet to the play queue.

