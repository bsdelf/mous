#include <map>
using namespace std;

#include <core/ITagParserFactory.h>
using namespace mous;

class TagParserFactory: public ITagParserFactory
{
public:
    TagParserFactory();
    virtual ~TagParserFactory();

    virtual void RegisterTagParserPlugin(const IPluginAgent* pAgent);
    virtual void RegisterTagParserPlugin(std::vector<const IPluginAgent*>& agents);

    virtual void UnregisterPlugin(const IPluginAgent* pAgent);
    virtual void UnregisterPlugin(std::vector<const IPluginAgent*>& agents);
    virtual void UnregisterAll();

    virtual ITagParser* CreateParser(const std::string& fileName) const;
    virtual void FreeParser(ITagParser* parser) const;

private:
    map<string, const IPluginAgent*> m_AgentMap;
    mutable map<ITagParser*, const IPluginAgent*> m_ParserParentMap;
};
