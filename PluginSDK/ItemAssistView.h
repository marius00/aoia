#ifndef ITEMASSISTVIEW_H
#define ITEMASSISTVIEW_H

#include <PluginSDK/IPluginView.h>


template < class T >
class ItemAssistView
  : public CWindowImpl<T>
  , public aoia::IPluginView
{
    typedef ItemAssistView<T> ThisType;

public:
    BEGIN_MSG_MAP_EX(ThisType)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
    END_MSG_MAP()

    LRESULT OnEraseBkgnd(HDC dc) { return 1; }

    ItemAssistView(void)
    {
    }

    virtual ~ItemAssistView(void)
    {
    }

    virtual connection_t connectStatusChanged(status_signal_t::slot_function_type slot)
    {
        return m_statusTextSignal.connect(slot);
    }

    virtual void disconnect(connection_t slot)
    {
        slot.disconnect();
    }

    virtual bool PreTranslateMsg(MSG* pMsg)
    {
        // Override in derived classes
        return false;
    }

    virtual void OnAOServerMessage(Parsers::AOMessageBase &msg)
    {
        // Override in derived classes
    }

    virtual void OnAOClientMessage(Parsers::AOClientMessageBase &msg)
    {
        // Override in derived classes
    }

    virtual HWND GetWindow() const
    {
        return m_hWnd;
    }

    virtual void OnActive(bool doActivation)
    {
        // Override in derived classes
    }

    virtual HWND GetToolbar() const
    {
        return m_toolbar.m_hWnd;
    }

    virtual std::tstring GetStatusText() const
    {
        return m_statusText;
    }

protected:
    WTL::CToolBarCtrl m_toolbar;

    void setStatusText(std::tstring const& status)
    {
        m_statusText = status;
        m_statusTextSignal();
    }

private:
    /// Signal triggered when status text has been updated.
    status_signal_t m_statusTextSignal;

    std::tstring m_statusText;
};

#endif // ITEMASSISTVIEW_H
