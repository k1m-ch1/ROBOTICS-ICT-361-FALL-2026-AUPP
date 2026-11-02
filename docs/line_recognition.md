# Line masking

First need to apply a mask over that which we consider a pixel belonging to the line.

We also need some heuristics to determine whether we're seeing any line such that we force stop, which I suppose can be done using something like the proportion of pixels belonging to a line and the overall pixels.

Anyway, in order to segment which pixels belong to a line, and which pixels doesn't belong to a line, we will first apply an HSV transform to our RGB color space.

Suppose we have an rgb vector $\mathbf{x}$. We convert it to HSV:

$$
\text{hsv}(\mathbf{x})
$$

Where the first parameter will be hue, second parameter will be saturation, and third parameter will be value.

For our purposes of black line detection, our threshold only considers the value parameter.

# Centroid

After we calculate the mask, if our "line density" is high enough, we will then calculate the centroid of the line in order to get a point, and then turn that line into a way point. We then run that point from the image and map it to an approximate point in global space, and feed it into our pure pursuit algorithm.

For discrete masses, the centroid is calculated as such. The center of mass definition states:

$$
M \vec{r_{cm}} = \sum_{i = 1}^{n} m_i \vec{r_i}
$$

In our case, our masses will be defined using the pixel values.

OpenCV can perform this calculation for us using its `cv2.moments` method.

`cv2.moments` returns a dictionary, with the following key definition:

- key `m00` maps to the sum of pixel intensities (total mass)
- key `m01` maps to the first-order moment along the y-axis
- key `m10` maps to the first-order moment along the x-axis

