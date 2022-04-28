# User Script APIs

## Callback Functions

User scripts are event driven programs. Callback functions need to be registered in the global scope by calling:

`setCallback`(`name`: String, `callback`: Function): undefined

The host program calls these functions at specific events according to their names. 

The following callback functions are called by the default "GamePlayer":

`init`(`width`: Number, `height`: Number): undefined

Called immediately after loading the script.

`dispose`(): undefined

Called before unloading the script.

`render`(`width`: Number, `height`: Number, `sizeChanged`: Boolean): undefined

Called when rendering a video frame.

`OnMouseDown`(`e`: Object): undefined

Called when mouse button is pressed down.

`OnMouseUp`(`e`: Object): undefined

Called when mouse button is up.

`OnMouseMove`(`e`: Object): undefined

Called when mouse pointer is moved.

`OnMouseWheel`(`e`: Object): undefined

Called when mouse wheel is moved.

## Global Functions

These are functions exposed through the "global object", so they can be called directly in user script.

`print`(`text1`: String, `text2`: String, ...): undefined

Print strings to `stdout` separated by spaces. Objects are not stringified automatically. To do that you need `JSON.stringify()`.

`setCallback`(`name`: String, `callback`: Function): undefined

Register a callback function.

`now`(): Number

Current time in milliseconds.

`getGLError`(): Number

Get the last OpenGL error code for debugging.

## class `Object3D`

Base class of all 3D objects visible to user script.


