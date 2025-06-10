# Scream

This repo continues the work previously made by Speechrezz [here](https://github.com/Speechrezz/Scream-Filter).

Many ideas used by our design came from reading the paper [The Art of VA Filter Design](https://archive.org/details/the-art-of-va-filter-design-rev.-2.1.2) (See Ch. 6 Nonlinearities)

### Comparisons to the original

The parameters used by this plugin do not, and cannot match 1:1 with the parameters used by the original synths filter. We explain exactly why below.

Instead we aim to capture a slightly larger parameter range, so as thgat similar, if not the same sounds can still be created only with slightly highler or lower parameter settings.

#### Gain

In the original synth, the oscillator had an "AMP" parameter controling volume, which was usually kept at 100%, and all the wavetables were (approximately) matched in loudness. In the original synth, when using the scream filter parameter configuration: 100%, 50%, 100% (Cutoff, scream, resonance respectively) with lower oscillator AMP (< 100), the character of the sound changes drastically. The lower the AMP parameter, the more resonant and feedbacky the signal sounds around the filter cutoff, and it sounds less saturated/overdriven. Keeping the AMP at 100% causes the filter to often sound overdriven when the "Resonance" is set to higher values (>50%).

In most cases people are expecting to get saturated sound with high Resonance levels. In order to acheive this, the input gain must be hitting approximately 0dB, which is a similar level to the what a synths filter would be processing.

Effect plugins cannot guarantee inputs stay consistent at any level. Users will constantly change the order of effect plugins, bypassing some plugins, duplicating them, removing them etc. All of this affects the gain that hits our internal distortion and wildly affects the resulting sound, just like any other distortion effect.

We could have added an autogain feature to boost incoming quiet signal, but this would likely ruin any signal with a long attack / swell, and capturing fast attacks in drum hits for example would be difficult, and likely ruin them too.

We decided to solve this problem by adding an input gain parameter to give users manual contol.

#### Keytracking

In the original synth, the internal filters cutoff and Q were keytracked. When applying the original filter on the synths oscillator, one would hear wildly different sounds depending on weather they played a low MIDI note (dull sound due to lower cutoff of internal lowpass filter, less Q), or pitched the oscillator down several octaves and played a high note (brighter sound due to lowpass)

This plugin is not a synth and cannot reliably know the pitch of the incoming signal. It is only possible to match the parameters 1:1 with the original if the pitch is known at all times.

We could have added pitch detection algorithms, but fast and low latency ones may lack accuracy and not capture the attack of signals very well, which could ruin the intended sound. More accurate pitch detectors may introduce an undesirable amount of latency and CPU usage, and still fail on complicated input signals such as percussion.

We could have the plugin accept MIDI input, but its not clear how the filter should handle chords, long release times, delays, pitch automation etc. In theory this solution may work very well on signals that are monophonic with no pitch automation, no delays, and with an instant release time. Requiring signals fit into this small category greatly restricts how the filter gets used, and we would like this filter to be used on anything. Additionally, routing MIDI to FX is very cumbersome in DAWs and most users wouldn't use the feature even if we offered it.

We decided it was best not to use any keytracking and instead offer the widest possble frequency range. This means our cutoffs will go much further than in the original synth.

The parameter "Resonance" in the orignal plugin is suspected to control both a boost to the gain used by an internal distortion as well as apply a boost to the Q used by internal lowpasses and highpasses. If this assumption is correct, matching the "Resonance" parameters by ear in both the original plugin and ours will be challenging.

The maximum drive that can be applied in our plugins Resonance parameter is approximately the same as the original, as well as the maximum possible Q values used by the internal LP and HP filters.

### Recommendations for comparisons

When doing comparisons, it is recommended to follow these steps in our plugin:

1. Play back the desired input audio and boost the input gain parameter and until the meter averages close to 0dB.
2. Set the Scream paramter to 0% and the Resonance parameter to 75%. This is only temporary.
3. Set the Cutoff parameter higher or lower depending on the MIDI input you are sending to the original synth. For example in the original synth, if the cutoff is at 100% and you play a low E1 midi note, the internal lowpass will be set to approxiamtely 5kHz. If you play E2, the LP cutoff will be at 10kHz, E3 will 20kHz, E0 2.5 kHz, and so on. This pattern can be observed with any spectrum analyser. We recommend you use one to match the LP cutoff in both plugins.
4. Tune the frequency of the Scream parameter by ear, or with the help of a frequency analyser
5. Tune the level of the Resonance parameter by ear, until you start hearing a similar tone. Very small adjustments to the input gain may help you match a slower sound

### Other noteworthy quirks

-   The original synths ADSR cannot set its internal release times to 0. The GUI may tell you the paramters release value is 0, but internally its not. When you release a MIDI note, the audio signal will exponentially decay in volume and will be virtually inaudible for about 100ms, but a distortion will boost that level much higher. This is a real nuisance when you start applying loads of distorions and/or compression after the synth, because this results in a nasty squeely distorted release sound, possibly followed by a click. The synth has its own distoriton effects, however they are sit in the signal chain before ADSR, resulting in smooth attacks and decays (assuming no 3rd party FX are applied afterwards).

-   The original filter suffered from self oscillation problems. In our filter, we employ clever gating tricks to prevent this.

### Room for improvement

If all parameters are correctly matched, one may notice very subtle characteristics to the distoriton applied in the original filter that we do not match exactly. These differences are observable when profiled by an oscilloscope, however the shape of the audio is still very similar and most importantly sounds "close enough". So far, no one who has beta tested the plugin has voiced this particular criticim.

Future contributions to the plugins distortion are welcome if they bring us even closer to the original. It's unlikely the averge user will benefit from a contribution like this, but it may help to satiate some purist out there.

It's worth noting that "the original" or "old" is not necessarily always better. Future filter designs can take inspiration from this design and change things like filter types (LP/HP/BP/AP/SVF), distoriton types, or change the whole signal chain, all to create cool new unique sounds, some of which may sound better to your ear.
