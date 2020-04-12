#pragma once

#include "Common.h"

class MicroChromoAudioProcessor;

//==============================================================================
class Project : public FileBasedDocument, public ChangeBroadcaster
{
public:
    //==============================================================================
    Project(MicroChromoAudioProcessor& p, String title);
    ~Project() = default;

    //==============================================================================
    std::unique_ptr<XmlElement> createXml();
    void loadFromXml(XmlElement* xml);

private:
    //==============================================================================
    String getDocumentTitle() override;
    Result loadDocument(const File& file) override;
    Result saveDocument(const File& file) override;
    File getLastDocumentOpened() override;
    void setLastDocumentOpened(const File& file) override;

    //==============================================================================
    MicroChromoAudioProcessor& processor;
    String _title;
};
