#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ColorPalette.h"

//==============================================================================
TapMatrixAudioProcessorEditor::TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Restore scale factor from processor state
    currentScaleFactor = p.getUIScaleFactor();
    
    // Set custom LookAndFeel
    setLookAndFeel (&customLookAndFeel);
    
    // Setup resize handle
    setupResizeHandle();
    
    // Setup 3D Surround Stage View
    addAndMakeVisible (surroundStageView);
    
    // Setup view preset selector
    setupViewPresetSelector();
    
    // Setup tap tab bar and panels
    setupTapPanels();
    
    // Apply restored scale factor to all child components
    updateAllComponentScales();
    
    // Start timer to sync view preset state (30 FPS is enough for UI sync)
    startTimerHz (30);
    
    // Set plugin window size based on restored scale factor
    // Base size: 1100x820 (aspect ratio 55:41)
    setSize (UIScaling::getWidthForScale (currentScaleFactor),
             UIScaling::getHeightForScale (currentScaleFactor));
}

TapMatrixAudioProcessorEditor::~TapMatrixAudioProcessorEditor()
{
    stopTimer();
    
    // Clear look and feel from all tap panel sliders before destroying
    for (auto& panel : tapPanels)
    {
        if (panel)
            panel->setSliderLookAndFeel (nullptr);
    }
    
    setLookAndFeel (nullptr);
}

//==============================================================================
void TapMatrixAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Clean dark background
    g.fillAll (ColorPalette::pluginBackground);
}

void TapMatrixAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const float scale = currentScaleFactor;
    
    // Scale all layout constants
    const int padding = static_cast<int> (20 * scale);
    const int viewportSize = static_cast<int> (400 * scale);  // Slightly smaller to make room for tap panel
    const int selectorHeight = static_cast<int> (28 * scale);
    const int selectorWidth = static_cast<int> (320 * scale);
    const int tabBarHeight = static_cast<int> (24 * scale);
    
    // Resize handle in bottom-right corner (always 16x16, unscaled)
    resizeHandle.setBounds (bounds.getRight() - ResizeHandle::handleSize,
                            bounds.getBottom() - ResizeHandle::handleSize,
                            ResizeHandle::handleSize,
                            ResizeHandle::handleSize);
    resizeHandle.toFront (false);
    
    // Left side: 3D viewport with padding
    auto viewportArea = bounds.removeFromLeft (viewportSize + padding * 2);
    viewportArea.reduce (padding, padding);
    
    // Position viewport
    surroundStageView.setBounds (viewportArea.removeFromTop (viewportSize));
    
    // View preset selector below viewport (centered)
    viewportArea.removeFromTop (static_cast<int> (10 * scale)); // spacing
    auto selectorArea = viewportArea.removeFromTop (selectorHeight);
    selectorArea = selectorArea.withSizeKeepingCentre (selectorWidth, selectorHeight);
    viewPresetSelector.setBounds (selectorArea);
    
    // Right side: Tap controls area
    auto controlsArea = bounds;
    controlsArea.reduce (padding, padding);
    
    // Tab bar at top
    tapTabBar.setBounds (controlsArea.removeFromTop (tabBarHeight));
    
    // Spacing between tab bar and panel
    controlsArea.removeFromTop (static_cast<int> (8 * scale));
    
    // Tap panels - use preferred height, constrained to available space
    for (auto& panel : tapPanels)
    {
        if (panel)
        {
            int preferredHeight = panel->getPreferredHeight();
            int panelHeight = juce::jmin (preferredHeight, controlsArea.getHeight());
            panel->setBounds (controlsArea.getX(), controlsArea.getY(), 
                              controlsArea.getWidth(), panelHeight);
        }
    }
}

void TapMatrixAudioProcessorEditor::setupViewPresetSelector()
{
    viewPresetSelector.onPresetSelected = [this](SurroundStageView::ViewPreset preset)
    {
        surroundStageView.setViewPreset (preset);
    };
    
    addAndMakeVisible (viewPresetSelector);
}

void TapMatrixAudioProcessorEditor::setupResizeHandle()
{
    resizeHandle.onResize = [this] (float newScale)
    {
        setUIScaleFactorAndResize (newScale);
    };
    
    addAndMakeVisible (resizeHandle);
}

void TapMatrixAudioProcessorEditor::setupTapPanels()
{
    // Setup tab bar
    tapTabBar.onTabSelected = [this] (int index) { showTapPanel (index); };
    addAndMakeVisible (tapTabBar);
    
    // Create all 8 tap panels
    for (int i = 0; i < NUM_TAPS; ++i)
    {
        tapPanels[i] = std::make_unique<TapPanel> (i, audioProcessor.getParameters());
        tapPanels[i]->setSliderLookAndFeel (&customLookAndFeel);
        addAndMakeVisible (*tapPanels[i]);
        
        // Only show the first panel initially
        tapPanels[i]->setVisible (i == 0);
    }
    
    currentTapIndex = 0;
}

void TapMatrixAudioProcessorEditor::showTapPanel (int index)
{
    if (index < 0 || index >= NUM_TAPS || index == currentTapIndex)
        return;
    
    // Hide current panel
    if (tapPanels[currentTapIndex])
        tapPanels[currentTapIndex]->setVisible (false);
    
    // Show new panel
    currentTapIndex = index;
    if (tapPanels[currentTapIndex])
        tapPanels[currentTapIndex]->setVisible (true);
}

void TapMatrixAudioProcessorEditor::setUIScaleFactorAndResize (float newScale)
{
    // Snap to 0.1 steps and clamp to valid range
    newScale = UIScaling::snapToStep (newScale);
    
    if (std::abs (newScale - currentScaleFactor) < 0.01f)
        return;  // No significant change
    
    currentScaleFactor = newScale;
    
    // Save to processor state for persistence
    audioProcessor.setUIScaleFactor (currentScaleFactor);
    
    // Update scale on all child components
    updateAllComponentScales();
    
    // Resize the window
    setSize (UIScaling::getWidthForScale (currentScaleFactor),
             UIScaling::getHeightForScale (currentScaleFactor));
}

void TapMatrixAudioProcessorEditor::updateAllComponentScales()
{
    // Update tap tab bar
    tapTabBar.setScaleFactor (currentScaleFactor);
    
    // Update all tap panels
    for (auto& panel : tapPanels)
    {
        if (panel)
            panel->setScaleFactor (currentScaleFactor);
    }
    
    // Update ViewPresetSelector with scale factor
    viewPresetSelector.setScaleFactor (currentScaleFactor);
    
    // TODO: Add setScaleFactor to SurroundStageView when needed
}

void TapMatrixAudioProcessorEditor::timerCallback()
{
    // Sync the selector with the SurroundStageView's current preset
    auto currentPreset = surroundStageView.getCurrentPreset();
    viewPresetSelector.setCurrentPreset (currentPreset);
}
