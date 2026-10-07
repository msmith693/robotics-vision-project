from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pkg_path = get_package_share_directory('robot_description')
    xacro_file = os.path.join(pkg_path, 'urdf', 'robot.urdf.xacro')
    # Command() object expands my xacro file into urdf, which is turned into a string and assigned to robot_description
    # This is how the urdf is fed into the robot_state_publisher Node, as a parameter.
    robot_description = ParameterValue(Command(['xacro ', xacro_file]), value_type = str)

    return LaunchDescription([
        Node(
            package='robot_state_publisher',
            executable = 'robot_state_publisher',
            parameters=[{'robot_description': robot_description}],
        ),
        Node(
            package = 'joint_state_publisher_gui',
            executable = 'joint_state_publisher_gui',
        ),
        Node(
            package='rviz2',
            executable = 'rviz2',
            output = 'screen',
        ),
    ])                                