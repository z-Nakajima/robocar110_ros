/*
 * Copyright (C) 2021 ZMP Inc info@zmp.co.jp
 *
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 *
 * Written by Andrei Pak
 */
#include <ros/ros.h>

#include "rc110_laserscans_to_pointcloud.hpp"

using namespace zmp;

int main(int argc, char** argv)
try {
    ros::init(argc, argv, "rc110_laserscans_to_pointcloud");

    ros::NodeHandle nh;
    ros::NodeHandle nhPrivate("~");

    Rc110LaserScansToPointCloud node(nh, nhPrivate);

    ros::spin();

    return EXIT_SUCCESS;
}  //
catch (std::exception& ex) {
    std::cerr << "Exception in main(): " << ex.what() << std::endl;
    return EXIT_FAILURE;
}  //
catch (...) {
    std::cerr << "Unknown exception in main()" << std::endl;
    return EXIT_FAILURE;
}
