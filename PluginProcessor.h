#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP.h"
#include "Params.h"

class ExpansorProcessor : public juce::AudioProcessor
{
public:
    ExpansorProcessor();
    ~ExpansorProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "LARGARTIXA SOUNDLAB EXPANSOR"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;

    // --- Datos para la interfaz (el audio los escribe; la interfaz los lee) ---
    std::atomic<float> peakL { 0.f }, peakR { 0.f }, peakM { 0.f }, peakS { 0.f };   // lineales; la interfaz los pone a 0
    std::atomic<float> correlation { 1.f };      // -1 .. +1
    std::atomic<float> widthIndex  { 0.f };      // relación Side/Mid de la salida (1.0 = 100 %)

    // Cola de muestras L/R de la salida para el vectorscopio
    juce::AbstractFifo gonioFifo { 16384 };
    std::array<float, 16384> gonioL {}, gonioR {};

private:
    exs::ExpansorParams readParams() const;
    float get (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    bool  on  (const char* id) const { return get (id) > 0.5f; }

    static void storeMax (std::atomic<float>& a, float v) noexcept
    {
        float cur = a.load (std::memory_order_relaxed);
        while (v > cur && ! a.compare_exchange_weak (cur, v)) {}
    }

    exs::Expansor dsp;
    double sampleRate = 44100.0, corrCoef = 0.999;
    double sumLL = 0, sumRR = 0, sumLR = 0, sumM = 0, sumS = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExpansorProcessor)
};
