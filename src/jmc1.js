function _to_base64(srcPtr, srcSize, dstSizePtr) {
  var subarray = HEAPU8.subarray(srcPtr, srcPtr + srcSize);
  var string = String.fromCharCode.apply('', subarray);
  var base64 = btoa(string);

  var ptr = Module._malloc(base64.length);
  stringToUTF8(base64, ptr, base64.length);
  setValue(dstSizePtr, base64.length, 'i32');

  return ptr;
}