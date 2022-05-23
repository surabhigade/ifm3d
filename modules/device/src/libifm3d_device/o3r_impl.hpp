// -*- c++ -*-
/*
 * Copyright 2018-present ifm electronic, gmbh
 * Copyright 2017 Love Park Robotics, LLC
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IFM3D_DEVICE_O3R_IMPL_HPP
#define IFM3D_DEVICE_O3R_IMPL_HPP

#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <map>
#include <mutex>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <glog/logging.h>
#include <xmlrpc-c/client.hpp>
#include <fmt/core.h>
#include <ifm3d/device/o3r.h>
#include <ifm3d/device/err.h>
#include <ifm3d/device/logging.h>
#include <xmlrpc_wrapper.hpp>

namespace ifm3d
{
  class XMLRPCWrapper;

  //============================================================
  // Impl interface
  //============================================================
  class O3R::Impl
  {
  public:
    explicit Impl(std::shared_ptr<XMLRPCWrapper> xwrapper);
    ~Impl();

    json Get(const std::vector<std::string>& path);
    json ResolveConfig(const json::json_pointer& ptr);
    void Set(const std::string& config);
    json GetInit();
    void SaveInit();
    std::string GetInitStatus();
    std::string GetSchema();
    void Lock(const std::string& password);
    void Unlock(const std::string& password);
    std::vector<PortInfo> Ports();
    PortInfo Port(const std::string& port);

    void FactoryReset(bool keepNetworkSettings);
    void Reboot();

  protected:
    std::shared_ptr<XMLRPCWrapper> xwrapper_;
  }; // end: class O3RCamera::Impl

} // end: namespace ifm3d

ifm3d::O3R::Impl::Impl(std::shared_ptr<XMLRPCWrapper> xwrapper)
  : xwrapper_(std::move(xwrapper))
{}

ifm3d::O3R::Impl::~Impl() {}

json
ifm3d::O3R::Impl::Get(const std::vector<std::string>& path)
{
  return json::parse(
    xmlrpc_c::value_string(this->xwrapper_->XCallMain("get", path)).cvalue());
}

json
ifm3d::O3R::Impl::ResolveConfig(const json::json_pointer& ptr)
{
  return this->Get({})[ptr];
}

void
ifm3d::O3R::Impl::Set(const std::string& config)
{
  this->xwrapper_->XCallMain("set", config);
}

json
ifm3d::O3R::Impl::GetInit()
{
  return json::parse(
    xmlrpc_c::value_string(this->xwrapper_->XCallMain("getInit")).cvalue());
}

void
ifm3d::O3R::Impl::SaveInit()
{
  this->xwrapper_->XCallMain("saveInit");
}

std::string
ifm3d::O3R::Impl::GetInitStatus()
{
  return xmlrpc_c::value_string(
           this->xwrapper_->XCallMain("getInitStatus",
                                      std::vector<std::string>()))
    .cvalue();
}

std::string
ifm3d::O3R::Impl::GetSchema()
{
  return xmlrpc_c::value_string(this->xwrapper_->XCallMain("getSchema"))
    .cvalue();
}

void
ifm3d::O3R::Impl::Lock(const std::string& password)
{
  this->xwrapper_->XCallMain("lock", password);
}

void
ifm3d::O3R::Impl::Unlock(const std::string& password)
{
  this->xwrapper_->XCallMain("unlock", password);
}

ifm3d::PortInfo
ifm3d::O3R::Impl::Port(const std::string& port)
{
  auto port_data =
    ResolveConfig(json::json_pointer(fmt::format("/ports/{0}", port)));

  if (port_data.is_null())
    {
      throw ifm3d::Error(IFM3D_INVALID_PORT);
    }

  return {port,
          port_data["/data/pcicTCPPort"_json_pointer],
          port_data["/info/features/type"_json_pointer]};
}

std::vector<ifm3d::PortInfo>
ifm3d::O3R::Impl::Ports()
{
  std::vector<ifm3d::PortInfo> result;

  auto ports = ResolveConfig("/ports"_json_pointer);
  for (const auto& port : ports.items())
    {
      auto port_key = port.key();
      auto port_data = port.value();

      result.push_back({port_key,
                        port_data["/data/pcicTCPPort"_json_pointer],
                        port_data["/info/features/type"_json_pointer]});
    }

  return result;
}

void
ifm3d::O3R::Impl::FactoryReset(bool keepNetworkSettings)
{
  this->xwrapper_->XCallMain("factoryReset", keepNetworkSettings);
}

void
ifm3d::O3R::Impl::Reboot()
{
  this->xwrapper_->XCallMain("reboot");
}

#endif // IFM3D_DEVICE_O3R_IMPL_HPP