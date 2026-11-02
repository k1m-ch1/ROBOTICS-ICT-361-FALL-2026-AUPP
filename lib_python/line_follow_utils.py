def image_pixel_to_coordinates_cm(
    x_pixel,
    y_pixel,
    image_width,
    image_height,
    pixel_per_cm
):
    # turns the pixel coordinates into real cartesian coordinates (assuming that the warping is very minimal)
    # we assume that the origin is in the center of the image in the x-axis
    transformed_x_pixel = x_pixel - image_width / 2

    # but the y-axis needs to be transformed
    transformed_y_pixel = image_height - y_pixel
    # now we scale it
    # NOTE: we'll have to shift it in the y-axis also later
    return (transformed_x_pixel/pixel_per_cm, transformed_y_pixel/pixel_per_cm)
