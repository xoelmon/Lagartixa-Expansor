#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
//  Parámetros
//==============================================================================
static String fmtHz (float v, int)  { return v >= 1000.f ? String (v / 1000.f, v >= 10000.f ? 1 : 2) + " kHz" : String ((int) std::round (v)) + " Hz"; }
static String fmtMs (float v, int)  { return v < 10.f ? String (v, 1) + " ms" : String ((int) std::round (v)) + " ms"; }
static String fmtPct (float v, int) { return String ((int) std::round (v)) + " %"; }
static String fmtDb (float v, int)  { return String (v, 1) + " dB"; }
static String fmtDeg (float v, int) { return String ((int) std::round (v)) + " °"; }

static float parseNumber (const String& t) { return t.retainCharacters ("0123456789.-").getFloatValue(); }
static float parseHz (const String& t)
{
    const float v = parseNumber (t);
    return t.containsIgnoreCase ("k") ? v * 1000.f : v;
}

static NormalisableRange<float> rangeWithCentre (float lo, float hi, float centre, float step = 0.f)
{
    NormalisableRange<float> r (lo, hi, step);
    r.setSkewForCentre (centre);
    return r;
}

AudioProcessorValueTreeState::ParameterLayout ExpansorProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto addFloat = [&] (const char* id, const String& name, NormalisableRange<float> range, float def,
                         std::function<String (float, int)> toText, std::function<float (const String&)> fromText)
    {
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, range, def,
                        AudioParameterFloatAttributes().withStringFromValueFunction (std::move (toText))
                                                        .withValueFromStringFunction (std::move (fromText))));
    };
    auto addBool = [&] (const char* id, const String& name, bool def)
    {
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
    };

    // ---- Bandas de ancho ----
    addFloat (ids::xoverLo, "Crossover Low",  rangeWithCentre (60.f, 400.f, 150.f),      150.f,  fmtHz, parseHz);
    addFloat (ids::xoverHi, "Crossover High", rangeWithCentre (1000.f, 12000.f, 4000.f), 4000.f, fmtHz, parseHz);
    addFloat (ids::wLo,  "Width Low",  NormalisableRange<float> (0.f, 200.f, 1.f), 100.f, fmtPct, parseNumber);
    addFloat (ids::wMid, "Width Mid",  NormalisableRange<float> (0.f, 200.f, 1.f), 125.f, fmtPct, parseNumber);
    addFloat (ids::wHi,  "Width High", NormalisableRange<float> (0.f, 200.f, 1.f), 150.f, fmtPct, parseNumber);

    // ---- Espacio y Haas ----
    addFloat (ids::spaceAmt, "Space Amount", NormalisableRange<float> (0.f, 100.f, 1.f), 20.f, fmtPct, parseNumber);
    addFloat (ids::spaceHp,  "Space Focus",  rangeWithCentre (100.f, 2000.f, 400.f), 300.f, fmtHz, parseHz);
    addFloat (ids::haasMs,   "Haas Time",    rangeWithCentre (0.5f, 30.f, 8.f), 10.f, fmtMs, parseNumber);
    addFloat (ids::haasAmt,  "Haas Amount",  NormalisableRange<float> (0.f, 100.f, 1.f), 0.f, fmtPct, parseNumber);

    // ---- Fase ----
    addFloat (ids::phaseDeg, "Phase Rotation", NormalisableRange<float> (-180.f, 180.f, 1.f), 0.f, fmtDeg, parseNumber);
    addBool (ids::invL,   "Invert L", false);
    addBool (ids::invR,   "Invert R", false);
    addBool (ids::swapLR, "Swap L/R", false);

    // ---- Salida ----
    addFloat (ids::mix,     "Mix",         NormalisableRange<float> (0.f, 100.f, 1.f), 100.f, fmtPct, parseNumber);
    addFloat (ids::outGain, "Output Gain", NormalisableRange<float> (-12.f, 12.f, 0.1f), 0.f, fmtDb, parseNumber);
    addBool (ids::monoCheck, "Mono Check", false);

    return layout;
}

//==============================================================================
ExpansorProcessor::ExpansorProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "EXPANSOR", createParameterLayout())
{
}

bool ExpansorProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == AudioChannelSet::stereo();
}

exs::ExpansorParams ExpansorProcessor::readParams() const
{
    exs::ExpansorParams p;
    p.xoverLo = get (ids::xoverLo);  p.xoverHi = get (ids::xoverHi);
    p.widthLo = get (ids::wLo) * 0.01f;  p.widthMid = get (ids::wMid) * 0.01f;  p.widthHi = get (ids::wHi) * 0.01f;
    p.spaceAmt = get (ids::spaceAmt) * 0.01f;  p.spaceHpHz = get (ids::spaceHp);
    p.haasMs = get (ids::haasMs);  p.haasAmt = get (ids::haasAmt) * 0.01f;
    p.phaseDeg = get (ids::phaseDeg);
    p.invL = on (ids::invL);  p.invR = on (ids::invR);  p.swapLR = on (ids::swapLR);  p.mono = on (ids::monoCheck);
    p.mix = get (ids::mix) * 0.01f;  p.gainDb = get (ids::outGain);
    return p;
}

void ExpansorProcessor::prepareToPlay (double sr, int)
{
    sampleRate = sr;
    corrCoef = std::exp (-1.0 / (0.3 * sr));                  // medias de 300 ms para la correlación
    sumLL = sumRR = sumLR = sumM = sumS = 0.0;
    dsp.prepare (sr);
    dsp.setParams (readParams());
    dsp.snap();
    gonioFifo.reset();
}

void ExpansorProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, n);
    if (buffer.getNumChannels() < 2 || n == 0)
        return;

    float* l = buffer.getWritePointer (0);
    float* r = buffer.getWritePointer (1);

    dsp.setParams (readParams());
    dsp.process (l, r, n);

    // ---- Medición de la salida: picos L/R/M/S, correlación y "ancho" ----
    float pL = 0.f, pR = 0.f, pM = 0.f, pS = 0.f;
    const double a = corrCoef;
    for (int i = 0; i < n; ++i)
    {
        const double x = l[i], y = r[i];
        const double m = 0.5 * (x + y), s = 0.5 * (x - y);
        sumLL = a * sumLL + (1.0 - a) * x * x;
        sumRR = a * sumRR + (1.0 - a) * y * y;
        sumLR = a * sumLR + (1.0 - a) * x * y;
        sumM  = a * sumM  + (1.0 - a) * m * m;
        sumS  = a * sumS  + (1.0 - a) * s * s;
        pL = jmax (pL, std::abs (l[i]));  pR = jmax (pR, std::abs (r[i]));
        pM = jmax (pM, (float) std::abs (m));  pS = jmax (pS, (float) std::abs (s));
    }
    storeMax (peakL, pL);  storeMax (peakR, pR);  storeMax (peakM, pM);  storeMax (peakS, pS);

    const double energy = sumLL * sumRR;
    if (energy > 1.0e-14)
    {
        correlation.store ((float) jlimit (-1.0, 1.0, sumLR / std::sqrt (energy)));
        widthIndex.store ((float) jmin (4.0, std::sqrt (sumS / (sumM + 1.0e-12))));
    }
    else
    {
        correlation.store (1.f);
        widthIndex.store (0.f);
    }

    // ---- Muestras para el vectorscopio ----
    const int toWrite = jmin (n, gonioFifo.getFreeSpace());
    if (toWrite > 0)
    {
        int s1, z1, s2, z2;
        gonioFifo.prepareToWrite (toWrite, s1, z1, s2, z2);
        for (int i = 0; i < z1; ++i) { gonioL[(size_t) (s1 + i)] = l[i];       gonioR[(size_t) (s1 + i)] = r[i]; }
        for (int i = 0; i < z2; ++i) { gonioL[(size_t) (s2 + i)] = l[z1 + i];  gonioR[(size_t) (s2 + i)] = r[z1 + i]; }
        gonioFifo.finishedWrite (z1 + z2);
    }
}

//==============================================================================
AudioProcessorEditor* ExpansorProcessor::createEditor() { return new ExpansorEditor (*this); }

void ExpansorProcessor::getStateInformation (MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void ExpansorProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (ValueTree::fromXml (*xml));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ExpansorProcessor(); }
