var Module = {
  preRun: [],
  /**
   */
  postRun: [initialize],
  print: (function() {
    return function(text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      console.log(text);
    };
  })(),

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
    if (text) Module.printErr('[post-exception status] ' + text);
  };
};

function initialize() {
  setOnError(function(msg) {
    var divMessage = document.getElementById('message');
    msg = msg.replace('\n', '<br/>', 'g');
    divMessage.innerHTML = divMessage.innerHTML + '<br/>' + msg;
  });
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

function toJson(mp) {
  console.error('fixme');
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

function convert() {
  document.getElementById('message').innerHTML = '';

  var jsonStr = document.getElementById('input').value;
  var mpArray = toMp(jsonStr);

  var mpArray8 = new Uint8Array(mpArray);
  var mpText = '';
  for (idx = 0; idx < mpArray8.length; idx++) {
    mpText += ' ' + ('0' + mpArray8[idx].toString(16)).slice(-2);
  }
  document.getElementById('output').innerHTML = mpText;
}

