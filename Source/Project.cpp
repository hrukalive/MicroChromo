#include "Project.h"
#include "PluginProcessor.h"

Project::Project(MicroChromoAudioProcessor& p, String title) :
    FileBasedDocument(".mcmproj", "*.mcmproj", "Open", "Save"),
    processor(p), _title(title) {}

std::unique_ptr<XmlElement> Project::createXml()
{
    return nullptr;
}

void Project::loadFromXml(XmlElement* xml)
{

}

String Project::getDocumentTitle()
{
    return _title;
}

Result Project::loadDocument(const File& file)
{
    return Result::ok();
}

Result Project::saveDocument(const File& file)
{
    return Result::ok();
}

File Project::getLastDocumentOpened()
{
    return File();
}

void Project::setLastDocumentOpened(const File& file)
{

}
