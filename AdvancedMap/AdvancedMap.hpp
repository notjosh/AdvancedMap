#ifndef AdvancedMap_hpp
#define AdvancedMap_hpp

#include <Headers/kern_patcher.hpp>

class AdvancedMapPlugin {
public:
    bool init();
    void deinit();

private:
    /**
     *  Patch kext if needed and prepare other patches
     *
     *  @param patcher KernelPatcher instance
     */
    void processPatcher(KernelPatcher &patcher);

    /**
     *  Hooked methods / callbacks
     */
    static void csValidatePage(vnode_t vp, memory_object_t pager, memory_object_offset_t page_offset, const void *data, int *validated_p, int *tainted_p, int *nx_p);

    /**
     *  Original method
     */
    mach_vm_address_t orgCsValidatePage {};
};

#endif
