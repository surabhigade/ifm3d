/*
 * Copyright 2021-present ifm electronic, gmbh
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <fstream>
#include <ifm3d/camera/camera_o3r.h>

// Path to config files are assuming you are running this example from the /build folder.

int main(){

    // Create the camera object
    auto cam = std::make_shared<ifm3d::O3RCamera>();

    // Get the current configuration of the camera in JSON format
    json conf = cam->Get();

    // Display the current configuration
    std::cout << std::setw(4) << conf << std::endl;

    // Configure the device from a json string
    cam->Set(R"("{"device":{"info": {"name": "my_new_o3r"}}}")");
    // Make the configuration persistent
    cam->SaveInit();

    return 0;
}