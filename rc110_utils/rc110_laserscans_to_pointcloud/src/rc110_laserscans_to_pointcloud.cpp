/*
 * Copyright (C) 2021 ZMP Inc info@zmp.co.jp
 *
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 *
 * Written by Andrei Pak
 */
#include "rc110_laserscans_to_pointcloud.hpp"

#include <sensor_msgs/point_cloud2_iterator.h>

namespace zmp
{
constexpr char BASE_FRAME_ID[] = "base_link";
constexpr char FRONT_LIDAR_FRAME_ID[] = "front_lidar";
constexpr char REAR_LIDAR_FRAME_ID[] = "rear_lidar";

namespace
{
/*
 * Add points from cloud to points, skipping ones that are too close.
 */
void addUniquePoints(const sensor_msgs::PointCloud2& cloud,
                     std::map<float, std::vector<uint8_t>>& points,
                     float angleAllowance)
{
    int pointSize = cloud.point_step;
    using Iter = sensor_msgs::PointCloud2ConstIterator<float>;
    for (Iter it = {cloud, "x"}; it != it.end(); ++it) {
        float x = it[0], y = it[1];
        float angle = atan2(y, x);

        auto nextIt = points.lower_bound(angle);
        if (nextIt != points.end() && angle + angleAllowance > nextIt->first) {
            continue;
        }
        if (nextIt != points.begin() && angle - angleAllowance < (--nextIt)->first) {
            continue;
        }

        auto pointBegin = reinterpret_cast<const uint8_t*>(&it[0]);
        points.emplace(angle, std::vector<uint8_t>(pointBegin, pointBegin + pointSize));
    }
}
}  // namespace

Rc110LaserScansToPointCloud::Rc110LaserScansToPointCloud()
{
    handlePrivate.param<std::string>("base_frame", baseFrame, BASE_FRAME_ID);
    handlePrivate.param<std::string>("front_lidar_frame", frontLidarFrame, FRONT_LIDAR_FRAME_ID);
    handlePrivate.param<std::string>("rear_lidar_frame", rearLidarFrame, REAR_LIDAR_FRAME_ID);

    frontTransformer.waitForTransform(baseFrame, frontLidarFrame, ros::Time(0), ros::Duration(5));
    rearTransformer.waitForTransform(baseFrame, rearLidarFrame, ros::Time(0), ros::Duration(5));

    sync = std::make_unique<ApproxSync>(ApproxSyncPolicy(3), frontSubscriber, rearSubscriber);
    sync->registerCallback(&Rc110LaserScansToPointCloud::onScans, this);

    auto onConnectCallback = [this](const ros::SingleSubscriberPublisher&) { onConnect(); };
    cloudPublisher = handle.advertise<sensor_msgs::PointCloud2>("points2", 1, onConnectCallback, onConnectCallback);
}

void Rc110LaserScansToPointCloud::onConnect()
{
    if (cloudPublisher.getNumSubscribers()) {
        if (!subscribed) {
            subscribed = true;
            frontSubscriber.subscribe(handle, "front_scan", 1);
            rearSubscriber.subscribe(handle, "rear_scan", 1);
        }
    } else {
        subscribed = false;
        frontSubscriber.unsubscribe();
        rearSubscriber.unsubscribe();
    }
}

void Rc110LaserScansToPointCloud::onScans(const sensor_msgs::LaserScan::ConstPtr& front,
                                          const sensor_msgs::LaserScan::ConstPtr& rear)
{
    if (front->header.frame_id != frontLidarFrame || rear->header.frame_id != rearLidarFrame) {
        ROS_ERROR_STREAM("Wrong scan frame id: " << front->header.frame_id << ", " << rear->header.frame_id);
        return;
    }
    float angleAllowance = front->angle_increment * 0.8;  // 80%

    sensor_msgs::PointCloud2 frontCloud, rearCloud;
    projection.transformLaserScanToPointCloud(baseFrame, *front, frontCloud, frontTransformer);
    projection.transformLaserScanToPointCloud(baseFrame, *rear, rearCloud, rearTransformer);

    // unique and sorted by angle points (-pi, pi]
    std::map<float, std::vector<uint8_t>> points;
    addUniquePoints(rearCloud, points, angleAllowance);
    addUniquePoints(frontCloud, points, angleAllowance);
    // \ Rear lidar as main gives a bit better result as coordinates origin is near it
    // \ , so all front lidar points that intersect with it are discarded.

    std::vector<uint8_t> data;
    for (auto& point : points) {
        data.insert(data.end(), point.second.begin(), point.second.end());
    }

    rearCloud.width = points.size();
    rearCloud.row_step = points.size() * rearCloud.point_step;
    rearCloud.data = data;
    cloudPublisher.publish(rearCloud);
}
}  // namespace zmp
