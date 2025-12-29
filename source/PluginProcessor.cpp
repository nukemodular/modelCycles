#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto pitchNoteChoices = []
    {
        juce::StringArray notes;
        const juce::StringArray names { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

        for (int octave = 0; octave <= 10; ++octave)
        {
            const int lastIdx = (octave == 10 ? 7 : 11); // C..G for octave 10
            for (int i = 0; i <= lastIdx; ++i)
            {
                const auto n = names[i];
                if (n.containsChar('#'))
                    notes.add(n + juce::String(octave));
                else
                    notes.add(n + " " + juce::String(octave));
            }
        }

        return notes;
    }();

    // Global controls (not track-dependent)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "patternBankGlobal", 1 },
        "Pattern Bank",
        juce::StringArray { "A", "B", "C", "D", "E", "F" },
        0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "patternIndexGlobal", 1 },
        "Pattern",
        juce::StringArray { "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16" },
        0));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { "mainVolumeGlobal", 1 },
        "Main Volume",
        0,
        127,
        100));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { "reverbSizeGlobal", 1 },
        "Reverb Size",
        0,
        127,
        64));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { "delayTimeFreeGlobal", 1 },
        "Delay Time",
        0,
        127,
        0));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "delayTimeSyncEnabled", 1 },
        "Delay Time Sync",
        false));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "delayTimeSyncIndexGlobal", 1 },
        "Delay Time (Sync)",
        juce::StringArray { "1", "2", "3", "4", "6", "8", "12", "16", "24", "32", "48", "64", "96", "128" },
        7));

    // Global overlay toggles
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "delaySendOverlayEnabled", 1 },
        "Delay Overlay Enabled",
        false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "reverbSendOverlayEnabled", 1 },
        "Reverb Overlay Enabled",
        false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "panningOverlayEnabled", 1 },
        "Panning Overlay Enabled",
        false));

    // Overlay swap parameters (used when the mini toggles are enabled in the UI).
    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { "delayFeedbackOverlay", 1 },
        "Delay Feedb (Overlay)",
        0,
        127,
        0));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { "reverbToneOverlay", 1 },
        "Reverb Tone (Overlay)",
        0,
        127,
        0));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { "panningOverlay", 1 },
        "Panning (Overlay)",
        -64,
        63,
        0));

    // Per-track controls (tracks 1-6)
    for (int track = 1; track <= 6; ++track)
    {
        const auto prefix = "t" + juce::String(track) + "_";

        // MIX footer track button state (MUTE/UNMUTE) when MIX mode is active.
        // True means the track is *unmuted* (button lit). Default: all unmuted.
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { prefix + "unmuted", 1 },
            "Unmuted (T" + juce::String(track) + ")",
            true));

        // MIX page controls (shown when MIX footer button is enabled)
        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "mixVolume", 1 },
            "Mix Volume (T" + juce::String(track) + ")",
            0,
            127,
            100));

        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "mixPan", 1 },
            "Mix Pan (T" + juce::String(track) + ")",
            -64,
            63,
            0));

        // Track machine selector (shown above each track button)
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { prefix + "machine", 1 },
            "Machine (T" + juce::String(track) + ")",
            juce::StringArray { "KICK", "SNARE", "METAL", "PERC", "TONE", "CHORD" },
            0));

        // Row 1 (col 1-5): PUNCH + PITCH/DECAY/COLOR/SHAPE
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { prefix + "punch", 1 },
            "Punch (T" + juce::String(track) + ")",
            false));

        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "pitch", 1 },
            "Pitch (T" + juce::String(track) + ")",
            -24,
            24,
            0));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { prefix + "pitchNote", 1 },
            "Pitch Note (T" + juce::String(track) + ")",
            pitchNoteChoices,
            0));

        for (auto name : { "decay", "color", "shape" })
        {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                juce::ParameterID { prefix + name, 1 },
                juce::String(name).toUpperCase() + " (T" + juce::String(track) + ")",
                0,
                127,
                0));
        }

        // Row 2 (col 1-5): GATE + SWEEP/CONTOUR/DELAY SEND/REVERB SEND
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { prefix + "gate", 1 },
            "Gate (T" + juce::String(track) + ")",
            false));

        for (auto name : { "sweep", "contour", "delaySend", "reverbSend" })
        {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                juce::ParameterID { prefix + name, 1 },
                juce::String(name).toUpperCase() + " (T" + juce::String(track) + ")",
                0,
                127,
                0));
        }

        // Row 3 (col 1-5): LFO MODE + LFO SPEED/VOL+DIST/SWING/CHANCE
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { prefix + "lfoMode", 1 },
            "LFO Mode (T" + juce::String(track) + ")",
            juce::StringArray { "FREE", "TRG", "HOLD", "ONE", "HALF" },
            0));

        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "lfoSpeed", 1 },
            "LFO Speed (T" + juce::String(track) + ")",
            -64,
            63,
            0));

        // LFO overlay (track-dependent): MULTIPLY/WAVEFORM/PHASE/DEPTH/DESTINATION/FADE
        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "lfoMultiply", 1 },
            "LFO Multiply (T" + juce::String(track) + ")",
            0,
            23,
            0));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { prefix + "lfoWaveform", 1 },
            "LFO Waveform (T" + juce::String(track) + ")",
            juce::StringArray { "TRI", "SIN", "SQR", "SAW", "ENV", "SAW-HLF", "S&H" },
            0));

        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "lfoPhase", 1 },
            "LFO Phase (T" + juce::String(track) + ")",
            0,
            127,
            0));

        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "lfoDepth", 1 },
            "LFO Depth (T" + juce::String(track) + ")",
            -64,
            63,
            0));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { prefix + "lfoDestination", 1 },
            "LFO Destination (T" + juce::String(track) + ")",
            juce::StringArray { " --- ", "PTCH", "FTUN", "DEC", "COLR", "SHPE", "SWEP", "CONT", "DELS", "REVS", "DIST", "PAN", "PAW", "GATE" },
            0));

        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { prefix + "lfoFade", 1 },
            "LFO Fade (T" + juce::String(track) + ")",
            -64,
            63,
            0));

        for (auto name : { "volDist", "swing", "chance" })
        {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                juce::ParameterID { prefix + name, 1 },
                juce::String(name).toUpperCase() + " (T" + juce::String(track) + ")",
                0,
                127,
                0));
        }
    }

    return layout;
}

PluginProcessor::PluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                      )
    , apvts(*this, nullptr, "PARAMS", createParameterLayout())
#else
    : apvts(*this, nullptr, "PARAMS", createParameterLayout())
#endif
{
    // Cache parameter pointers for the audio/MIDI thread (never call getRawParameterValue in processBlock).
    for (int track = 1; track <= 6; ++track)
    {
        const auto id = juce::String("t") + juce::String(track) + "_pitch";
        if (auto* p = apvts.getRawParameterValue(id))
            trackPitchSemitones[(size_t) (track - 1)] = p;
        else
            trackPitchSemitones[(size_t) (track - 1)] = &fallbackZero;
    }
}

PluginProcessor::~PluginProcessor() = default;

const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int)
{
}

const juce::String PluginProcessor::getProgramName (int)
{
    return {};
}

void PluginProcessor::changeProgramName (int, const juce::String&)
{
}

void PluginProcessor::prepareToPlay (double, int)
{
}

void PluginProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOutput = layouts.getMainOutputChannelSet();

   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    if (mainOutput != juce::AudioChannelSet::mono()
     && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    #if ! JucePlugin_IsSynth
    if (mainOutput != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
   #endif
}
#endif

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

   #if JucePlugin_IsMidiEffect
    buffer.clear();

    juce::MidiBuffer output;

    for (const auto metadata : midi)
    {
        auto message = metadata.getMessage();
        const auto samplePosition = metadata.samplePosition;

        if (message.isNoteOnOrOff())
        {
            // Track mapping: MIDI channels 1-6 -> tracks 1-6.
            const int channelIdx = message.getChannel() - 1;
            const int semis = (channelIdx >= 0 && channelIdx < (int) trackPitchSemitones.size())
                                ? (int) std::lround(trackPitchSemitones[(size_t) channelIdx]->load())
                                : 0;

            const int note = message.getNoteNumber();
            const int shifted = juce::jlimit(0, 127, note + semis);
            message.setNoteNumber(shifted);
        }

        output.addEvent(message, samplePosition);
    }

    midi.swapWith(output);
   #else
    juce::ignoreUnused (midi);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        juce::ignoreUnused (data);
    }
   #endif
}

bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
