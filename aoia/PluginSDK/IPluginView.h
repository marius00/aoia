#ifndef PLUGINVIEWINTERFACE_H
#define PLUGINVIEWINTERFACE_H

#include <boost/smart_ptr.hpp>
#include <boost/signal.hpp>
#include <Parsers/AOMessageBase.h>
#include <Parsers/AOClientMessageBase.h>

namespace aoia
{
    struct IPluginView
    {
        typedef boost::signal<void ()>  status_signal_t;
        typedef boost::signals::connection connection_t;

        virtual connection_t connectStatusChanged(status_signal_t::slot_function_type slot) = 0;
        virtual void disconnect(connection_t slot) = 0;

        virtual void OnAOServerMessage(Parsers::AOMessageBase &msg) = 0;
        virtual void OnAOClientMessage(Parsers::AOClientMessageBase &msg) = 0;
        virtual bool PreTranslateMsg(MSG* pMsg) = 0;
        virtual HWND GetWindow() const = 0;
        virtual void OnActive(bool doActivation) = 0;
        virtual HWND GetToolbar() const = 0;
        virtual std::tstring GetStatusText() const = 0;
    };

    typedef boost::shared_ptr<IPluginView> IPluginViewPtr;
}

/// Signature of plugin DLL factory function.
typedef aoia::IPluginViewPtr (*AOIA_CreatePlugin)(std::string const&);

#endif // PLUGINVIEWINTERFACE_H
