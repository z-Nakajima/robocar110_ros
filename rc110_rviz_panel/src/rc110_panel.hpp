/*
 * Copyright (C) 2020 ZMP Inc info@zmp.co.jp
 * All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *
 * Written by Andrei Pak
 */
#pragma once

#ifndef Q_MOC_RUN
#include <ackermann_msgs/AckermannDriveStamped.h>
#include <nav_msgs/Odometry.h>
#include <ros/ros.h>
#include <rviz/panel.h>
#include <sensor_msgs/BatteryState.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Temperature.h>
#endif

namespace Ui
{
class PanelWidget;
}
class QTreeWidgetItem;
class QStatusBar;

namespace zmp
{
/**
 * Widget with RC 1/10 sensors that can be displayed in rviz.
 */
class Rc110Panel : public rviz::Panel
{
    Q_OBJECT

    enum TREE_ITEM_GROUP { DRIVE, BATTERY, TEMPERATURE, IMU };
    static constexpr const char* TREE_ITEM_GROUP_NAMES[] = {"Drive", "Battery", "Temperature", "IMU"};

public:
    explicit Rc110Panel(QWidget* parent = nullptr);
    ~Rc110Panel() override;

private:
	void onEnableBoard(bool on);
    void setBoardStatus(uint8_t request);
    QTreeWidgetItem* getTreeItem(TREE_ITEM_GROUP group, const char* name) const;
    void onDriveStatus(const ackermann_msgs::AckermannDriveStamped& driveStatus);
    void onOdometry(const nav_msgs::Odometry& odometry);
    void onServoBattery(const sensor_msgs::BatteryState& batteryState);
    void onMotorBattery(const sensor_msgs::BatteryState& batteryState);
    void onBaseboardTemperature(const sensor_msgs::Temperature& temperature);
    void onServoTemperature(const sensor_msgs::Temperature& temperature);
    void onImu(const sensor_msgs::Imu& imu);

private:
    std::unique_ptr<Ui::PanelWidget> ui;
    ros::NodeHandle handle;
    QVector<ros::Subscriber> subscribers;
    QHash<TREE_ITEM_GROUP, QTreeWidgetItem*> treeItems;
    QStatusBar* statusBar;
};
}  // namespace zmp