[<--Home](index.html)

# class FileLoader

Provides a few interfaces to loading local files into memory.

No constructor, exposed as a global object [`fileLoader`](index.html#global-objects).

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [loadBinaryFile()](#loadbinaryfile)                           | Load a binary file into memory.                                |
| [loadTextFile()](#loadtextfile)                               | Load a text file into memory.                                  |

# Methods

## loadBinaryFile()

`.loadBinaryFile`(`name`: String): ArrayBuffer

Load a binary file into memory.

## loadTextFile()

`.loadTextFile`(`name`: String): String

Load a text file (utf8 encoding assumed) into memory.

