/*
 * Copyright (C) 2017 Love Park Robotics, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distribted on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ifm3d/camera/err.h>
#include <cstring>

// library errors
const int IFM3D_NO_ERRORS = 0;
const int IFM3D_XMLRPC_FAILURE = -100000;
const int IFM3D_XMLRPC_TIMEOUT = -100001;

// sensor errors
const int IFM3D_INVALID_PARAM = 101000;
const int IFM3D_INVALID_VALUE_TYPE = 101001;
const int IFM3D_VALUE_OUT_OF_RANGE = 101002;
const int IFM3D_READONLY_PARAM = 101003;
const int IFM3D_SESSION_ALREADY_ACTIVE = 101004;
const int IFM3D_INVALID_PASSWORD = 101005;
const int IFM3D_INVALID_SESSIONID = 101006;
const int IFM3D_COULD_NOT_REBOOT = 101007;
const int IFM3D_EEPROM_FAIL = 101046;

const char *ifm3d::strerror(int errnum)
{
  switch (errnum)
    {
    case IFM3D_NO_ERRORS:
      return "OK";
    case IFM3D_XMLRPC_FAILURE:
      return "Unknown XMLRPC failure";
    case IFM3D_XMLRPC_TIMEOUT:
      return "XMLRPC Timeout - can you `ping' the sensor?";
    case IFM3D_INVALID_PARAM:
      return "Sensor: The parameter name is invalid";
    case IFM3D_INVALID_VALUE_TYPE:
      return "Sensor: Parameter value data type is invalid";
    case IFM3D_VALUE_OUT_OF_RANGE:
      return "Sensor: Value out of range";
    case IFM3D_READONLY_PARAM:
      return "Sensor: Cannot mutate a read-only parameter";
    case IFM3D_SESSION_ALREADY_ACTIVE:
      return "Sensor: Device already has an edit-session active";
    case IFM3D_INVALID_PASSWORD:
      return "Sensor: Invalid password";
    case IFM3D_INVALID_SESSIONID:
      return "Sensor: Invalid session id";
    case IFM3D_COULD_NOT_REBOOT:
      return "Sensor: Could not execute reboot command";
    case IFM3D_EEPROM_FAIL:
      return "Sensor: Failed to read EEPROM";
    default:
      return ::strerror(errnum);
    }
}

ifm3d::error_t::error_t(int errnum)
  : std::exception(), errnum_(errnum) { }

int ifm3d::error_t::code() const noexcept
{
  return this->errnum_;
}

const char *ifm3d::error_t::what() const noexcept
{
  return ifm3d::strerror(this->code());
}
