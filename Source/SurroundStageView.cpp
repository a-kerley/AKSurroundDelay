#include "SurroundStageView.h"
#include "ColorPalette.h"
#include <cmath>

//==============================================================================
// Shader source code (GLSL 120 for macOS compatibility)
//==============================================================================
const char* SurroundStageView::vertexShaderSource = R"(
    attribute vec3 aPos;
    
    uniform mat4 uProjection;
    uniform mat4 uView;
    uniform mat4 uModel;
    
    void main()
    {
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    }
)";

const char* SurroundStageView::fragmentShaderSource = R"(
    uniform vec4 uColor;
    
    void main()
    {
        gl_FragColor = uColor;
    }
)";

//==============================================================================
SurroundStageView::SurroundStageView()
{
    // Attach OpenGL context to this component
    openGLContext.setRenderer (this);
    openGLContext.setMultisamplingEnabled (true);  // Enable antialiasing
    
    // Request higher quality pixel format with more multisample samples
    juce::OpenGLPixelFormat pixelFormat;
    pixelFormat.multisamplingLevel = 8;  // 8x MSAA for crisp lines
    openGLContext.setPixelFormat (pixelFormat);
    
    openGLContext.attachTo (*this);
    
    // Enable mouse events
    setInterceptsMouseClicks (true, true);
    
    // Set default view
    setViewPreset (ViewPreset::Angle);
}

SurroundStageView::~SurroundStageView()
{
    openGLContext.detach();
}

//==============================================================================
// OpenGL Renderer Callbacks
//==============================================================================
void SurroundStageView::newOpenGLContextCreated()
{
    // Create shaders
    if (!createShaders())
    {
        DBG ("Failed to create shaders!");
        return;
    }
    
    // Get attribute location
    attribPosition = juce::gl::glGetAttribLocation (shaderProgram->getProgramID(), "aPos");
    
    // Create geometry
    createRoomWallsGeometry();
    createRoomEdgesGeometry();
    createGridGeometry();
    createSphereGeometry();
    createFrontLabelGeometry();
    
    // Enable depth testing
    juce::gl::glEnable (juce::gl::GL_DEPTH_TEST);
    
    // Enable blending for transparency
    juce::gl::glEnable (juce::gl::GL_BLEND);
    juce::gl::glBlendFunc (juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable line smoothing
    juce::gl::glEnable (juce::gl::GL_LINE_SMOOTH);
    juce::gl::glHint (juce::gl::GL_LINE_SMOOTH_HINT, juce::gl::GL_NICEST);
    
    // Enable multisampling
    juce::gl::glEnable (juce::gl::GL_MULTISAMPLE);
}

void SurroundStageView::renderOpenGL()
{
    // Clear with viewport background color
    juce::gl::glClearColor (ColorPalette::viewport3DBackground.r, 
                            ColorPalette::viewport3DBackground.g, 
                            ColorPalette::viewport3DBackground.b, 
                            ColorPalette::viewport3DBackground.a);
    juce::gl::glClear (juce::gl::GL_COLOR_BUFFER_BIT | juce::gl::GL_DEPTH_BUFFER_BIT);
    
    if (shaderProgram == nullptr || attribPosition < 0)
        return;
    
    shaderProgram->use();
    
    // Set matrices
    auto projection = getProjectionMatrix();
    auto view = getViewMatrix();
    juce::Matrix3D<float> model; // Identity
    
    if (uniformProjectionMatrix >= 0)
        juce::gl::glUniformMatrix4fv (uniformProjectionMatrix, 1, juce::gl::GL_FALSE, projection.mat);
    if (uniformViewMatrix >= 0)
        juce::gl::glUniformMatrix4fv (uniformViewMatrix, 1, juce::gl::GL_FALSE, view.mat);
    if (uniformModelMatrix >= 0)
        juce::gl::glUniformMatrix4fv (uniformModelMatrix, 1, juce::gl::GL_FALSE, model.mat);
    
    // Set line width
    juce::gl::glLineWidth (1.0f);
    
    // Enable vertex attribute
    juce::gl::glEnableVertexAttribArray ((GLuint)attribPosition);
    
    //==========================================================================
    // DRAW ORDER: Back-to-front for proper transparency
    // 1. Room walls (semi-transparent) - provides depth for occlusion
    // 2. Floor grid
    // 3. Sphere (solid)
    // 4. Room edges (wireframe) - on top of walls
    // 5. FRONT label
    //==========================================================================
    
    //==========================================================================
    // Draw room wall faces (semi-transparent dark grey)
    if (uniformColor >= 0)
        juce::gl::glUniform4f (uniformColor, ColorPalette::roomWallsColour.r, 
                               ColorPalette::roomWallsColour.g, ColorPalette::roomWallsColour.b, 
                               ColorPalette::roomWallsColour.a);
    
    if (roomWallsVBO != 0)
    {
        juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, roomWallsVBO);
        juce::gl::glVertexAttribPointer ((GLuint)attribPosition, 3, juce::gl::GL_FLOAT, 
                                         juce::gl::GL_FALSE, 3 * sizeof(float), nullptr);
        juce::gl::glDrawArrays (juce::gl::GL_TRIANGLES, 0, roomWallsVertexCount);
    }
    
    //==========================================================================
    // Draw floor grid (darker grey)
    if (uniformColor >= 0)
        juce::gl::glUniform4f (uniformColor, ColorPalette::gridColour.r, 
                               ColorPalette::gridColour.g, ColorPalette::gridColour.b, 
                               ColorPalette::gridColour.a);
    
    if (gridVBO != 0)
    {
        juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, gridVBO);
        juce::gl::glVertexAttribPointer ((GLuint)attribPosition, 3, juce::gl::GL_FLOAT, 
                                         juce::gl::GL_FALSE, 3 * sizeof(float), nullptr);
        juce::gl::glDrawArrays (juce::gl::GL_LINES, 0, gridVertexCount);
    }
    
    //==========================================================================
    // Draw listener sphere (dark grey solid)
    if (uniformColor >= 0)
        juce::gl::glUniform4f (uniformColor, ColorPalette::sphereColour.r, 
                               ColorPalette::sphereColour.g, ColorPalette::sphereColour.b, 
                               ColorPalette::sphereColour.a);
    
    if (sphereVBO != 0)
    {
        juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, sphereVBO);
        juce::gl::glVertexAttribPointer ((GLuint)attribPosition, 3, juce::gl::GL_FLOAT, 
                                         juce::gl::GL_FALSE, 3 * sizeof(float), nullptr);
        juce::gl::glDrawArrays (juce::gl::GL_TRIANGLES, 0, sphereVertexCount);
    }
    
    //==========================================================================
    // Draw room wireframe edges (white, on top of walls)
    if (uniformColor >= 0)
        juce::gl::glUniform4f (uniformColor, ColorPalette::roomEdgesColour.r, 
                               ColorPalette::roomEdgesColour.g, ColorPalette::roomEdgesColour.b, 
                               ColorPalette::roomEdgesColour.a);
    
    if (roomEdgesVBO != 0)
    {
        juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, roomEdgesVBO);
        juce::gl::glVertexAttribPointer ((GLuint)attribPosition, 3, juce::gl::GL_FLOAT, 
                                         juce::gl::GL_FALSE, 3 * sizeof(float), nullptr);
        juce::gl::glDrawArrays (juce::gl::GL_LINES, 0, roomEdgesVertexCount);
    }
    
    //==========================================================================
    // Draw "FRONT" label (grey)
    if (uniformColor >= 0)
        juce::gl::glUniform4f (uniformColor, 0.6f, 0.6f, 0.6f, 1.0f);
    
    if (frontLabelVBO != 0)
    {
        juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, frontLabelVBO);
        juce::gl::glVertexAttribPointer ((GLuint)attribPosition, 3, juce::gl::GL_FLOAT, 
                                         juce::gl::GL_FALSE, 3 * sizeof(float), nullptr);
        juce::gl::glDrawArrays (juce::gl::GL_LINES, 0, frontLabelVertexCount);
    }
    
    // Disable vertex attribute
    juce::gl::glDisableVertexAttribArray ((GLuint)attribPosition);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
}

void SurroundStageView::openGLContextClosing()
{
    // Clean up shader
    shaderProgram.reset();
    
    // Clean up VBOs
    if (roomWallsVBO != 0)
        juce::gl::glDeleteBuffers (1, &roomWallsVBO);
    
    if (roomEdgesVBO != 0)
        juce::gl::glDeleteBuffers (1, &roomEdgesVBO);
    
    if (gridVBO != 0)
        juce::gl::glDeleteBuffers (1, &gridVBO);
    
    if (sphereVBO != 0)
        juce::gl::glDeleteBuffers (1, &sphereVBO);
    
    if (frontLabelVBO != 0)
        juce::gl::glDeleteBuffers (1, &frontLabelVBO);
    
    roomWallsVBO = 0;
    roomEdgesVBO = 0;
    gridVBO = 0;
    sphereVBO = 0;
    frontLabelVBO = 0;
}

//==============================================================================
// Component Overrides
//==============================================================================
void SurroundStageView::paint (juce::Graphics& g)
{
    // OpenGL handles all rendering, but we can draw overlays here if needed
    juce::ignoreUnused (g);
}

void SurroundStageView::resized()
{
    // OpenGL viewport is handled automatically by JUCE's OpenGLContext
}

void SurroundStageView::mouseDown (const juce::MouseEvent& event)
{
    lastMousePos = event.position;
}

void SurroundStageView::mouseDrag (const juce::MouseEvent& event)
{
    auto delta = event.position - lastMousePos;
    lastMousePos = event.position;
    
    // Update camera angles
    // Horizontal: drag right = rotate clockwise (azimuth increases)
    azimuth += delta.x * dragSensitivity;
    // Vertical: drag up = look up (elevation increases)
    elevation += delta.y * dragSensitivity;
    
    // Wrap azimuth to 0-360 range
    while (azimuth < 0.0f) azimuth += 360.0f;
    while (azimuth >= 360.0f) azimuth -= 360.0f;
    
    // Clamp elevation to avoid gimbal lock at poles (-89 to +89 degrees)
    elevation = juce::jlimit (-89.0f, 89.0f, elevation);
    
    currentPreset = ViewPreset::Custom;
    
    // Trigger repaint
    repaint();
}

void SurroundStageView::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    juce::ignoreUnused (event);
    
    zoom -= wheel.deltaY * 0.5f;
    zoom = juce::jlimit (minZoom, maxZoom, zoom);
    
    repaint();
}

//==============================================================================
// View Presets
//==============================================================================
void SurroundStageView::setViewPreset (ViewPreset preset)
{
    currentPreset = preset;
    
    float targetAzimuth = azimuth;
    float targetElevation = elevation;
    
    switch (preset)
    {
        case ViewPreset::Angle:
            // Looking from back-left, so front wall is at top-left
            targetAzimuth = 225.0f;  // Back-left corner
            targetElevation = 30.0f;
            break;
            
        case ViewPreset::Left:
            // Looking FROM the left side (camera on left, looking right)
            targetAzimuth = 270.0f;
            targetElevation = 0.0f;
            break;
            
        case ViewPreset::Top:
            // Looking from above, with front wall at top of screen
            targetAzimuth = 180.0f;  // Rotated so +Y (front) is at top
            targetElevation = 89.0f; // Just under 90 to avoid gimbal lock
            break;
            
        case ViewPreset::Right:
            // Looking FROM the right side (camera on right, looking left)
            targetAzimuth = 90.0f;
            targetElevation = 0.0f;
            break;
            
        case ViewPreset::Back:
            // Looking FROM the back (camera behind, looking at front wall)
            targetAzimuth = 180.0f;
            targetElevation = 0.0f;
            break;
            
        case ViewPreset::Custom:
            // Don't change angles for custom
            return;
    }
    
    // Start animation
    animationStartAzimuth = azimuth;
    animationStartElevation = elevation;
    animationTargetAzimuth = targetAzimuth;
    animationTargetElevation = targetElevation;
    animationProgress = 0.0f;
    isAnimating = true;
    
    startTimerHz (60);  // 60 FPS animation
}

void SurroundStageView::timerCallback()
{
    if (!isAnimating)
    {
        stopTimer();
        return;
    }
    
    // Smooth easing animation
    animationProgress += animationSpeed;
    
    if (animationProgress >= 1.0f)
    {
        animationProgress = 1.0f;
        isAnimating = false;
        stopTimer();
    }
    
    // Ease-out cubic
    float t = 1.0f - std::pow (1.0f - animationProgress, 3.0f);
    
    // Handle azimuth wrapping for shortest path
    float azimuthDiff = animationTargetAzimuth - animationStartAzimuth;
    if (azimuthDiff > 180.0f) azimuthDiff -= 360.0f;
    if (azimuthDiff < -180.0f) azimuthDiff += 360.0f;
    
    azimuth = animationStartAzimuth + azimuthDiff * t;
    while (azimuth < 0.0f) azimuth += 360.0f;
    while (azimuth >= 360.0f) azimuth -= 360.0f;
    
    elevation = animationStartElevation + (animationTargetElevation - animationStartElevation) * t;
    
    repaint();
}

//==============================================================================
// Geometry Generation
//==============================================================================
void SurroundStageView::createRoomWallsGeometry()
{
    // Create solid wall faces (triangles) for proper depth testing
    // 6 faces: floor, ceiling, and 4 walls
    const float h = halfHeight;
    
    std::vector<float> vertices;
    
    // Helper to add a quad as 2 triangles
    auto addQuad = [&vertices](float x1, float y1, float z1,
                               float x2, float y2, float z2,
                               float x3, float y3, float z3,
                               float x4, float y4, float z4) {
        // Triangle 1
        vertices.insert(vertices.end(), {x1, y1, z1, x2, y2, z2, x3, y3, z3});
        // Triangle 2
        vertices.insert(vertices.end(), {x1, y1, z1, x3, y3, z3, x4, y4, z4});
    };
    
    // Floor (Z = -h) - skip this, we have the grid
    // addQuad(-1, -1, -h,  1, -1, -h,  1, 1, -h,  -1, 1, -h);
    
    // Ceiling (Z = +h)
    addQuad(-1.0f, -1.0f, h,  -1.0f, 1.0f, h,  1.0f, 1.0f, h,  1.0f, -1.0f, h);
    
    // Back wall (Y = -1)
    addQuad(-1.0f, -1.0f, -h,  -1.0f, -1.0f, h,  1.0f, -1.0f, h,  1.0f, -1.0f, -h);
    
    // Front wall (Y = +1)
    addQuad(-1.0f, 1.0f, -h,  1.0f, 1.0f, -h,  1.0f, 1.0f, h,  -1.0f, 1.0f, h);
    
    // Left wall (X = -1)
    addQuad(-1.0f, -1.0f, -h,  -1.0f, 1.0f, -h,  -1.0f, 1.0f, h,  -1.0f, -1.0f, h);
    
    // Right wall (X = +1)
    addQuad(1.0f, -1.0f, -h,  1.0f, -1.0f, h,  1.0f, 1.0f, h,  1.0f, 1.0f, -h);
    
    roomWallsVertexCount = (int)(vertices.size() / 3);
    
    // Create VBO
    juce::gl::glGenBuffers (1, &roomWallsVBO);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, roomWallsVBO);
    juce::gl::glBufferData (juce::gl::GL_ARRAY_BUFFER, 
                            (GLsizeiptr)(vertices.size() * sizeof(float)), 
                            vertices.data(), 
                            juce::gl::GL_STATIC_DRAW);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
}

void SurroundStageView::createRoomEdgesGeometry()
{
    // 12 edges of the cuboid (wireframe)
    // X: -1 to +1, Y: -1 to +1, Z: -halfHeight to +halfHeight
    const float h = halfHeight;
    
    // Define the 12 edges of the cuboid (24 vertices, 2 per edge)
    std::vector<float> vertices = {
        // Bottom face edges (Z = -h)
        -1.0f, -1.0f, -h,   1.0f, -1.0f, -h,  // Back edge
         1.0f, -1.0f, -h,   1.0f,  1.0f, -h,  // Right edge
         1.0f,  1.0f, -h,  -1.0f,  1.0f, -h,  // Front edge
        -1.0f,  1.0f, -h,  -1.0f, -1.0f, -h,  // Left edge
        
        // Top face edges (Z = +h)
        -1.0f, -1.0f,  h,   1.0f, -1.0f,  h,  // Back edge
         1.0f, -1.0f,  h,   1.0f,  1.0f,  h,  // Right edge
         1.0f,  1.0f,  h,  -1.0f,  1.0f,  h,  // Front edge
        -1.0f,  1.0f,  h,  -1.0f, -1.0f,  h,  // Left edge
        
        // Vertical edges (connecting top and bottom)
        -1.0f, -1.0f, -h,  -1.0f, -1.0f,  h,  // Back-left
         1.0f, -1.0f, -h,   1.0f, -1.0f,  h,  // Back-right
         1.0f,  1.0f, -h,   1.0f,  1.0f,  h,  // Front-right
        -1.0f,  1.0f, -h,  -1.0f,  1.0f,  h,  // Front-left
    };
    
    roomEdgesVertexCount = (int)(vertices.size() / 3);
    
    // Create VBO
    juce::gl::glGenBuffers (1, &roomEdgesVBO);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, roomEdgesVBO);
    juce::gl::glBufferData (juce::gl::GL_ARRAY_BUFFER, 
                            (GLsizeiptr)(vertices.size() * sizeof(float)), 
                            vertices.data(), 
                            juce::gl::GL_STATIC_DRAW);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
}

void SurroundStageView::createGridGeometry()
{
    const float h = -halfHeight; // Floor level
    const int divisions = gridDivisions;
    
    std::vector<float> vertices;
    
    // Grid lines in X direction (parallel to X axis)
    for (int i = 0; i <= divisions; ++i)
    {
        float y = -1.0f + (2.0f * i / divisions);
        vertices.insert (vertices.end(), { -1.0f, y, h,  1.0f, y, h });
    }
    
    // Grid lines in Y direction (parallel to Y axis)
    for (int i = 0; i <= divisions; ++i)
    {
        float x = -1.0f + (2.0f * i / divisions);
        vertices.insert (vertices.end(), { x, -1.0f, h,  x, 1.0f, h });
    }
    
    gridVertexCount = (int)(vertices.size() / 3);
    
    // Create VBO
    juce::gl::glGenBuffers (1, &gridVBO);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, gridVBO);
    juce::gl::glBufferData (juce::gl::GL_ARRAY_BUFFER, 
                            (GLsizeiptr)(vertices.size() * sizeof(float)), 
                            vertices.data(), 
                            juce::gl::GL_STATIC_DRAW);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
}

void SurroundStageView::createSphereGeometry()
{
    // Create a solid filled sphere representing the listener
    // Using triangle strips for proper depth testing
    const int segments = sphereSegments;
    const float radius = sphereRadius;
    
    // Sphere center position: center of room
    const float centerX = 0.0f;
    const float centerY = 0.0f;
    const float centerZ = 0.0f;
    
    std::vector<float> vertices;
    
    // Build sphere using triangles (not lines)
    // Each quad on the sphere surface is split into 2 triangles
    for (int lat = 0; lat < segments; ++lat)
    {
        float theta1 = juce::MathConstants<float>::pi * lat / segments;
        float theta2 = juce::MathConstants<float>::pi * (lat + 1) / segments;
        
        float z1 = radius * std::cos(theta1);
        float r1 = radius * std::sin(theta1);
        float z2 = radius * std::cos(theta2);
        float r2 = radius * std::sin(theta2);
        
        for (int lon = 0; lon < segments; ++lon)
        {
            float phi1 = 2.0f * juce::MathConstants<float>::pi * lon / segments;
            float phi2 = 2.0f * juce::MathConstants<float>::pi * (lon + 1) / segments;
            
            // Four corners of the quad
            float x1 = r1 * std::cos(phi1);
            float y1 = r1 * std::sin(phi1);
            float x2 = r1 * std::cos(phi2);
            float y2 = r1 * std::sin(phi2);
            float x3 = r2 * std::cos(phi1);
            float y3 = r2 * std::sin(phi1);
            float x4 = r2 * std::cos(phi2);
            float y4 = r2 * std::sin(phi2);
            
            // Triangle 1: top-left, bottom-left, bottom-right
            vertices.insert(vertices.end(), {
                centerX + x1, centerY + y1, centerZ + z1,
                centerX + x3, centerY + y3, centerZ + z2,
                centerX + x4, centerY + y4, centerZ + z2
            });
            
            // Triangle 2: top-left, bottom-right, top-right
            vertices.insert(vertices.end(), {
                centerX + x1, centerY + y1, centerZ + z1,
                centerX + x4, centerY + y4, centerZ + z2,
                centerX + x2, centerY + y2, centerZ + z1
            });
        }
    }
    
    sphereVertexCount = (int)(vertices.size() / 3);
    
    // Create VBO
    juce::gl::glGenBuffers (1, &sphereVBO);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, sphereVBO);
    juce::gl::glBufferData (juce::gl::GL_ARRAY_BUFFER, 
                            (GLsizeiptr)(vertices.size() * sizeof(float)), 
                            vertices.data(), 
                            juce::gl::GL_STATIC_DRAW);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
}

void SurroundStageView::createFrontLabelGeometry()
{
    // Create "FRONT" text on the front wall (Y = 1.0) using line segments
    // Letters are drawn in the XZ plane at Y = 0.99 (slightly in front of wall)
    // Each letter is approximately 0.15 wide, 0.2 tall
    
    const float y = 0.99f;       // Just in front of front wall
    const float letterH = 0.15f; // Letter height
    const float letterW = 0.10f; // Letter width
    const float spacing = 0.03f; // Space between letters
    const float z = 0.0f;        // Centered vertically
    
    // Total width: 5 letters * letterW + 4 spaces
    const float totalWidth = 5.0f * letterW + 4.0f * spacing;
    float x = -totalWidth / 2.0f; // Start position (centered)
    
    std::vector<float> vertices;
    
    // Helper lambda to add a line
    auto addLine = [&](float x1, float z1, float x2, float z2) {
        vertices.insert(vertices.end(), { x1, y, z1, x2, y, z2 });
    };
    
    // Letter F
    float lx = x;
    addLine(lx, z - letterH/2, lx, z + letterH/2);                    // Vertical
    addLine(lx, z + letterH/2, lx + letterW, z + letterH/2);          // Top horizontal
    addLine(lx, z, lx + letterW * 0.7f, z);                           // Middle horizontal
    x += letterW + spacing;
    
    // Letter R
    lx = x;
    addLine(lx, z - letterH/2, lx, z + letterH/2);                    // Vertical
    addLine(lx, z + letterH/2, lx + letterW, z + letterH/2);          // Top horizontal
    addLine(lx + letterW, z + letterH/2, lx + letterW, z);            // Right vertical top
    addLine(lx, z, lx + letterW, z);                                  // Middle horizontal
    addLine(lx, z, lx + letterW, z - letterH/2);                      // Diagonal leg
    x += letterW + spacing;
    
    // Letter O
    lx = x;
    addLine(lx, z - letterH/2, lx, z + letterH/2);                    // Left vertical
    addLine(lx + letterW, z - letterH/2, lx + letterW, z + letterH/2);// Right vertical
    addLine(lx, z + letterH/2, lx + letterW, z + letterH/2);          // Top horizontal
    addLine(lx, z - letterH/2, lx + letterW, z - letterH/2);          // Bottom horizontal
    x += letterW + spacing;
    
    // Letter N
    lx = x;
    addLine(lx, z - letterH/2, lx, z + letterH/2);                    // Left vertical
    addLine(lx + letterW, z - letterH/2, lx + letterW, z + letterH/2);// Right vertical
    addLine(lx, z + letterH/2, lx + letterW, z - letterH/2);          // Diagonal
    x += letterW + spacing;
    
    // Letter T
    lx = x;
    addLine(lx, z + letterH/2, lx + letterW, z + letterH/2);          // Top horizontal
    addLine(lx + letterW/2, z - letterH/2, lx + letterW/2, z + letterH/2); // Vertical
    
    frontLabelVertexCount = (int)(vertices.size() / 3);
    
    // Create VBO
    juce::gl::glGenBuffers (1, &frontLabelVBO);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, frontLabelVBO);
    juce::gl::glBufferData (juce::gl::GL_ARRAY_BUFFER, 
                            (GLsizeiptr)(vertices.size() * sizeof(float)), 
                            vertices.data(), 
                            juce::gl::GL_STATIC_DRAW);
    juce::gl::glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
}

//==============================================================================
// Matrix Calculations
//==============================================================================
juce::Matrix3D<float> SurroundStageView::getProjectionMatrix() const
{
    auto bounds = getLocalBounds().toFloat();
    float aspectRatio = bounds.getWidth() / bounds.getHeight();
    
    // Perspective projection
    float fov = 45.0f; // Field of view in degrees
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    
    float fovRad = fov * juce::MathConstants<float>::pi / 180.0f;
    float tanHalfFov = std::tan (fovRad / 2.0f);
    
    juce::Matrix3D<float> projection;
    std::fill (std::begin(projection.mat), std::end(projection.mat), 0.0f);
    
    projection.mat[0]  = 1.0f / (aspectRatio * tanHalfFov);
    projection.mat[5]  = 1.0f / tanHalfFov;
    projection.mat[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    projection.mat[11] = -1.0f;
    projection.mat[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    
    return projection;
}

juce::Matrix3D<float> SurroundStageView::getViewMatrix() const
{
    // Convert angles to radians
    float azimuthRad = azimuth * juce::MathConstants<float>::pi / 180.0f;
    float elevationRad = elevation * juce::MathConstants<float>::pi / 180.0f;
    
    // Calculate camera position (orbit around origin)
    float camX = zoom * std::cos (elevationRad) * std::sin (azimuthRad);
    float camY = zoom * std::cos (elevationRad) * std::cos (azimuthRad);
    float camZ = zoom * std::sin (elevationRad);
    
    // Look-at matrix (camera at camPos, looking at origin)
    juce::Vector3D<float> eye (camX, camY, camZ);
    juce::Vector3D<float> target (0.0f, 0.0f, 0.0f);
    
    // Since elevation is clamped to ±89°, we never hit the poles
    // so Z-up works perfectly and avoids gimbal lock
    juce::Vector3D<float> up (0.0f, 0.0f, 1.0f);
    
    // Calculate look-at matrix
    auto forward = (target - eye);
    float forwardLen = std::sqrt (forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    forward = forward * (1.0f / forwardLen);
    
    // Cross product: right = forward × up
    juce::Vector3D<float> right (
        forward.y * up.z - forward.z * up.y,
        forward.z * up.x - forward.x * up.z,
        forward.x * up.y - forward.y * up.x
    );
    float rightLen = std::sqrt (right.x * right.x + right.y * right.y + right.z * right.z);
    right = right * (1.0f / rightLen);
    
    // Cross product: correctedUp = right × forward
    juce::Vector3D<float> correctedUp (
        right.y * forward.z - right.z * forward.y,
        right.z * forward.x - right.x * forward.z,
        right.x * forward.y - right.y * forward.x
    );
    
    // Build view matrix
    juce::Matrix3D<float> view;
    view.mat[0] = right.x;
    view.mat[1] = correctedUp.x;
    view.mat[2] = -forward.x;
    view.mat[3] = 0.0f;
    
    view.mat[4] = right.y;
    view.mat[5] = correctedUp.y;
    view.mat[6] = -forward.y;
    view.mat[7] = 0.0f;
    
    view.mat[8] = right.z;
    view.mat[9] = correctedUp.z;
    view.mat[10] = -forward.z;
    view.mat[11] = 0.0f;
    
    // Translation
    view.mat[12] = -(right.x * eye.x + right.y * eye.y + right.z * eye.z);
    view.mat[13] = -(correctedUp.x * eye.x + correctedUp.y * eye.y + correctedUp.z * eye.z);
    view.mat[14] = (forward.x * eye.x + forward.y * eye.y + forward.z * eye.z);
    view.mat[15] = 1.0f;
    
    return view;
}

//==============================================================================
// Shader Creation
//==============================================================================
bool SurroundStageView::createShaders()
{
    shaderProgram = std::make_unique<juce::OpenGLShaderProgram> (openGLContext);
    
    if (!shaderProgram->addVertexShader (vertexShaderSource))
    {
        DBG ("Vertex shader error: " << shaderProgram->getLastError());
        return false;
    }
    
    if (!shaderProgram->addFragmentShader (fragmentShaderSource))
    {
        DBG ("Fragment shader error: " << shaderProgram->getLastError());
        return false;
    }
    
    if (!shaderProgram->link())
    {
        DBG ("Shader link error: " << shaderProgram->getLastError());
        return false;
    }
    
    // Get uniform locations
    uniformProjectionMatrix = juce::gl::glGetUniformLocation (shaderProgram->getProgramID(), "uProjection");
    uniformViewMatrix = juce::gl::glGetUniformLocation (shaderProgram->getProgramID(), "uView");
    uniformModelMatrix = juce::gl::glGetUniformLocation (shaderProgram->getProgramID(), "uModel");
    uniformColor = juce::gl::glGetUniformLocation (shaderProgram->getProgramID(), "uColor");
    
    return true;
}
