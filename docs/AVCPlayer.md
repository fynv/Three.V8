[<--Home](index.html)

# class AVCPlayer

Play back a raw opus stream.

Can be used as an image source.

`class AVCPlayer`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [AVCPlayer()](#avcplayer)               | Creates an AVC-player.                                         |
| **Properties**                          |                                                                |
| [width](#width)                         | Width of the image source.                                     |
| [height](#height)                       | Height of the image source.                                    |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |
| [addPacket()](#addpacket)               | Add an AVC packet to the play queue.                           |
| [updateTexture()](#updatetexture)       | Add an AVC packet to the play queue.                           |


# Constructors

## AVCPlayer()

`AVCPlayer`()

Creates an AVC-player.

# Properties

## width

 `.width`: Number

Width of the image source.

Read-only.

## height

 `.height`: Number

Height of the image source.

Read-only.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## addPacket()

`.addPacket`(`data`: ArrayBuffer): undefined

Add an AVC packet to the play queue.

## updateTexture()

`.updateTexture`(): undefined

Attempt to read new frames and update the texture data.




