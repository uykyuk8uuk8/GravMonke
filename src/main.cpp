#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/RaycastHit.hpp"

ModInfo modInfo;

using namespace UnityEngine;

Logger& getLogger()
{
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

bool allowGravMonke = false;
bool reset = false;

using SetGravity = function_ptr_t<void, Vector3&>;
MAKE_HOOK_OFFSETLESS(Player_GetSlidePercentage, float, Il2CppObject* self, RaycastHit raycastHit)
{
    static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
    if (allowGravMonke)
    {
        reset = false;
        Vector3 gravity = raycastHit.get_normal() * -9.81;
        set_gravity(gravity);
    }
    else if (!reset)
    {
        reset = true;
        Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f);
        set_gravity(gravity);
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
        static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
        Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f);
        set_gravity(gravity);
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

    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_GetSlidePercentage, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "GetSlidePercentage", 1));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PhotonNetworkController_OnJoinedRoom, il2cpp_utils::FindMethodUnsafe("", "PhotonNetworkController", "OnJoinedRoom", 0));
    
    getLogger().info("Mod loaded!");
}