//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#pragma once

#include <json.hpp>

namespace PvsStudio
{

class SerializationError : public std::runtime_error
{
public:
  SerializationError(const std::string &arg) : runtime_error(arg) {}
};

namespace detail
{

struct JsonDeserializer
{
  const nlohmann::json &json;

  template <typename T>
  JsonDeserializer &Optional(const std::string &name, T &field)
  {
    auto it = json.find(name);
    if (it != json.end())
    {
      field = it->get<T>();
    }

    return *this;
  }

  template <typename T, typename U>
  JsonDeserializer &Optional(const std::string &name, T &field, U &&defaultValue)
  {
    auto it = json.find(name);
    if (it != json.end())
    {
      field = it->get<T>();
    }
    else
    {
      field = std::forward<U>(defaultValue);
    }

    return *this;
  }

  template <typename T>
  JsonDeserializer &Required(const std::string &name, T &field)
  {
    auto it = json.find(name);
    if (it != json.end())
    {
      field = it->get<T>();
    }
    else
    {
      throw SerializationError("field " + name + " is not found");
    }

    return *this;
  }
};

struct JsonSerializer
{
  nlohmann::json &json;

  template <typename T>
  JsonSerializer &Optional(const std::string &name, T &field)
  {
    json[name] = field;
    return *this;
  }

  template <typename T, typename U>
  JsonSerializer &Optional(const std::string &name, T &field, U&&)
  {
    json[name] = field;
    return *this;
  }

  template <typename T>
  JsonSerializer &Required(const std::string &name, T &field)
  {
    json[name] = field;
    return *this;
  }
};

}

template <typename T>
auto from_json(const nlohmann::json &json, T &st) -> decltype(std::declval<T&>().Serialize(std::declval<detail::JsonDeserializer&>()), void())
{
  detail::JsonDeserializer d{json};
  st.Serialize(d);
}

template <typename T>
auto to_json(nlohmann::json &json, const T &st) -> decltype(std::declval<T&>().Serialize(std::declval<detail::JsonSerializer&>()), void())
{
  detail::JsonSerializer d{json};
  const_cast<T&>(st).Serialize(d);
}

}

