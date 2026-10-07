from setuptools import find_packages, setup
import os
from glob import glob

package_name = 'robot_bringup'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    # Where to put files, what to copy pair
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*launch.[pxy][yma]*'))),
        (os.path.join('share', package_name, 'config'), glob(os.path.join('config', 'motor_params.yaml'))),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='msmith',
    maintainer_email='msmith@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'simple_node = robot_bringup.simple_node:main',
            'battery_monitor = robot_bringup.battery_monitor:main',
            'dashboard = robot_bringup.dashboard:main',
            'motor_controller = robot_bringup.motor_controller:main',
            'shelf_dispatcher = robot_bringup.shelf_dispatched.main',
        ],
    },
)
