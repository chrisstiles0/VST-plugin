# User manual

## Install and load in FL Studio

1. Close FL Studio and copy the entire **Wavetable Synth.vst3** folder to `C:\Program Files\Common Files\VST3\`. Keep the bundle's `Contents` directory intact. Windows may request administrator approval.
2. Open FL Studio x64. Go to **Options → Manage plugins**, enable **Verify plugins**, and click **Find installed plugins**. For an update, enable **Rescan previously verified plugins**.
3. Find **Wavetable Synth** as a generator, add it to the Channel Rack, and open its Piano Roll. [Image-Line plug-in installation guide](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm)

## Shape a sound

| Control | What to listen for |
| --- | --- |
| Table and Position | Classic blends sine, triangle, saw, and square. Hollow blends sine, triangle, square, and a narrow pulse. Each waveform preview shows one cycle of the selected shape. |
| Transpose, Fine tune, and Level | Set each oscillator's pitch and level separately. Oscillator 2 starts muted in Init. |
| Voices, Detune, and Spread | Layer up to seven oscillators per note. Detune separates their pitches. Spread pans them across the stereo field. |
| Amp envelope | Attack, Decay, Sustain, and Release set the shape for each new note. Replay a note after changing these controls. |
| Low-pass filter | Lower Cutoff to darken a sound. Raise Resonance to emphasise the region near cutoff. |
| LFO | Choose Free-running or Note-triggered, then set Rate. Position and Cutoff under the LFO set independent depths. At full depth, the LFO can move Position by up to 50 percentage points and Cutoff by up to four octaves in either direction within their usable ranges. |
| Output | Adjust Level while listening to a single note and a full chord. |

While notes play, the LFO marker follows the most recently started sounding note. The readout shows that note's effective position and cutoff. When position is modulated, the oscillator panel overlays the moving shape on a dim preview of the base shape.

## Layer a sub or import a wavetable

Load **Layered bass** to hear oscillator 1 sweep above a sine sub. To build that layer on another patch, set oscillator 2 to **Classic**, leave **Position** at 0%, set **Transpose** to −12 st, and raise its **Level**. The position LFO acts on oscillator 1. Both oscillators feed the same low-pass filter and amp envelope.

To use your own wavetable, click **Import WAV** in oscillator 2. Choose a mono WAV containing 1 to 256 consecutive cycles of exactly 2,048 samples each. Import selects **Imported** in the table menu. Raise **Level** and move **Position** to blend the frames. The FL Studio project stores the wavetable with the instance, so you can move the original WAV after saving.

## Start from a patch

Select a factory sound in **Load patch**. Loading it replaces the instance's sound settings.

- **Init** starts with a sine, one voice, and zero LFO depth. For a sub-bass, play MIDI notes 33–43. Try a slightly longer Attack for a softer onset and adjust Level with the mix playing.
- **Wide unison lead** separates seven slightly detuned voices across stereo. Try lowering Spread to hear the difference.
- **Modulated bass** uses a note-triggered sweep. Hold a note and turn the two LFO depths down one at a time to hear each destination.
- **Evolving pad** fades in across a chord and uses a slow free-running LFO. Leave room for its release at the end of a phrase.
- **Layered bass** combines the modulated oscillator with a sine one octave below it. Solo either layer by turning down the other oscillator's Level.

## Automate controls and reopen a project

Right-click a parameter control to reset it to its Init value or use any actions the host provides. In FL Studio, you can also move a control and use **Tools → Last tweaked → Create automation clip**. Save the project to keep each instance's settings and automation with its pattern.

## First-build audition

At 120 BPM, try these dry phrases. MIDI note numbers avoid differences in octave labels.

| Patch | Phrase |
| --- | --- |
| Wide unison lead | Notes 69, 72, 76, 74 on consecutive quarter notes. |
| Modulated bass | Hold note 45 for two beats, then note 48 for two beats. |
| Evolving pad | Hold notes 57, 60, 64, and 67 for two bars, then let the release finish. |

Start with the Init sub-bass. Change one control at a time and compare the sound and the displays. Try the three patches together, then check keyboard focus, value readability, and alignment at 100% and 150% Windows scaling. The [PRD](PRD.md#acceptance-checks) covers stereo routing, automation, recall, export, and the performance run.
