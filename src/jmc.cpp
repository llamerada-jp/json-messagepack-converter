
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
std::string json_to_mp(const std::string& json_str);
void pack_mp(msgpack::packer<msgpack::sbuffer>& pk, const picojson::value& json);
std::string mem_to_str(const uint8_t* mem);
uint8_t* str_to_mem(const std::string& str);
}  // namespace jmc

// Prototype of C.
#ifdef __cplusplus
extern "C" {
#endif
  // Methods.
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
  //@fixme
  return "";
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
