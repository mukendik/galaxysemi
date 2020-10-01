// -------------------------------------------------------------------------- //
// memsize.cpp
// -------------------------------------------------------------------------- //
// RSS stands for Resident Set Size. That's the amount of memory that your
// process is actively using, and is mapped in real RAM (not swapped out to
// disk).
// If the RSS is going up, but your total virtual image size is stable, then
// you're not allocating more memory but making more active use of the memory
// you already allocated. That in itself isn't a problem, it's just a sign that
// your process is doing a lot of stuff with an increasing amount of the memory
// it already allocated.
//
// http://www.unixtop.org/
// top-3.8beta1/machine/m_macosx.c
// top-3.8beta1/machine/m_linux.c
// https://src.chromium.org/svn/trunk/src/base/process/process_metrics_mac.cc
// svn checkout
//  https://src.chromium.org/svn/trunk/src/base/process/ process_metrics
// -------------------------------------------------------------------------- //
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

#if defined(__APPLE__) && defined(__MACH__)
# include <sys/sysctl.h>
# include <mach/mach.h>
# include <mach/mach_vm.h>
# include <mach/shared_region.h>
# include <set>
# include <errno.h>

// -------------------------------------------------------------------------- //
// GetCPUTypeForProcess
// -------------------------------------------------------------------------- //
bool GetCPUTypeForProcess(pid_t, cpu_type_t* cpu_type)
{
    size_t len = sizeof(*cpu_type);
    int result = sysctlbyname("sysctl.proc_cputype",
                              cpu_type,
                              &len,
                              NULL,
                              0);
    if (result != 0) {
        std::cout << "error: sysctlbyname(sysctl.proc_cputype)" << std::endl;
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------- //
// IsAddressInSharedRegion
// -------------------------------------------------------------------------- //
bool IsAddressInSharedRegion(mach_vm_address_t addr, cpu_type_t type)
{
    if (type == CPU_TYPE_I386) {
        return addr >= SHARED_REGION_BASE_I386 &&
               addr < (SHARED_REGION_BASE_I386 + SHARED_REGION_SIZE_I386);
    } else if (type == CPU_TYPE_X86_64) {
        return addr >= SHARED_REGION_BASE_X86_64 &&
               addr < (SHARED_REGION_BASE_X86_64 + SHARED_REGION_SIZE_X86_64);
    } else {
        return false;
    }
}

// -------------------------------------------------------------------------- //
// privateAndShared
// -------------------------------------------------------------------------- //
int privateAndShared()
{
    pid_t pid = ::getpid();

    // This is a rough approximation of the algorithm that libtop uses.
    kern_return_t kr;
    size_t private_pages_count = 0;
    size_t shared_pages_count = 0;

    mach_port_t task = mach_task_self();

    cpu_type_t cpu_type;
    if (! GetCPUTypeForProcess(pid, &cpu_type)) {
        return EXIT_FAILURE;
    }

    // The same region can be referenced multiple times. To avoid double
    // counting we need to keep track of which regions we've already counted.
    std::set<int> seen_objects;

    // We iterate through each VM region in the task's address map. For shared
    // memory we add up all the pages that are marked as shared. Like libtop we
    // try to avoid counting pages that are also referenced by other tasks.
    // Since we don't have access to the VM regions of other tasks the only
    // hint we have is if the address is in the shared region area.
    //
    // Private memory is much simpler. We simply count the pages that are
    // marked as private or copy on write (COW).
    //
    // See libtop_update_vm_regions in
    // http://www.opensource.apple.com/source/top/top-67/libtop.c
    mach_vm_size_t size = 0;
    for (mach_vm_address_t address = MACH_VM_MIN_ADDRESS;; address += size) {
        vm_region_top_info_data_t info;
        mach_msg_type_number_t info_count = VM_REGION_TOP_INFO_COUNT;
        mach_port_t object_name;
        kr = mach_vm_region(task,
                            &address,
                            &size,
                            VM_REGION_TOP_INFO,
                            (vm_region_info_t) &info,
                            &info_count,
                            &object_name);
        if (kr == KERN_INVALID_ADDRESS) {
            // We're at the end of the address space.
            break;
        } else if (kr != KERN_SUCCESS) {
            std::cout << "Calling mach_vm_region failed with error: "
                      << mach_error_string(kr) << std::endl;
            return EXIT_FAILURE;
        }

        if (IsAddressInSharedRegion(address, cpu_type) &&
            info.share_mode != SM_PRIVATE) {
            continue;
        }

        if (info.share_mode == SM_COW && info.ref_count == 1) {
            info.share_mode = SM_PRIVATE;
        }

        switch (info.share_mode) {
        case SM_PRIVATE:
            private_pages_count += info.private_pages_resident;
            private_pages_count += info.shared_pages_resident;
            break;
        case SM_COW:
            private_pages_count += info.private_pages_resident;
        // Fall through
        case SM_SHARED:
            if (seen_objects.count(info.obj_id) == 0) {
                // Only count the first reference to this region.
                seen_objects.insert(info.obj_id);
                shared_pages_count += info.shared_pages_resident;
            }
            break;
        default:
            break;
        }
    }

    vm_size_t page_size;
    kr = host_page_size(task, &page_size);
    if (kr != KERN_SUCCESS) {
        std::cout << "Failed to fetch host page size, error: "
                  << mach_error_string(kr) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "private resident memory = "
              << private_pages_count * page_size << std::endl;
    std::cout << " shared resident memory = "
              << shared_pages_count * page_size << std::endl;

    return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------- //
// rsize mac
// -------------------------------------------------------------------------- //
vm_size_t rsize()
{
    kern_return_t kr;
    struct task_basic_info_64 ti;
    mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;

    mach_port_t task = mach_task_self();

    kr = task_info(task, TASK_BASIC_INFO_64, (task_info_t) &ti, &count);
    if (kr != KERN_SUCCESS) {
        std::cerr << "error: task_info" << std::endl;
        return 0;
    }

    return ti.resident_size;
}

// -------------------------------------------------------------------------- //
// meminfo mac
// -------------------------------------------------------------------------- //
void meminfo()
{
    unsigned long maxmem = 0;
    size_t size;
    kern_return_t status;
    vm_statistics_data_t vm_stats;

    size = sizeof(maxmem);
    sysctlbyname("hw.memsize", &maxmem, &size, NULL, 0);

    unsigned int count = HOST_VM_INFO_COUNT;
    status = host_statistics(mach_host_self(), HOST_VM_INFO,
                             (host_info_t) &vm_stats, &count);
    if (status != KERN_SUCCESS) {
        std::cerr << "error:  vm_statistics() failed: " << ::strerror(errno)
                  << std::endl;
        return;
    }

    std::cout << "total = " << maxmem << std::endl;
    std::cout << " free = " << vm_stats.free_count * getpagesize()
              << std::endl;
}
#endif

#if defined(linux) || defined(__linux)
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

// -------------------------------------------------------------------------- //
// rsize linux
// -------------------------------------------------------------------------- //
int rsize()
{
    char buff[4096];
    char* p;
    int fd;
    int len;

    fd = ::open("/proc/self/statm", O_RDONLY);
    len = ::read(fd, buff, sizeof(buff) - 1);
    ::close(fd);
    buff[len] = '\0';
    p = ::strtok(buff, " ");
    p = ::strtok(NULL, " ");

    return ::atoi(p) * ::getpagesize();
}

// -------------------------------------------------------------------------- //
// meminfo linux
// -------------------------------------------------------------------------- //
void meminfo()
{
    char buff[4096];
    char* p;
    int fd;
    int len;
    unsigned long memtotal = 0;
    unsigned long memfree = 0;

    fd = ::open("/proc/meminfo", O_RDONLY);
    len = ::read(fd, buff, sizeof(buff) - 1);
    ::close(fd);
    buff[len] = '\0';
    p = buff;
    p = ::strstr(p, "MemTotal:") + sizeof("MemTotal:");
    ::sscanf(p, "%lu", &memtotal);
    p = ::strstr(p, "MemFree:") + sizeof("MemFree:");
    ::sscanf(p, "%lu", &memfree);

    std::cout << "total = " << memtotal * 1024 << std::endl;
    std::cout << " free = " << memfree * 1024 << std::endl;
}
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#include <windows.h>
#include <Psapi.h>

// -------------------------------------------------------------------------- //
// rsize windows
// -------------------------------------------------------------------------- //
int rsize()
{
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
}

// -------------------------------------------------------------------------- //
// meminfo windows
// -------------------------------------------------------------------------- //
void meminfo()
{
    PERFORMANCE_INFORMATION pi;
    if (! GetPerformanceInfo(&pi, sizeof(pi))) {
        return;
    }

    std::cout << "total = " << pi.PhysicalTotal * pi.PageSize << std::endl;
    std::cout << " free = " << pi.PhysicalAvailable * pi.PageSize << std::endl;
}

#endif

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int, char**)
{
    const unsigned int MALLOC_SIZE = 10000000;

#if defined(__APPLE__) && defined(__MACH__)
    //privateAndShared();
#endif

    meminfo();
    std::cout << "rsize = " << rsize() << std::endl;

    std::cout << "----------------------------------------------- unused malloc"
              << std::endl;
    char* m = (char*) ::malloc(MALLOC_SIZE);

    meminfo();
    std::cout << "rsize = " << rsize() << std::endl;

    std::cout << "------------------------------------------------- used malloc"
              << std::endl;
    for (unsigned int i = 0; i < MALLOC_SIZE; ++i) {
        m[i] = i % 256;
    }

    meminfo();
    std::cout << "rsize = " << rsize() << std::endl;

    std::cout << "-------------------------------------------------------- free"
              << std::endl;
    ::free(m);

    meminfo();
    std::cout << "rsize = " << rsize() << std::endl;

    return EXIT_SUCCESS;
}
