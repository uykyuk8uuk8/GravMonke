#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/RaycastHit.hpp"
#include "UnityEngine/Physics.hpp"

#include "config.hpp"
#include "GravMonkeWatchView.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Register.hpp"
#include "custom-types/shared/register.hpp"

ModInfo modInfo;

using namespace UnityEngine;

Logger& getLogger()
{
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

bool allowGravMonke = false;
bool reset = true;
Vector3 gravityWas = Vector3(0.0f, 0.0f, 0.0f);

using SetGravity = function_ptr_t<void, Vector3&>;
MAKE_HOOK_OFFSETLESS(Player_GetSlidePercentage, float, Il2CppObject* self, RaycastHit raycastHit)
{
    static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
    if (allowGravMonke && config.enabled)
    {
        if (reset)
        {
            gravityWas = Physics::get_gravity();
        }
        reset = false;
        Vector3 gravity = raycastHit.get_normal() * -9.81;
        set_gravity(gravity);
    }
    else if (!reset)
    {
        reset = true;
        set_gravity(gravityWas);
    }

    return Player_GetSlidePercentage(self, raycastHit);
}

MAKE_HOOK_OFFSETLESS(PhotonNetworkController_OnJoinedRoom, void, Il2CppObject* self)
{
    PhotonNetworkController_OnJoinedRoom(self);

    Il2CppObject* currentRoom = CRASH_UNLESS(il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "get_CurrentRoom"));

    if (currentRoom)
    {
        // get wether or not this is a private room
        allowGravMonke = !CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(currentRoom, "get_IsVisible"));
    }
    else allowGravMonke = true;

    if (!allowGravMonke)
    {
        if (reset)
        {
            gravityWas = Physics::get_gravity();
        }
        reset = false;
        static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
        set_gravity(gravityWas);
    }
}

extern "C" void setup(ModInfo& info)
{
    info.id = ID;
    info.version = VERSION;

    modInfo = info;
}

extern "C" void load()
{
    getLogger().info("Loading mod...");

    GorillaUI::Init();

    if (!LoadConfig()) 
            SaveConfig();

    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_GetSlidePercentage, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "GetSlidePercentage", 1));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PhotonNetworkController_OnJoinedRoom, il2cpp_utils::FindMethodUnsafe("", "PhotonNetworkController", "OnJoinedRoom", 0));
    
    custom_types::Register::RegisterType<GravMonke::GravMonkeWatchView>(); 
    GorillaUI::Register::RegisterWatchView<GravMonke::GravMonkeWatchView*>("Grav Monke", VERSION);

    getLogger().info("Mod loaded!");
}