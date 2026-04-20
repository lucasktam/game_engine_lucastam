#ifndef CAMERA_H
#define CAMERA_H

class Camera {
public:
    static void SetPosition(float x, float y);
    static float GetPositionX();
    static float GetPositionY();
    
    static void SetZoom(float zoom_factor);
    static float GetZoom();

    // Helper for the Renderer
    static float x;
    static float y;
    static float zoom;
};

#endif