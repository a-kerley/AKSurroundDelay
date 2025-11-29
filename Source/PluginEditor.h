#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>

//==============================================================================
/**
 * Graphical timeline view showing tap positions and timing
 */
class TimelineView : public juce::Component
{
public:
    TimelineView (TapMatrixAudioProcessor& proc) : processor (proc) {}
    
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Dark background
        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRect (bounds);
        
        // Draw timing grid (10ms, 40ms, 100ms, 200ms, 500ms, 1s, 2s, 2.5s)
        g.setColour (juce::Colour (0xff2a2a2a));
        const float timeMarks[] = { 10, 40, 100, 200, 500, 1000, 2000, 2500 };
        
        for (float timeMs : timeMarks)
        {
            float x = (timeMs / 2500.0f) * bounds.getWidth();
            g.drawVerticalLine (static_cast<int>(x), 0.0f, bounds.getHeight());
            
            // Draw time label
            g.setColour (juce::Colour (0xff4a4a4a));
            g.setFont (juce::FontOptions (10.0f));
            juce::String label = timeMs < 1000 ? juce::String((int)timeMs) + "ms" 
                                               : juce::String(timeMs / 1000.0f, 1) + "s";
            g.drawText (label, x + 2, 2, 50, 15, juce::Justification::left);
        }
        
        // Draw LPF visualization (shaded region showing filtered frequencies)
        auto lpfFreq = processor.getParameters().getRawParameterValue ("lpfFreq")->load();
        if (lpfFreq < 19000.0f)  // Only show if LPF is actively filtering
        {
            // Map LPF frequency (20-20000 Hz) to timeline position (logarithmic scale)
            // We'll use a simplified mapping where lower frequencies appear earlier in time
            // This is conceptual - showing that high frequencies are "filtered out" towards the right
            float lpfNormalized = (std::log (lpfFreq / 20.0f)) / (std::log (20000.0f / 20.0f));
            float lpfX = lpfNormalized * bounds.getWidth();
            
            // Draw shaded region to the right of LPF cutoff (representing filtered area)
            juce::Rectangle<float> filteredArea (lpfX, 0.0f, bounds.getWidth() - lpfX, bounds.getHeight());
            g.setColour (juce::Colour (0xff4a9eff).withAlpha (0.06f));
            g.fillRect (filteredArea);
            
            // Draw LPF cutoff line
            g.setColour (juce::Colour (0xff4a9eff).withAlpha (0.35f));
            g.drawVerticalLine (static_cast<int> (lpfX), 0.0f, bounds.getHeight());
            
            // Draw LPF frequency label
            g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
            g.setColour (juce::Colour (0xff4a9eff));
            juce::String lpfLabel = lpfFreq < 1000 ? juce::String ((int)lpfFreq) + "Hz"
                                                    : juce::String (lpfFreq / 1000.0f, 1) + "kHz";
            g.drawText ("LPF:" + lpfLabel, lpfX + 3, bounds.getHeight() - 18, 80, 12, juce::Justification::left);
        }
        
        // Draw delay lines radiating from taps
        g.setColour (juce::Colour (0xff333333));
        for (int i = 0; i < 8; ++i)
        {
            auto delayMs = processor.getParameters().getRawParameterValue (
                juce::String ("delayTime") + juce::String (i + 1))->load();
            float x = (delayMs / 2500.0f) * bounds.getWidth();
            float y = bounds.getHeight() * 0.5f;
            
            // Draw cross pattern
            g.drawLine (x - 20, y, x + 20, y, 1.0f);
            g.drawLine (x, y - 20, x, y + 20, 1.0f);
            g.drawLine (x - 15, y - 15, x + 15, y + 15, 1.0f);
            g.drawLine (x - 15, y + 15, x + 15, y - 15, 1.0f);
        }
        
        // Draw tap dots
        for (int i = 0; i < 8; ++i)
        {
            auto delayMs = processor.getParameters().getRawParameterValue (
                juce::String ("delayTime") + juce::String (i + 1))->load();
            auto gainDb = processor.getParameters().getRawParameterValue (
                juce::String ("gain") + juce::String (i + 1))->load();
            
            if (gainDb > -90.0f)  // Only show active taps
            {
                float x = (delayMs / 2500.0f) * bounds.getWidth();
                float y = bounds.getHeight() * 0.5f;
                
                // Color based on tap index
                juce::Colour tapColours[] = {
                    juce::Colour (0xff00ff00), juce::Colour (0xffffff00), juce::Colour (0xffff0000),
                    juce::Colour (0xffff8800), juce::Colour (0xffaa00ff), juce::Colour (0xff00ffff),
                    juce::Colour (0xffff00ff), juce::Colour (0xff0088ff)
                };
                
                juce::Colour baseColour = tapColours[i];
                
                // Apply damping saturation (0% damping = full color, 100% = desaturated)
                auto damping = processor.getParameters().getRawParameterValue (
                    juce::String ("damping") + juce::String (i + 1))->load();
                float saturation = 1.0f - (damping * 0.7f);  // Don't go completely grey
                juce::Colour tapColour = baseColour.withSaturation (saturation);
                
                // Apply transparency for inactive taps (low gain)
                float alpha = juce::jmap (gainDb, -90.0f, -40.0f, 0.3f, 1.0f);
                if (gainDb > -40.0f) alpha = 1.0f;
                
                // Draw reverb tail (horizontal gradient behind puck)
                auto reverb = processor.getParameters().getRawParameterValue (
                    juce::String ("reverb") + juce::String (i + 1))->load();
                if (reverb > 0.01f)
                {
                    float tailLength = reverb * 40.0f;  // Up to 40px tail
                    juce::ColourGradient gradient (tapColour.withAlpha (reverb * 0.3f * alpha), x, y,
                                                  tapColour.withAlpha (0.0f), x + tailLength, y, false);
                    g.setGradientFill (gradient);
                    g.fillRect (x, y - 3.0f, tailLength, 6.0f);
                }
                
                // Draw crosstalk cross icon
                auto crosstalk = processor.getParameters().getRawParameterValue (
                    juce::String ("crosstalk") + juce::String (i + 1))->load();
                if (crosstalk > 0.01f)
                {
                    float crossSize = 6.0f + (crosstalk * 4.0f);
                    g.setColour (tapColour.withAlpha (crosstalk * 0.6f * alpha));
                    g.drawLine (x - crossSize, y, x + crossSize, y, 2.0f);
                    g.drawLine (x, y - crossSize, x, y + crossSize, 2.0f);
                }
                
                // Draw feedback rings (concentric circles)
                auto feedback = processor.getParameters().getRawParameterValue (
                    juce::String ("feedback") + juce::String (i + 1))->load();
                if (feedback > 0.01f)
                {
                    int numRings = static_cast<int>(feedback * 3.0f) + 1;  // 1-4 rings
                    for (int ring = 1; ring <= numRings; ++ring)
                    {
                        float ringRadius = 30.0f + (ring * 8.0f);
                        float ringAlpha = (1.0f - (ring * 0.2f)) * feedback * alpha;
                        g.setColour (tapColour.withAlpha (ringAlpha * 0.4f));
                        g.drawEllipse (x - ringRadius, y - ringRadius, 
                                      ringRadius * 2, ringRadius * 2, 1.5f);
                    }
                }
                
                tapColour = tapColour.withAlpha (alpha);
                
                // Highlight hovered or dragged tap
                bool isHovered = (i == hoveredTap);
                bool isDragged = (i == draggedTap);
                
                if (isDragged)
                {
                    // Draw glow for dragged tap
                    g.setColour (tapColour.withAlpha (0.3f));
                    g.fillEllipse (x - 16, y - 16, 32, 32);
                }
                else if (isHovered)
                {
                    // Draw subtle glow for hovered tap
                    g.setColour (tapColour.withAlpha (0.2f));
                    g.fillEllipse (x - 14, y - 14, 28, 28);
                }
                
                // Draw level meter ring (expanding with level)
                float level = processor.getTapLevel (i);
                if (level > 0.001f)
                {
                    float meterRadius = 12.0f + (level * 15.0f);  // 12-27px radius
                    g.setColour (tapColour.withAlpha (0.5f));
                    g.drawEllipse (x - meterRadius, y - meterRadius, 
                                  meterRadius * 2, meterRadius * 2, 2.0f);
                }
                
                g.setColour (tapColour);
                g.fillEllipse (x - 8, y - 8, 16, 16);
                g.setColour (tapColour.darker (0.3f));
                g.drawEllipse (x - 10, y - 10, 20, 20, 2.0f);
                
                // Draw tap number inside puck
                g.setColour (juce::Colours::black);
                g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
                g.drawText (juce::String (i + 1), x - 8, y - 8, 16, 16, juce::Justification::centred);
            }
        }
    }
    
    void mouseMove (const juce::MouseEvent& e) override
    {
        hoveredTap = getTapAtPosition (e.getPosition());
        repaint();
    }
    
    void mouseExit (const juce::MouseEvent&) override
    {
        hoveredTap = -1;
        repaint();
    }
    
    void mouseDown (const juce::MouseEvent& e) override
    {
        draggedTap = getTapAtPosition (e.getPosition());
        if (draggedTap >= 0)
        {
            dragStartPosition = e.getPosition();
            auto delayMs = processor.getParameters().getRawParameterValue (
                juce::String ("delayTime") + juce::String (draggedTap + 1))->load();
            auto gainDb = processor.getParameters().getRawParameterValue (
                juce::String ("gain") + juce::String (draggedTap + 1))->load();
            initialDragDelayMs = delayMs;
            initialDragGainDb = gainDb;
        }
    }
    
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (draggedTap >= 0)
        {
            auto bounds = getLocalBounds().toFloat();
            auto delta = e.getPosition() - dragStartPosition;
            
            // Horizontal drag = delay time (0-2500ms)
            float newDelayMs = initialDragDelayMs + (delta.x / bounds.getWidth()) * 2500.0f;
            newDelayMs = juce::jlimit (0.0f, 2500.0f, newDelayMs);
            
            // Vertical drag = gain (-90dB to 0dB, inverted for screen coords)
            float newGainDb = initialDragGainDb - (delta.y / bounds.getHeight()) * 90.0f;
            newGainDb = juce::jlimit (-90.0f, 0.0f, newGainDb);
            
            // Update parameters
            auto* delayParam = processor.getParameters().getParameter (
                juce::String ("delayTime") + juce::String (draggedTap + 1));
            auto* gainParam = processor.getParameters().getParameter (
                juce::String ("gain") + juce::String (draggedTap + 1));
            
            if (delayParam)
                delayParam->setValueNotifyingHost (delayParam->convertTo0to1 (newDelayMs));
            if (gainParam)
                gainParam->setValueNotifyingHost (gainParam->convertTo0to1 (newGainDb));
            
            repaint();
        }
    }
    
    void mouseUp (const juce::MouseEvent&) override
    {
        draggedTap = -1;
        repaint();
    }
    
private:
    TapMatrixAudioProcessor& processor;
    int hoveredTap = -1;
    int draggedTap = -1;
    juce::Point<int> dragStartPosition;
    float initialDragDelayMs = 0.0f;
    float initialDragGainDb = 0.0f;
    
    int getTapAtPosition (juce::Point<int> pos) const
    {
        auto bounds = getLocalBounds().toFloat();
        const float hitRadius = 12.0f;
        
        for (int i = 0; i < 8; ++i)
        {
            auto gainDb = processor.getParameters().getRawParameterValue (
                juce::String ("gain") + juce::String (i + 1))->load();
            
            if (gainDb > -90.0f)
            {
                auto delayMs = processor.getParameters().getRawParameterValue (
                    juce::String ("delayTime") + juce::String (i + 1))->load();
                
                float x = (delayMs / 2500.0f) * bounds.getWidth();
                float y = bounds.getHeight() * 0.5f;
                
                float dx = pos.x - x;
                float dy = pos.y - y;
                
                if (std::sqrt (dx * dx + dy * dy) < hitRadius)
                    return i;
            }
        }
        return -1;
    }
};

//==============================================================================
/**
 * Stereo field visualization
 */
class StereoFieldView : public juce::Component
{
public:
    StereoFieldView (TapMatrixAudioProcessor& proc) : processor (proc) {}
    
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Dark background
        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRect (bounds);
        
        // Draw "STEREO" label
        g.setColour (juce::Colour (0xff4a4a4a));
        g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
        g.drawText ("STEREO", bounds, juce::Justification::centred);
        
        // Draw concentric circles for depth
        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();
        
        for (int ring = 1; ring <= 4; ++ring)
        {
            float radius = ring * 25.0f;
            g.setColour (juce::Colour (0xff2a2a2a).withAlpha (0.3f));
            g.drawEllipse (centerX - radius, centerY - radius, radius * 2, radius * 2, 1.0f);
        }
        
        // Draw tap positions
        for (int i = 0; i < 8; ++i)
        {
            auto gainDb = processor.getParameters().getRawParameterValue (
                juce::String ("gain") + juce::String (i + 1))->load();
            
            if (gainDb > -90.0f)
            {
                auto panX = processor.getParameters().getRawParameterValue (
                    juce::String ("panX") + juce::String (i + 1))->load();
                auto panY = processor.getParameters().getRawParameterValue (
                    juce::String ("panY") + juce::String (i + 1))->load();
                
                float x = centerX + panX * 80.0f;
                float y = centerY - panY * 80.0f;  // Flip Y for screen coords
                
                juce::Colour tapColours[] = {
                    juce::Colour (0xff00ff00), juce::Colour (0xffffff00), juce::Colour (0xffff0000),
                    juce::Colour (0xffff8800), juce::Colour (0xffaa00ff), juce::Colour (0xff00ffff),
                    juce::Colour (0xffff00ff), juce::Colour (0xff0088ff)
                };
                
                juce::Colour baseColour = tapColours[i];
                
                // Apply damping saturation
                auto damping = processor.getParameters().getRawParameterValue (
                    juce::String ("damping") + juce::String (i + 1))->load();
                float saturation = 1.0f - (damping * 0.7f);
                juce::Colour tapColour = baseColour.withSaturation (saturation);
                
                // Apply transparency for inactive taps
                float alpha = juce::jmap (gainDb, -90.0f, -40.0f, 0.3f, 1.0f);
                if (gainDb > -40.0f) alpha = 1.0f;
                
                // Draw reverb tail (radial gradient)
                auto reverb = processor.getParameters().getRawParameterValue (
                    juce::String ("reverb") + juce::String (i + 1))->load();
                if (reverb > 0.01f)
                {
                    float tailRadius = 30.0f * reverb;
                    g.setColour (tapColour.withAlpha (reverb * 0.2f * alpha));
                    g.fillEllipse (x - tailRadius, y - tailRadius, tailRadius * 2, tailRadius * 2);
                }
                
                // Draw crosstalk cross icon
                auto crosstalk = processor.getParameters().getRawParameterValue (
                    juce::String ("crosstalk") + juce::String (i + 1))->load();
                if (crosstalk > 0.01f)
                {
                    float crossSize = 5.0f + (crosstalk * 3.0f);
                    g.setColour (tapColour.withAlpha (crosstalk * 0.7f * alpha));
                    g.drawLine (x - crossSize, y, x + crossSize, y, 1.5f);
                    g.drawLine (x, y - crossSize, x, y + crossSize, 1.5f);
                }
                
                // Draw feedback rings
                auto feedback = processor.getParameters().getRawParameterValue (
                    juce::String ("feedback") + juce::String (i + 1))->load();
                if (feedback > 0.01f)
                {
                    int numRings = static_cast<int>(feedback * 3.0f) + 1;
                    for (int ring = 1; ring <= numRings; ++ring)
                    {
                        float ringRadius = 20.0f + (ring * 6.0f);
                        float ringAlpha = (1.0f - (ring * 0.2f)) * feedback * alpha;
                        g.setColour (tapColour.withAlpha (ringAlpha * 0.4f));
                        g.drawEllipse (x - ringRadius, y - ringRadius, 
                                      ringRadius * 2, ringRadius * 2, 1.5f);
                    }
                }
                
                tapColour = tapColour.withAlpha (alpha);
                
                // Highlight hovered or dragged tap
                bool isHovered = (i == hoveredTap);
                bool isDragged = (i == draggedTap);
                
                if (isDragged)
                {
                    g.setColour (tapColour.withAlpha (0.3f));
                    g.fillEllipse (x - 12, y - 12, 24, 24);
                }
                else if (isHovered)
                {
                    g.setColour (tapColour.withAlpha (0.2f));
                    g.fillEllipse (x - 10, y - 10, 20, 20);
                }
                
                // Draw level meter ring
                float level = processor.getTapLevel (i);
                if (level > 0.001f)
                {
                    float meterRadius = 8.0f + (level * 12.0f);  // 8-20px radius
                    g.setColour (tapColour.withAlpha (0.5f));
                    g.drawEllipse (x - meterRadius, y - meterRadius, 
                                  meterRadius * 2, meterRadius * 2, 2.0f);
                }
                
                g.setColour (tapColour);
                g.fillEllipse (x - 6, y - 6, 12, 12);
                
                // Draw tap number
                g.setColour (juce::Colours::black);
                g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
                g.drawText (juce::String (i + 1), x - 6, y - 6, 12, 12, juce::Justification::centred);
            }
        }
    }
    
    void mouseMove (const juce::MouseEvent& e) override
    {
        hoveredTap = getTapAtPosition (e.getPosition());
        repaint();
    }
    
    void mouseExit (const juce::MouseEvent&) override
    {
        hoveredTap = -1;
        repaint();
    }
    
    void mouseDown (const juce::MouseEvent& e) override
    {
        draggedTap = getTapAtPosition (e.getPosition());
        if (draggedTap >= 0)
        {
            dragStartPosition = e.getPosition();
            auto panX = processor.getParameters().getRawParameterValue (
                juce::String ("panX") + juce::String (draggedTap + 1))->load();
            auto panY = processor.getParameters().getRawParameterValue (
                juce::String ("panY") + juce::String (draggedTap + 1))->load();
            initialDragPanX = panX;
            initialDragPanY = panY;
        }
    }
    
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (draggedTap >= 0)
        {
            auto bounds = getLocalBounds().toFloat();
            float centerX = bounds.getCentreX();
            float centerY = bounds.getCentreY();
            auto delta = e.getPosition() - dragStartPosition;
            
            // X drag = panX (-1 to +1)
            float newPanX = initialDragPanX + (delta.x / 80.0f);
            newPanX = juce::jlimit (-1.0f, 1.0f, newPanX);
            
            // Y drag = panY (-1 to +1, inverted for screen coords)
            float newPanY = initialDragPanY - (delta.y / 80.0f);
            newPanY = juce::jlimit (-1.0f, 1.0f, newPanY);
            
            // Update parameters
            auto* panXParam = processor.getParameters().getParameter (
                juce::String ("panX") + juce::String (draggedTap + 1));
            auto* panYParam = processor.getParameters().getParameter (
                juce::String ("panY") + juce::String (draggedTap + 1));
            
            if (panXParam)
                panXParam->setValueNotifyingHost (panXParam->convertTo0to1 (newPanX));
            if (panYParam)
                panYParam->setValueNotifyingHost (panYParam->convertTo0to1 (newPanY));
            
            repaint();
        }
    }
    
    void mouseUp (const juce::MouseEvent&) override
    {
        draggedTap = -1;
        repaint();
    }
    
private:
    TapMatrixAudioProcessor& processor;
    int hoveredTap = -1;
    int draggedTap = -1;
    juce::Point<int> dragStartPosition;
    float initialDragPanX = 0.0f;
    float initialDragPanY = 0.0f;
    
    int getTapAtPosition (juce::Point<int> pos) const
    {
        auto bounds = getLocalBounds().toFloat();
        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();
        const float hitRadius = 10.0f;
        
        for (int i = 0; i < 8; ++i)
        {
            auto gainDb = processor.getParameters().getRawParameterValue (
                juce::String ("gain") + juce::String (i + 1))->load();
            
            if (gainDb > -90.0f)
            {
                auto panX = processor.getParameters().getRawParameterValue (
                    juce::String ("panX") + juce::String (i + 1))->load();
                auto panY = processor.getParameters().getRawParameterValue (
                    juce::String ("panY") + juce::String (i + 1))->load();
                
                float x = centerX + panX * 80.0f;
                float y = centerY - panY * 80.0f;  // Flip Y
                
                float dx = pos.x - x;
                float dy = pos.y - y;
                
                if (std::sqrt (dx * dx + dy * dy) < hitRadius)
                    return i;
            }
        }
        return -1;
    }
};

//==============================================================================
/**
 * Vertical fader component
 */
class VerticalFader : public juce::Component
{
public:
    VerticalFader (const juce::String& labelText, juce::Colour color = juce::Colour (0xff51cf66))
        : label (labelText), colour (color)
    {
        slider.setSliderStyle (juce::Slider::LinearVertical);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
        slider.setColour (juce::Slider::trackColourId, juce::Colour (0xff1a1a1a));
        slider.setColour (juce::Slider::thumbColourId, colour);
        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff0a0a0a));
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);
    }
    
    void paint (juce::Graphics& g) override
    {
        // Draw label at top
        g.setColour (juce::Colour (0xff8a8a8a));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (label, 0, 0, getWidth(), 20, juce::Justification::centred);
    }
    
    void resized() override
    {
        slider.setBounds (getLocalBounds().withTrimmedTop (20));
    }
    
    juce::Slider& getSlider() { return slider; }
    
private:
    juce::Slider slider;
    juce::String label;
    juce::Colour colour;
};

//==============================================================================
/**
 * TapMatrix Audio Processor Editor
 * 
 * Professional UI with timeline view, tap selection, and vertical faders
 */
class TapMatrixAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor&);
    ~TapMatrixAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TapMatrixAudioProcessor& audioProcessor;
    
    // Current selected tap (0-7)
    int selectedTap = 0;
    
    // Visual components
    TimelineView timelineView;
    StereoFieldView stereoFieldView;
    
    // Tap selection buttons
    std::array<std::unique_ptr<juce::TextButton>, 8> tapButtons;
    
    // Per-tap faders (only for selected tap)
    std::unique_ptr<VerticalFader> gainFader;
    std::unique_ptr<VerticalFader> delayFader;
    std::unique_ptr<VerticalFader> feedbackFader;
    std::unique_ptr<VerticalFader> crosstalkFader;
    std::unique_ptr<VerticalFader> dampingFader;
    std::unique_ptr<VerticalFader> reverbFader;
    std::unique_ptr<VerticalFader> panXFader;
    std::unique_ptr<VerticalFader> panYFader;
    std::unique_ptr<VerticalFader> panZFader;
    
    // Global faders
    std::unique_ptr<VerticalFader> hpfFader;
    std::unique_ptr<VerticalFader> lpfFader;
    std::unique_ptr<VerticalFader> duckingFader;
    std::unique_ptr<VerticalFader> mixFader;
    std::unique_ptr<VerticalFader> outputFader;
    
    // Global controls
    juce::ComboBox reverbTypeCombo;
    juce::Label reverbTypeLabel;
    juce::ToggleButton syncToggle, tapeModeToggle;
    
    // Attachments for selected tap
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crosstalkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panXAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panYAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panZAttachment;
    
    // Global attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpfAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpfAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> duckingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> reverbTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> tapeModeAttachment;
    
    void selectTap (int tapIndex);
    void updateTapAttachments();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapMatrixAudioProcessorEditor)
};
