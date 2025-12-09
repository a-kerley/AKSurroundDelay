#include "TapPanel.h"

//==============================================================================
// TAP PANEL IMPLEMENTATION
//==============================================================================

TapPanel::TapPanel (int tapIndex_, juce::AudioProcessorValueTreeState& apvts)
    : tapIndex (tapIndex_)
{
    // Get accent color from palette (tap 0-7 → palette colors 0-7)
    // If we have more taps than colors, wrap around
    int colorIndex = tapIndex % ColorPalette::paletteSize;
    accentColour = ColorPalette::palettePairs[colorIndex].background;
    textColour = ColorPalette::palettePairs[colorIndex].text;
    
    // Setup power toggle
    powerToggle.setAccentColour (accentColour);
    powerToggle.setClickingTogglesState (true);
    powerToggle.setToggleState (true, juce::dontSendNotification);  // Default to enabled
    addAndMakeVisible (powerToggle);
    
    // Build parameter ID suffix (1-indexed: 1, 2, 3, ..., 8)
    juce::String suffix = juce::String (tapIndex + 1);
    
    // Setup level fader - parameter is "gainN"
    levelFader.setAccentColour (accentColour);
    levelFader.setValueTextColour (textColour);
    levelFader.setValueSuffix ("dB");
    levelFader.attachToParameter (apvts, "gain" + suffix);
    addAndMakeVisible (levelFader);
    
    // Setup time fader - parameter is "delayTimeN"
    timeFader.setAccentColour (accentColour);
    timeFader.setValueTextColour (textColour);
    timeFader.setValueSuffix ("ms");
    timeFader.attachToParameter (apvts, "delayTime" + suffix);
    addAndMakeVisible (timeFader);
    
    // Setup position control group
    positionGroup = std::make_unique<PositionControlGroup> (tapIndex, apvts);
    positionGroup->setAccentColour (accentColour);
    positionGroup->setValueTextColour (textColour);
    positionGroup->setHeightEnabled (false);  // Disabled by default until surround format detected
    addAndMakeVisible (*positionGroup);
}

TapPanel::~TapPanel()
{
}

void TapPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cornerRadius = baseCornerRadius * currentScaleFactor;
    float borderWidth = baseBorderWidth * currentScaleFactor;
    float headerHeight = baseHeaderHeight * currentScaleFactor;
    float padding = basePadding * currentScaleFactor;
    
    // Draw rounded border
    g.setColour (accentColour);
    g.drawRoundedRectangle (bounds.reduced (borderWidth * 0.5f), cornerRadius, borderWidth);
    
    // Draw header background (top rounded corners only)
    auto headerBounds = bounds.removeFromTop (headerHeight);
    juce::Path headerPath;
    headerPath.addRoundedRectangle (headerBounds.getX() + borderWidth * 0.5f, 
                                     headerBounds.getY() + borderWidth * 0.5f,
                                     headerBounds.getWidth() - borderWidth, 
                                     headerBounds.getHeight() - borderWidth * 0.5f,
                                     cornerRadius - borderWidth * 0.5f, cornerRadius - borderWidth * 0.5f, 
                                     true, true, false, false);
    g.setColour (accentColour.withAlpha (0.25f));
    g.fillPath (headerPath);
    
    // Draw header separator line
    g.setColour (accentColour.withAlpha (0.5f));
    g.drawHorizontalLine ((int) (headerBounds.getBottom()), 
                          headerBounds.getX() + borderWidth, 
                          headerBounds.getRight() - borderWidth);
    
    // Draw header text "TAP N"
    float fontSize = baseHeaderFontSize * currentScaleFactor;
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (fontSize).withStyle ("Bold"));
    
    auto textBounds = headerBounds.reduced (padding, 0);
    textBounds.removeFromRight (basePowerButtonSize * currentScaleFactor + padding);  // Leave room for power button
    g.drawText ("TAP " + juce::String (tapIndex + 1), textBounds, juce::Justification::centredLeft);
}

void TapPanel::resized()
{
    auto bounds = getLocalBounds();
    float headerHeight = baseHeaderHeight * currentScaleFactor;
    float padding = basePadding * currentScaleFactor;
    float borderWidth = baseBorderWidth * currentScaleFactor;
    float powerButtonSize = basePowerButtonSize * currentScaleFactor;
    
    // Header area
    auto headerBounds = bounds.removeFromTop ((int) headerHeight);
    headerBounds.reduce ((int) padding, 0);
    
    // Power toggle in header (right side)
    auto powerBounds = headerBounds.removeFromRight ((int) powerButtonSize);
    powerBounds = powerBounds.withSizeKeepingCentre ((int) powerButtonSize, (int) powerButtonSize);
    powerToggle.setBounds (powerBounds);
    
    // Content area (below header)
    bounds.reduce ((int) (padding + borderWidth), (int) padding);
    
    // Layout faders horizontally
    int faderSpacing = (int) (12.0f * currentScaleFactor);
    int xPos = bounds.getX();
    int yPos = bounds.getY();
    
    levelFader.setBounds (xPos, yPos, levelFader.getPreferredWidth(), levelFader.getPreferredHeight());
    xPos += levelFader.getPreferredWidth() + faderSpacing;
    
    timeFader.setBounds (xPos, yPos, timeFader.getPreferredWidth(), timeFader.getPreferredHeight());
    xPos += timeFader.getPreferredWidth() + faderSpacing;
    
    // Position control group
    if (positionGroup)
        positionGroup->setBounds (xPos, yPos, positionGroup->getPreferredWidth(), positionGroup->getPreferredHeight());
}

void TapPanel::setScaleFactor (float scale)
{
    scale = juce::jlimit (1.0f, 3.0f, scale);
    if (std::abs (scale - currentScaleFactor) < 0.01f)
        return;
    
    currentScaleFactor = scale;
    levelFader.setScaleFactor (scale);
    timeFader.setScaleFactor (scale);
    if (positionGroup)
        positionGroup->setScaleFactor (scale);
    resized();
    repaint();
}

void TapPanel::setSliderLookAndFeel (juce::LookAndFeel* lf)
{
    levelFader.getSlider().setLookAndFeel (lf);
    timeFader.getSlider().setLookAndFeel (lf);
    if (positionGroup)
        positionGroup->setSliderLookAndFeel (lf);
}

int TapPanel::getPreferredHeight() const
{
    float headerHeight = baseHeaderHeight * currentScaleFactor;
    float padding = basePadding * currentScaleFactor;
    
    // Header + padding + fader height + bottom padding
    int faderHeight = levelFader.getPreferredHeight();
    return (int) (headerHeight + padding + faderHeight + padding);
}

//==============================================================================
// TAP TAB BAR IMPLEMENTATION
//==============================================================================

TapTabBar::TapTabBar()
{
}

void TapTabBar::paint (juce::Graphics& g)
{
    float cornerRadius = baseCornerRadius * currentScaleFactor;
    float fontSize = baseFontSize * currentScaleFactor;
    float powerIconSize = basePowerIconSize * currentScaleFactor;
    
    for (int i = 0; i < NUM_TABS; ++i)
    {
        auto tabRect = tabBounds[i].toFloat();
        
        // Get color for this tab
        int colorIndex = i % ColorPalette::paletteSize;
        auto bgColour = ColorPalette::palettePairs[colorIndex].background;
        auto txtColour = ColorPalette::palettePairs[colorIndex].text;
        
        bool isSelected = (i == selectedTab);
        
        // Draw tab background
        if (isSelected)
        {
            // Selected tab: filled background
            g.setColour (bgColour);
            g.fillRoundedRectangle (tabRect, cornerRadius);
        }
        else
        {
            // Unselected tab: just border
            g.setColour (bgColour);
            g.drawRoundedRectangle (tabRect.reduced (0.5f), cornerRadius, 2.0f);
        }
        
        // Draw tab text and power icon as a centered group
        g.setColour (isSelected ? txtColour : bgColour);
        g.setFont (juce::FontOptions (fontSize).withStyle ("Bold"));
        
        // Calculate text width to center the text+icon group
        juce::String tabText = "TAP " + juce::String (i + 1);
        float textWidth = g.getCurrentFont().getStringWidthFloat (tabText);
        float iconGap = 6.0f * currentScaleFactor;  // Gap between text and icon
        float groupWidth = textWidth + iconGap + powerIconSize;
        
        // Center the group within the tab
        float groupStartX = tabRect.getCentreX() - groupWidth * 0.5f;
        
        // Draw text - add a bit of extra width to prevent ellipsis
        g.drawText (tabText, 
                    (int) groupStartX, (int) tabRect.getY(), 
                    (int) (textWidth + 4.0f), (int) tabRect.getHeight(),
                    juce::Justification::centredLeft);
        
        // Draw power icon right after text
        float iconCenterX = groupStartX + textWidth + iconGap + powerIconSize * 0.5f;
        float iconCenterY = tabRect.getCentreY();
        float iconRadius = powerIconSize * 0.35f;
        float iconLineThickness = powerIconSize * 0.12f;
        
        g.setColour (isSelected ? txtColour : bgColour);
        
        // Power arc
        juce::Path arc;
        arc.addCentredArc (iconCenterX, iconCenterY, iconRadius, iconRadius, 0.0f,
                           juce::MathConstants<float>::pi * 0.25f,
                           juce::MathConstants<float>::pi * 1.75f, true);
        g.strokePath (arc, juce::PathStrokeType (iconLineThickness));
        
        // Power line
        g.drawLine (iconCenterX, iconCenterY - iconRadius - iconLineThickness * 0.5f,
                    iconCenterX, iconCenterY - iconRadius * 0.3f, iconLineThickness);
    }
}

void TapTabBar::resized()
{
    auto bounds = getLocalBounds();
    float tabSpacing = baseTabSpacing * currentScaleFactor;
    
    // Calculate tab width (equal distribution)
    float totalSpacing = tabSpacing * (NUM_TABS - 1);
    float tabWidth = (bounds.getWidth() - totalSpacing) / NUM_TABS;
    
    float xPos = 0.0f;
    for (int i = 0; i < NUM_TABS; ++i)
    {
        tabBounds[i] = juce::Rectangle<int> ((int) xPos, 0, (int) tabWidth, bounds.getHeight());
        xPos += tabWidth + tabSpacing;
    }
}

void TapTabBar::mouseDown (const juce::MouseEvent& event)
{
    auto pos = event.getPosition();
    
    for (int i = 0; i < NUM_TABS; ++i)
    {
        if (tabBounds[i].contains (pos))
        {
            setSelectedTab (i);
            break;
        }
    }
}

void TapTabBar::setSelectedTab (int index)
{
    index = juce::jlimit (0, NUM_TABS - 1, index);
    if (index != selectedTab)
    {
        selectedTab = index;
        repaint();
        
        if (onTabSelected)
            onTabSelected (selectedTab);
    }
}

void TapTabBar::setScaleFactor (float scale)
{
    scale = juce::jlimit (1.0f, 3.0f, scale);
    if (std::abs (scale - currentScaleFactor) < 0.01f)
        return;
    
    currentScaleFactor = scale;
    resized();
    repaint();
}
