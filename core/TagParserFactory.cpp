#include "TagParserFactory.h"
#include <scx/FileHelper.hpp>

ITagParserFactory* ITagParserFactory::Create()
{
    return new TagParserFactory();
}

void ITagParserFactory::Free(ITagParserFactory* factory)
{
    if (factory != NULL)
        delete factory;
}

TagParserFactory::TagParserFactory()
{
}

TagParserFactory::~TagParserFactory()
{
    UnregisterAll();
}

void TagParserFactory::RegisterTagParserPlugin(const IPluginAgent* pAgent)
{
    ITagParser* parser = static_cast<ITagParser*>(pAgent->CreateObject());
    if (parser != NULL) {
        const vector<string>& key = parser->GetFileSuffix();
        pAgent->FreeObject(parser);
        for (size_t i = 0; i < key.size(); ++i) {
            AgentMapIter iter = m_AgentMap.find(key[i]);
            if (iter == m_AgentMap.end()) {
                m_AgentMap[key[i]] = pAgent;
            }
        }
    }
}

void TagParserFactory::RegisterTagParserPlugin(std::vector<const IPluginAgent*>& agents)
{
    for (size_t i = 0; i < agents.size(); ++i) {
        RegisterTagParserPlugin(agents[i]);
    }
}

void TagParserFactory::UnregisterPlugin(const IPluginAgent* pAgent)
{
    ITagParser* parser = static_cast<ITagParser*>(pAgent->CreateObject());
    if (parser != NULL) {
        const vector<string>& key = parser->GetFileSuffix();
        pAgent->FreeObject(parser);
        for (size_t i = 0; i < key.size(); ++i) {
            AgentMapIter iter = m_AgentMap.find(key[i]);
            if (iter != m_AgentMap.end() && pAgent == iter->second) {
                m_AgentMap.erase(iter);
                // we do not care about the TagParser in use, let it leak/crash!
            }
        }
    }
}

void TagParserFactory::UnregisterPlugin(std::vector<const IPluginAgent*>& agents)
{
    for (size_t i = 0; i < agents.size(); ++i) {
        UnregisterPlugin(agents[i]);
    }
}

void TagParserFactory::UnregisterAll()
{
    while (!m_AgentMap.empty()) {
        UnregisterPlugin(m_AgentMap.begin()->second);
    }
}

ITagParser* TagParserFactory::CreateParser(const std::string& fileName) const
{
    ITagParser* parser = NULL;
    const string& ext = scx::FileHelper::FileSuffix(fileName);
    AgentMapConstIter iter = m_AgentMap.find(ext);
    if (iter == m_AgentMap.end()) {
        iter = m_AgentMap.find("*");
    }
    if (iter != m_AgentMap.end()) {
        parser =  static_cast<ITagParser*>(iter->second->CreateObject());
    }
    if (parser != NULL) {
        m_ParserParentMap[parser] = iter->second;
    }
    return parser;
}

void TagParserFactory::FreeParser(ITagParser* parser) const
{
    ParserParentMapIter iter = m_ParserParentMap.find(parser);
    if (iter != m_ParserParentMap.end()) {
        iter->second->FreeObject(parser);
        m_ParserParentMap.erase(iter);
    }
}



