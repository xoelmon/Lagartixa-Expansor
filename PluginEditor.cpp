#include "PluginEditor.h"
#include "UI.h"

using namespace juce;

//==============================================================================
//  Panel principal: tamaño lógico fijo (1100 x 670) que se escala al redimensionar la ventana
//
//    [ STEREO FIELD            ]  [ WIDTH BANDS            ]
//    [ vectorscopio + medidores]  [ SPACE ]  [ HAAS ]
//    [ correlación             ]  [ PHASE        ] [ OUTPUT ]
//==============================================================================
class MainPanel : public Component, private Timer
{
public:
    static constexpr int W = 1100, H = 670;

    explicit MainPanel (ExpansorProcessor& p)
        : proc (p), gonio (p), bandDisplay (p.apvts),
          meterL (1, false, 48.f), meterR (1, false, 48.f), meterM (1, false, 48.f), meterS (1, false, 48.f)
    {
        setLookAndFeel (&laf);
        texture = gfx::makeBrushedTexture();

        for (auto* c : std::initializer_list<Component*> { &gonio, &corrBar, &bandDisplay, &meterL, &meterR, &meterM, &meterS })
            addAndMakeVisible (*c);

        // --- bandas ---
        wLoK  = addKnob (ids::wLo,  "Low");
        wMidK = addKnob (ids::wMid, "Mid");
        wHiK  = addKnob (ids::wHi,  "High");
        xLoK  = addKnob (ids::xoverLo, "X-Over 1");
        xHiK  = addKnob (ids::xoverHi, "X-Over 2");
        // --- espacio / haas ---
        spaceAmtK = addKnob (ids::spaceAmt, "Amount");
        spaceHpK  = addKnob (ids::spaceHp,  "Focus");
        haasMsK   = addKnob (ids::haasMs,   "Time");
        haasAmtK  = addKnob (ids::haasAmt,  "Amount");
        // --- fase / salida ---
        phaseK = addKnob (ids::phaseDeg, "Rotation", true);
        mixK   = addKnob (ids::mix,      "Mix");
        gainK  = addKnob (ids::outGain,  "Gain", true);

        using BA = AudioProcessorValueTreeState::ButtonAttachment;
        auto addToggle = [&] (ToggleButton& b, const char* id, std::unique_ptr<BA>& att)
        {
            addAndMakeVisible (b);
            att = std::make_unique<BA> (p.apvts, id, b);
        };
        addToggle (invLBtn,  ids::invL,      invLAtt);
        addToggle (invRBtn,  ids::invR,      invRAtt);
        addToggle (swapBtn,  ids::swapLR,    swapAtt);
        addToggle (monoBtn,  ids::monoCheck, monoAtt);

        setSize (W, H);
        startTimerHz (30);
    }

    ~MainPanel() override { stopTimer(); setLookAndFeel (nullptr); }

    //--------------------------------------------------------------------------
    void resized() override
    {
        stereoPanel = { 14.f,  70.f,  552.f, 586.f };
        bandsPanel  = { 580.f, 70.f,  506.f, 262.f };
        spacePanel  = { 580.f, 342.f, 248.f, 150.f };
        haasPanel   = { 838.f, 342.f, 248.f, 150.f };
        phasePanel  = { 580.f, 502.f, 302.f, 154.f };
        outPanel    = { 892.f, 502.f, 194.f, 154.f };

        gonio.setBounds (34, 106, 384, 384);
        meterL.setBounds (430, 120, 28, 370);
        meterR.setBounds (464, 120, 28, 370);
        meterM.setBounds (498, 120, 28, 370);
        meterS.setBounds (532, 120, 28, 370);
        corrBar.setBounds (34, 502, 526, 40);

        bandDisplay.setBounds (592, 104, 482, 124);
        const float kw = 482.f / 5.f;
        Knob* bk[5] = { wLoK, wMidK, wHiK, xLoK, xHiK };
        for (int i = 0; i < 5; ++i)
            bk[i]->setBounds ((int) (592.f + (float) i * kw), 234, (int) kw, 92);

        spaceAmtK->setBounds (594, 378, 110, 104);
        spaceHpK->setBounds (710, 378, 110, 104);
        haasMsK->setBounds (852, 378, 110, 104);
        haasAmtK->setBounds (968, 378, 110, 104);

        phaseK->setBounds (592, 534, 120, 112);
        invLBtn.setBounds (726, 540, 150, 26);
        invRBtn.setBounds (726, 572, 150, 26);
        swapBtn.setBounds (726, 604, 150, 26);

        mixK->setBounds (898, 532, 92, 86);
        gainK->setBounds (990, 532, 92, 86);
        monoBtn.setBounds (904, 622, 176, 26);
    }

    //--------------------------------------------------------------------------
    void paint (Graphics& g) override
    {
        g.setGradientFill (ColourGradient (Colour (0xff131316), 0, 0, col::bg, 0, (float) H, false));
        g.fillAll();
        g.setTiledImageFill (texture, 0, 0, 0.05f);
        g.fillAll();

        paintHeader (g);
        for (auto* r : { &stereoPanel, &bandsPanel, &spacePanel, &haasPanel, &phasePanel, &outPanel })
            gfx::drawMetalPanel (g, *r, texture);

        // títulos de los paneles
        g.setFont (gfx::titleFont (17.f));
        g.setColour (col::bone);
        const std::pair<const char*, Rectangle<float>*> titles[6] = {
            { "STEREO FIELD", &stereoPanel }, { "WIDTH BANDS", &bandsPanel }, { "SPACE", &spacePanel },
            { "HAAS", &haasPanel }, { "PHASE", &phasePanel }, { "OUTPUT", &outPanel } };
        for (auto& t : titles)
            g.drawText (t.first, Rectangle<float> (t.second->getX() + 18.f, t.second->getY() + 6.f, 220.f, 24.f), Justification::centredLeft, false);

        // etiquetas de los medidores y notas
        g.setFont (gfx::uiFont (10.f));
        g.setColour (col::steelHi);
        const char* lbl[4] = { "L", "R", "M", "S" };
        for (int i = 0; i < 4; ++i)
            g.drawText (lbl[i], 430 + i * 34, 106, 28, 12, Justification::centred, false);
        g.setColour (col::dim);
        g.drawText ("MONO-SAFE", Rectangle<int> (730, 350, 90, 14), Justification::centredRight, false);
        g.drawText ("MONO-SAFE", Rectangle<int> (988, 350, 90, 14), Justification::centredRight, false);
        g.drawText ("rotates R vs L, all frequencies", Rectangle<int> (726, 632, 152, 12), Justification::centredLeft, false);

        paintReadouts (g);
    }

private:
    void paintReadouts (Graphics& g)
    {
        const float c = corr;
        String status;
        Colour sc = col::bone;
        if (c < -0.2f)      { status = "PHASE PROBLEM";    sc = col::glow; }
        else if (c < 0.2f)  { status = "VERY WIDE";        sc = col::ember; }
        else if (c < 0.85f) { status = "GOOD STEREO";      sc = col::bone; }
        else                { status = "NARROW / MONO";    sc = col::steelHi; }

        const int y = 556;
        g.setFont (gfx::uiFont (10.f));
        g.setColour (col::dim);
        g.drawText ("CORRELATION", 40, y, 160, 12, Justification::centredLeft, false);
        g.drawText ("STEREO WIDTH", 220, y, 160, 12, Justification::centredLeft, false);
        g.drawText ("STATUS", 400, y, 150, 12, Justification::centredLeft, false);

        g.setFont (gfx::titleFont (30.f));
        g.setColour (c < 0.f ? col::glow : col::bone);
        g.drawText ((c >= 0.f ? "+" : "") + String (c, 2), 40, y + 14, 170, 36, Justification::centredLeft, false);
        g.setColour (col::bone);
        g.drawText (String ((int) std::round (widthIdx * 100.f)) + " %", 220, y + 14, 170, 36, Justification::centredLeft, false);
        g.setFont (gfx::titleFont (20.f));
        g.setColour (sc);
        g.drawText (status, 400, y + 20, 160, 28, Justification::centredLeft, false);

        g.setFont (gfx::uiFont (10.f));
        g.setColour (col::dim);
        g.drawFittedText ("+1 = identical L/R (mono)   0 = uncorrelated   -1 = opposite phase (cancels in mono)",
                          Rectangle<int> (40, y + 56, 510, 26), Justification::topLeft, 2);
    }

    void paintHeader (Graphics& g)
    {
        const Rectangle<float> r (0.f, 0.f, (float) W, 62.f);
        g.setGradientFill (ColourGradient (Colour (0xff222327), 0, 0, Colour (0xff0b0b0d), 0, r.getBottom(), false));
        g.fillRect (r);
        g.setTiledImageFill (texture, 0, 0, 0.10f);
        g.fillRect (r);

        // dientes de sierra / cuchilla en el borde inferior
        const float y0 = 52.f, step = 18.f;
        Path teeth;
        teeth.startNewSubPath (0.f, y0 - 3.f);
        for (float x = 0.f; x < (float) W; x += step)
        {
            teeth.lineTo (x + step * 0.5f, y0 + 10.f);
            teeth.lineTo (x + step, y0 - 3.f);
        }
        teeth.lineTo ((float) W, 0.f);
        teeth.lineTo (0.f, 0.f);
        teeth.closeSubPath();
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillPath (teeth, AffineTransform::translation (0.f, 2.f));
        g.setGradientFill (ColourGradient (Colour (0xff34353a), 0, 0, Colour (0xff0f0f11), 0, y0 + 10.f, false));
        g.fillPath (teeth);
        g.setColour (col::blood);
        g.fillRect (0.f, y0 - 4.f, (float) W, 1.5f);

        // logotipo
        const String name ("LARGARTIXA SOUNDLAB EXPANSOR");
        const Rectangle<int> tb (28, 4, 840, 50);
        g.setFont (gfx::titleFont (36.f));
        g.setColour (col::blood.withAlpha (0.28f));
        for (auto o : { Point<int> (-2, 0), Point<int> (2, 0), Point<int> (0, -2), Point<int> (0, 2) })
            g.drawText (name, tb.translated (o.x, o.y), Justification::centredLeft, false);
        g.setColour (Colours::black);
        g.drawText (name, tb.translated (2, 3), Justification::centredLeft, false);
        g.setGradientFill (ColourGradient (Colour (0xfff4f5f7), 0, 8, Colour (0xff6e717a), 0, 52, false));
        g.drawText (name, tb, Justification::centredLeft, false);

        g.setFont (gfx::uiFont (12.f));
        g.setColour (col::dim);
        g.drawText ("WIDTH  //  SPACE  //  PHASE", Rectangle<int> (800, 14, 276, 30), Justification::centredRight, false);
    }

    Knob* addKnob (const char* id, const String& name, bool bipolar = false)
    {
        knobs.push_back (std::make_unique<Knob> (proc.apvts, id, name, bipolar));
        addAndMakeVisible (*knobs.back());
        return knobs.back().get();
    }

    void timerCallback() override
    {
        constexpr float dt = 1.f / 30.f;
        meterL.update (proc.peakL.exchange (0.f), 0.f, dt);
        meterR.update (proc.peakR.exchange (0.f), 0.f, dt);
        meterM.update (proc.peakM.exchange (0.f), 0.f, dt);
        meterS.update (proc.peakS.exchange (0.f), 0.f, dt);

        gonio.tick();
        corr = proc.correlation.load();
        widthIdx = proc.widthIndex.load();
        corrBar.setValue (corr);
        corrBar.tick();
        bandDisplay.repaint();
        repaint (34, 548, 530, 100);
    }

    ExpansorProcessor& proc;
    MetalLookAndFeel laf;
    Image texture;

    Goniometer gonio;
    CorrelationBar corrBar;
    BandDisplay bandDisplay;
    MeterBar meterL, meterR, meterM, meterS;

    std::vector<std::unique_ptr<Knob>> knobs;
    Knob *wLoK = nullptr, *wMidK = nullptr, *wHiK = nullptr, *xLoK = nullptr, *xHiK = nullptr;
    Knob *spaceAmtK = nullptr, *spaceHpK = nullptr, *haasMsK = nullptr, *haasAmtK = nullptr;
    Knob *phaseK = nullptr, *mixK = nullptr, *gainK = nullptr;

    ToggleButton invLBtn { "INVERT L" }, invRBtn { "INVERT R" }, swapBtn { "SWAP L/R" }, monoBtn { "MONO CHECK" };
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> invLAtt, invRAtt, swapAtt, monoAtt;

    float corr = 1.f, widthIdx = 0.f;
    Rectangle<float> stereoPanel, bandsPanel, spacePanel, haasPanel, phasePanel, outPanel;
};

//==============================================================================
ExpansorEditor::ExpansorEditor (ExpansorProcessor& p)
    : AudioProcessorEditor (&p), panel (std::make_unique<MainPanel> (p))
{
    addAndMakeVisible (*panel);
    const double aspect = (double) MainPanel::H / (double) MainPanel::W;
    setResizable (true, true);
    setResizeLimits (770, (int) (770.0 * aspect), 1650, (int) (1650.0 * aspect));
    getConstrainer()->setFixedAspectRatio ((double) MainPanel::W / (double) MainPanel::H);
    setSize (MainPanel::W, MainPanel::H);
}

ExpansorEditor::~ExpansorEditor() = default;

void ExpansorEditor::resized()
{
    panel->setTransform (AffineTransform::scale ((float) getWidth() / (float) MainPanel::W));
    panel->setBounds (0, 0, MainPanel::W, MainPanel::H);
}
