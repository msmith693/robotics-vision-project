import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    params_file = os.path.join(
        get_package_share_directory('robot_bringup'),
        'config', 'motor_params.yaml')

    return LaunchDescription([
        Node(
            package = 'robot_bringup',
            executable = 'battery_monitor',
        ),
        Node(
            package = 'robot_bringup',
            executable = 'dashboard',
        ),
        Node(
            package = 'robot_bringup',
            executable = 'motor_controller',
            parameters = [params_file],
        ),
    ])