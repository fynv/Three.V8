[<--Home](index.html)

# class MMAudio

Class that represents a background audio-file player. 

`class MMAudio`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [MMAudio()](#mmaudio)                   | Creates a audio-file player.                                   |
| **Properties**                          |                                                                |
| [looping](#looping)                     | Whether loop the media when EOF met.                           |
| [isPlaying](#isplaying)                 | Whether the media is currently being played.                   |
| [duration](#duration)                   | Duration of the media in seconds.                              |
| [position](#position)                   | Playback position of the current media in seconds.             |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |
| [play()](#play)                         | Start playback of the media.                                   |
| [pause()](#pause)                       | Pause playback of the media.                                   |
| [setPosition()](#setposition)           | Apply seeking.                                                 |
| [setAudioDevice()](#setaudiodevice)     | Change the audio-out device.                                   |

# Constructors

## MMAudio()

`MMAudio`(`idx_audio_dev`: Number, `speed`: Number)

Creates a audio-file player.

### Parameters

`idx_audio_dev`: index of the audio-out device.

`speed` : playback speed. Default 1.0.


# Properties

## looping

`.looping`: Boolean

Whether loop the media when EOF met.

Readable and writable.

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



