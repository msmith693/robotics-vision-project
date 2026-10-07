import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32
from robot_interfaces.msg import BatteryStatus

class Dashboard(Node):
    def __init__(self):
        super().__init__('dashboard')
        self.subscription = self.create_subscription(BatteryStatus, 'battery_level', self.battery_callback, 10)

    def battery_callback(self, msg):
        if msg.percentage<20:
            self.get_logger().warning(f'Battery health: {msg.battery_health}')
        else:
            self.get_logger().info(f'Dashboard sees: {msg.percentage:.1f}%')

def main(args = None):
    rclpy.init(args = args)
    rclpy.spin(Dashboard())
    rclpy.shutdown()

if __name__ == '__main__':
    main()

