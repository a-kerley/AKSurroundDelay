#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include <array>

//==============================================================================
/**
 * SurroundStageView - 3D OpenGL visualization of the surround stage
 * 
 * Features:
 * - Wireframe cuboid room (square floor, shorter height)
 * - Floor grid (4×4)
 * - Listener sphere at center
 * - Mouse-drag rotation (orbit camera)
 * - View preset support with smooth animation
 * 
 * Coordinate system:
 * - X: Left (-1) to Right (+1)
 * - Y: Back (-1) to Front (+1, toward screen/LCR)
 * - Z: Floor (-0.6) to Ceiling (+0.6)
 */
class SurroundStageView : public juce::Component,
                          public juce::OpenGLRenderer,
                          private juce::Timer
{
public:
    //==========================================================================
    // Room dimensions (normalized)
    static constexpr float roomWidth  = 2.0f;   // X: -1 to +1
    static constexpr float roomDepth  = 2.0f;   // Y: -1 to +1
    static constexpr float roomHeight = 1.2f;   // Z: -0.6 to +0.6
    static constexpr float halfHeight = 0.6f;
    
    // Grid settings
    static constexpr int gridDivisions = 4;
    
    // Listener sphere settings (centered in room)
    static constexpr int sphereSegments = 8;     // Minimal wireframe
    static constexpr float sphereRadius = 0.08f;  // Small listener marker
    
    //==========================================================================
    // View presets
    enum class ViewPreset
    {
        Angle,  // Default isometric-ish view
        Left,
        Top,
        Right,
        Back,
        Custom  // User-rotated
    };
    
    //==========================================================================
    SurroundStageView();
    ~SurroundStageView() override;
    
    //==========================================================================
    // OpenGLRenderer callbacks
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
    
    //==========================================================================
    // Component overrides
    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    
    //==========================================================================
    // View control
    void setViewPreset (ViewPreset preset);
    ViewPreset getCurrentPreset() const { return currentPreset; }
    
    // Camera access for external controls
    float getAzimuth() const { return azimuth; }
    float getElevation() const { return elevation; }
    float getZoom() const { return zoom; }
    
    void setAzimuth (float degrees) { azimuth = degrees; currentPreset = ViewPreset::Custom; }
    void setElevation (float degrees) { elevation = degrees; currentPreset = ViewPreset::Custom; }
    void setZoom (float z) { zoom = juce::jlimit (minZoom, maxZoom, z); }

private:
    //==========================================================================
    // OpenGL context
    juce::OpenGLContext openGLContext;
    
    //==========================================================================
    // Shader program
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    
    // Uniform locations
    GLint uniformProjectionMatrix = -1;
    GLint uniformViewMatrix = -1;
    GLint uniformModelMatrix = -1;
    GLint uniformColor = -1;
    
    // Attribute location
    GLint attribPosition = -1;
    
    //==========================================================================
    // Geometry buffers (VBO only, no VAO for macOS compatibility)
    GLuint roomWallsVBO = 0;    // Semi-transparent wall faces (triangles)
    int roomWallsVertexCount = 0;
    
    GLuint roomEdgesVBO = 0;    // Wireframe edges (lines)
    int roomEdgesVertexCount = 0;
    
    GLuint gridVBO = 0;
    int gridVertexCount = 0;
    
    GLuint sphereVBO = 0;
    int sphereVertexCount = 0;
    
    GLuint frontLabelVBO = 0;
    int frontLabelVertexCount = 0;
    
    //==========================================================================
    // Camera state
    float azimuth = 225.0f;     // Horizontal rotation (degrees) - back-left view
    float elevation = 30.0f;    // Vertical rotation (degrees)
    float zoom = 4.5f;          // Camera distance (increased for better view)
    
    static constexpr float minZoom = 2.0f;
    static constexpr float maxZoom = 10.0f;
    
    // Mouse drag tracking
    juce::Point<float> lastMousePos;
    float dragSensitivity = 0.5f;
    
    // Current view preset
    ViewPreset currentPreset = ViewPreset::Angle;
    
    //==========================================================================
    // Animation state
    bool isAnimating = false;
    float animationProgress = 0.0f;
    float animationSpeed = 0.08f;  // Speed of animation (0-1 per frame)
    float animationStartAzimuth = 0.0f;
    float animationStartElevation = 0.0f;
    float animationTargetAzimuth = 0.0f;
    float animationTargetElevation = 0.0f;
    
    // Timer callback for animation
    void timerCallback() override;
    
    //==========================================================================
    // Geometry generation
    void createRoomWallsGeometry();  // Solid wall faces for depth
    void createRoomEdgesGeometry();  // Wireframe edges
    void createGridGeometry();
    void createSphereGeometry();
    void createFrontLabelGeometry();
    
    //==========================================================================
    // Matrix calculations
    juce::Matrix3D<float> getProjectionMatrix() const;
    juce::Matrix3D<float> getViewMatrix() const;
    
    //==========================================================================
    // Shader source
    static const char* vertexShaderSource;
    static const char* fragmentShaderSource;
    
    //==========================================================================
    // Helper to create shader program
    bool createShaders();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SurroundStageView)
};
