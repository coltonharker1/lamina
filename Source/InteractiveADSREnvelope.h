#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * InteractiveADSREnvelope - Draggable ADSR curve visualization
 *
 * Features:
 * - Visual ADSR curve with draggable control points
 * - Attack/Decay adjust time horizontally
 * - Sustain adjusts level vertically
 * - Release adjusts time horizontally
 * - Display values below the curve
 */
class InteractiveADSREnvelope : public juce::Component
{
public:
    InteractiveADSREnvelope()
    {
    }

    ~InteractiveADSREnvelope() override = default;

    // Set parameter pointers
    void setAttackParameter(juce::AudioParameterFloat* param) { attackParam = param; }
    void setDecayParameter(juce::AudioParameterFloat* param) { decayParam = param; }
    void setSustainParameter(juce::AudioParameterFloat* param) { sustainParam = param; }
    void setReleaseParameter(juce::AudioParameterFloat* param) { releaseParam = param; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Background
        g.setColour(juce::Colour(0xff0a0f1a));  // Dark slate background
        g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

        // Border
        g.setColour(juce::Colour(0xff1e293b));
        g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);

        // Get current ADSR values (parameters are in milliseconds and 0-100 for sustain)
        float attackMs = attackParam ? attackParam->get() : 10.0f;
        float decayMs = decayParam ? decayParam->get() : 300.0f;
        float sustainPercent = sustainParam ? sustainParam->get() : 80.0f;
        float sustain = sustainPercent / 100.0f;  // Convert to 0-1 for visualization
        float releaseMs = releaseParam ? releaseParam->get() : 500.0f;

        // Calculate graph dimensions (scale to component size)
        const float padding = 12.0f;
        const float valueLabelHeight = 35.0f;  // Space for value labels at bottom
        const float graphWidth = bounds.getWidth() - (padding * 2);
        const float graphHeight = bounds.getHeight() - valueLabelHeight - (padding * 2);
        const float graphTop = padding;
        const float graphBottom = graphTop + graphHeight;
        const float graphLeft = padding;
        const float graphRight = graphLeft + graphWidth;

        // Use fixed-width zones for each ADSR phase - this prevents handles from moving each other
        // Each phase gets 25% of the width, and handles move within their zone based on parameter value
        const float zoneWidth = graphWidth / 4.0f;

        // Define zones (each is 25% of total width)
        float attackZoneStart = graphLeft;
        float attackZoneEnd = graphLeft + zoneWidth;
        float decayZoneStart = attackZoneEnd;
        float decayZoneEnd = decayZoneStart + zoneWidth;
        float sustainZoneStart = decayZoneEnd;
        float sustainZoneEnd = sustainZoneStart + zoneWidth;
        float releaseZoneStart = sustainZoneEnd;
        float releaseZoneEnd = graphRight;

        // Calculate control point X positions based on parameter values within each zone
        // Attack: maps 1-5000ms to position within attack zone
        float attackNormalized = (attackMs - 1.0f) / (5000.0f - 1.0f);  // 0 to 1
        float attackX = attackZoneStart + attackNormalized * zoneWidth;

        // Decay: maps 10-5000ms to position within decay zone
        float decayNormalized = (decayMs - 10.0f) / (5000.0f - 10.0f);  // 0 to 1
        float decayX = decayZoneStart + decayNormalized * zoneWidth;

        // Sustain: fixed in middle of sustain zone (horizontal position doesn't change)
        float sustainPointX = sustainZoneStart + zoneWidth * 0.5f;

        // Release: maps 10-5000ms to position within release zone
        float releaseNormalized = (releaseMs - 10.0f) / (5000.0f - 10.0f);  // 0 to 1
        float releaseX = releaseZoneStart + releaseNormalized * zoneWidth;

        // Y positions based on levels
        float sustainY = graphTop + graphHeight * (1.0f - sustain);
        float peakY = graphTop;
        float startY = graphBottom;
        float endY = graphBottom;

        // Store for mouse interaction
        attackPoint = {attackX, peakY};
        decayPoint = {decayX, sustainY};
        sustainPoint = {sustainPointX, sustainY};
        releasePoint = {releaseX, endY};

        // Draw grid lines
        g.setColour(juce::Colour(0xff334155));
        g.drawLine(graphLeft, graphTop, graphLeft, graphBottom, 1.0f);
        g.drawLine(graphLeft, graphBottom, graphRight, graphBottom, 1.0f);

        // Horizontal grid lines (dashed)
        g.setColour(juce::Colour(0xff1e293b));
        for (float ratio : {0.25f, 0.5f, 0.75f})
        {
            float y = graphTop + graphHeight * ratio;
            float dashLength = 2.0f;
            float gapLength = 2.0f;
            for (float x = graphLeft; x < graphRight; x += dashLength + gapLength)
            {
                g.drawLine(x, y, juce::jmin(x + dashLength, graphRight), y, 1.0f);
            }
        }

        // Draw ADSR curve path
        juce::Path curvePath;
        curvePath.startNewSubPath(graphLeft, startY);
        curvePath.lineTo(attackPoint.x, attackPoint.y);
        curvePath.lineTo(decayPoint.x, decayPoint.y);
        curvePath.lineTo(sustainPoint.x, sustainPoint.y);
        curvePath.lineTo(releasePoint.x, releasePoint.y);

        // Fill with gradient
        juce::Path fillPath = curvePath;
        fillPath.lineTo(releasePoint.x, startY);
        fillPath.lineTo(graphLeft, startY);
        fillPath.closeSubPath();

        juce::ColourGradient gradient(
            juce::Colour(0x66f472b6), bounds.getCentreX(), graphTop,
            juce::Colour(0x1af472b6), bounds.getCentreX(), graphBottom,
            false
        );
        g.setGradientFill(gradient);
        g.fillPath(fillPath);

        // Draw curve line with glow
        g.setColour(juce::Colour(0xfff472b6));  // Pink #f472b6
        g.strokePath(curvePath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw control points
        auto drawControlPoint = [&](juce::Point<float> point, bool isHover, bool isDragging)
        {
            float radius = isDragging ? 7.0f : (isHover ? 6.0f : 5.0f);

            // Glow effect for active/hover
            if (isDragging || isHover)
            {
                g.setColour(juce::Colour(0x80f472b6));
                g.fillEllipse(point.x - radius - 3, point.y - radius - 3, (radius + 3) * 2, (radius + 3) * 2);
            }

            // Main circle
            g.setColour(juce::Colour(0xfff472b6));
            g.fillEllipse(point.x - radius, point.y - radius, radius * 2, radius * 2);

            // White border
            g.setColour(juce::Colours::white);
            g.drawEllipse(point.x - radius, point.y - radius, radius * 2, radius * 2, isDragging ? 2.5f : 2.0f);
        };

        drawControlPoint(attackPoint, hoverPoint == 0, dragPoint == 0);
        drawControlPoint(decayPoint, hoverPoint == 1, dragPoint == 1);
        drawControlPoint(sustainPoint, hoverPoint == 2, dragPoint == 2);
        drawControlPoint(releasePoint, hoverPoint == 3, dragPoint == 3);

        // Draw phase labels (use zone boundaries, not point positions)
        g.setColour(juce::Colour(0xff64748b));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));

        g.drawText("A", juce::Rectangle<float>(attackZoneStart, graphBottom + 5, zoneWidth, 15), juce::Justification::centred);
        g.drawText("D", juce::Rectangle<float>(decayZoneStart, graphBottom + 5, zoneWidth, 15), juce::Justification::centred);
        g.drawText("S", juce::Rectangle<float>(sustainZoneStart, graphBottom + 5, zoneWidth, 15), juce::Justification::centred);
        g.drawText("R", juce::Rectangle<float>(releaseZoneStart, graphBottom + 5, zoneWidth, 15), juce::Justification::centred);

        // Draw value labels at bottom
        float labelY = graphBottom + 35;
        g.setColour(juce::Colour(0xff64748b));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText("Attack", juce::Rectangle<float>(graphLeft, labelY, graphWidth / 4, 12), juce::Justification::centred);
        g.drawText("Decay", juce::Rectangle<float>(graphLeft + graphWidth / 4, labelY, graphWidth / 4, 12), juce::Justification::centred);
        g.drawText("Sustain", juce::Rectangle<float>(graphLeft + graphWidth / 2, labelY, graphWidth / 4, 12), juce::Justification::centred);
        g.drawText("Release", juce::Rectangle<float>(graphLeft + 3 * graphWidth / 4, labelY, graphWidth / 4, 12), juce::Justification::centred);

        g.setColour(juce::Colour(0xffd1d5db));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(juce::String(attackMs, 0) + "ms", juce::Rectangle<float>(graphLeft, labelY + 12, graphWidth / 4, 12), juce::Justification::centred);
        g.drawText(juce::String(decayMs, 0) + "ms", juce::Rectangle<float>(graphLeft + graphWidth / 4, labelY + 12, graphWidth / 4, 12), juce::Justification::centred);
        g.drawText(juce::String(sustainPercent, 0) + "%", juce::Rectangle<float>(graphLeft + graphWidth / 2, labelY + 12, graphWidth / 4, 12), juce::Justification::centred);
        g.drawText(juce::String(releaseMs, 0) + "ms", juce::Rectangle<float>(graphLeft + 3 * graphWidth / 4, labelY + 12, graphWidth / 4, 12), juce::Justification::centred);
    }

    void mouseMove(const juce::MouseEvent& event) override
    {
        auto pos = event.position;
        hoverPoint = getPointAtPosition(pos);
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        hoverPoint = -1;
        repaint();
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        dragPoint = getPointAtPosition(event.position);
        if (dragPoint != -1)
        {
            dragStartValue = getPointValue(dragPoint);
        }
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (dragPoint == -1 || !attackParam || !decayParam || !sustainParam || !releaseParam)
            return;

        auto pos = event.position;
        const float padding = 12.0f;
        const float valueLabelHeight = 35.0f;  // Must match paint() value
        const float graphWidth = getWidth() - (padding * 2);
        const float graphHeight = getHeight() - valueLabelHeight - (padding * 2);
        const float graphTop = padding;
        const float graphLeft = padding;

        // Fixed zones - each phase gets 25% of width
        const float zoneWidth = graphWidth / 4.0f;
        float attackZoneStart = graphLeft;
        float decayZoneStart = attackZoneStart + zoneWidth;
        float sustainZoneStart = decayZoneStart + zoneWidth;
        float releaseZoneStart = sustainZoneStart + zoneWidth;

        if (dragPoint == 0)  // Attack handle - controls attack time only
        {
            // Constrain to attack zone
            float normalizedX = juce::jlimit(0.0f, 1.0f, (pos.x - attackZoneStart) / zoneWidth);
            // Map normalized position to attack parameter range (1-5000ms)
            float newAttackMs = 1.0f + normalizedX * (5000.0f - 1.0f);
            attackParam->setValueNotifyingHost(attackParam->convertTo0to1(newAttackMs));
        }
        else if (dragPoint == 1)  // Decay handle - controls decay time AND sustain level
        {
            // Horizontal: decay time within decay zone
            float normalizedX = juce::jlimit(0.0f, 1.0f, (pos.x - decayZoneStart) / zoneWidth);
            // Map to decay parameter range (10-5000ms)
            float newDecayMs = 10.0f + normalizedX * (5000.0f - 10.0f);
            decayParam->setValueNotifyingHost(decayParam->convertTo0to1(newDecayMs));

            // Vertical: sustain level
            float normalizedY = juce::jlimit(0.0f, 1.0f, (pos.y - graphTop) / graphHeight);
            float newSustainPercent = juce::jlimit(0.0f, 100.0f, (1.0f - normalizedY) * 100.0f);
            sustainParam->setValueNotifyingHost(sustainParam->convertTo0to1(newSustainPercent));
        }
        else if (dragPoint == 2)  // Sustain handle - controls sustain level ONLY (vertical only)
        {
            // Only vertical movement - adjusts sustain level
            float normalizedY = juce::jlimit(0.0f, 1.0f, (pos.y - graphTop) / graphHeight);
            float newSustainPercent = juce::jlimit(0.0f, 100.0f, (1.0f - normalizedY) * 100.0f);
            sustainParam->setValueNotifyingHost(sustainParam->convertTo0to1(newSustainPercent));
            // Horizontal movement is completely ignored - sustain point stays fixed horizontally
        }
        else if (dragPoint == 3)  // Release handle - controls release time only
        {
            // Horizontal: release time within release zone
            float normalizedX = juce::jlimit(0.0f, 1.0f, (pos.x - releaseZoneStart) / zoneWidth);
            // Map to release parameter range (10-5000ms)
            float newReleaseMs = 10.0f + normalizedX * (5000.0f - 10.0f);
            releaseParam->setValueNotifyingHost(releaseParam->convertTo0to1(newReleaseMs));
        }

        repaint();
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        dragPoint = -1;
        repaint();
    }

private:
    int getPointAtPosition(juce::Point<float> pos)
    {
        const float hitRadius = 12.0f;  // Larger hitbox for easier clicking

        if (pos.getDistanceFrom(attackPoint) <= hitRadius) return 0;
        if (pos.getDistanceFrom(decayPoint) <= hitRadius) return 1;
        if (pos.getDistanceFrom(sustainPoint) <= hitRadius) return 2;
        if (pos.getDistanceFrom(releasePoint) <= hitRadius) return 3;

        return -1;
    }

    float getPointValue(int point)
    {
        if (!attackParam || !decayParam || !sustainParam || !releaseParam)
            return 0.0f;

        switch (point)
        {
            case 0: return attackParam->get();
            case 1: return decayParam->get();
            case 2: return sustainParam->get();
            case 3: return releaseParam->get();
            default: return 0.0f;
        }
    }

    juce::AudioParameterFloat* attackParam = nullptr;
    juce::AudioParameterFloat* decayParam = nullptr;
    juce::AudioParameterFloat* sustainParam = nullptr;
    juce::AudioParameterFloat* releaseParam = nullptr;

    juce::Point<float> attackPoint;
    juce::Point<float> decayPoint;
    juce::Point<float> sustainPoint;
    juce::Point<float> releasePoint;

    int hoverPoint = -1;
    int dragPoint = -1;
    float dragStartValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InteractiveADSREnvelope)
};
