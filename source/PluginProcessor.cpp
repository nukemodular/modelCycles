#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID { "transpose", 1 },
        "Transpose",
        -24,
        24,
        0));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "gate", 1 },
        "Gate",
        false));

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

    const auto transposeSemitones = static_cast<int>(apvts.getRawParameterValue("transpose")->load());
    if (transposeSemitones != 0)
    {
        juce::MidiBuffer output;

        for (const auto metadata : midi)
        {
            auto message = metadata.getMessage();
            const auto samplePosition = metadata.samplePosition;

            if (message.isNoteOnOrOff())
            {
                const auto note = message.getNoteNumber();
                const auto shifted = note + transposeSemitones;

                if (shifted >= 0 && shifted <= 127)
                {
                    message.setNoteNumber(shifted);
                    output.addEvent(message, samplePosition);
                }
            }
            else
            {
                output.addEvent(message, samplePosition);
            }
        }

        midi.swapWith(output);
    }
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
