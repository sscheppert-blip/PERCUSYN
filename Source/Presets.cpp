#include "Presets.h"

#include <BinaryData.h>

namespace percusyn
{
namespace
{
constexpr const char* format = "percusyn-preset";

// Schema v1 had one HiHat voice with an open/closed switch and a Kick (model.js migrateV1).
juce::NamedValueSet migrateV1 (const juce::NamedValueSet& in)
{
    juce::NamedValueSet out;
    const bool open = (int) in.getWithDefault ("hihat_hat_open", 0) == 1;
    for (const auto& nv : in)
    {
        const auto id = nv.name.toString();
        if (id.startsWith ("kick_") || id.endsWith ("_hat_open"))
            continue;
        if (id.startsWith ("hihat_"))
        {
            const auto key = id.substring (6);
            const bool activeField = key.endsWith ("_active");
            out.set ("closedhat_" + key, activeField && open ? juce::var (0) : nv.value);
            out.set ("openhat_" + key, activeField && ! open ? juce::var (0) : nv.value);
        }
        else
            out.set (nv.name, nv.value);
    }
    if (open)
    {
        out.set ("closedhat_amp_decay", 75);
        out.set ("closedhat_osc_decay", 60);
    }
    else
    {
        out.set ("openhat_amp_decay", 680);
        out.set ("openhat_osc_decay", 650);
    }
    return out;
}

juce::String sanitizeName (juce::String name)
{
    name = name.trim().substring (0, 64);
    return name.isEmpty() ? juce::String ("Untitled") : name;
}
} // namespace

Presets::Presets (juce::AudioProcessorValueTreeState& s, Pattern& p) : state (s), pattern (p)
{
    refresh();
}

juce::File Presets::userFolder()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("PERCUSYN")
        .getChildFile ("Presets");
}

void Presets::refresh()
{
    list.clear();

    juce::StringArray resources;
    for (int i = 0; i < PresetData::namedResourceListSize; ++i)
        resources.add (PresetData::namedResourceList[i]);
    juce::Array<std::pair<juce::String, juce::String>> factory;   // original filename, json
    for (const auto& r : resources)
    {
        int size = 0;
        if (const char* data = PresetData::getNamedResource (r.toRawUTF8(), size))
            factory.add ({ PresetData::getNamedResourceOriginalFilename (r.toRawUTF8()), juce::String::fromUTF8 (data, size) });
    }
    std::sort (factory.begin(), factory.end(), [] (const auto& a, const auto& b) { return a.first < b.first; });
    for (const auto& [filename, json] : factory)
    {
        PresetEntry e;
        e.factory = true;
        e.json = json;
        e.name = juce::JSON::parse (json).getProperty ("name", filename).toString();
        list.add (e);
    }

    auto files = userFolder().findChildFiles (juce::File::findFiles, false, "*.json");
    files.sort();
    for (const auto& f : files)
    {
        const auto data = juce::JSON::parse (f);
        if (data.getProperty ("format", {}).toString() != format)
            continue;
        PresetEntry e;
        e.factory = false;
        e.file = f;
        e.name = data.getProperty ("name", f.getFileNameWithoutExtension()).toString();
        list.add (e);
    }
}

int Presets::indexOf (const juce::String& name) const
{
    for (int i = 0; i < list.size(); ++i)
        if (list[i].name == name)
            return i;
    return -1;
}

void Presets::setParam (const juce::String& id, float value)
{
    if (auto* param = state.getParameter (id))
    {
        const auto& range = param->getNormalisableRange();
        param->setValueNotifyingHost (range.convertTo0to1 (juce::jlimit (range.start, range.end, value)));
    }
}

void Presets::init()
{
    for (int p = 0; p < numGlobalParams; ++p)
        setParam (globalParamId (p), globalParamDefs[(size_t) p].def);
    for (int v = 0; v < numVoices; ++v)
        for (int p = 0; p < numVoiceParams; ++p)
            setParam (voiceParamId (v, p), voiceDefault (v, p));
    pattern.clearAll();
}

juce::var Presets::toVar (const juce::String& name) const
{
    auto* params = new juce::DynamicObject();
    for (int p = 0; p < numGlobalParams; ++p)
        params->setProperty (globalParamId (p), state.getRawParameterValue (globalParamId (p))->load());
    for (int v = 0; v < numVoices; ++v)
    {
        for (int p = 0; p < numVoiceParams; ++p)
            params->setProperty (voiceParamId (v, p), state.getRawParameterValue (voiceParamId (v, p))->load());
        for (int i = 0; i < numSteps; ++i)
        {
            for (auto f : { StepField::Active, StepField::Velocity, StepField::Probability, StepField::Condition })
                params->setProperty (Pattern::fieldId (v, i, f), pattern.getField (v, i, f));
            params->setProperty (Pattern::octaveId (v, i), 0);
        }
    }

    auto* root = new juce::DynamicObject();
    root->setProperty ("format", format);
    root->setProperty ("schemaVersion", 2);
    root->setProperty ("name", sanitizeName (name));
    root->setProperty ("parameters", juce::var (params));
    return juce::var (root);
}

bool Presets::fromVar (const juce::var& data, juce::String& name, juce::String& error)
{
    const int schema = (int) data.getProperty ("schemaVersion", 0);
    if (data.getProperty ("format", {}).toString() != format || (schema != 1 && schema != 2)
        || ! data.getProperty ("name", {}).isString())
    {
        error = "Unsupported PERCUSYN preset (schema v1/v2 required).";
        return false;
    }
    auto* obj = data.getProperty ("parameters", {}).getDynamicObject();
    if (obj == nullptr)
    {
        error = "The preset has no parameter object.";
        return false;
    }
    const auto values = schema == 1 ? migrateV1 (obj->getProperties()) : obj->getProperties();

    // Like the mockup: anything missing from the preset returns to its schema default.
    auto valueOr = [&values] (const juce::String& id, float fallback)
    {
        const auto* v = values.getVarPointer (id);
        if (v == nullptr || ! (v->isDouble() || v->isInt() || v->isInt64() || v->isBool()))
            return fallback;
        const auto d = (double) *v;
        return std::isfinite (d) ? (float) d : fallback;
    };

    for (int p = 0; p < numGlobalParams; ++p)
        setParam (globalParamId (p), valueOr (globalParamId (p), globalParamDefs[(size_t) p].def));
    for (int v = 0; v < numVoices; ++v)
    {
        for (int p = 0; p < numVoiceParams; ++p)
            setParam (voiceParamId (v, p), valueOr (voiceParamId (v, p), voiceParamDefs[(size_t) p].def));
        for (int i = 0; i < numSteps; ++i)
        {
            pattern.setField (v, i, StepField::Active, valueOr (Pattern::fieldId (v, i, StepField::Active), 0));
            pattern.setField (v, i, StepField::Velocity, valueOr (Pattern::fieldId (v, i, StepField::Velocity), 100));
            pattern.setField (v, i, StepField::Probability, valueOr (Pattern::fieldId (v, i, StepField::Probability), 100));
            pattern.setField (v, i, StepField::Condition, valueOr (Pattern::fieldId (v, i, StepField::Condition), 0));
        }
    }
    name = sanitizeName (data.getProperty ("name", {}).toString());
    return true;
}

bool Presets::load (int index, juce::String& error)
{
    if (! juce::isPositiveAndBelow (index, list.size()))
    {
        error = "Preset not found.";
        return false;
    }
    const auto& e = list.getReference (index);
    const auto data = e.factory ? juce::JSON::parse (e.json) : juce::JSON::parse (e.file);
    juce::String name;
    return fromVar (data, name, error);
}

bool Presets::saveUser (const juce::String& rawName, juce::String& error)
{
    const auto name = sanitizeName (rawName);
    auto folder = userFolder();
    if (! folder.createDirectory())
    {
        error = "Could not create the preset folder.";
        return false;
    }
    const auto file = folder.getChildFile (juce::File::createLegalFileName (name) + ".json");
    if (! file.replaceWithText (juce::JSON::toString (toVar (name))))
    {
        error = "Could not write " + file.getFullPathName();
        return false;
    }
    refresh();
    return true;
}

bool Presets::deleteUser (int index, juce::String& error)
{
    if (! juce::isPositiveAndBelow (index, list.size()) || list[index].factory)
    {
        error = "Factory presets are read-only.";
        return false;
    }
    if (! list[index].file.deleteFile())
    {
        error = "Could not delete the preset file.";
        return false;
    }
    refresh();
    return true;
}

bool Presets::exportTo (const juce::File& file, const juce::String& name, juce::String& error) const
{
    if (! file.replaceWithText (juce::JSON::toString (toVar (name))))
    {
        error = "Could not write " + file.getFullPathName();
        return false;
    }
    return true;
}

bool Presets::importFrom (const juce::File& file, juce::String& name, juce::String& error)
{
    juce::var data;
    const auto result = juce::JSON::parse (file.loadFileAsString(), data);
    if (result.failed())
    {
        error = "Import failed: the file is not valid JSON.";
        return false;
    }
    return fromVar (data, name, error);
}
} // namespace percusyn
