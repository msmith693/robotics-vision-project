// Copyright 2023 ros2_control Development Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ros2_control_demo_example_7/r6bot_hardware.hpp"
#include <cstddef>
#include <libserial/SerialPortConstants.h>
// #include <rclcpp/logger.hpp>
#include <rclcpp/utilities.hpp>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include <libserial/SerialPort.h>

namespace ros2_control_demo_example_7 {
CallbackReturn
RobotSystem::on_init(const hardware_interface::HardwareInfo &info) {
  if (hardware_interface::SystemInterface::on_init(info) !=
      CallbackReturn::SUCCESS) {
    return CallbackReturn::ERROR;
  }

  // cfg_ struct available system wide (placed in header). Allows parameters to
  // be accessed from anywhere within the class
  cfg_.joint_0_name = info_.hardware_parameters["joint_0_name"];
  cfg_.joint_1_name = info_.hardware_parameters["joint_1_name"];
  cfg_.joint_2_name = info_.hardware_parameters["joint_2_name"];
  cfg_.joint_3_name = info_.hardware_parameters["joint_3_name"];
  cfg_.joint_4_name = info_.hardware_parameters["joint_4_name"];
  cfg_.joint_5_name = info_.hardware_parameters["joint_5_name"];
  cfg_.loop_rate = std::stof(info_.hardware_parameters["loop_rate"]);
  cfg_.device = info_.hardware_parameters["device"];
  cfg_.timeout_ms = std::stoi(info_.hardware_parameters["timeout_ms"]);
  cfg_.step_counts_per_rad =
      std::stof(info_.hardware_parameters["step_counts_per_rad"]);
  cfg_.joint_0_reduction =
      std::stof(info_.hardware_parameters["joint_0_reduction"]);
  cfg_.joint_1_reduction =
      std::stof(info_.hardware_parameters["joint_1_reduction"]);
  cfg_.joint_2_reduction =
      std::stof(info_.hardware_parameters["joint_2_reduction"]);
  cfg_.joint_3_reduction =
      std::stof(info_.hardware_parameters["joint_3_reduction"]);
  cfg_.joint_4_reduction =
      std::stof(info_.hardware_parameters["joint_4_reduction"]);
  cfg_.joint_5_reduction =
      std::stof(info_.hardware_parameters["joint_5_reduction"]);

  // robot has 6 joints and 2 interfaces
  joint_position_.assign(6, 0);
  joint_velocity_.assign(6, 0);
  joint_position_command_.assign(6, 0);

  joint_reductions_ = {cfg_.joint_0_reduction, cfg_.joint_1_reduction,
                       cfg_.joint_2_reduction, cfg_.joint_3_reduction,
                       cfg_.joint_4_reduction, cfg_.joint_5_reduction};

  // Populate all state interfaces with joint names
  for (const auto &joint : info_.joints) {
    for (const auto &interface : joint.state_interfaces) {
      joint_interfaces[interface.name].push_back(joint.name);
    }
  }

  return CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface>
RobotSystem::export_state_interfaces() {
  std::vector<hardware_interface::StateInterface> state_interfaces;

  int ind1 = 0;
  for (const auto &joint_name : joint_interfaces["position"]) {
    state_interfaces.emplace_back(joint_name, "position",
                                  &joint_position_[ind1++]);
  }

  int ind2 = 0;
  for (const auto &joint_name : joint_interfaces["velocity"]) {
    state_interfaces.emplace_back(joint_name, "velocity",
                                  &joint_velocity_[ind2++]);
  }

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface>
RobotSystem::export_command_interfaces() {
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  int ind = 0;
  for (const auto &joint_name : joint_interfaces["position"]) {
    command_interfaces.emplace_back(joint_name, "position",
                                    &joint_position_command_[ind++]);
  }
  return command_interfaces;
}

hardware_interface::CallbackReturn
RobotSystem::on_activate(const rclcpp_lifecycle::State & /*previous_state*/) {
  RCLCPP_INFO(get_logger(), "Activating... please wait...");
  try {
    // Open Serial Port
    serial_port_.Open(cfg_.device);
    serial_port_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
    RCLCPP_INFO(get_logger(), "Port opened, waiting for Arduino reset...");
    rclcpp::sleep_for(std::chrono::seconds(2));
    RCLCPP_INFO(get_logger(), "Wrote initial command to Arduino.");
    serial_port_.Write("0,0,0,0,0,0\n");
  } catch (const std::exception &e) {
    RCLCPP_ERROR(get_logger(), "Activation failed: %s", e.what());
    return hardware_interface::CallbackReturn::ERROR;
  }
  RCLCPP_INFO(get_logger(), "System successfully activated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn
RobotSystem::on_deactivate(const rclcpp_lifecycle::State & /*previous_state*/) {
  RCLCPP_INFO(get_logger(), "Deactivating... please wait...");
  serial_port_.Close();
  RCLCPP_INFO(get_logger(), "Successfully deactivated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

return_type RobotSystem::read(
    const rclcpp::Time &,
    const rclcpp::Duration &period) { // Period object already created by
                                      // controller manager. Can just use.
  if (!serial_port_.IsOpen()) {
    RCLCPP_ERROR(get_logger(), "Arduino connection lost.");
    return hardware_interface::return_type::ERROR;
  }
  // string to be filled with serial info
  std::string received_joint_poses;
  serial_port_.ReadLine(received_joint_poses, '\n', cfg_.timeout_ms);

  // Rate limit the position updates. This is WRONG, causes read timeout 
  // if ((time.nanoseconds() - prev_read_log_time) >=
  //     2'000'000'000) { // position updates every half second
  RCLCPP_INFO(get_logger(), "Incoming serial message: '%s'",
              received_joint_poses.c_str());
  //   prev_read_log_time = time.nanoseconds();
  // }

  // store time since joint state interfaces were last updated
  double delta_seconds = period.seconds();

  std::string delimiter = ",";
  size_t del_pos = received_joint_poses.find(delimiter);

  // Update joint_position_ vector with joint poses from serial connection
  for (size_t i = 0; i < joint_position_.size(); i++) {
    // Store previous position and time elapsed to calculate velocity of joint

    double pos_prev = joint_position_[i];

    double joint_pos = std::stod(received_joint_poses.substr(0, del_pos));
    double joint_pos_rad =
        joint_pos / (cfg_.step_counts_per_rad * joint_reductions_[i]);
    // RCLCPP_INFO(get_logger(), "Parsed joint pos in degs: '%s'",
    //             std::to_string(joint_pos_rad*deg_per_rad).c_str());
    double joint_vel = (joint_pos_rad - pos_prev) / delta_seconds;

    // Update state_interfaces
    if (i == 3) {
      joint_position_[i] = -joint_pos_rad;
      joint_velocity_[i] = -joint_vel;
    } else {
      joint_position_[i] = -joint_pos_rad;
      joint_velocity_[i] = -joint_vel;
    }

    received_joint_poses.erase(0, del_pos + delimiter.length());
    del_pos = received_joint_poses.find(delimiter);
  }

  return return_type::OK;
}

return_type RobotSystem::write(const rclcpp::Time &,
                               const rclcpp::Duration &) {

  if (!serial_port_.IsOpen()) {
    return hardware_interface::return_type::ERROR;
  }

  std::string command;

  for (size_t i = 0; i < joint_position_command_.size(); i++) {
    // COndiitonal to flip the commands for certain joints - joint 3 is the only
    // 'normal' joint.
    if (i == 3) {
      command +=
          std::to_string(-(joint_position_command_[i] *
                           cfg_.step_counts_per_rad * joint_reductions_[i]));
    } else {
      command +=
          std::to_string(-(joint_position_command_[i] *
                           cfg_.step_counts_per_rad * joint_reductions_[i]));
    }
    // If on last joint in joint_position vector, append \n
    if (i == joint_position_command_.size() - 1) {
      command += '\n';
    } else {
      command += ',';
    }
  }
  // if ((time.nanoseconds() - prev_write_log_time) >=
  // 2'000'000'000) { // position updates every half second
  RCLCPP_INFO(get_logger(), "Message to be sent to arduino: '%s'",
              command.c_str());
  // prev_write_log_time = time.nanoseconds();
  // }
  serial_port_.Write(command);

  return return_type::OK;
}

} // namespace ros2_control_demo_example_7

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(ros2_control_demo_example_7::RobotSystem,
                       hardware_interface::SystemInterface)
