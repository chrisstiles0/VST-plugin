# Plug-in editor style guide

Use these defaults for the production editor. Dimensions are logical pixels so the layout can scale with Windows display settings.

## Typography

Use **Segoe UI** for labels, controls, and values. Fall back to the Windows system UI font if it is unavailable. Use sentence case for labels and show units alongside values, such as `12 st` or `420 Hz`.

| Role | Size | Weight |
| --- | --- | --- |
| Instrument name | 20 px | Semibold |
| Section heading | 14 px | Semibold |
| Control label | 13 px | Regular |
| Value and unit | 12 px | Regular |
| Tooltip and helper text | 12 px | Regular |

## Colour

| Role | Hex | Use |
| --- | --- | --- |
| Canvas | `#111418` | Editor background |
| Panel | `#1B2027` | Control groups |
| Control surface | `#272F39` | Buttons, selectors, and editable fields |
| Hover surface | `#303A46` | Pointer hover on interactive surfaces |
| Border | `#5E6D80` | Control outlines and dividers |
| Text | `#F1F4F8` | Primary labels and values |
| Muted text | `#AAB6C5` | Secondary labels and units |
| Accent | `#62CBE3` | Active arcs, selection marks, and enabled toggles |
| Text on accent | `#111418` | Labels on filled accent controls |
| Focus | `#D3F6FF` | Keyboard focus outline |
| Warning | `#FFB86B` | Clipping or another condition requiring attention |

## Geometry

| Element | Default |
| --- | --- |
| Initial editor | 760 × 480 px |
| Spacing grid | 4 px |
| Editor margin | 16 px |
| Panel gap and inner padding | 12 px |
| Label to control gap | 4 px |
| Panel radius | 8 px |
| Button and selector radius | 4 px |
| Standard button, selector, or value field height | 32 px |
| Standard rotary control diameter | 40 px |
| Focus outline | 2 px |

Group the controls by signal flow and keep labels and values in the same position for controls of the same type. Use the same control geometry across oscillator, envelope, filter, LFO, and output sections.

## Interaction states

- Use the hover surface on interactive fields and keep the label readable.
- Pair the accent colour with a visible mark, position, or label for toggles and selections. Use a clip label or indicator shape alongside the warning colour.
- Draw the focus outline while a control has keyboard focus. Keep it visible on selected controls.
- Dim disabled controls and remove them from keyboard navigation.
- Show the current value during adjustment. Keep value precision and units consistent within a parameter type.

Define the shared visual values once when the editor is implemented. Check text fit, focus visibility, and control alignment at 100% and 150% Windows display scaling in FL Studio.
