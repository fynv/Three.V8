[<--Home](index.html)

# class HttpClient

Provides a few interfaces to make HTTP requests.

No constructor, exposed as a global object [`http`](index.html#global-objects).

| Name                                                          | Description                                                    |
| --------------------------------------------------------------| -------------------------------------------------------------- |
| **Methods**                                                   |                                                                |
| [get](#get)                                                   | Makes a synchronized HTTP Get Request.                         |
| [getAsync](#getasync)                                         | Makes an asynchronized HTTP Get Request.                       |


# Methods

## get()

`.get`(`url`: String, `is_string = False`: Boolean): String/ArrayBuffer

Makes a synchronized HTTP Get Request.

Returns String when `is_string == True`.

Return ArrayBuffer when `is_string == False`.

## getAsync()

`.getAsync`(`url`:String, `is_string = False`: Boolean, `callback`: Function) : undefined

Makes an asynchronized HTTP Get Request.

When `is_string == True`:

`callback`(`result`: Boolean, `text`: String): undefined

When `is_string == False`:

`callback`(`result`: Boolean, `data`: ArrayBuffer): undefined
