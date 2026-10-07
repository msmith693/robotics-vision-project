import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32
from robot_interfaces.msg import BatteryStatus 
import random

class BatteryMonitor(Node):
    def __init__(self):
        super().__init__('battery_monitor')
        self.publisher_ = self.create_publisher(BatteryStatus, 'battery_level', 10)
        
        self.timer = self.create_timer(1.0, self.publish_battery)
        self.level = 100.0

    def publish_battery(self):
        self.level = max(0.0, self.level-random.uniform(5, 10))
        msg = BatteryStatus()
        msg.percentage = self.level
        msg.voltage = 24.1
        msg.is_charging = False
        msg.battery_health = 'Good'
        if self.level < 20:
            msg.battery_health = 'Warning'
        self.publisher_.publish(msg)
        self.get_logger().info(f'Battery: {self.level:.1f}%')

def main(args = None):
    rclpy.init(args=args)
    rclpy.spin(BatteryMonitor())
    rclpy.shutdown()


if __name__ == '__main__':
    main()
