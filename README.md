#Ray tracing and path tracing (computer graphics)#

To render an image using ray tracing, we need to define camera, geometry object and light source. Then we can render every pixel by tracing rays from camera.

First is to design the camera class, which should include its 3d position, 3d orientation (lookat & up direction). As well as an image plane to record the projected image. I assume the image plane is perpendicular to the camera's lookat direction, I also set image plane's width, height and resolution. After that, we can generate rays from camera.

Then I detect whether the emitted rays will intersect with geometry objects, if the result is true, I further calculate the color of the hit point. To do the hit test, I define a virtual function rayHitTest in the geometry base class, all geometry objects inherit from this base class. To accelerate the rayHitTest in the mesh class, I split 3d space using kdTree(?) and sperate its faces into different cells.

Finally I calculate color of the hit point. This code can have either point lights or cube map textures in the lighting environment. For the texture light, I sample small area lights according to the texture's radiance. The final color consists of diffuse light and specular light. For the reflection light, I do not accomplish the global lighting, I only calculate one reflection ray, and I set the maximum recursion depth to 3.
