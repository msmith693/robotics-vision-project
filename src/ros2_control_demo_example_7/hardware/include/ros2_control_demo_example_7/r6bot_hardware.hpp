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

#ifndef ROS2_CONTROL_DEMO_EXAMPLE_7__R6BOT_HARDWARE_HPP_
#define ROS2_CONTROL_DEMO_EXAMPLE_7__R6BOT_HARDWARE_HPP_

#include "string"
#include "unordered_map"
#include "vector"

#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/duration.hpp"

#include "rclcpp/macros.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"
#include "ros2_control_demo_example_7/arduino_comms.hpp"
#include "ros2_control_demo_example_7/joint.hpp"
#include <rclcpp/logger.hpp>

#include <libserial/SerialPort.h>

using hardware_interface::return_type;

namespace ros2_control_demo_example_7 {
using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class RobotSystem : public hardware_interface::SystemInterface {

  struct Config {
    std::string joint_0_name = "";
    std::string joint_1_name = "";
    std::string joint_2_name = "";
    std::string joint_3_name = "";
    std::string joint_4_name = "";
    std::string joint_5_name = "";
    float loop_rate = 0.0;
    std::string device = "";
    // int baud_rate = 0;
    int timeout_ms = 0;
    int step_counts_per_rad = 0;
    float joint_0_reduction = 0;
    float joint_1_reduction = 0;
    float joint_2_reduction = 0;
    float joint_3_reduction = 0;
    float joint_4_reduction = 0;
    float joint_5_reduction = 0;
  };

  // Creating own helper method get_logger() to account for the fact that
  // SystemInterface class does not have a get_logger() method.
  rclcpp::Logger get_logger() { return rclcpp::get_logger("R6Bot"); }

public:
  CallbackReturn on_init(const hardware_interface::HardwareInfo &info) override;

  CallbackReturn
  on_activate(const rclcpp_lifecycle::State &previous_state) override;

  CallbackReturn
  on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

  std::vector<hardware_interface::StateInterface>
  export_state_interfaces() override;

  std::vector<hardware_interface::CommandInterface>
  export_command_interfaces() override;

  return_type read(const rclcpp::Time &time,
                   const rclcpp::Duration &period) override;

  return_type write(const rclcpp::Time & /*time*/,
                    const rclcpp::Duration & /*period*/) override;

private:
  // ArduinoComms comms_;
  Config cfg_;
  
  LibSerial::SerialPort serial_port_;
  //   unsigned int read_timeout_ms_;
  //   /// The size of this vector is (standard_interfaces_.size() x nr_joints)
  std::vector<double> joint_position_;
  std::vector<double> joint_velocity_;
  std::vector<double> joint_position_command_;
  

  std::vector<float> joint_reductions_;

  std::unordered_map<std::string, std::vector<std::string>> joint_interfaces = {
      {"position", {}}, {"velocity", {}}};
};

} // namespace ros2_control_demo_example_7

#endif // ROS2_CONTROL_DEMO_EXAMPLE_7__R6BOT_HARDWARE_HPP_
