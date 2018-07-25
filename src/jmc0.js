var Module = {
  preRun: [],
  /**
   */
  postRun: [initialize],
  print: function(text) {
    if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
    console.log(text);
  },

  printErr: function(text) {
    if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
    console.error(text);
  },

  setStatus: function(text) {
    console.log(text);
  },

  totalDependencies: 0,

  monitorRunDependencies: function(left) {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' +
                            this.totalDependencies + ')' : 'All downloads complete.');
  }
};

window.onerror = function(event) {
  // TODO: do not warn on ok events like simulating an infinite loop or exitStatus
  Module.setStatus('Exception thrown, see JavaScript console');
  Module.setStatus = function(text) {
    if (text) {
      Module.printErr('[post-exception status] ' + text);
    }
  };
};

function initialize() {
  setOnError(printLog);
  setFileChangeEvent();
}

function setOnError(cb) {
  /**
   */
  function cbWrap(msgPtr) {
    var msgStr = Pointer_stringify(msgPtr);
    cb(msgStr);
  }

  /**
   * addFunctionを用いてJavaScriptの関数から関数ポインタのアドレスを取得する。
   * addFunctionを用いると、ポインタ領域を消費するので、同じ関数を何度も登録してはならない。
   * 使い終わったあとにremoveFunctionを呼び出してもよいが、領域にnullが設定されるのみで、
   * 領域が再利用可能になっていないようだ。
   * 正しく使っても不足する場合、emccのRESERVED_FUNCTION_POINTERSオプションで領域のサイズ指定可能。
   */
  var fncPtr = addFunction(cbWrap, 'vi');

  /**
   */
  ccall('set_on_error', 'null', ['number'], [fncPtr]);
}

function matchExt(fname, exts) {
  fname = fname.toLowerCase();
  for (var idx = 0; idx < exts.length; idx++) {
    var ext = '.' + exts[idx].toLowerCase();
    if ((fname.lastIndexOf(ext) + ext.length === fname.length) &&
        (ext.length <= fname.length)) {
      return true;
    }
  }
  return false;
}

function setFileChangeEvent() {
  var fileForm = document.getElementById('file');
  fileForm.addEventListener('change', function(e) {
    var result = e.target.files[0];
    var fname = result.name;
    var reader = new FileReader();

    if (matchExt(fname, ['json', 'js', 'txt'])) {
      // JSONのときはテキストとして読み出して、バイナリファイルとして保存する。
      reader.readAsText(result);
      reader.addEventListener('load', function() {
        var mpBuffer = toMp(reader.result);
        saveArrayBuffer(mpBuffer, 'application/octe-binary', 'output.msg');
      });
      
    } else {
      // MessagePackのときはArrayBufferとして読み出して、テキストファイルとして保存する。
      reader.readAsArrayBuffer(result);
      reader.addEventListener('load', function() {
        var jsBuffer = toJson(reader.result);
        saveArrayBuffer(jsBuffer, 'text/json', 'output.json');
      });
    }
  });
}

function toJson(mp) {
  var mpSize = mp.length;
  var mpPtr = Module._malloc(4 + mpSize + 1);
  setValue(mpPtr, mpSize, 'i32');
  var mpU8 = new Uint8Array(mp);
  for (var idx = 0; idx < mpSize; idx++) {
    HEAPU8[mpPtr + 4 + idx] = mpU8[idx];
  }

  var jsonPtr = ccall('to_json', 'number', ['number'], [mpPtr]);

  var jsonSize = getValue(jsonPtr, 'i32');
  var jsonArray = Module.buffer.slice(jsonPtr + 4, jsonPtr + 4 + jsonSize);

  Module._free(mpPtr);
  Module._free(jsonPtr);

  return jsonArray;
}

function toMp(jsonStr) {
  var jsonSize = lengthBytesUTF8(jsonStr);
  var jsonPtr = Module._malloc(4 + jsonSize + 1);
  setValue(jsonPtr, jsonSize, 'i32');
  stringToUTF8(jsonStr, jsonPtr + 4, jsonSize + 1);

  var mpPtr = ccall('to_mp', 'number', ['number'], [jsonPtr]);

  var mpSize = getValue(mpPtr, 'i32');
  var mpArray = Module.buffer.slice(mpPtr + 4, mpPtr + 4 + mpSize);

  Module._free(jsonPtr);
  Module._free(mpPtr);

  return mpArray;
}

function clearLog() {
  document.getElementById('message').innerHTML = '';
}

function printLog(str) {
  var message = document.getElementById('message');
  str = str.replace('\n', '<br/>', 'g');
  message.innerHTML = message.innerHTML + '<br/>' + str;
}

function saveArrayBuffer(buffer, type, fname) {
  var blob = new Blob([buffer], {type: type});
  var a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.target = '_blank';
  a.download = fname;
  a.click();
}

function convert() {
  clearLog();
  var jsonStr = document.getElementById('io').value;
  var mpBuffer = toMp(jsonStr);
  saveArrayBuffer(mpBuffer, 'application/octe-binary', 'output.msg');
}

