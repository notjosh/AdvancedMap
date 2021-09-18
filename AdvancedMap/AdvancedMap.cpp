#include <Headers/kern_api.hpp>
#include <Headers/kern_util.hpp>

#include "AdvancedMap.hpp"

#define MODULE_SHORT "advmap"

// DeviceSupportsAdvancedMapRendering = UoeNJcQ336tc+F5/1r4tqQ
// RenderWideGamutImagesAtDisplayTime = jMiqevikb6QWeHOhvLsw6A  // arbitrary, conveniently the same length, seems to return true in the same(ish) cases as having a Good™ GPU
static const uint8_t find[] = "DeviceSupportsAdvancedMapRendering";
static const uint8_t repl[] = "RenderWideGamutImagesAtDisplayTime";
static const uint8_t findmd5[] = "UoeNJcQ336tc+F5/1r4tqQ";
static const uint8_t replmd5[] = "jMiqevikb6QWeHOhvLsw6A";

static_assert(sizeof(find) == sizeof(repl), "Invalid patch size");
static_assert(sizeof(findmd5) == sizeof(replmd5), "Invalid patch size");

static AdvancedMapPlugin *callbackAdvancedMap = nullptr;

bool AdvancedMapPlugin::init()
{
    DBGLOG(MODULE_SHORT, "testing, 123");
    DBGLOG(MODULE_SHORT, "init");

    callbackAdvancedMap = this;

    LiluAPI::Error error = lilu.onPatcherLoad([](void *user, KernelPatcher &patcher) {
        callbackAdvancedMap->processPatcher(patcher);
    }, this);

    if (error != LiluAPI::Error::NoError) {
        SYSLOG(MODULE_SHORT, "Failed to register onPatcherLoad method: %d", error);
        return false;
    }

    DBGLOG(MODULE_SHORT, "completed init");

    return true;
}

void AdvancedMapPlugin::deinit()
{
}

void AdvancedMapPlugin::csValidatePage(vnode_t vp, memory_object_t pager, memory_object_offset_t page_offset, const void *data, int *validated_p, int *tainted_p, int *nx_p) {
    FunctionCast(csValidatePage,
                 callbackAdvancedMap->orgCsValidatePage)(vp,
                                                         pager,
                                                         page_offset,
                                                         data,
                                                         validated_p,
                                                         tainted_p,
                                                         nx_p);

    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    if (vn_getpath(vp, path, &pathlen) == 0) {
        if (UNLIKELY(UserPatcher::matchSharedCachePath(path))) {
            if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(data),
                                                       PAGE_SIZE,
                                                       find,
                                                       sizeof(find),
                                                       repl,
                                                       sizeof(repl)))) {
                DBGLOG(MODULE_SHORT, "patched advanced map support (via string)");
            }

            if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(data),
                                                       PAGE_SIZE,
                                                       findmd5,
                                                       sizeof(findmd5),
                                                       replmd5,
                                                       sizeof(replmd5)))) {
                DBGLOG(MODULE_SHORT, "patched advanced map support (via md5)");
            }
        }
    }
}

void AdvancedMapPlugin::processPatcher(KernelPatcher &patcher)
{
    DBGLOG(MODULE_SHORT, "start processPatcher");

    const char* symbol = "_cs_validate_page";
    mach_vm_address_t kern = patcher.solveSymbol(KernelPatcher::KernelID, symbol);

    if (patcher.getError() == KernelPatcher::Error::NoError) {
        SYSLOG(MODULE_SHORT, "found symbol: '%s' at: %p", symbol, kern);

        orgCsValidatePage = patcher.routeFunctionLong(kern,
                                                      reinterpret_cast<mach_vm_address_t>(csValidatePage),
                                                      true,
                                                      true);

        if (patcher.getError() != KernelPatcher::Error::NoError) {
            SYSLOG(MODULE_SHORT, "failed to hook %s", symbol);
        } else {
            DBGLOG(MODULE_SHORT, "hooked %s", symbol);
        }
    } else {
        SYSLOG(MODULE_SHORT, "failed to find symbol: '%s' (error: %d)", symbol, patcher.getError());
    }

    DBGLOG(MODULE_SHORT, "end processPatcher");
}
