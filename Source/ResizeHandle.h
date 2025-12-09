#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * UI Scaling Constants
 * 
 * These define the base size and scaling constraints for the plugin window.
 * All child components should use getScaleFactor() from their parent editor
 * to scale their dimensions dynamically.
 */
namespace UIScaling
{
    // Base dimensions at 1x scale (aspect ratio 55:41)
    static constexpr int baseWidth = 1100;
    static constexpr int baseHeight = 820;
    static constexpr float aspectRatio = 55.0f / 41.0f;  // ≈ 1.341
    
    // Scale factor range (stepped by 0.1)
    static constexpr float minScale = 1.0f;
    static constexpr float maxScale = 3.0f;
    static constexpr float scaleStep = 0.1f;
    
    // Helper to snap scale to nearest 0.1 step
    inline float snapToStep (float scale)
    {
        return juce::jlimit (minScale, maxScale,
                            std::round (scale / scaleStep) * scaleStep);
    }
    
    // Get width for a given scale factor
    inline int getWidthForScale (float scale) { return static_cast<int> (baseWidth * scale); }
    
    // Get height for a given scale factor  
    inline int getHeightForScale (float scale) { return static_cast<int> (baseHeight * scale); }
    
    // Calculate scale factor from a width (maintaining aspect ratio)
    inline float getScaleFromWidth (int width)
    {
        return snapToStep (static_cast<float> (width) / baseWidth);
    }
    
    // Calculate scale factor from a height (maintaining aspect ratio)
    inline float getScaleFromHeight (int height)
    {
        return snapToStep (static_cast<float> (height) / baseHeight);
    }
}

//==============================================================================
/**
 * Resize Handle Component
 * 
 * A triangular drag handle for the bottom-right corner of the plugin window.
 * Allows resizing while maintaining the 55:41 aspect ratio.
 * Scale factor is stepped to 0.1 increments (1.0, 1.1, 1.2, ... 3.0).
 */
class ResizeHandle : public juce::Component
{
public:
    //==========================================================================
    // CONFIGURATION
    //==========================================================================
    static constexpr int handleSize = 16;  // Size of the triangular handle (unscaled)
    
    //==========================================================================
    // CALLBACK
    //==========================================================================
    /** Called when user drags to resize. Provides the new scale factor (1.0-3.0, stepped by 0.1) */
    std::function<void (float newScaleFactor)> onResize;
    
    //==========================================================================
    // CONSTRUCTOR
    //==========================================================================
    ResizeHandle()
    {
        setMouseCursor (juce::MouseCursor::BottomRightCornerResizeCursor);
    }
    
    //==========================================================================
    // PAINT - Draw triangular resize grip
    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Draw subtle triangular grip lines
        g.setColour (juce::Colour (0xff4a4a4a)); /* #4a4a4a */
        
        // Draw 3 diagonal lines in bottom-right corner
        for (int i = 0; i < 3; ++i)
        {
            float offset = (i + 1) * 4.0f;
            g.drawLine (bounds.getRight() - offset, bounds.getBottom(),
                       bounds.getRight(), bounds.getBottom() - offset, 1.5f);
        }
    }
    
    //==========================================================================
    // MOUSE HANDLING
    //==========================================================================
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (auto* parent = getParentComponent())
        {
            dragStartSize = parent->getLocalBounds();
            dragStartPos = e.getScreenPosition();
        }
    }
    
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (onResize == nullptr)
            return;
            
        auto* parent = getParentComponent();
        if (parent == nullptr)
            return;
        
        // Calculate delta from drag start
        auto currentPos = e.getScreenPosition();
        int deltaX = currentPos.x - dragStartPos.x;
        int deltaY = currentPos.y - dragStartPos.y;
        
        // Use the larger delta to determine new size (maintain aspect ratio)
        int newWidth = dragStartSize.getWidth() + deltaX;
        int newHeight = dragStartSize.getHeight() + deltaY;
        
        // Calculate scale from both dimensions and use the larger one
        float scaleFromWidth = UIScaling::getScaleFromWidth (newWidth);
        float scaleFromHeight = UIScaling::getScaleFromHeight (newHeight);
        float newScale = std::max (scaleFromWidth, scaleFromHeight);
        
        // Clamp to valid range
        newScale = juce::jlimit (UIScaling::minScale, UIScaling::maxScale, newScale);
        
        // Notify parent of new scale
        onResize (newScale);
    }
    
private:
    juce::Rectangle<int> dragStartSize;
    juce::Point<int> dragStartPos;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizeHandle)
};
