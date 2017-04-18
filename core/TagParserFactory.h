#pragma once

#include <unordered_map>
using namespace std;

#include <core/ITagParserFactory.h>
using namespace mous;

class TagParserFactory: public ITagParserFactory
{
public:
    TagParserFactory();
    virtual ~TagParserFactory();

    virtual void RegisterTagParserPlugin(const Plugin* pAgent);
    virtual void RegisterTagParserPlugin(std::vector<const Plugin*>& agents);

    virtual void UnregisterPlugin(const Plugin* pAgent);
    virtual void UnregisterPlugin(std::vector<const Plugin*>& agents);
    virtual void UnregisterAll();

    virtual ITagParser* CreateParser(const std::string& fileName) const;
    virtual void FreeParser(ITagParser* parser) const;

private:
    unordered_map<string, const Plugin*> m_AgentMap;
    mutable unordered_map<ITagParser*, const Plugin*> m_ParserParentMap;
};
