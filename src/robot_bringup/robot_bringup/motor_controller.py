import rclpy
import time
from rclpy.node import Node
from robot_interfaces.srv import ResetEncoder
from rclpy.action import ActionServer
from robot_interfaces.action import MoveToShelf

class MotorController(Node):
    def __init__(self):
        super().__init__("motor_controller")
        self.declare_parameter('max_speed', 0.8)
        self.declare_parameter('wheel_radius', 0.05)
        self.encoder_ticks = 15420
        self.srv = self.create_service(ResetEncoder, 'reset_encoder', self.handle_reset)

        # Action code
        self._action_server = ActionServer(
            self, MoveToShelf, 'move_to_shelf', self.execute_move)


    def handle_reset(self, request, response):
        self.get_logger().info(f'Resetting encoder from {self.encoder_ticks} ticks')
        self.encoder_ticks = 0
        response.success = True
        response.message = 'Encoder reset to zero'
        return response

    def execute_move(self, goal_handle):
        target = goal_handle.request.shelf_number
        self.get_logger().info(f'Moving to shelf {target}')
        distance = 18.0
        feedback = MoveToShelf.Feedback()

        while distance >0:
            if goal_handle.is_cancel_requested:
                goal_handle.canceled()
                result = MoveToShelf.Result()
                result.success = False
                result.final_message = 'Cancelled mid-route'
                return result

            distance -= 3
            feedback.distance_remaining = max(distance, 0)
            feedback.status = 'moving'
            goal_handle.publish_feedback(feedback)
            time.sleep(0.5)

        goal_handle.succeed()
        result = MoveToShelf.Result()
        result.success = True
        result.final_message = f'Arrived at shelf {target}'
        return result

    def get_max_speed(self):
        return self.get_parameter('max_speed').get_parameter_value().double_value

def main(args = None):
    rclpy.init(args = args)
    rclpy.spin(MotorController())
    rclpy.shutdown()

if __name__ == '__main__':
    main()