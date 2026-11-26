#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

//==============================================================================
//  Paleta: negro carbón, acero cepillado y rojo sangre
//==============================================================================
namespace col
{
    inline const juce::Colour bg       { 0xff09090b };
    inline const juce::Colour panelTop { 0xff2b2c31 };
    inline const juce::Colour panelBot { 0xff131315 };
    inline const juce::Colour steelHi  { 0xffa0a4ac };
    inline const juce::Colour steel    { 0xff6c7078 };
    inline const juce::Colour steelDk  { 0xff2c2d31 };
    inline const juce::Colour blood    { 0xffc8101a };
    inline const juce::Colour glow     { 0xffff3b30 };
    inline const juce::Colour ember    { 0xffff7a1a };
    inline const juce::Colour bone     { 0xffe9e5dc };
    inline const juce::Colour text     { 0xffb4b7be };
    inline const juce::Colour dim      { 0xff7b7e86 };
    inline const juce::Colour screen   { 0xff0b0b0d };
}

namespace gfx
{
inline juce::Font titleFont (float h)
{
    juce::Font f (juce::FontOptions ("Impact", h, juce::Font::plain));
    f.setExtraKerningFactor (0.07f);
    return f;
}

inline juce::Font uiFont (float h, bool bold = true)
{
    juce::Font f (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
    f.setExtraKerningFactor (0.04f);
    return f;
}

// Textura de "acero cepillado": se repite en mosaico sobre los paneles
inline juce::Image makeBrushedTexture()
{
    juce::Image img (juce::Image::ARGB, 256, 256, true);
    juce::Graphics g (img);
    juce::Random rnd (1337);
    for (int y = 0; y < 256; ++y)
        for (int x = 0; x < 256;)
        {
            const int len = 8 + rnd.nextInt (90);
            const bool light = rnd.nextBool();
            g.setColour ((light ? juce::Colours::white : juce::Colours::black).withAlpha (rnd.nextFloat() * 0.6f));
            g.fillRect (x, y, len, 1);
            x += len;
        }
    return img;
}

inline void drawScrew (juce::Graphics& g, juce::Point<float> c, float angle)
{
    const float r = 4.5f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb0b4bc), c.x - 2, c.y - 2,
                                              juce::Colour (0xff25262a), c.x + r, c.y + r, true));
    g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
    g.setColour (juce::Colour (0xff09090a));
    g.drawEllipse (c.x - r, c.y - r, r * 2, r * 2, 1.f);
    juce::Path slot;
    slot.addRectangle (-r * 0.8f, -0.8f, r * 1.6f, 1.6f);
    g.fillPath (slot, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
}

inline void drawMetalPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::Image& tex)
{
    juce::Path p;
    p.addRoundedRectangle (r, 7.f);

    g.setColour (juce::Colours::black.withAlpha (0.5f));            // sombra
    g.fillRoundedRectangle (r.translated (0.f, 2.f), 7.f);

    g.setGradientFill (juce::ColourGradient (col::panelTop, r.getX(), r.getY(), col::panelBot, r.getX(), r.getBottom(), false));
    g.fillPath (p);
    g.setTiledImageFill (tex, 0, 0, 0.11f);
    g.fillPath (p);

    g.setColour (juce::Colour (0x40ffffff));                         // brillo del borde superior
    g.drawLine (r.getX() + 8.f, r.getY() + 1.f, r.getRight() - 8.f, r.getY() + 1.f, 1.f);
    g.setColour (juce::Colour (0xff050506));
    g.strokePath (p, juce::PathStrokeType (1.6f));

    drawScrew (g, { r.getX() + 9.f,     r.getY() + 9.f },      0.4f);
    drawScrew (g, { r.getRight() - 9.f, r.getY() + 9.f },      1.9f);
    drawScrew (g, { r.getX() + 9.f,     r.getBottom() - 9.f }, 2.6f);
    drawScrew (g, { r.getRight() - 9.f, r.getBottom() - 9.f }, 0.9f);
}

inline void drawScreen (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff141417), r.getX(), r.getY(), col::screen, r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (juce::Colour (0xff000000));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.f, 1.5f);
    g.setColour (juce::Colour (0x22ffffff));
    g.drawRoundedRectangle (r.expanded (1.f), 5.f, 1.f);
}
} // namespace gfx

//==============================================================================
//  LookAndFeel: knobs de acero dentados con aguja roja, botones con LED
//==============================================================================
class MetalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MetalLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, col::text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, col::text);
        setColour (juce::TextButton::textColourOffId, col::bone);
        setColour (juce::TextButton::textColourOnId, col::bone);
        setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0e0e10));
        setColour (juce::TextEditor::textColourId, col::bone);
        setColour (juce::TextEditor::outlineColourId, col::blood);
        setColour (juce::TextEditor::focusedOutlineColourId, col::glow);
        setColour (juce::TextEditor::highlightColourId, col::blood);
        setColour (juce::CaretComponent::caretColourId, col::glow);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        using namespace juce;
        const auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        g.setGradientFill (ColourGradient (down ? Colour (0xff4a1414) : (over ? Colour (0xff3d3f46) : Colour (0xff2e3035)), 0.f, r.getY(),
                                           Colour (0xff131315), 0.f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 4.f);
        g.setColour (Colour (0xff050506));
        g.drawRoundedRectangle (r, 4.f, 1.5f);
        g.setColour (over || down ? col::blood : Colour (0x30ffffff));
        g.drawRoundedRectangle (r.reduced (1.5f), 3.f, 1.f);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override { return gfx::uiFont (13.f); }

    juce::Font getLabelFont (juce::Label& l) override
    {
        if (dynamic_cast<juce::Slider*> (l.getParentComponent()) != nullptr)
            return gfx::uiFont (12.5f, false);
        return l.getFont();
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider& s) override
    {
        using namespace juce;
        const auto bounds = Rectangle<int> (x, y, w, h).toFloat().reduced (1.f);
        const float r = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto c = bounds.getCentre();
        const float angle = startAngle + pos * (endAngle - startAngle);
        const bool bipolar = (bool) s.getProperties().getWithDefault ("bipolar", false);
        const bool enabled = s.isEnabled();

        // --- carril + arco de valor ---
        const float arcR = r - 2.5f;
        Path track;
        track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, endAngle, true);
        g.setColour (Colour (0xff040405));
        g.strokePath (track, PathStrokeType (4.5f, PathStrokeType::curved, PathStrokeType::rounded));

        float a0 = startAngle, a1 = angle;
        if (bipolar)
        {
            const float mid = startAngle + 0.5f * (endAngle - startAngle);
            a0 = jmin (mid, angle);  a1 = jmax (mid, angle);
        }
        if (a1 - a0 > 0.01f)
        {
            Path val;
            val.addCentredArc (c.x, c.y, arcR, arcR, 0.f, a0, a1, true);
            g.setColour (col::glow.withAlpha (enabled ? 0.28f : 0.08f));
            g.strokePath (val, PathStrokeType (8.f, PathStrokeType::curved, PathStrokeType::rounded));
            g.setColour (enabled ? col::glow : col::dim);
            g.strokePath (val, PathStrokeType (3.f, PathStrokeType::curved, PathStrokeType::rounded));
        }

        // --- cuerpo del knob: engranaje dentado ("knurled") ---
        const float rk = arcR - 5.f;
        g.setColour (Colours::black.withAlpha (0.65f));
        g.fillEllipse (c.x - rk - 1.f, c.y - rk + 2.f, rk * 2.f + 2.f, rk * 2.f + 2.f);

        const int teeth = 22;
        Path gear;
        for (int i = 0; i < teeth * 2; ++i)
        {
            const float a  = (float) i * MathConstants<float>::pi / (float) teeth;
            const float rr = (i % 2 == 0) ? rk : rk - 3.2f;
            const float px = c.x + std::sin (a) * rr, py = c.y - std::cos (a) * rr;
            if (i == 0) gear.startNewSubPath (px, py); else gear.lineTo (px, py);
        }
        gear.closeSubPath();
        g.setGradientFill (ColourGradient (Colour (0xff80848c), c.x - rk * 0.7f, c.y - rk * 0.9f,
                                           Colour (0xff141517), c.x + rk * 0.7f, c.y + rk, false));
        g.fillPath (gear);
        g.setColour (Colour (0xff050506));
        g.strokePath (gear, PathStrokeType (1.2f));

        // --- tapa central ---
        const float rc = rk * 0.66f;
        g.setGradientFill (ColourGradient (Colour (0xff1b1c1f), c.x, c.y - rc, Colour (0xff3d3f46), c.x, c.y + rc, false));
        g.fillEllipse (c.x - rc, c.y - rc, rc * 2.f, rc * 2.f);
        g.setColour (Colour (0x30ffffff));
        g.drawEllipse (c.x - rc, c.y - rc, rc * 2.f, rc * 2.f, 1.f);
        g.setColour (Colour (0xff050506));
        g.drawEllipse (c.x - rc - 1.f, c.y - rc - 1.f, rc * 2.f + 2.f, rc * 2.f + 2.f, 1.f);

        // --- aguja ---
        Path needle;
        needle.addRoundedRectangle (-1.8f, -rc * 0.95f, 3.6f, rc * 0.62f, 1.6f);
        const auto xf = AffineTransform::rotation (angle).translated (c.x, c.y);
        g.setColour (col::glow.withAlpha (enabled ? 0.35f : 0.f));
        g.strokePath (needle, PathStrokeType (4.f), xf);
        g.setColour (enabled ? col::bone : col::dim);
        g.fillPath (needle, xf);
        g.setColour (enabled ? col::glow : col::steel);
        Path tip;
        tip.addRoundedRectangle (-1.8f, -rc * 0.95f, 3.6f, rc * 0.25f, 1.4f);
        g.fillPath (tip, xf);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool /*down*/) override
    {
        using namespace juce;
        auto r = b.getLocalBounds().toFloat();
        const float led = jmin (r.getHeight() - 8.f, 15.f);
        const Rectangle<float> ledR (r.getX() + 4.f, r.getCentreY() - led * 0.5f, led, led);
        const bool on = b.getToggleState();

        if (on)
        {
            g.setColour (col::glow.withAlpha (0.30f));
            g.fillEllipse (ledR.expanded (5.f));
        }
        g.setGradientFill (ColourGradient (on ? col::glow.brighter (0.5f) : Colour (0xff2a1a1a), ledR.getCentreX(), ledR.getY(),
                                           on ? col::blood.darker (0.5f) : Colours::black, ledR.getCentreX(), ledR.getBottom(), false));
        g.fillEllipse (ledR);
        g.setColour (Colour (0xff0b0b0d));
        g.drawEllipse (ledR, 2.f);
        g.setColour (Colour (0x40ffffff));
        g.drawEllipse (ledR.expanded (1.f), 1.f);

        g.setFont (gfx::titleFont (17.f));
        g.setColour (on ? col::bone : (over ? col::text : col::dim));
        g.drawText (b.getButtonText(), r.withTrimmedLeft (ledR.getRight() + 8.f), Justification::centredLeft, false);
    }
};

//==============================================================================
//  Knob con nombre arriba y valor abajo (editable con doble clic en el número)
//==============================================================================
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& apvts, const char* paramId, const juce::String& name, bool bipolar = false, int textWidth = 64)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, textWidth, 16);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        slider.setMouseDragSensitivity (220);
        slider.getProperties().set ("bipolar", bipolar);
        addAndMakeVisible (slider);

        label.setText (name.toUpperCase(), juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (gfx::uiFont (11.f));
        label.setColour (juce::Label::textColourId, col::steelHi);
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        label.setBounds (r.removeFromTop (14));
        slider.setBounds (r);
    }

    juce::Slider slider;

private:
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

//==============================================================================
//  Medidor LED segmentado (nivel, o reducción de ganancia que cae desde arriba)
//==============================================================================
class MeterBar : public juce::Component
{
public:
    MeterBar (int numChannels, bool isGainReduction, float rangeDb)
        : channels (numChannels), gr (isGainReduction), range (rangeDb)
    {
        disp.fill (0.f);  peak.fill (0.f);  hold.fill (0.f);
        setInterceptsMouseClicks (false, false);
    }

    // Medidores de nivel: valores lineales. Para GR: dB positivos.
    void update (float v0, float v1, float dt)
    {
        const float in[2] = { v0, v1 };
        for (int c = 0; c < channels; ++c)
            step (c, gr ? in[c] : juce::Decibels::gainToDecibels (in[c], -range) + range, dt);
        repaint();
    }

    // Medidores directamente en dB (LUFS / RMS): -range .. 0
    void updateDb (float db0, float db1, float dt)
    {
        const float in[2] = { db0, db1 };
        for (int c = 0; c < channels; ++c)
            step (c, in[c] + range, dt);
        repaint();
    }

    void setFallRate (float dbPerSecond) { fall = dbPerSecond; }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat();
        gfx::drawScreen (g, r);
        r = r.reduced (3.f);

        const int nSeg = 30;
        const float gap = 1.5f;
        const float chW = (r.getWidth() - gap * (float) (channels - 1)) / (float) channels;
        const float segH = (r.getHeight() - gap * (float) (nSeg - 1)) / (float) nSeg;

        for (int c = 0; c < channels; ++c)
        {
            const float x = r.getX() + (float) c * (chW + gap);
            const int lit = (int) std::ceil (disp[(size_t) c] / range * (float) nSeg - 0.01f);
            const int pk  = (int) std::ceil (peak[(size_t) c] / range * (float) nSeg - 0.01f);

            for (int s = 0; s < nSeg; ++s)
            {
                const float frac = (float) s / (float) (nSeg - 1);              // 0 = abajo
                Colour on = gr ? col::ember : (frac < 0.62f ? col::blood : frac < 0.86f ? col::ember : Colour (0xffffd23f));
                const bool isLit = s < lit;      // nivel: sube desde abajo · GR: baja desde arriba (la posición y ya lo gestiona)
                const bool isPeak = ! gr && pk > 0 && s == pk - 1;
                const float y = gr ? r.getY() + (float) s * (segH + gap)
                                   : r.getBottom() - (float) (s + 1) * segH - (float) s * gap;
                g.setColour (isLit || isPeak ? on.brighter (isPeak ? 0.4f : 0.f) : Colour (0xff1c1214));
                g.fillRect (x, y, chW, segH);
            }
        }
    }

private:
    void step (int c, float v, float dt)
    {
        const auto i = (size_t) c;
        v = juce::jlimit (0.f, range, v);
        disp[i] = std::max (v, disp[i] - (fall > 0.f ? fall : (gr ? 70.f : 40.f)) * dt);
        if (v >= peak[i]) { peak[i] = v; hold[i] = 0.9f; }
        else if ((hold[i] -= dt) <= 0.f) peak[i] = std::max (0.f, peak[i] - 25.f * dt);
    }

    int channels;
    bool gr;
    float range;
    float fall = 0.f;
    std::array<float, 2> disp, peak, hold;
};

//==============================================================================
//  Vectorscopio / goniómetro: cada punto es una muestra (L,R) girada 45 grados.
//  Vertical = Mid (mono), horizontal = Side. Una línea vertical = mono; una nube ancha = estéreo amplio;
//  una línea horizontal = fases opuestas (peligro en mono). Se auto-escala para verse siempre.
//==============================================================================
class Goniometer : public juce::Component
{
public:
    explicit Goniometer (ExpansorProcessor& p) : proc (p) { setInterceptsMouseClicks (false, false); }

    void resized() override
    {
        scope = juce::Image (juce::Image::RGB, juce::jmax (1, getWidth()), juce::jmax (1, getHeight()), true);
        scope.clear (scope.getBounds(), col::screen);
    }

    void tick()
    {
        if (! scope.isValid()) return;

        pts.clear();
        const int ready = proc.gonioFifo.getNumReady();
        if (ready > 0)
        {
            int s1, z1, s2, z2;
            proc.gonioFifo.prepareToRead (ready, s1, z1, s2, z2);
            for (int i = 0; i < z1; ++i) pts.push_back ({ proc.gonioL[(size_t) (s1 + i)], proc.gonioR[(size_t) (s1 + i)] });
            for (int i = 0; i < z2; ++i) pts.push_back ({ proc.gonioL[(size_t) (s2 + i)], proc.gonioR[(size_t) (s2 + i)] });
            proc.gonioFifo.finishedRead (z1 + z2);
        }
        if (pts.size() > kMaxPts)
            pts.erase (pts.begin(), pts.begin() + (long) (pts.size() - kMaxPts));

        const float radius = (float) juce::jmin (getWidth(), getHeight()) * 0.5f - 6.f;
        const float cx = (float) getWidth() * 0.5f, cy = (float) getHeight() * 0.5f;
        constexpr float k45 = 0.70710678f;

        float pk = 0.05f;                                        // auto-ganancia: instantánea al subir, lenta al bajar
        for (auto& q : pts)
            pk = juce::jmax (pk, std::abs ((q.second - q.first) * k45), std::abs ((q.first + q.second) * k45));
        const float target = 0.9f * radius / pk;
        scale = target < scale ? target : scale + (target - scale) * 0.03f;
        if (scale <= 0.f) scale = target;

        juce::Graphics g (scope);
        g.setColour (col::screen.withAlpha (0.30f));            // persistencia (estela)
        g.fillRect (scope.getBounds());

        if (pts.size() > 1)
        {
            juce::Path path;
            bool first = true;
            for (auto& q : pts)
            {
                const float x = cx + (q.second - q.first) * k45 * scale;
                const float y = cy - (q.first + q.second) * k45 * scale;
                if (first) { path.startNewSubPath (x, y); first = false; } else path.lineTo (x, y);
            }
            g.setColour (col::glow.withAlpha (0.28f));
            g.strokePath (path, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour (juce::Colour (0xffff8a6a).withAlpha (0.85f));
            g.strokePath (path, juce::PathStrokeType (1.1f));
        }
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        const auto b = getLocalBounds().toFloat();
        g.drawImageAt (scope, 0, 0);

        const float radius = jmin (b.getWidth(), b.getHeight()) * 0.5f - 6.f;
        const auto c = b.getCentre();
        g.setColour (Colour (0x1cffffff));
        g.drawEllipse (c.x - radius, c.y - radius, radius * 2.f, radius * 2.f, 1.f);
        g.drawEllipse (c.x - radius * 0.5f, c.y - radius * 0.5f, radius, radius, 1.f);
        g.drawLine (c.x, c.y - radius, c.x, c.y + radius, 1.f);                   // eje Mid
        g.drawLine (c.x - radius, c.y, c.x + radius, c.y, 1.f);                   // eje Side
        g.setColour (Colour (0x30ffffff));
        const float d = radius * 0.7071f;
        g.drawLine (c.x - d, c.y - d, c.x + d, c.y + d, 1.f);                     // eje L
        g.drawLine (c.x + d, c.y - d, c.x - d, c.y + d, 1.f);                     // eje R

        g.setFont (gfx::uiFont (11.f));
        g.setColour (col::steelHi);
        g.drawText ("M",  Rectangle<float> (c.x - 20.f, b.getY() + 3.f, 40.f, 14.f), Justification::centred, false);
        g.drawText ("L",  Rectangle<float> (c.x - d - 22.f, c.y - d - 16.f, 20.f, 14.f), Justification::centred, false);
        g.drawText ("R",  Rectangle<float> (c.x + d + 2.f, c.y - d - 16.f, 20.f, 14.f), Justification::centred, false);
        g.drawText ("S",  Rectangle<float> (b.getRight() - 20.f, c.y - 7.f, 16.f, 14.f), Justification::centred, false);
        g.setColour (col::dim);
        g.drawText ("AUTO", Rectangle<float> (b.getX() + 6.f, b.getBottom() - 18.f, 50.f, 14.f), Justification::centredLeft, false);

        g.setColour (Colours::black);                                              // marco del cristal
        g.drawRoundedRectangle (b.reduced (0.5f), 4.f, 1.5f);
        g.setColour (Colour (0x22ffffff));
        g.drawRoundedRectangle (b.expanded (1.f), 5.f, 1.f);
    }

private:
    static constexpr size_t kMaxPts = 700;
    ExpansorProcessor& proc;
    juce::Image scope;
    std::vector<std::pair<float, float>> pts;
    float scale = 0.f;
};

//==============================================================================
//  Medidor de correlación de fase (-1 .. +1)
//==============================================================================
class CorrelationBar : public juce::Component
{
public:
    CorrelationBar() { setInterceptsMouseClicks (false, false); }

    void setValue (float v) { target = v; }
    void tick() { disp += (target - disp) * 0.22f; repaint(); }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        const auto full = getLocalBounds().toFloat();
        gfx::drawScreen (g, full);
        const auto r = Rectangle<float> (full.getX() + 8.f, full.getY() + 6.f, full.getWidth() - 16.f, full.getHeight() - 22.f);
        auto X = [&] (float v) { return jmap (v, -1.f, 1.f, r.getX(), r.getRight()); };

        g.setColour (Colour (0xff1c0e10));                                     // zona de fases opuestas
        g.fillRect (r.getX(), r.getY(), X (0.f) - r.getX(), r.getHeight());
        g.setColour (Colour (0xff121214));
        g.fillRect (X (0.f), r.getY(), r.getRight() - X (0.f), r.getHeight());

        const float v = jlimit (-1.f, 1.f, disp);
        const Colour c = v >= 0.f ? col::ember.interpolatedWith (col::bone, v * 0.5f) : col::glow;
        const auto bar = Rectangle<float> (jmin (X (0.f), X (v)), r.getY() + 2.f, std::abs (X (v) - X (0.f)), r.getHeight() - 4.f);
        g.setColour (c.withAlpha (0.35f));
        g.fillRect (bar.expanded (0.f, 2.f));
        g.setColour (c);
        g.fillRect (bar);
        g.setColour (col::bone);
        g.fillRect (X (v) - 1.5f, r.getY(), 3.f, r.getHeight());

        g.setColour (Colour (0x44ffffff));
        for (float t : { -1.f, -0.5f, 0.f, 0.5f, 1.f })
            g.drawVerticalLine ((int) X (t), r.getBottom() - 4.f, r.getBottom());
        g.setFont (gfx::uiFont (9.f, false));
        g.setColour (col::dim);
        g.drawText ("-1", Rectangle<float> (X (-1.f) - 4.f, r.getBottom() + 1.f, 24.f, 12.f), Justification::centredLeft, false);
        g.drawText ("0",  Rectangle<float> (X (0.f) - 12.f, r.getBottom() + 1.f, 24.f, 12.f), Justification::centred, false);
        g.drawText ("+1", Rectangle<float> (X (1.f) - 20.f, r.getBottom() + 1.f, 24.f, 12.f), Justification::centredRight, false);
    }

private:
    float target = 1.f, disp = 1.f;
};

//==============================================================================
//  Display de las 3 bandas de ancho: arrastra las líneas verticales (crossovers) y la altura de cada banda
//==============================================================================
class BandDisplay : public juce::Component
{
public:
    explicit BandDisplay (juce::AudioProcessorValueTreeState& a) : apvts (a) {}

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        const auto full = getLocalBounds().toFloat();
        gfx::drawScreen (g, full);
        const auto r = plot();

        g.saveState();
        g.reduceClipRegion (r.getSmallestIntegerContainer());
        for (float f : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f })
        {
            g.setColour (Colour ((int) f == 100 || (int) f == 1000 || (int) f == 10000 ? 0x22ffffff : 0x0fffffff));
            g.drawVerticalLine ((int) fx (f, r), r.getY(), r.getBottom());
        }
        for (float w : { 0.f, 50.f, 100.f, 150.f, 200.f })
        {
            g.setColour (Colour (w == 100.f ? 0x38ffffff : 0x0fffffff));
            g.drawHorizontalLine ((int) wy (w, r), r.getX(), r.getRight());
        }

        const float xs[4] = { r.getX(), fx (val (ids::xoverLo), r), fx (val (ids::xoverHi), r), r.getRight() };
        const char* ids3[3] = { ids::wLo, ids::wMid, ids::wHi };
        const char* names[3] = { "LOW", "MID", "HIGH" };
        const Colour cols[3] = { Colour (0xffb01820), col::glow, col::ember };
        for (int i = 0; i < 3; ++i)
        {
            const float w = val (ids3[i]);
            const auto rect = Rectangle<float> (xs[i], wy (w, r), xs[i + 1] - xs[i], r.getBottom() - wy (w, r));
            const bool active = (i == dragBand) || (dragging < 0 && i == hoverBand);
            g.setGradientFill (ColourGradient (cols[i].withAlpha (active ? 0.62f : 0.46f), 0.f, rect.getY(),
                                               cols[i].withAlpha (0.10f), 0.f, rect.getBottom(), false));
            g.fillRect (rect);
            g.setColour (cols[i].withAlpha (0.25f));
            g.fillRect (rect.getX(), rect.getY() - 2.f, rect.getWidth(), 5.f);
            g.setColour (cols[i].brighter (0.3f));
            g.fillRect (rect.getX() + 1.f, rect.getY() - 1.f, rect.getWidth() - 2.f, 2.f);

            g.setColour (col::bone.withAlpha (0.9f));
            g.setFont (gfx::uiFont (11.f));
            g.drawText (String (names[i]) + "  " + String ((int) w) + "%",
                        Rectangle<float> (rect.getX(), r.getBottom() - 18.f, rect.getWidth(), 14.f), Justification::centred, false);
        }
        for (int i = 1; i <= 2; ++i)                                         // crossovers
        {
            const bool active = (dragging == 10 + i) || (dragging < 0 && hoverX == i);
            g.setColour (active ? col::bone : col::steelHi.withAlpha (0.8f));
            g.fillRect (xs[i] - 1.f, r.getY(), 2.f, r.getHeight());
            g.fillRoundedRectangle (xs[i] - 5.f, r.getY(), 10.f, 12.f, 2.f);
        }
        g.restoreState();

        g.setFont (gfx::uiFont (10.f, false));
        g.setColour (col::dim);
        for (float f : { 100.f, 1000.f, 10000.f })
            g.drawText (f >= 1000.f ? String ((int) (f / 1000.f)) + "k" : String ((int) f),
                        Rectangle<float> (fx (f, r) - 20.f, r.getBottom() + 1.f, 40.f, 12.f), Justification::centred, false);
        for (float w : { 0.f, 100.f, 200.f })
            g.drawText (String ((int) w) + "%", Rectangle<float> (full.getX() + 2.f, wy (w, r) - 7.f, 30.f, 14.f), Justification::centredRight, false);
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int hx = crossoverAt (e.position.x), hb = bandAt (e.position.x);
        if (hx != hoverX || hb != hoverBand) { hoverX = hx; hoverBand = hb; repaint(); }
        setMouseCursor (hx > 0 ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::UpDownResizeCursor);
    }
    void mouseExit (const juce::MouseEvent&) override { hoverX = -1; hoverBand = -1; repaint(); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const int hx = crossoverAt (e.position.x);
        if (hx > 0) { dragging = 10 + hx; gesture (hx == 1 ? ids::xoverLo : ids::xoverHi, true); }
        else        { dragBand = bandAt (e.position.x); dragging = dragBand; gesture (bandId (dragBand), true); }
        mouseDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const auto r = plot();
        if (dragging > 10)
        {
            const char* id = dragging == 11 ? ids::xoverLo : ids::xoverHi;
            setParam (id, xf (juce::jlimit (r.getX(), r.getRight(), e.position.x), r));
        }
        else if (dragging >= 0)
        {
            const float w = juce::jlimit (0.f, 200.f, (r.getBottom() - e.position.y) / r.getHeight() * 200.f);
            setParam (bandId (dragBand), std::round (w));
        }
        repaint();
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (dragging > 10) gesture (dragging == 11 ? ids::xoverLo : ids::xoverHi, false);
        else if (dragging >= 0) gesture (bandId (dragBand), false);
        dragging = dragBand = -1;
        repaint();
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        if (crossoverAt (e.position.x) > 0) return;
        const int b = bandAt (e.position.x);
        gesture (bandId (b), true);  setParam (bandId (b), 100.f);  gesture (bandId (b), false);
    }

private:
    juce::Rectangle<float> plot() const { return getLocalBounds().toFloat().withTrimmedLeft (34.f).withTrimmedBottom (14.f).reduced (0.f, 4.f).withTrimmedRight (6.f); }
    static float fx (float f, juce::Rectangle<float> r) { return r.getX() + r.getWidth() * std::log (f / 20.f) / std::log (1000.f); }
    static float xf (float x, juce::Rectangle<float> r) { return 20.f * std::exp (std::log (1000.f) * (x - r.getX()) / r.getWidth()); }
    static float wy (float w, juce::Rectangle<float> r) { return r.getBottom() - w / 200.f * r.getHeight(); }

    float val (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    static const char* bandId (int b) { return b == 0 ? ids::wLo : b == 1 ? ids::wMid : ids::wHi; }

    int crossoverAt (float x) const
    {
        const auto r = plot();
        if (std::abs (x - fx (val (ids::xoverLo), r)) < 8.f) return 1;
        if (std::abs (x - fx (val (ids::xoverHi), r)) < 8.f) return 2;
        return -1;
    }
    int bandAt (float x) const
    {
        const auto r = plot();
        return x < fx (val (ids::xoverLo), r) ? 0 : x < fx (val (ids::xoverHi), r) ? 1 : 2;
    }

    void setParam (const char* id, float v)
    {
        if (auto* p = apvts.getParameter (id)) p->setValueNotifyingHost (p->convertTo0to1 (v));
    }
    void gesture (const char* id, bool begin)
    {
        if (auto* p = apvts.getParameter (id)) { if (begin) p->beginChangeGesture(); else p->endChangeGesture(); }
    }

    juce::AudioProcessorValueTreeState& apvts;
    int hoverX = -1, hoverBand = -1, dragging = -1, dragBand = -1;
};
