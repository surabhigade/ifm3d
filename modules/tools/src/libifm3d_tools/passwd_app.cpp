/*
 * Copyright 2018 ifm electronic, gmbh
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ifm3d/tools/passwd_app.h>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <ifm3d/tools/cmdline_app.h>
#include <ifm3d/camera/camera.h>

namespace po = boost::program_options;

ifm3d::PasswdApp::PasswdApp(int argc,
                            const char** argv,
                            const std::string& name)
  : ifm3d::CmdLineApp(argc, argv, name)
{
  // clang-format off
	this->local_opts_.add_options()
	 ("new",po::value<std::string>(),
	  "password to be set on sensor")
	 ("disable",po::bool_switch()->default_value(false),
	  "disable password on sensor");
  // clang-format on

  po::store(po::command_line_parser(argc, argv)
              .options(this->local_opts_)
              .allow_unregistered()
              .run(),
            this->vm_);
  po::notify(this->vm_);
}

int
ifm3d::PasswdApp::Run()
{
  if (this->vm_.count("help"))
    {
      this->_LocalHelp();
      return 0;
    }

  auto const new_password = vm_.count("new") ? true : false;
  auto const disable =
    vm_.count("disable") ? vm_["disable"].as<bool>() : false;
  std::string password = "";

  if (new_password && disable)
    {
      std::cerr << "invalid option combination" << std::endl;
      throw po::validation_error(po::validation_error::invalid_option);
    }
  else if (new_password)
    {
      password = this->vm_["new"].as<std::string>();
      this->cam_->SetPassword(password);
    }
  else if (disable)
    {
      this->cam_->SetPassword(password);
    }
  return 0;
}
