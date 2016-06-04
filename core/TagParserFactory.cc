#include "TagParserFactory.h"
#include <scx/FileHelper.hpp>

ITagParserFactory* ITagParserFactory::Create()
{
    return new TagParserFactory();
}

void ITagParserFactory::Free(ITagParserFactory* factory)
{
    if (factory != nullptr)
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
    const auto& suffixList = parser->FileSuffix();
    if (parser != nullptr) {
        pAgent->FreeObject(parser);
        for (const string& suffix: suffixList) {
            auto iter = m_AgentMap.find(suffix);
            if (iter == m_AgentMap.end()) {
                m_AgentMap.emplace(suffix, pAgent);
            }
        }
    }
}

void TagParserFactory::RegisterTagParserPlugin(std::vector<const IPluginAgent*>& agents)
{
    for (auto agent: agents) {
        RegisterTagParserPlugin(agent);
    }
}

void TagParserFactory::UnregisterPlugin(const IPluginAgent* pAgent)
{
    ITagParser* parser = static_cast<ITagParser*>(pAgent->CreateObject());
    const auto& suffixList = parser->FileSuffix();
    if (parser != nullptr) {
        pAgent->FreeObject(parser);
        for (const string& suffix: suffixList) {
            auto iter = m_AgentMap.find(suffix);
            if (iter != m_AgentMap.end() && pAgent == iter->second) {
                m_AgentMap.erase(iter);
                // we do not care about the TagParser in use, let it leak/crash!
            }
        }
    }
}

void TagParserFactory::UnregisterPlugin(std::vector<const IPluginAgent*>& agents)
{
    for (auto agent: agents) {
        UnregisterPlugin(agent);
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
    ITagParser* parser = nullptr;
    const string& suffix = scx::FileHelper::FileSuffix(fileName);
    auto iter = m_AgentMap.find(suffix);
    if (iter == m_AgentMap.end()) {
        iter = m_AgentMap.find("*");
    }
    if (iter != m_AgentMap.end()) {
        parser =  static_cast<ITagParser*>(iter->second->CreateObject());
    }
    if (parser != nullptr) {
        m_ParserParentMap[parser] = iter->second;
    }
    return parser;
}

void TagParserFactory::FreeParser(ITagParser* parser) const
{
    auto iter = m_ParserParentMap.find(parser);
    if (iter != m_ParserParentMap.end()) {
        iter->second->FreeObject(parser);
        m_ParserParentMap.erase(iter);
    }
}



