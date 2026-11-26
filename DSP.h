#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <vector>

namespace exs
{
//==============================================================================
//  Filtros base
//==============================================================================
struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;

    void reset() { z1 = z2 = 0; }
    void setLowPass  (double sr, double f, double q = 0.70710678) { design (sr, f, q, false); }
    void setHighPass (double sr, double f, double q = 0.70710678) { design (sr, f, q, true); }
    void copyCoeffsFrom (const Biquad& o) { b0 = o.b0; b1 = o.b1; b2 = o.b2; a1 = o.a1; a2 = o.a2; }

    inline float process (float x) noexcept
    {
        const double in = x;
        const double y = b0 * in + z1;
        z1 = b1 * in - a1 * y + z2;
        z2 = b2 * in - a2 * y;
        return (float) y;
    }

private:
    void design (double sr, double f, double q, bool highpass)
    {
        f = juce::jlimit (10.0, sr * 0.49, f);
        const double w0 = juce::MathConstants<double>::twoPi * f / sr;
        const double cw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q);
        const double a0 = 1.0 + alpha;
        if (highpass) { b0 = (1.0 + cw) * 0.5 / a0;  b1 = -(1.0 + cw) / a0; }
        else          { b0 = (1.0 - cw) * 0.5 / a0;  b1 =  (1.0 - cw) / a0; }
        b2 = b0;
        a1 = -2.0 * cw / a0;
        a2 = (1.0 - alpha) / a0;
    }
};

// Linkwitz-Riley de 4º orden (2 Butterworth en cascada): LP + HP suman plano (all-pass)
struct LR4
{
    Biquad s1, s2;
    void setLow  (double sr, double f) { s1.setLowPass (sr, f);  s2.copyCoeffsFrom (s1); }
    void setHigh (double sr, double f) { s1.setHighPass (sr, f); s2.copyCoeffsFrom (s1); }
    inline float process (float x) noexcept { return s2.process (s1.process (x)); }
};

// Divide una señal en 3 bandas (grave / medio / agudo) que vuelven a sumar plano.
// El grave pasa además por un all-pass a la frecuencia alta para que las tres bandas estén en fase.
struct Crossover3
{
    LR4 lp1, hp1, lp2m, hp2m, lp2l, hp2l;

    void set (double sr, double f1, double f2)
    {
        lp1.setLow (sr, f1);   hp1.setHigh (sr, f1);
        lp2m.setLow (sr, f2);  hp2m.setHigh (sr, f2);
        lp2l.setLow (sr, f2);  hp2l.setHigh (sr, f2);
    }

    inline void split (float x, float& lo, float& mid, float& hi) noexcept
    {
        const float low = lp1.process (x), rest = hp1.process (x);
        mid = lp2m.process (rest);
        hi  = hp2m.process (rest);
        lo  = lp2l.process (low) + hp2l.process (low);
    }
};

//==============================================================================
//  Giro de fase sin latencia: dos cadenas de all-pass cuyas salidas difieren 90 grados (+-1.7)
//  entre 20 Hz y 22 kHz (red de 8 polos de Olli Niemitalo). Combinando A (rama I retrasada una muestra)
//  y B (rama Q):   cos(t)*A + sin(t)*B  =  A con la fase adelantada t grados, a TODAS las frecuencias.
//==============================================================================
struct Z2Allpass
{
    double a2 = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    void reset() { x1 = x2 = y1 = y2 = 0; }
    inline float process (float x) noexcept
    {
        const double y = a2 * ((double) x + y2) - x2;      // H(z) = (a^2 - z^-2) / (1 - a^2 z^-2)
        x2 = x1;  x1 = x;  y2 = y1;  y1 = y;
        return (float) y;
    }
};

struct PhaseChain
{
    std::array<Z2Allpass, 4> s;
    float z = 0.f;
    bool oneSampleDelay = false;

    void init (const double* a, bool delay)
    {
        for (size_t i = 0; i < 4; ++i) s[i].a2 = a[i] * a[i];
        oneSampleDelay = delay;
        reset();
    }
    void reset() { for (auto& st : s) st.reset();  z = 0.f; }

    inline float process (float x) noexcept
    {
        float y = x;
        for (auto& st : s) y = st.process (y);
        if (! oneSampleDelay) return y;
        const float out = z;  z = y;
        return out;
    }
};

inline constexpr double kPhaseI[4] = { 0.6923878, 0.9360654322959, 0.9882295226860, 0.9987488452737 };
inline constexpr double kPhaseQ[4] = { 0.4021921162426, 0.8561710882420, 0.9722909545651, 0.9952884791278 };

//==============================================================================
//  Difusor de Schroeder (all-pass con retardo): suena plano pero reparte la fase y el tiempo
//==============================================================================
struct SchroederAP
{
    std::vector<float> buf;
    int pos = 0;
    float g = 0.6f;

    void prepare (int delaySamples, float gain)
    {
        buf.assign ((size_t) std::max (1, delaySamples), 0.f);
        pos = 0;  g = gain;
    }
    inline float process (float x) noexcept
    {
        const float del = buf[(size_t) pos];
        const float v = x + g * del;
        buf[(size_t) pos] = v;
        if (++pos >= (int) buf.size()) pos = 0;
        return del - g * v;
    }
};

// Línea de retardo con interpolación lineal (para el Haas)
struct FracDelay
{
    std::vector<float> buf;
    int pos = 0;

    void prepare (int maxSamples) { buf.assign ((size_t) std::max (4, maxSamples), 0.f);  pos = 0; }
    inline void write (float x) noexcept { buf[(size_t) pos] = x;  if (++pos >= (int) buf.size()) pos = 0; }
    inline float read (float delaySamples) const noexcept
    {
        const int size = (int) buf.size();
        float t = (float) pos - 1.f - delaySamples;
        while (t < 0.f) t += (float) size;
        const int i0 = (int) t % size;
        const float frac = t - std::floor (t);
        const int i1 = (i0 + 1) % size;
        return buf[(size_t) i0] * (1.f - frac) + buf[(size_t) i1] * frac;
    }
};

//==============================================================================
//  EXPANSOR
//    entrada -> [swap / invertir polaridad] -> [giro de fase R respecto a L]
//            -> M/S -> ancho por bandas (solo el Side cambia) -> + Espacio + Haas (se inyectan en el Side)
//            -> L/R -> mezcla dry/wet -> ganancia
//  Espacio, Haas y ancho solo tocan el Side: la suma a mono (L+R) NO cambia, así que no hay problemas de fase.
//  (el giro de fase sí cambia la relación L/R, por eso es un control aparte)
//==============================================================================
struct ExpansorParams
{
    float xoverLo = 150.f, xoverHi = 4000.f;
    float widthLo = 1.f, widthMid = 1.f, widthHi = 1.f;      // 0..2
    float spaceAmt = 0.f, spaceHpHz = 300.f;                  // 0..1
    float haasMs = 10.f, haasAmt = 0.f;                       // 0..1
    float phaseDeg = 0.f;
    bool  invL = false, invR = false, swapLR = false, mono = false;
    float mix = 1.f, gainDb = 0.f;
};

class Expansor
{
public:
    void prepare (double sr)
    {
        sampleRate = sr;
        chL.init (kPhaseI, true);  chR.init (kPhaseI, true);  chQ.init (kPhaseQ, false);
        dryL.init (kPhaseI, true); dryR.init (kPhaseI, true);

        const double ms[3] = { 3.1, 4.7, 7.3 };
        const float  gs[3] = { 0.62f, 0.58f, 0.54f };
        for (size_t i = 0; i < 3; ++i) diff[i].prepare ((int) std::lround (sr * ms[i] * 0.001), gs[i]);

        haasDelay.prepare ((int) (sr * 0.05) + 8);
        haasHp.setHighPass (sr, 150.0);
        haasHp.reset();  spaceHp.reset();

        for (auto* s : { &wLo, &wMid, &wHi, &spaceAmt, &haasAmt, &haasTime, &angle, &mixS, &gainS })
            s->reset (sr, 0.03);
        setParams (ExpansorParams{});
        snap();
    }

    // Salta directamente a los valores objetivo (se usa al arrancar para evitar un fundido de entrada)
    void snap()
    {
        for (auto* s : { &wLo, &wMid, &wHi, &spaceAmt, &haasAmt, &haasTime, &angle, &mixS, &gainS })
            s->setCurrentAndTargetValue (s->getTargetValue());
    }

    void setParams (const ExpansorParams& p)
    {
        flags = p;
        wLo.setTargetValue (p.widthLo);  wMid.setTargetValue (p.widthMid);  wHi.setTargetValue (p.widthHi);
        spaceAmt.setTargetValue (p.spaceAmt);
        haasAmt.setTargetValue (p.haasAmt);
        haasTime.setTargetValue (p.haasMs * 0.001f * (float) sampleRate);
        angle.setTargetValue (p.phaseDeg);
        mixS.setTargetValue (p.mix);
        gainS.setTargetValue (juce::Decibels::decibelsToGain (p.gainDb));
        xoM.set (sampleRate, p.xoverLo, p.xoverHi);
        xoS.set (sampleRate, p.xoverLo, p.xoverHi);
        spaceHp.setHighPass (sampleRate, p.spaceHpHz);
    }

    void process (float* l, float* r, int n)
    {
        constexpr float degToRad = juce::MathConstants<float>::pi / 180.f;

        for (int i = 0; i < n; ++i)
        {
            const float l0 = l[i], r0 = r[i];

            // dry coherente (misma red all-pass que el wet)
            const float dL = dryL.process (l0), dR = dryR.process (r0);

            // 1) swap + polaridad
            float xl = flags.swapLR ? r0 : l0, xr = flags.swapLR ? l0 : r0;
            if (flags.invL) xl = -xl;
            if (flags.invR) xr = -xr;

            // 2) giro de fase de R respecto a L
            const float aL = chL.process (xl);
            const float aR = chR.process (xr);
            const float qR = chQ.process (xr);
            const float ang = angle.getNextValue() * degToRad;
            const float rL = aL;
            const float rR = std::cos (ang) * aR + std::sin (ang) * qR;

            // 3) M/S y ancho por bandas: la parte Mid no cambia, la Side se escala por banda
            const float m = 0.5f * (rL + rR), sd = 0.5f * (rL - rR);
            float mLo, mMid, mHi, sLo, sMid, sHi;
            xoM.split (m, mLo, mMid, mHi);
            xoS.split (sd, sLo, sMid, sHi);
            const float mOut = mLo + mMid + mHi;
            float sOut = wLo.getNextValue() * sLo + wMid.getNextValue() * sMid + wHi.getNextValue() * sHi;

            // 4) Espacio: Mid (sin graves) difundido por all-passes, inyectado en el Side
            float d = spaceHp.process (mOut);
            for (auto& ap : diff) d = ap.process (d);
            sOut += spaceAmt.getNextValue() * d;

            // 5) Haas: Mid retrasado, inyectado en el Side
            haasDelay.write (haasHp.process (mOut));
            sOut += haasAmt.getNextValue() * haasDelay.read (juce::jmax (1.f, haasTime.getNextValue()));

            // 6) de vuelta a L/R
            float wetL = mOut + sOut, wetR = mOut - sOut;
            if (flags.mono) wetL = wetR = mOut;

            const float mx = mixS.getNextValue(), g = gainS.getNextValue();
            l[i] = (dL + (wetL - dL) * mx) * g;
            r[i] = (dR + (wetR - dR) * mx) * g;
        }
    }

private:
    double sampleRate = 44100.0;
    ExpansorParams flags;
    PhaseChain chL, chR, chQ, dryL, dryR;
    Crossover3 xoM, xoS;
    Biquad spaceHp, haasHp;
    std::array<SchroederAP, 3> diff;
    FracDelay haasDelay;
    juce::SmoothedValue<float> wLo, wMid, wHi, spaceAmt, haasAmt, haasTime, angle, mixS, gainS;
};

} // namespace exs
