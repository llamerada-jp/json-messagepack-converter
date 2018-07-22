/*
 * Copyright 2018 Yuji Ito <llamerada.jp@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <emscripten.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <msgpack.hpp>
#include <picojson.h>

#include <cstdint>

namespace jmc {
// Values.
void (*on_error)(const char*) = nullptr;

// Methods.
std::string mp_to_json(const std::string& mp_str);
picojson::value pack_json(msgpack::object obj);
std::string json_to_mp(const std::string& json_str);
void pack_mp(msgpack::packer<msgpack::sbuffer>& pk, const picojson::value& json);
std::string mem_to_str(const uint8_t* mem);
uint8_t* str_to_mem(const std::string& str);
}  // namespace jmc

// Prototype of C.
#ifdef __cplusplus
extern "C" {
#endif
  // Methods for importing.
  extern char* to_base64(const uint8_t* src_addr, uint32_t src_size, uint32_t* dst_size);

  // Methods for exporting.
  EMSCRIPTEN_KEEPALIVE void set_on_error(void (*cb)(const char*));
  EMSCRIPTEN_KEEPALIVE uint8_t* to_json(const uint8_t* mp_addr);
  EMSCRIPTEN_KEEPALIVE uint8_t* to_mp(const uint8_t* json_addr);
#ifdef __cplusplus
}
#endif

void set_on_error(void (*cb)(const char*)) {
  jmc::on_error = cb;
}

uint8_t* to_json(const uint8_t* mp_addr) {
  std::string mp_str = jmc::mem_to_str(mp_addr);
  std::string json_str = jmc::mp_to_json(mp_str);
  return jmc::str_to_mem(json_str);
}

uint8_t* to_mp(const uint8_t* json_addr) {
  std::string json_str = jmc::mem_to_str(json_addr);
  std::string mp_str = jmc::json_to_mp(json_str);
  return jmc::str_to_mem(mp_str);
}

namespace jmc {
std::string mp_to_json(const std::string& mp_str) {
  // Decode MessagePack from string.
  msgpack::object_handle handle = msgpack::unpack(mp_str.data(), mp_str.size());
  msgpack::object obj = handle.get();

  picojson::value json = pack_json(obj);
  return json.to_str();
}

picojson::value pack_json(msgpack::object obj) {
  switch (obj.type) {
    case msgpack::type::NIL: {
      return picojson::value();
    } break;

    case msgpack::type::BOOLEAN: {
      return picojson::value(obj.via.boolean);
    } break;

    case msgpack::type::POSITIVE_INTEGER: {
      return picojson::value(static_cast<double>(obj.via.i64));
    } break;

    case msgpack::type::NEGATIVE_INTEGER: {
      return picojson::value(static_cast<double>(obj.via.u64));
    } break;

    case msgpack::type::FLOAT32:
    case msgpack::type::FLOAT64: {
      return picojson::value(obj.via.f64);
    } break;

    case msgpack::type::STR: {
      return picojson::value(std::string(obj.via.str.ptr, obj.via.str.size));
    } break;

    case msgpack::type::BIN: {
      char* str_addr;
      uint32_t str_size;
      str_addr = to_base64(reinterpret_cast<const uint8_t*>(obj.via.bin.ptr), obj.via.bin.size, &str_size);
      std::string base64(str_addr, str_size);
      free(str_addr);
      return picojson::value(base64);
    } break;

    case msgpack::type::ARRAY: {
      picojson::array json_array;
      for (int idx = 0; idx < obj.via.array.size; idx++) {
        json_array.push_back(pack_json(*(obj.via.array.ptr + idx)));
      }
      return picojson::value(json_array);
    } break;

    case msgpack::type::MAP: {
      picojson::object json_map;
      for (int idx = 0; idx < obj.via.map.size; idx++) {
        msgpack::object_kv* kv = obj.via.map.ptr + idx;
        picojson::value key = pack_json(kv->key);
        picojson::value val = pack_json(kv->val);
        if (key.is<std::string>()) {
          json_map.insert(std::make_pair(key.get<std::string>(), val));
        } else {
          on_error("ignore key");
        }
      }
      return picojson::value(json_map);
    } break;

    default: {
      on_error("unsupported type");
      return picojson::value();
    } break;
  }
}

std::string json_to_mp(const std::string& json_str) {
  // Deconde json from string.
  picojson::value json;
  std::string err = picojson::parse(json, json_str);
  if (!err.empty()) {
    on_error(err.c_str());
    return "";
  }

  // Convert json to messagepack.
  msgpack::sbuffer buffer;
  msgpack::packer<msgpack::sbuffer> pk(&buffer);
  pack_mp(pk, json);

  // Encode messagepack to string.
  return std::string(buffer.data(), buffer.size());
}

void pack_mp(msgpack::packer<msgpack::sbuffer>& pk, const picojson::value& json) {
  if (json.is<picojson::null>()) {
    pk.pack_nil();

  } else if (json.is<bool>()) {
    if (json.get<bool>()) {
      pk.pack_true();
    } else {
      pk.pack_false();
    }

  } else if (json.is<double>()) {
    pk.pack_double(json.get<double>());

  } else if (json.is<std::string>()) {
    pk.pack(json.get<std::string>());

  } else if (json.is<picojson::array>()) {
    const picojson::array& array = json.get<picojson::array>();
    pk.pack_array(array.size());
    for (auto& it : array) {
      pack_mp(pk, it);
    }

  } else if (json.is<picojson::object>()) {
    const picojson::object& object = json.get<picojson::object>();
    pk.pack_map(object.size());
    for (auto& it : object) {
      pk.pack(it.first);
      pack_mp(pk, it.second);
    }
  }
}

std::string mem_to_str(const uint8_t* mem) {
  uint32_t str_size = *reinterpret_cast<const uint32_t*>(mem);
  return std::string(reinterpret_cast<const char*>(mem + sizeof(uint32_t)), str_size);
}

uint8_t* str_to_mem(const std::string& str) {
  uint32_t mem_size = str.size();
  std::unique_ptr<uint8_t[]> mem(new uint8_t[sizeof(uint32_t) + mem_size]);
  *reinterpret_cast<uint32_t*>(mem.get()) = mem_size;
  memcpy(mem.get() + sizeof(uint32_t), str.data(), mem_size);
  return mem.release();
}

}  // namespace jmc
