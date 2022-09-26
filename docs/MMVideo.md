[<--Home](index.html)

# class MMVideo

Class that represents a background video-file player. 

Can be used as an image source.

`class MMVideo`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [MMVideo()](#mmvideo)                   | Creates a video-file player.                                   |
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
| [setAudioDevice()](#setaudiodevice)     | Change the audio-out device.                                   |

# Constructors

## MMVideo()

`MMVideo`(`playAudio`: Boolean, `idx_audio_dev`: Number, `speed`: Number)

Creates a video-file player.

### Parameters

`playAudio`: whether to play audio. Default true.

`idx_audio_dev`: index of the audio-out device.

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

## setAudioDevice()

`.setAudioDevice`(`idx_audio_dev`: Number): undefined

Change the audio-out device.


