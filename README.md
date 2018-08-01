# JSON MessagePack converter

### [Link to running sample.](https://llamerada-jp.github.io/json-messagepack-converter/jmc.html)

* You can convert between JSON and MessagePack without accessing to any server.
* This program output JSON(.json or .js) or MessagePack formatted file, when you input opposite formatted file.
* This program skips the elements when miss to convert it, because format of JSON and MessagePack have not perfect compatibility.

<br/>  

* サーバアクセスなしにJSONとMessagePackの相互変換ができます。
* JSON(.jsonまたは.js)形式のファイルまたはMessagePack形式のファイルを指定すると、対となる形式のファイルとして出力します。
* JSONとMessagePackの表現能力は完全には一致しないため、変換ができない要素があったときは、メッセージを出力してその要素をスキップします。

## About WebAssembly

We can get below knowledges of WebAssembly by this project.

* Build the WebAssembly project by using emscripten and CMake.
* Build the external library (MessagePack) by using emscripten.
* Call C/C++ functions by JavaScript environment.
* Call JavaScript functions by C/C++ environment.
* Call JavaScript callback functions by C/C++ environment.

<br/>

このプロジェクトはWebAssemblyを使い、以下の機能を確認したサンプルプロジェクトです。

* emscriptenとCMakeを用いたWebAssemblyのビルド。
* emscriptenを用いた外部ライブラリ(MessagePack)のビルド。
* JavaScriptからのC関数の呼び出し。
* CからJavaScript関数の呼び出し。
* JavaScriptからのコールバック関数の登録。

## How to build and run

You should install emscipten before build.

```sh
$ cd <workdir>
$ git clone https://github.com/juj/emsdk.git
$ cd emsdk
$ ./emsdk install --build=Release sdk-incoming-64bit binaryen-master-64bit
$ ./emsdk activate --global --build=Release sdk-incoming-64bit binaryen-master-64bit
$ source ./emsdk_env.sh
```

And you can build the project by a build script.

```sh
$ cd <workdir>
$ git clone https://github.com/llamerada-jp/json-messagepack-converter.git
$ cd json-messagepack-converter
$ git submodule update --init --recursive
$ sh build_wa.sh
```

After build, you can run it by procedure of below.

```sh
$ cd <workdir>/json-messagepack-converter/docs
$ emrun --no_browser jmc.html
```

Please access to http://localhost:6931/jmc.html by your Web browser.
