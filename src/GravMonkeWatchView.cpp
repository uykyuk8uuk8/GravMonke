#include "GravMonkeWatchView.hpp"
#include "config.hpp"
#include "monkecomputer/shared/ViewLib/MonkeWatch.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "UnityEngine/Vector3.hpp"

DEFINE_TYPE(GravMonke::GravMonkeWatchView);

using namespace GorillaUI;
using namespace UnityEngine;

extern bool allowGravMonke;
extern Vector3 gravityWas;

namespace GravMonke
{
    void GravMonkeWatchView::Awake()
    {
        toggleHandler = new UIToggleInputHandler(EKeyboardKey::Enter, EKeyboardKey::Enter, true);
    }

    void GravMonkeWatchView::DidActivate(bool firstActivation)
    {
        std::function<void(bool)> fun = std::bind(&GravMonkeWatchView::OnToggle, this, std::placeholders::_1);
        toggleHandler->toggleCallback = fun;
        Redraw();
    }

    void GravMonkeWatchView::Redraw()
    {
        text = "";

        DrawHeader();
        DrawBody();

        watch->Redraw();
    }

    void GravMonkeWatchView::DrawHeader()
    {
        text += "<color=#ffff00>== <color=#fdfdfd>Grav Monke/Group</color> ==</color>\n";
    }

    void GravMonkeWatchView::DrawBody()
    {
        text += "\nGrav Monke is:\n  ";
        text += config.enabled ? "<color=#00ff00>enabled</color>" : "<color=#ff0000>disabled</color>";

        if (config.enabled && !allowGravMonke)
        {
            text += "\n\nBut is disabled\ndue to not being in\na private room\n";
        }
    }

    void GravMonkeWatchView::OnToggle(bool value)
    {
        config.enabled = value;

        if (!value && allowGravMonke)
        {
            using SetGravity = function_ptr_t<void, Vector3&>;
            static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
            set_gravity(gravityWas);
        }

        SaveConfig();
    }

    void GravMonkeWatchView::OnKeyPressed(int key)
    {
        toggleHandler->HandleKey((EKeyboardKey)key);
        Redraw();
    }
}