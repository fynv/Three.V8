[<--Home](index.html)

# class ProbeGridLoader

Provides a few interfaces to load a [ProbeGrid](ProbeGrid.html) from local files or from memory.

No constructor, exposed as a global object [`probeGridLoader`](index.html#global-objects).

`class ProbeGridLoader`

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [loadFile()](#loadfile)                                       | Load a probe-grid from a local file.                           |
| [loadMemory()](#loadmemory)                                   | Load a probe-grid from a memory buffer.                        |


# Methods

## loadFile()

`.loadFile`(`name`: String): [ProbeGrid](ProbeGrid.html)

Load a probe-grid from a local file.

## loadMemory()

`.loadMemory`(`buf`: ArrayBuffer): [ProbeGrid](ProbeGrid.html)

Load a probe-grid from a memory buffer.
