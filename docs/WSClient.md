[<--Home](index.html)

# class WSClient

Maintains a websocket connection at the client side.

`class WSClient`

| Name                                    | Description                                                    |
| ----------------------------------------| -------------------------------------------------------------- |
| **Constructors**                        |                                                                |
| [WSClient()](#wsclient)                 | Creates a websocket client.                                    |
| **Properties**                          |                                                                |
| [onOpen](#onopen)                       | Callback function called when connection is established.       |
| [onMessage](#onmessage)                 | Callback function for recieving messages.                      |
| **Methods**                             |                                                                |
| [dispose()](#dispose)                   | Dispose the unmanaged resource.                                |
| [send()](#send)                         | Send a message.                                                |


# Constructors

## WSClient()

`WSClient`(`url`: String)

Creates a websocket client. 

### Parameters

`url` : URL to connect to.

# Properties

## onOpen

 `.onOpen`: Function
 `.onOpen`(): undefined

Callback function called when connection is established.

## onMessage

`.onMessage`: Function
`.onMessage`(`buf`: ArrayBuffer): undefined
`.onMessage`(`text`: String): undefined

Callback function for recieving messages.


# Methods

## dispose()

`.dispose`(): undefined

Dispose the unmanaged resource.

## send()

`.send`(`buf`: ArrayBuffer): undefined
`.send`(`text`: String): undefined

Send a message.

