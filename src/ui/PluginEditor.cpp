#include "PluginEditor.h"
#include "../dsp/WavetableImport.h"

#include <cmath>

namespace wave
{
namespace
{
const auto canvas = juce::Colour(0xff111418);
const auto panel = juce::Colour(0xff1b2027);
const auto surface = juce::Colour(0xff272f39);
const auto hover = juce::Colour(0xff303a46);
const auto border = juce::Colour(0xff5e6d80);
const auto text = juce::Colour(0xfff1f4f8);
const auto muted = juce::Colour(0xffaab6c5);
const auto accent = juce::Colour(0xff62cbe3);
const auto focus = juce::Colour(0xffd3f6ff);

juce::Font font(float height, bool semibold = false)
{
    return juce::Font(juce::FontOptions("Segoe UI", semibold ? "Semibold" : "Regular", height));
}

class EditorLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    EditorLookAndFeel()
    {
        setColour(juce::Label::textColourId, text);
        setColour(juce::Slider::textBoxTextColourId, text);
        setColour(juce::Slider::textBoxBackgroundColourId, surface);
        setColour(juce::Slider::textBoxOutlineColourId, border);
        setColour(juce::Slider::textBoxHighlightColourId, hover);
        setColour(juce::ComboBox::textColourId, text);
        setColour(juce::ComboBox::backgroundColourId, surface);
        setColour(juce::ComboBox::outlineColourId, border);
        setColour(juce::ComboBox::arrowColourId, accent);
        setColour(juce::PopupMenu::backgroundColourId, panel);
        setColour(juce::PopupMenu::textColourId, text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
        setColour(juce::PopupMenu::highlightedTextColourId, canvas);
        setColour(juce::TextEditor::focusedOutlineColourId, focus);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override { return font(13); }
    juce::Font getPopupMenuFont() override { return font(13); }

    juce::Label* createSliderTextBox(juce::Slider& slider) override
    {
        auto* label = LookAndFeel_V4::createSliderTextBox(slider);
        label->setFont(font(12));
        return label;
    }

    void drawComboBox(juce::Graphics& graphics, int width, int height, bool pressed,
                      int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width),
                                             static_cast<float>(height)).reduced(1.0f);
        graphics.setColour(pressed || box.isMouseOver(true) ? hover : surface);
        graphics.fillRoundedRectangle(bounds, 4.0f);
        const auto focused = box.hasKeyboardFocus(true);
        graphics.setColour(focused ? focus : border);
        graphics.drawRoundedRectangle(bounds, 4.0f, focused ? 2.0f : 1.0f);

        const auto x = bounds.getRight() - 13.0f;
        const auto y = bounds.getCentreY();
        juce::Path arrow;
        arrow.startNewSubPath(x - 4.0f, y - 2.0f);
        arrow.lineTo(x, y + 2.0f);
        arrow.lineTo(x + 4.0f, y - 2.0f);
        graphics.setColour(accent);
        graphics.strokePath(arrow, juce::PathStrokeType(1.5f));
    }

    void drawRotarySlider(juce::Graphics& graphics, int x, int y, int width, int height,
                          float value, float start, float end, juce::Slider& slider) override
    {
        const auto centre = juce::Point<float>(static_cast<float>(x) + static_cast<float>(width) * 0.5f,
                                               static_cast<float>(y) + static_cast<float>(height) * 0.5f);
        const auto radius = std::min(18.0f, static_cast<float>(std::min(width, height)) * 0.5f - 2.0f);
        const auto angle = juce::jmap(value, start, end);
        const auto dial = juce::Rectangle<float>(radius * 1.45f, radius * 1.45f).withCentre(centre);

        graphics.setColour(slider.isMouseOverOrDragging() ? hover : surface);
        graphics.fillEllipse(dial);
        graphics.setColour(border);
        graphics.drawEllipse(dial, 1.0f);

        const auto drawArc = [&](float arcStart, float arcEnd, juce::Colour colour, float thickness)
        {
            juce::Path arc;
            arc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, arcStart, arcEnd, true);
            graphics.setColour(colour);
            graphics.strokePath(arc, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
        };
        drawArc(start, end, border, 2.0f);
        drawArc(start, angle, accent, 2.0f);

        const auto direction = juce::Point<float>(std::sin(angle), -std::cos(angle));
        graphics.setColour(accent);
        graphics.drawLine({ centre + direction * 5.0f, centre + direction * 10.0f }, 2.0f);

        if (slider.hasKeyboardFocus(true))
        {
            graphics.setColour(focus);
            graphics.drawEllipse(dial.expanded(2.0f), 2.0f);
        }
    }
};

template <typename Base>
class ParameterControl : public Base
{
public:
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (!event.mods.isPopupMenu())
            Base::mouseDown(event);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (!event.mods.isPopupMenu())
            Base::mouseDrag(event);
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        if (!event.mods.isPopupMenu())
            Base::mouseDoubleClick(event);
    }
};

class Knob final : public ParameterControl<juce::Slider>
{
public:
    bool keyPressed(const juce::KeyPress& key) override
    {
        const auto code = key.getKeyCode();
        if (code == juce::KeyPress::leftKey || code == juce::KeyPress::downKey
            || code == juce::KeyPress::rightKey || code == juce::KeyPress::upKey)
        {
            const auto direction = code == juce::KeyPress::leftKey || code == juce::KeyPress::downKey
                                       ? -1.0 : 1.0;
            const auto value = getValue();
            const auto proportion = valueToProportionOfLength(value);
            const auto next = proportionOfLengthToValue(juce::jlimit(0.0, 1.0, proportion + 0.02 * direction));
            setValue(value + direction * std::max(getInterval(), std::abs(next - value)),
                     juce::sendNotificationSync);
            return true;
        }
        return Slider::keyPressed(key);
    }
};

struct KnobControl
{
    juce::Label label;
    Knob knob;
    std::unique_ptr<ParameterState::SliderAttachment> attachment;
};

struct Selector
{
    ParameterControl<juce::ComboBox> box;
    std::unique_ptr<ParameterState::ComboBoxAttachment> attachment;
};

const juce::Rectangle<int> oscillatorPanel { 16, 68, 728, 164 };
const juce::Rectangle<int> layerPanel { 16, 244, 728, 164 };
const juce::Rectangle<int> envelopePanel { 16, 420, 242, 220 };
const juce::Rectangle<int> filterPanel { 270, 420, 136, 220 };
const juce::Rectangle<int> lfoPanel { 418, 420, 224, 220 };
const juce::Rectangle<int> outputPanel { 654, 420, 90, 220 };
const juce::Rectangle<int> wavePlot { 28, 112, 224, 104 };
const juce::Rectangle<int> layerPlot { 28, 288, 224, 104 };
const juce::Rectangle<int> importLabel { 554, 288, 178, 32 };
const juce::Rectangle<int> envelopePlot { 28, 458, 218, 45 };
const juce::Rectangle<int> filterPlot { 282, 458, 112, 45 };
const juce::Rectangle<int> lfoPlot { 430, 501, 200, 38 };
const juce::Rectangle<int> modulationReadout { 430, 615, 200, 19 };

class Editor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    Editor(juce::AudioProcessor& processor, ParameterState& parameterState, const SynthEngine& synth,
           WavetableImports& wavetableImports)
        : AudioProcessorEditor(processor), state(parameterState), engine(synth), imports(wavetableImports)
    {
        setLookAndFeel(&theme);
        setOpaque(true);
        setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);

        patch.addItemList(patchNames(), 1);
        patch.setTextWhenNothingSelected("Load patch");
        patch.setTitle("Load factory patch");
        patch.setTooltip("Replace this instance's settings with a factory patch");
        patch.onChange = [this]
        {
            if (const auto id = patch.getSelectedId(); id > 0)
                loadPatch(state, id - 1);
            patch.setSelectedId(0, juce::dontSendNotification);
        };
        addAndMakeVisible(patch);

        addSelector(tableSelector, table, "Select the built-in wavetable");
        addKnob(position, "Position", "Blend between the frames in the selected table");
        addKnob(tune, "Transpose", "Transpose all notes by semitones");
        addKnob(fine, "Fine tune", "Adjust pitch in cents");
        addKnob(unison, "Voices", "Set the number of oscillator voices per note");
        addKnob(detune, "Detune", "Spread the pitch of the unison voices");
        addKnob(spread, "Spread", "Spread the unison voices across the stereo field");
        addKnob(level, "Level", "Set the level of oscillator 1");
        addSelector(layerSelector, table2, "Select a built-in or imported wavetable for oscillator 2");
        addKnob(position2, "Position", "Blend between frames in oscillator 2's table");
        addKnob(tune2, "Transpose", "Transpose oscillator 2 by semitones");
        addKnob(fine2, "Fine tune", "Adjust oscillator 2's pitch in cents");
        addKnob(level2, "Level", "Raise this level to add oscillator 2 to the mix");
        importButton.setTooltip("Import a mono WAV with 2,048 samples per frame");
        importButton.setColour(juce::TextButton::buttonColourId, surface);
        importButton.setColour(juce::TextButton::textColourOffId, text);
        importButton.onClick = [this] { chooseWavetable(); };
        addAndMakeVisible(importButton);
        addKnob(attack, "Attack", "Set how quickly a new note reaches its peak");
        addKnob(decay, "Decay", "Set how quickly a new note falls to the sustain level");
        addKnob(sustain, "Sustain", "Set the level a new note holds");
        addKnob(release, "Release", "Set the release time for new notes");
        addKnob(cutoff, "Cutoff", "Set the low-pass filter frequency");
        addKnob(resonance, "Resonance", "Emphasise frequencies near cutoff");
        addSelector(modeSelector, lfoMode, "Run continuously or restart the LFO for each note");
        addKnob(lfoRate, "Rate", "Set the LFO speed in cycles per second");
        addKnob(positionDepth, "Position", "Set the LFO depth for wavetable position");
        addKnob(cutoffDepth, "Cutoff", "Set the LFO depth for filter cutoff");
        addKnob(output, "Level", "Set the final stereo output level");

        setSize(760, 656);
        startTimerHz(30);
    }

    ~Editor() override
    {
        stopTimer();
        setLookAndFeel(nullptr);
    }

    void resized() override
    {
        patch.setBounds(540, 18, 204, 32);
        tableSelector.box.setBounds(264, 112, 164, 32);
        layerSelector.box.setBounds(264, 288, 164, 32);
        importButton.setBounds(440, 288, 104, 32);
        modeSelector.box.setBounds(430, 458, 200, 32);
        place({ position, tune, fine, unison, detune, spread, level }, { 264, 151, 468, 68 });
        place({ position2, tune2, fine2, level2 }, { 264, 327, 468, 68 });
        place({ attack, decay, sustain, release }, { 28, 509, 218, 76 });
        place({ cutoff, resonance }, { 280, 509, 116, 76 });
        place({ lfoRate, positionDepth, cutoffDepth }, { 430, 544, 200, 70 });
        place({ output }, { 666, 508, 66, 76 });
    }

    void paint(juce::Graphics& graphics) override
    {
        graphics.fillAll(canvas);
        graphics.setColour(accent);
        graphics.fillRoundedRectangle(16.0f, 20.0f, 4.0f, 29.0f, 2.0f);
        graphics.setColour(text);
        graphics.setFont(font(20, true));
        graphics.drawText("Wavetable Synth", 29, 14, 310, 24, juce::Justification::centredLeft);
        graphics.setColour(muted);
        graphics.setFont(font(12));
        graphics.drawText("Stereo wavetable instrument", 30, 38, 310, 15, juce::Justification::centredLeft);

        drawPanel(graphics, oscillatorPanel, "Oscillator 1");
        drawPanel(graphics, layerPanel, "Oscillator 2");
        drawPanel(graphics, envelopePanel, "Amp envelope");
        drawPanel(graphics, filterPanel, "Low-pass filter");
        drawPanel(graphics, lfoPanel, "LFO");
        drawPanel(graphics, outputPanel, "Output");

        graphics.setFont(font(12));
        graphics.setColour(muted);
        graphics.drawText("Applies to new notes", 28, 602, 218, 20, juce::Justification::centred);
        graphics.drawText("Stereo", 666, 460, 66, 24, juce::Justification::centred);
        paintDisplays(graphics);
    }

private:
    void chooseWavetable()
    {
        chooser = std::make_unique<juce::FileChooser>("Import a wavetable", juce::File(), "*.wav");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                             [safe = juce::Component::SafePointer<Editor>(this)](const juce::FileChooser& selected)
        {
            if (safe == nullptr || selected.getResults().isEmpty())
                return;

            juce::String error;
            auto imported = ImportedWavetable::load(selected.getResult(), error);
            if (imported == nullptr)
            {
                juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                       "Wavetable import", error, {}, safe.getComponent());
                return;
            }

            safe->imports.replace(std::move(imported));
            auto& parameter = *safe->state.getParameter(parameterId(table2));
            parameter.beginChangeGesture();
            parameter.setValueNotifyingHost(parameter.convertTo0to1(importedTableIndex));
            parameter.endChangeGesture();
        });
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (!event.mods.isPopupMenu())
            return;

        // A value field can be a child of its knob. Route both surfaces to the same parameter.
        const auto* source = event.eventComponent;
        for (std::size_t i = 0; i < knobs.size(); ++i)
        {
            auto& knob = knobs[i].knob;
            if (&knob == source || knob.isParentOf(source))
            {
                showContextMenu(static_cast<Parameter>(i), knob);
                return;
            }
        }

        if (&tableSelector.box == source || tableSelector.box.isParentOf(source))
            showContextMenu(table, tableSelector.box);
        else if (&layerSelector.box == source || layerSelector.box.isParentOf(source))
            showContextMenu(table2, layerSelector.box);
        else if (&modeSelector.box == source || modeSelector.box.isParentOf(source))
            showContextMenu(lfoMode, modeSelector.box);
    }

    void showContextMenu(Parameter parameter, juce::Component& control)
    {
        auto& value = *state.getParameter(parameterId(parameter));
        juce::PopupMenu menu;
        if (const auto* context = getHostContext())
            if (const auto hostMenu = context->getContextMenuForParameter(&value))
                menu = hostMenu->getEquivalentPopupMenu();

        const auto defaults = value.getDefaultValue();
        if (menu.getNumItems() != 0)
            menu.addSeparator();
        menu.addItem("Reset", value.getValue() != defaults, false,
                     [safe = juce::Component::SafePointer<Editor>(this), parameter, defaults]
        {
            if (safe != nullptr)
            {
                auto& target = *safe->state.getParameter(parameterId(parameter));
                target.beginChangeGesture();
                target.setValueNotifyingHost(defaults);
                target.endChangeGesture();
            }
        });
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(control).withMousePosition());
    }

    static void drawPanel(juce::Graphics& graphics, juce::Rectangle<int> bounds, const char* heading)
    {
        graphics.setColour(panel);
        graphics.fillRoundedRectangle(bounds.toFloat(), 8.0f);
        graphics.setColour(border.withAlpha(0.6f));
        graphics.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 8.0f, 1.0f);
        graphics.setColour(text);
        graphics.setFont(font(14, true));
        graphics.drawText(heading, bounds.withTrimmedLeft(12).removeFromTop(35),
                          juce::Justification::centredLeft);
    }

    void addSelector(Selector& selector, Parameter parameter, const char* tooltip)
    {
        const auto* choice = static_cast<juce::AudioParameterChoice*>(state.getParameter(parameterId(parameter)));
        selector.box.addItemList(choice->choices, 1);
        selector.box.setTitle(parameterName(parameter));
        selector.box.setTooltip(tooltip);
        selector.box.addMouseListener(this, true);
        addAndMakeVisible(selector.box);
        selector.attachment = std::make_unique<ParameterState::ComboBoxAttachment>(
            state, parameterId(parameter), selector.box);
    }

    void addKnob(Parameter parameter, const char* label, const char* tooltip)
    {
        auto& control = knobs[parameter];
        control.label.setText(label, juce::dontSendNotification);
        control.label.setJustificationType(juce::Justification::centred);
        control.label.setMinimumHorizontalScale(0.84f);
        control.label.setFont(font(13));
        addAndMakeVisible(control.label);

        auto* value = state.getParameter(parameterId(parameter));
        control.knob.setName(parameterName(parameter));
        control.knob.setTitle(parameterName(parameter));
        control.knob.setTooltip(tooltip);
        control.knob.setWantsKeyboardFocus(true);
        control.knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        control.knob.setRotaryParameters(juce::degreesToRadians(225.0f), juce::degreesToRadians(495.0f), true);
        control.knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, parameter == cutoff ? 54 : 47, 23);
        control.knob.addMouseListener(this, true);
        addAndMakeVisible(control.knob);
        control.attachment = std::make_unique<ParameterState::SliderAttachment>(
            state, parameterId(parameter), control.knob);

        const auto unit = value->getLabel();
        const auto scaledUnit = unit == "Hz" && parameter == cutoff ? juce::String("kHz")
                               : unit == "ms" ? juce::String("s") : juce::String();
        control.knob.textFromValueFunction = [value, unit, scaledUnit](double amount)
        {
            if (amount >= 1000.0 && scaledUnit.isNotEmpty())
                return juce::String(amount / 1000.0, 2).trimCharactersAtEnd("0")
                           .trimCharactersAtEnd(".") + " " + scaledUnit;

            const auto display = value->getText(value->convertTo0to1(static_cast<float>(amount)), 0);
            return unit.isEmpty() || display.endsWith(unit) ? display : display + (unit == "%" ? "" : " ") + unit;
        };
        if (scaledUnit.isNotEmpty())
        {
            control.knob.valueFromTextFunction = [value, unit, scaledUnit](const juce::String& entry)
            {
                const auto trimmed = entry.trimEnd();
                if (trimmed.endsWithIgnoreCase(scaledUnit)
                    && (unit != "ms" || !trimmed.endsWithIgnoreCase(unit)))
                    return trimmed.getDoubleValue() * 1000.0;
                return static_cast<double>(value->convertFrom0to1(value->getValueForText(entry)));
            };
        }
        control.knob.updateText();
    }

    void place(std::initializer_list<Parameter> group, juce::Rectangle<int> area)
    {
        const auto cellWidth = area.getWidth() / static_cast<int>(group.size());
        for (const auto parameter : group)
        {
            auto cell = area.removeFromLeft(cellWidth);
            knobs[parameter].label.setBounds(cell.removeFromTop(17));
            knobs[parameter].knob.setBounds(cell);
        }
    }

    static void drawGrid(juce::Graphics& graphics, juce::Rectangle<int> bounds)
    {
        graphics.setColour(canvas);
        graphics.fillRoundedRectangle(bounds.toFloat(), 5.0f);
        graphics.setColour(border.withAlpha(0.38f));
        graphics.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 5.0f, 1.0f);
        auto grid = bounds.reduced(7);
        graphics.drawHorizontalLine(grid.getCentreY(), static_cast<float>(grid.getX()),
                                    static_cast<float>(grid.getRight()));
        for (int i = 1; i < 4; ++i)
            graphics.drawVerticalLine(grid.getX() + grid.getWidth() * i / 4,
                                      static_cast<float>(grid.getY()), static_cast<float>(grid.getBottom()));
    }

    template <typename Sample>
    static void drawCurve(juce::Graphics& graphics, juce::Rectangle<int> bounds, juce::Colour colour,
                          const Sample& sample)
    {
        juce::Path path;
        auto plot = bounds.reduced(8).toFloat();
        constexpr int points = 160;
        for (int i = 0; i <= points; ++i)
        {
            const auto phase = static_cast<float>(i) / static_cast<float>(points);
            const auto x = plot.getX() + plot.getWidth() * phase;
            const auto y = plot.getCentreY() - sample(phase) * plot.getHeight() * 0.45f;
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        graphics.setColour(colour);
        graphics.strokePath(path, juce::PathStrokeType(1.6f));
    }

    void paintDisplays(juce::Graphics& graphics) const
    {
        const auto mod = engine.modulation();
        const auto base = static_cast<float>(knobs[position].knob.getValue() * 0.01);
        const auto tableIndex = tableSelector.box.getSelectedItemIndex();
        const auto showingModulation = mod.playing && std::abs(mod.position - base) > 0.005f;
        drawGrid(graphics, wavePlot);
        if (showingModulation)
            drawCurve(graphics, wavePlot, muted.withAlpha(0.45f),
                      [&](float phase) { return engine.preview(phase, base, tableIndex); });

        const auto currentPosition = showingModulation ? mod.position : base;
        drawCurve(graphics, wavePlot, accent,
                  [&](float phase) { return engine.preview(phase, currentPosition, tableIndex); });

        const auto imported = imports.snapshot();
        const auto layerTable = layerSelector.box.getSelectedItemIndex();
        const auto layerPosition = static_cast<float>(knobs[position2].knob.getValue() * 0.01);
        drawGrid(graphics, layerPlot);
        drawCurve(graphics, layerPlot, accent, [&](float phase)
        {
            return engine.previewLayer(phase, layerPosition, layerTable, imported.get());
        });
        graphics.setFont(font(12));
        graphics.setColour(muted);
        graphics.drawFittedText(imported != nullptr ? imported->name() : "No WAV imported",
                                importLabel, juce::Justification::centredLeft, 1);

        paintEnvelope(graphics);

        const auto cutoffHz = mod.playing ? mod.cutoff : static_cast<float>(knobs[cutoff].knob.getValue());
        const auto damping = juce::jmap(static_cast<float>(knobs[resonance].knob.getValue() * 0.01),
                                        1.41421356f, 0.36f);
        drawGrid(graphics, filterPlot);
        drawCurve(graphics, filterPlot, accent, [cutoffHz, damping](float proportion)
        {
            const auto ratio = 20.0f * std::pow(1000.0f, proportion) / cutoffHz;
            const auto decibels = -20.0f * std::log10(std::hypot(1.0f - ratio * ratio, damping * ratio));
            return juce::jmap(juce::jlimit(-36.0f, 12.0f, decibels), -36.0f, 12.0f, -1.0f, 1.0f);
        });

        drawGrid(graphics, lfoPlot);
        drawCurve(graphics, lfoPlot, accent, [](float phase)
        {
            return std::sin(juce::MathConstants<float>::twoPi * phase);
        });

        if (mod.playing || modeSelector.box.getSelectedItemIndex() == 0)
        {
            const auto plot = lfoPlot.reduced(8).toFloat();
            const auto x = plot.getX() + plot.getWidth() * mod.phase;
            const auto y = plot.getCentreY() - std::sin(juce::MathConstants<float>::twoPi * mod.phase)
                                              * plot.getHeight() * 0.45f;
            graphics.setColour(focus);
            graphics.fillEllipse(x - 2.5f, y - 2.5f, 5.0f, 5.0f);
        }

        const auto reading = "Live  " + juce::String(juce::roundToInt(currentPosition * 100.0f))
                             + "% position   /   " + juce::String(juce::roundToInt(cutoffHz)) + " Hz";
        graphics.setFont(font(12));
        graphics.setColour(muted);
        graphics.drawText(reading, modulationReadout, juce::Justification::centred);
    }

    void paintEnvelope(juce::Graphics& graphics) const
    {
        drawGrid(graphics, envelopePlot);
        const auto plot = envelopePlot.reduced(8).toFloat();
        const auto time = [this](Parameter parameter)
        {
            return static_cast<float>(std::log1p(knobs[parameter].knob.getValue()));
        };
        const auto attackTime = time(attack);
        const auto decayTime = time(decay);
        constexpr float holdTime = 5.0f;
        const auto scale = plot.getWidth() / (attackTime + decayTime + holdTime + time(release));
        const auto attackEnd = plot.getX() + attackTime * scale;
        const auto decayEnd = attackEnd + decayTime * scale;
        const auto sustainY = plot.getBottom()
                            - plot.getHeight() * static_cast<float>(knobs[sustain].knob.getValue() * 0.01);
        juce::Path curve;
        curve.startNewSubPath(plot.getBottomLeft());
        curve.lineTo(attackEnd, plot.getY());
        curve.lineTo(decayEnd, sustainY);
        curve.lineTo(decayEnd + holdTime * scale, sustainY);
        curve.lineTo(plot.getBottomRight());
        graphics.setColour(accent);
        graphics.strokePath(curve, juce::PathStrokeType(1.6f));
    }

    void timerCallback() override
    {
        repaint(wavePlot);
        repaint(layerPlot);
        repaint(importLabel);
        repaint(envelopePlot);
        repaint(filterPlot);
        repaint(lfoPlot);
        repaint(modulationReadout);
    }

    EditorLookAndFeel theme;
    ParameterState& state;
    const SynthEngine& engine;
    WavetableImports& imports;
    juce::ComboBox patch;
    Selector tableSelector;
    Selector layerSelector;
    Selector modeSelector;
    juce::TextButton importButton { "Import WAV" };
    std::unique_ptr<juce::FileChooser> chooser;
    std::array<KnobControl, parameterCount> knobs;
    juce::TooltipWindow tooltips { this, 600 };
};
}

juce::AudioProcessorEditor* createWavetableEditor(juce::AudioProcessor& processor,
                                                ParameterState& state, const SynthEngine& engine,
                                                WavetableImports& imports)
{
    return new Editor(processor, state, engine, imports);
}
}
