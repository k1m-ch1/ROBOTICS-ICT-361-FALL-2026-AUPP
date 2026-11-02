def get_curvature(x_w, y_w):
    # given way points, returns the curvature of the pure pursuit curve
    return (2*x_w)/(x_w**2 + y_w**2)

def get_normalized_angular_effort(
    x_w,
    y_w,
    r
):
    # we assume that the linear effort is 1, and we compute the normalized angular effort
    # r is the distance from one of the motor to the center of the right axel for a 
    # 2 wheeled differential drive robot
    # negative sign is there because, our convention is positive angular velocity 
    # means that the robot will rotate in the ccw direction
    # however, positive curvature means turn right
    return -r*get_curvature(x_w, y_w)
