#pragma once

#include <vector>
#include "Interface.h"

#define COPY_INTERFACE_WRAPPER_COMMON(ctor, interface_type, symbol_name)\
public:\
    ctor(const std::shared_ptr<Plugin>& plugin): m_plugin(plugin) {\
        using FuncGetInterface = interface_type* (*)(void);\
        auto get_interface = plugin->Symbol<FuncGetInterface>(symbol_name);\
        if (get_interface) {\
            m_interface = get_interface();\
            if (m_interface) {\
                m_data = m_interface->create();\
            }\
        }\
    }\
\
    ctor(const ctor& that): ctor(that.m_plugin) {\
    }\
\
    ctor(ctor&& that) {\
        m_data = that.m_data;\
        that.m_data = nullptr;\
        m_interface = that.m_interface;\
        that.m_interface = nullptr;\
        m_plugin = std::move(that.m_plugin);\
    }\
\
    ~ctor() {\
        if (m_interface) {\
            if (m_data) {\
                m_interface->destroy(m_data);\
                m_data = nullptr;\
            }\
            m_interface = nullptr;\
        }\
    }\
\
    explicit operator bool () const {\
        return m_plugin && m_interface && m_data;\
    }\
\
    const std::shared_ptr<Plugin>& GetPlugin() const {\
        return m_plugin;\
    }\
\
    std::vector<const BaseOption*> GetOptions() const {\
        std::vector<const BaseOption*> options;\
        const auto** option = m_interface->get_options(m_data);\
        if (option) {\
            for (; *option; ++option) {\
                printf("%p\n", *option);\
                options.push_back(*option);\
            }\
        }\
        return options;\
    }\
\
private:\
    std::shared_ptr<Plugin> m_plugin;\
    interface_type* m_interface = nullptr;\
    void* m_data = nullptr

// NOTE: 
// Following code works, but has three drawbacks:
// - requires handwriting "trait"
// - code bloating, 600 bytes roughly
// - requires subclass to call base class's ctors

/*
namespace mous {

struct BaseOption;

template<typename T>
struct InterfaceTrait {};

template<>
struct InterfaceTrait<OutputInterface> {
    constexpr static const char* GetSymbolName() {
        return "MousGetOutputInterface";
    }
};

template<>
struct InterfaceTrait<DecoderInterface> {
    constexpr static const char* GetSymbolName() {
        return "MousGetDecoderInterface";
    }
};

template<typename T>
class InterfaceWrapper {
public:
    InterfaceWrapper(const std::shared_ptr<Plugin>& plugin): m_plugin(plugin) {
        using FuncGetInterface = T* (*)(void);
        auto symbol_name = InterfaceTrait<T>::GetSymbolName();
        auto get_interface = plugin->Symbol<FuncGetInterface>(symbol_name);
        if (get_interface) {
            m_interface = get_interface();
            if (m_interface) {
                m_data = m_interface->create();
            }
        }
    }

    InterfaceWrapper(const InterfaceWrapper& that): InterfaceWrapper(that.m_plugin) {
    }

    InterfaceWrapper(InterfaceWrapper&& that) {
        m_data = that.m_data;
        that.m_data = nullptr;
        m_interface = that.m_interface;
        that.m_interface = nullptr;
        m_plugin = std::move(that.m_plugin);
    }

    ~InterfaceWrapper() {
        if (m_interface) {
            if (m_data) {
                m_interface->destroy(m_data);
                m_data = nullptr;
            }
            m_interface = nullptr;
        }
    }

    explicit operator bool () const {
        return m_plugin && m_interface && m_data;
    }

    const std::shared_ptr<Plugin>& GetPlugin() const {
        return m_plugin;
    }
    
    std::vector<const BaseOption*> GetOptions() const {
        std::vector<const BaseOption*> options;
        const auto* option = m_interface->get_options(m_data);
        for (; option; ++option) {
            options.push_back(option);
        }
        return options;
    }

protected:
    std::shared_ptr<Plugin> m_plugin;
    T* m_interface = nullptr;
    void* m_data = nullptr;
};

}
*/
