#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>

#include "AdvancedMap.hpp"

static AdvancedMapPlugin advancedMap;

static const char *bootargOff[] {
    "-advmapoff"
};

static const char *bootargDebug[] {
    "-advmapdbg"
};

static const char *bootargBeta[] {
    "-advmapbeta"
};

PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
    KernelVersion::Monterey,
    KernelVersion::Monterey,
    []() {
        advancedMap.init();
    }
};

