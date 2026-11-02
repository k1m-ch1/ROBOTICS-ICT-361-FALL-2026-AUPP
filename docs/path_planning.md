# Pure Pursuit Algorithm

A simple path planning algorithm requires just one way point from the robot's reference frame, and then we construct a circle that crosses that point, given that you are currently at the origin $(0, 0)$. We assume that the robot maneuvers on a 2d plane for now.

The radius will be of length $R$, and our waypoint will be $W = (x_w, y_w)$.

We want to construct a circle which has the equation:

$$
(x - R)^2 + y^2 = R^2
$$

We want to determine $R$, given that it must pass through the point $W = (x_w, y_w)$. We want to determine $R$ in terms of $x_w$ and $y_w$.

So we get:

$$
(x_w - R)^2 + y_w^2 = R^2
$$

So we must write $R$ in terms of $x_w$ and $y_w$.

$$
(x_w - R)^2 - R^2 = - y_w^2
$$

$$
(x_w - R + R)(x_w - R - R) = - y_w^2
$$

$$
x_w(x_w - 2R) = - y_w^2
$$

$$
x_w^2 + y_w^2 = 2R x_w
$$

$$
R = \frac{x_w^2 + y_w^2}{2 x_w}
$$

# Making the robot follow the curve of radius $R$

So how do we actually do this?

Well, we can't do it exactly, but for a differential drive robot (without encoders also), we make an approximation such that the robot somewhat follows it.

We'll first derive it in terms of pure kinematics (using velocity, angular velocity, and the center of mass to the wheel distance), and then we'll make the approximations to what we actually control.

Right now, we can control the robot's linear speed $v_l = \left| \vec{v} \right|$, and also the angular velocity $\dot{\theta}$ separately.

If we want the robot to move in a circle, we essentially want the following relation

$$
v_l = R \dot{\theta}
$$

If we control the velocity $v$, then to get $\dot{\theta}$:

$$
\dot{\theta} = \frac{v_l}{R}
$$

NOTE: you must worry about signs as well

# Approximations

So we don't actually control the linear velocity $v$, and angular velocity $\dot{\theta}$, rather, we control a PWM signal $u_l$ for linear velocity and $u_a$ for angular velocity.

What we want to do now is make sure the unit scaling is reasonable, such that we turn in a way that approximates theoretical curve we generate.

Let's say that we don't care about air resistance, or the fact that using 4 motors would allow the car to counteract all of the resistances more than using just 1 motor. That is to say, let's say that when we apply a pwm signal $u_m$ to any of the motor, we get the same velocity $v_m$, and if we apply the pwm signal to all 4 motors, each motor would still have the same velocity $v_m$. Also, $u_m$ will be normalized, and will remove dead zone also, such that the scaling is approximately linear.

We apply a linear approximation that says:

$$
v_m = K_m u_m
$$

And so we'll state that:

$$
v_l = K_m u_l
$$

Because we ignore like, all of the resistances and stuff.

Now, for angular velocity, we essentially require one parameter $r$ which is the distance between one of the motor to the center of axel of the robot. For 4 motor differential drive (slip is guaranteed, even on a theoretical model), we'll approximately state:

$$
v_a = r \dot{\theta}
$$

Where $v_a$ is the additional velocity of the wheel which the robot to rotate in the x-z plane.

Using the linear approximation:

$$
K_m u_a = r \dot{\theta} \implies u_a = \frac{r}{K_m} \dot{\theta}
$$

Thus, given theoretical values for $v$, or the pwm signal $u_l$, and the waypoints, we can get calculate $u_a$ also and send that to the mixer to get the car to move in approximately the curve we generated.

# Summary

Thus, our final equations are, given $u_l$, $W = (x_w, y_w)$, we can determine:

First, we can calculate the path's circle radius

$$
R = \frac{x_w^2 + y_w^2}{2 x_w}
$$

Then, we can calculate the angular "effort":

$$
u_a = \frac{r}{K_m} \dot{\theta}
$$

But to follow a circular path, our $\dot{\theta}$ must follow the relation:

$$
v_l = R \dot{\theta} = K_m u_l
$$

Thus:

$$
\dot{\theta} = \frac{K_m u_l}{R}
$$

And so:

$$
u_a = \frac{r}{K_m} \times \frac{K_m u_l}{R} = \frac{r u_l}{R}
$$

Note that these parameters doesn't necessarily need to reflect the actual measurements, because we're doing quite a lot of approximations, thus, these equations simply just give us some sort of structure.

# Final equations

Given $u_l$ and $W = (x_w, y_w)$

$$
R = \frac{x_w^2 + y_w^2}{2 x_w}
$$

and:

$$
u_a = \frac{r u_l}{R}
$$

NOTE: on the robot, we can set different speeds for linear and angular velocity, but to make this work, we'll need to set the maximum linear and angular velocity to be the same.

# Implementation details

So it might be possible that $x_w \approx 0$, which may cause like overflows, so we'll use the curvature $\kappa$ instead

$$
\kappa = \frac{1}{R}
$$

thus:

$$
\kappa = \frac{2 x_w}{x_2^2 + y_w^2}
$$

And so:

$$
u_a = r \kappa u_l
$$
