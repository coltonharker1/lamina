#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

/**
 * InteractiveADSREnvelope - Draggable ADSR curve visualization
 *
 * Features:
 * - Visual ADSR curve with draggable control points
 * - Attack/Decay adjust time horizontally
 * - Sustain adjusts level vertically
 * - Release adjusts time horizontally
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
        constexpr juce::uint32 paper = 0xfff4f1ea;
        constexpr juce::uint32 ink = 0xff050505;
        constexpr juce::uint32 quietInk = 0xaa000000;
        constexpr juce::uint32 faintInk = 0x18000000;

        g.fillAll(juce::Colour(paper));

        g.setColour(juce::Colour(ink));
        g.drawRect(bounds, 1);

        auto title = bounds.removeFromTop(titleStripHeight);
        g.drawLine(static_cast<float>(title.getX()), static_cast<float>(title.getBottom()) - 0.5f,
                   static_cast<float>(title.getRight()), static_cast<float>(title.getBottom()) - 0.5f, 1.0f);
        g.setFont(juce::Font(juce::FontOptions(7.5f)));
        g.drawText("ENVELOPE CONTOUR", title.reduced(10, 0), juce::Justification::centredLeft);
        g.drawText("A / D / S / R", title.reduced(10, 0), juce::Justification::centredRight);

        // Get current ADSR values (parameters are in milliseconds and 0-100 for sustain)
        float attackMs = attackParam ? attackParam->get() : 10.0f;
        float decayMs = decayParam ? decayParam->get() : 300.0f;
        float sustainPercent = sustainParam ? sustainParam->get() : 80.0f;
        float sustain = sustainPercent / 100.0f;  // Convert to 0-1 for visualization
        float releaseMs = releaseParam ? releaseParam->get() : 500.0f;

        const auto plot = getPlotMetrics(bounds);
        const float graphWidth = plot.graphWidth;
        const float graphHeight = plot.graphHeight;
        const float graphTop = plot.graphTop;
        const float graphBottom = plot.graphBottom;
        const float graphLeft = plot.graphLeft;
        const float graphRight = plot.graphRight;

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

        // Calculate control point X positions based on parameter values within each zone
        float attackNormalized = timeToVisual(attackMs, 1.0f, 5000.0f);
        float attackX = attackZoneStart + attackNormalized * zoneWidth;

        float decayNormalized = timeToVisual(decayMs, 10.0f, 5000.0f);
        float decayX = decayZoneStart + decayNormalized * zoneWidth;

        // Sustain: fixed in middle of sustain zone (horizontal position doesn't change)
        float sustainPointX = sustainZoneStart + zoneWidth * 0.5f;

        float releaseNormalized = timeToVisual(releaseMs, 10.0f, 5000.0f);
        float releaseX = releaseZoneStart + releaseNormalized * zoneWidth;

        // Y positions based on levels
        float sustainY = graphTop + graphHeight * (1.0f - sustain);
        float peakY = graphTop + 2.0f;
        float startY = graphBottom;
        float endY = graphBottom;

        // Store for mouse interaction
        attackPoint = {attackX, peakY};
        decayPoint = {decayX, sustainY};
        sustainPoint = {sustainPointX, sustainY};
        releasePoint = {releaseX, endY};

        g.setColour(juce::Colour(faintInk));
        for (int i = 1; i < 4; ++i)
        {
            const float x = graphLeft + zoneWidth * static_cast<float>(i);
            g.drawLine(x, graphTop, x, graphBottom, 0.65f);
        }
        for (float ratio : { 0.25f, 0.5f, 0.75f })
        {
            const float y = graphTop + graphHeight * ratio;
            for (float x = graphLeft; x < graphRight; x += 6.0f)
                g.drawLine(x, y, juce::jmin(x + 2.0f, graphRight), y, 0.55f);
        }
        g.setColour(juce::Colour(ink).withAlpha(0.62f));
        g.drawLine(graphLeft, graphBottom, graphRight, graphBottom, 1.0f);

        // Draw ADSR curve path
        juce::Path curvePath;
        curvePath.startNewSubPath(graphLeft, startY);
        curvePath.lineTo(attackPoint.x, attackPoint.y);
        curvePath.lineTo(decayPoint.x, decayPoint.y);
        curvePath.lineTo(sustainPoint.x, sustainPoint.y);
        curvePath.lineTo(releasePoint.x, releasePoint.y);

        juce::Path fillPath = curvePath;
        fillPath.lineTo(releasePoint.x, startY);
        fillPath.lineTo(graphLeft, startY);
        fillPath.closeSubPath();

        g.setColour(juce::Colour(ink).withAlpha(0.045f));
        g.fillPath(fillPath);

        g.setColour(juce::Colour(ink));
        g.strokePath(curvePath, juce::PathStrokeType(1.9f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(juce::Colour(ink).withAlpha(0.24f));
        g.strokePath(curvePath, juce::PathStrokeType(4.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw control points
        auto drawControlPoint = [&](juce::Point<float> point, bool isHover, bool isDragging)
        {
            float radius = isDragging ? 6.4f : (isHover ? 5.8f : 5.0f);
            g.setColour(juce::Colour(paper));
            g.fillEllipse(point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f);
            g.setColour(juce::Colour(ink));
            g.drawEllipse(point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f, isDragging ? 1.8f : 1.2f);
            g.drawLine(point.x - radius * 0.55f, point.y, point.x + radius * 0.55f, point.y, 0.8f);
            g.drawLine(point.x, point.y - radius * 0.55f, point.x, point.y + radius * 0.55f, 0.8f);
        };

        drawControlPoint(attackPoint, hoverPoint == 0, dragPoint == 0);
        drawControlPoint(decayPoint, hoverPoint == 1, dragPoint == 1);
        drawControlPoint(sustainPoint, hoverPoint == 2, dragPoint == 2);
        drawControlPoint(releasePoint, hoverPoint == 3, dragPoint == 3);

        g.setColour(juce::Colour(quietInk));
        g.setFont(juce::Font(juce::FontOptions(8.0f)));

        g.drawText("A", juce::Rectangle<float>(attackZoneStart, graphBottom + 1, zoneWidth, phaseLabelHeight), juce::Justification::centred);
        g.drawText("D", juce::Rectangle<float>(decayZoneStart, graphBottom + 1, zoneWidth, phaseLabelHeight), juce::Justification::centred);
        g.drawText("S", juce::Rectangle<float>(sustainZoneStart, graphBottom + 1, zoneWidth, phaseLabelHeight), juce::Justification::centred);
        g.drawText("R", juce::Rectangle<float>(releaseZoneStart, graphBottom + 1, zoneWidth, phaseLabelHeight), juce::Justification::centred);
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
        auto bounds = getLocalBounds();
        bounds.removeFromTop(titleStripHeight);

        const auto plot = getPlotMetrics(bounds);
        const float graphWidth = plot.graphWidth;
        const float graphHeight = plot.graphHeight;
        const float graphTop = plot.graphTop;
        const float graphLeft = plot.graphLeft;

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
            float newAttackMs = visualToTime(normalizedX, 1.0f, 5000.0f);
            attackParam->setValueNotifyingHost(attackParam->convertTo0to1(newAttackMs));
        }
        else if (dragPoint == 1)  // Decay handle - controls decay time AND sustain level
        {
            // Horizontal: decay time within decay zone
            float normalizedX = juce::jlimit(0.0f, 1.0f, (pos.x - decayZoneStart) / zoneWidth);
            float newDecayMs = visualToTime(normalizedX, 10.0f, 5000.0f);
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
            float newReleaseMs = visualToTime(normalizedX, 10.0f, 5000.0f);
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
    static constexpr int titleStripHeight = 24;
    static constexpr float phaseLabelHeight = 15.0f;

    struct PlotMetrics
    {
        float graphLeft = 0.0f;
        float graphTop = 0.0f;
        float graphRight = 0.0f;
        float graphBottom = 0.0f;
        float graphWidth = 0.0f;
        float graphHeight = 0.0f;
    };

    static PlotMetrics getPlotMetrics(juce::Rectangle<int> contentBounds)
    {
        constexpr float horizontalPadding = 9.0f;
        constexpr float topPadding = 15.0f;
        constexpr float bottomPadding = 7.0f;

        PlotMetrics metrics;
        metrics.graphLeft = static_cast<float>(contentBounds.getX()) + horizontalPadding;
        metrics.graphTop = static_cast<float>(contentBounds.getY()) + topPadding;
        metrics.graphWidth = static_cast<float>(contentBounds.getWidth()) - horizontalPadding * 2.0f;
        metrics.graphHeight = static_cast<float>(contentBounds.getHeight()) - topPadding - bottomPadding - phaseLabelHeight;
        metrics.graphRight = metrics.graphLeft + metrics.graphWidth;
        metrics.graphBottom = metrics.graphTop + metrics.graphHeight;
        return metrics;
    }

    static float timeToVisual(float value, float minValue, float maxValue)
    {
        value = juce::jlimit(minValue, maxValue, value);
        return std::log(value / minValue) / std::log(maxValue / minValue);
    }

    static float visualToTime(float normalised, float minValue, float maxValue)
    {
        normalised = juce::jlimit(0.0f, 1.0f, normalised);
        return minValue * std::pow(maxValue / minValue, normalised);
    }

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
