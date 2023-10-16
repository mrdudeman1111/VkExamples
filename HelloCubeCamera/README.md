    Changes:
        Added camera class.
            Shown in main() render loop.
        added more lines to Descriptor creation.

Cameras are complicated structures, every camera consists of the basic WVP, but in addition, they contain a special matrix called the "Camera Matrix".

        The view matrix doesn't actually move the camera around space, it move space around the camera. That sounds weird but when you look at what happens
    when you transform a 3D vector with the WVP Matrices it will make sense.
     When going through the steps of Perspective projection, you place the Vector on the world matrix, because this is an identity most of the time it won't translate, rotate, or
    scale the vector, afterwards, you transform the vector by the view matrix. When you transform by the view matrix, the returned value will be the Vector's position RELATIVE
    to the position of the camera, we are Translating it from WORLD SPACE(relative to the scene's origin aka 0,0,0) to CAMERA SPACE(relative to the camera's position)
    The result of this transformation is still in 3D, (It has a position like (12.23,120.28,63.93) or some other arbitrary position). We can't render the mesh to the screen
    until we have the Meshes' Vertex positions on the screen so that we can assemble the polygons on screen with minimal effort. To normalize the Vertices to the screen,
    we use the Projection matrix. When we transform by the projection matrix it returns the Position of the Vertex on the screen, in 2D coordinates, so that we can find exactly 
    what pixel it occupies on the screen. After that we fill in the faces and edges and send it off to the Fragment Shader to be filled in and colored.

    The World Matrix is the scene origin, 
    the View Matrix is the camera,
    and the Projection matrix is the Lens and applies distortion to the final image to make it look natural.

    Then there's the Camera's Matrix, which contains it's Translation, Rotation, and Scale. This is a lot easier to understand, you use this matrix to move the Camera
    throughout space. We can translate the camera matrix with glm::translate() to move the camera and rotate the camera matrix with glm::rotate() to re-orient the camera to face 
    something. For example's sake, let's say the camera matrix has been translated by 10 units down the Z (0,0,10), If we were to transform a 0,0,0 vector by the camera matrix,
    the resulting vector would be (0,0,10). But if we translated that same vector by the inverse of the camera matrix (the view matrix), it would return the vector's position
    relative to the Camera's position, aka (0,0,-10). That seems simple when it's just translation, but when you add orientation, it gets a lot more complicated. For that reason
    we have the camera matrix, because we can translate it and rotate it to the correct position and orientation and then just invert it. It's easier to debug and visualize this
    way.
