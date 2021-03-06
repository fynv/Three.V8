# Three.V8

Three.V8 is a proof of concept (POC) of an embedabble 3D rendering engine using JavaScript as user script.

Currently, the whole project requires Visual Studio 2019 or later to be built, becuase:

* V8 binary is acquired from [NuGet](https://www.nuget.org/packages/v8-v142-x64/)
* A C# WPF GUI is provided as an example of how to embed the engine.

The core of the code is designed to be portable, wherever V8 and OpenGL are available.

OpenGL is choosen as the underlying graphics API. One of the ultimate goal of the project is to make it possible to embed a 3D rendering engine directly into an open-source web browser like [Chrome](https://www.chromium.org/developers/how-tos/get-the-code/), hopefully to be compatible with the WebGL2 context of H5 Canvas.

An alternative is to run [Three.js](https://threejs.org/) directly in an embedded V8 environment, replacing WebGL with OpenGL bindings. This project explores a different approach by implementing most of the engine using C++ and exposing the high-level objects. In this way, the performance sensitive tasks can be done more efficiently. It will also use the more modernized features of the native OpenGL rather than the GLES3 subset.

## Subprojects

* ThreeEngine: C++ 3D rendering engine.
* Test.Cpp: Simple example using the C++ engine directly.
* Three.V8: V8 bindings of the 3D rendering engine.
* Test.V8: Simple example using the V8 engine.
* Three.V8.CLR: C# bindings of the V8 engine (itself) and OpenGL.
* GamePlayer: C# WPF GUI embedding the V8 engine.
* game: Game script test code. The math library and other utilities are borrowed directly from Three.js. Most assets are from [discoverthreejs.com](https://discoverthreejs.com/)and [sketchfab.com](https://sketchfab.com/tags/glb).

## 3rd-party libraries

* [V8](https://www.nuget.org/packages/v8-v142-x64): For running user script.
* [Three.js](https://threejs.org/): Reusing existing JS utilities.
* [tiny-gltf](https://github.com/syoyo/tinygltf): For loading GLTF models.
* [draco](https://github.com/google/draco): Mesh decoder. Used by tiny-gltf.
* [stb_image](https://github.com/nothings/stb): For loading images.
* [glm](https://github.com/g-truc/glm): For geometry maths.
* [glew](https://github.com/nigels-com/glew): For OpenGL Wrangling.
* [bvh](https://github.com/madmann91/bvh): For acceleration of ray-casting.
* [crc64](https://github.com/srned/baselib): For state hashing.

## Building

Building is straightforward using Visual Studio 2019. Just make sure to resolve the git submodules first.

## Runing GamePlayer

Run the executable "GamePlayer\bin\Release\GamePlayer.exe":

![screenshot.png](docs/screenshot.png)

The script "../../../game/bundle.js" is loaded by default. You can load your own script by clicking the button "Load Script".

The engine doesn't support ES6 modules eg. "import". User scripts need to be pre-bundled using bundlers like [rollup.js](https://rollupjs.org/)

## License
The source code is licensed under ['"Anti 996" License'](https://github.com/996icu/996.ICU/blob/master/LICENSE) by Fei Yang and Vulcan Eon (??????????????????????????????????????????).

## Documentation

[User Script APIs](docs/UserScriptAPIs.md)

[Features and Code Snippets](docs/Features.md)

