#pragma once

#include <cmath>
#include <array>

/**
 * Pre-computed lookup tables for expensive math operations
 * These replace per-sample calls to std::cos, std::exp, std::pow, etc.
 */
class LookupTables
{
public:
    static constexpr int TABLE_SIZE = 4096;
    static constexpr float TABLE_SIZE_F = static_cast<float>(TABLE_SIZE);
    static constexpr float INV_TABLE_SIZE = 1.0f / TABLE_SIZE_F;

    // Singleton access
    static const LookupTables& getInstance()
    {
        static LookupTables instance;
        return instance;
    }

    //==============================================================================
    // Envelope lookups (input: 0-1 progress, output: 0-1 amplitude)

    /** Hann window: 0.5 * (1 - cos(2 * pi * x)) */
    inline float getHann(float progress01) const
    {
        return interpolate(hannTable, progress01);
    }

    /** Gaussian envelope: exp(-((x-0.5)*6)^2 * 0.5) */
    inline float getGaussian(float progress01) const
    {
        return interpolate(gaussianTable, progress01);
    }

    /** Exponential attack (0-0.5): 1 - exp(-10*x) */
    inline float getExpAttack(float progress01) const
    {
        // Map 0-0.5 to 0-1 for lookup
        float mapped = progress01 * 2.0f;
        return interpolate(expAttackTable, mapped);
    }

    /** Exponential decay (0.5-1): exp(-10*(x-0.5)) */
    inline float getExpDecay(float progress01) const
    {
        // Map 0.5-1 to 0-1 for lookup
        float mapped = (progress01 - 0.5f) * 2.0f;
        return interpolate(expDecayTable, mapped);
    }

    //==============================================================================
    // Pitch ratio lookup (semitones to ratio)
    // Range: -48 to +48 semitones (4 octaves each direction)

    /** Convert semitones to pitch ratio: 2^(semitones/12) */
    inline float getPitchRatio(float semitones) const
    {
        // Map -48 to +48 semitones to 0-1 range
        float normalized = (semitones + 48.0f) / 96.0f;
        normalized = std::max(0.0f, std::min(1.0f, normalized));
        return interpolate(pitchRatioTable, normalized);
    }

    //==============================================================================
    // Fast sin/cos for pan calculations (input: 0-1 maps to 0 to pi/2)

    inline float getSin(float x01) const
    {
        return interpolate(sinTable, x01);
    }

    inline float getCos(float x01) const
    {
        return interpolate(cosTable, x01);
    }

    //==============================================================================
    // Fast Gaussian random approximation (Box-Muller replacement)
    // Uses pre-computed sqrt(-2*log(x)) table

    inline float getGaussianRandom(float uniform01) const
    {
        // Clamp to avoid log(0)
        float clamped = std::max(0.0001f, std::min(0.9999f, uniform01));
        return interpolate(sqrtNegLogTable, clamped);
    }

    //==============================================================================
    // Filter coefficient lookup: exp(-2*pi*cutoff)
    // Input: normalized cutoff 0-1 (maps to 20Hz-20kHz at 44.1kHz)

    inline float getFilterCoeff(float normalizedCutoff) const
    {
        return interpolate(filterCoeffTable, normalizedCutoff);
    }

private:
    LookupTables()
    {
        buildTables();
    }

    void buildTables()
    {
        const float pi = 3.14159265358979323846f;
        const float twoPi = 2.0f * pi;

        for (int i = 0; i <= TABLE_SIZE; ++i)
        {
            float x = static_cast<float>(i) * INV_TABLE_SIZE;

            // Hann window: 0.5 * (1 - cos(2*pi*x))
            hannTable[i] = 0.5f * (1.0f - std::cos(twoPi * x));

            // Gaussian: exp(-((x-0.5)*6)^2 * 0.5)
            float gx = (x - 0.5f) * 6.0f;
            gaussianTable[i] = std::exp(-gx * gx * 0.5f);

            // Exponential attack: 1 - exp(-10*x) for x in 0-1 (maps to 0-0.5 progress)
            expAttackTable[i] = 1.0f - std::exp(-10.0f * x);

            // Exponential decay: exp(-10*x) for x in 0-1 (maps to 0.5-1 progress)
            expDecayTable[i] = std::exp(-10.0f * x);

            // Pitch ratio: 2^((x*96 - 48)/12) for -48 to +48 semitones
            float semitones = x * 96.0f - 48.0f;
            pitchRatioTable[i] = std::pow(2.0f, semitones / 12.0f);

            // Sin table: sin(x * pi/2) for x in 0-1
            sinTable[i] = std::sin(x * pi * 0.5f);

            // Cos table: cos(x * pi/2) for x in 0-1
            cosTable[i] = std::cos(x * pi * 0.5f);

            // sqrt(-2*log(x)) for Gaussian random (Box-Muller optimization)
            // x maps from 0.0001 to 0.9999 to avoid infinity
            float u = 0.0001f + x * 0.9998f;
            sqrtNegLogTable[i] = std::sqrt(-2.0f * std::log(u));

            // Filter coefficient: exp(-2*pi*cutoff)
            // Cutoff normalized: 0 = 20Hz/44100, 1 = 20000Hz/44100
            float minNorm = 20.0f / 44100.0f;
            float maxNorm = 20000.0f / 44100.0f;
            float cutoff = minNorm + x * (maxNorm - minNorm);
            filterCoeffTable[i] = std::exp(-twoPi * cutoff);
        }
    }

    inline float interpolate(const std::array<float, TABLE_SIZE + 1>& table, float x01) const
    {
        // Linear interpolation
        float indexF = x01 * TABLE_SIZE_F;
        int index0 = static_cast<int>(indexF);
        index0 = std::max(0, std::min(TABLE_SIZE - 1, index0));
        int index1 = index0 + 1;
        float frac = indexF - static_cast<float>(index0);

        return table[index0] + frac * (table[index1] - table[index0]);
    }

    // Tables with +1 size for safe interpolation at boundaries
    std::array<float, TABLE_SIZE + 1> hannTable;
    std::array<float, TABLE_SIZE + 1> gaussianTable;
    std::array<float, TABLE_SIZE + 1> expAttackTable;
    std::array<float, TABLE_SIZE + 1> expDecayTable;
    std::array<float, TABLE_SIZE + 1> pitchRatioTable;
    std::array<float, TABLE_SIZE + 1> sinTable;
    std::array<float, TABLE_SIZE + 1> cosTable;
    std::array<float, TABLE_SIZE + 1> sqrtNegLogTable;
    std::array<float, TABLE_SIZE + 1> filterCoeffTable;
};
