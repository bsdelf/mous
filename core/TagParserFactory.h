#include <core/ITagParserFactory.h>
#include <map>
using namespace std;
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
    typedef map<string, const IPluginAgent*>::iterator AgentMapIter;
    typedef map<string, const IPluginAgent*>::const_iterator AgentMapConstIter;

    mutable map<ITagParser*, const IPluginAgent*> m_ParserParentMap;
    typedef map<ITagParser*, const IPluginAgent*>::iterator ParserParentMapIter;
};
