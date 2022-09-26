[<--Home](index.html)

# class MMLazyVideo

Class that represents an image source from a video-file.

`class MMLazyVideo`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [MMLazyVideo()](#mmlazyvideo)           | Creates a video-file source.                                   |
| **Properties**                          |                                                                |
| [looping](#looping)                     | Whether loop the media when EOF met.                           |
| [width](#width)                         | Width of the image source.                                     |
| [height](#height)                       | Height of the image source.                                    |
| [isPlaying](#isplaying)                 | Whether the media is currently being played.                   |
| [duration](#duration)                   | Duration of the media in seconds.                              |
| [position](#position)                   | Playback position of the current media in seconds.             |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |
| [updateTexture()](#updatetexture)       | Attempt to read new frames and update the texture data.        |
| [play()](#play)                         | Start playback of the media.                                   |
| [pause()](#pause)                       | Pause playback of the media.                                   |
| [setPosition()](#setposition)           | Apply seeking.                                                 |


# Constructors

## MMLazyVideo()

`MMLazyVideo`(`speed`: Number)

Creates a video-file source.

### Parameters

`speed` : playback speed. Default 1.0.

# Properties

## looping

`.looping`: Boolean

Whether loop the media when EOF met.

Readable and writable.

## width

 `.width`: Number

Width of the image source.

Read-only.

## height

 `.height`: Number

Height of the image source.

Read-only.

## isPlaying

`.isPlaying`: Boolean

Whether the media is currently being played.

Read-only.

## duration

`.duration`: Number

Duration of the media in seconds.

Read-only.

## position

`.position`: Number

Playback position of the current media in seconds.

Read-only.

# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## updateTexture()

`.updateTexture`(): undefined

Attempt to read new frames and update the texture data.

## play()

`.play`(): undefined

Start playback of the media.

## pause()

`.pause`(): undefined

Pause playback of the media.

## setPosition()

`setPosition`(`position`: Number): undefined

Apply seeking, set the current position (in secs) to `position`.


