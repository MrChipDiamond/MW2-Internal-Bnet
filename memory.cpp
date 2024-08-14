
#include <sys/stat.h>
#include "memory.h"
#include <Psapi.h>
#include "sdk.h"
#include "xor.hpp"
#define INRANGE(x,a,b)    (x >= a && x <= b)
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

void memory::memcopy(BYTE* dst, BYTE* src, size_t size) {
    DWORD oldprotect; //variable to hold protection
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect); //make memory read/writeable and save the old protection
    memcpy(dst, src, size); //typical memcopy
    VirtualProtect(dst, size, oldprotect, &oldprotect); //restore our old protection
}

uintptr_t memory::modinfo(info param) {
    MODULEINFO moduleInfo{};
    if (!GetModuleInformation((HANDLE)-1, GetModuleHandle(NULL), &moduleInfo, sizeof(MODULEINFO)) || !moduleInfo.lpBaseOfDll) {
        std::cout << xorstr("Couldnt GetModuleInformation") << std::endl;
        return 0;
    }
    switch (param)
    {
    case 1:
        return (uintptr_t)moduleInfo.lpBaseOfDll; //returns module base
        break;
    case 2:
        return (uintptr_t)moduleInfo.SizeOfImage; //return image size
        break;
    }
}
uintptr_t memory::find_ida_sig(const char* mod, const char* sig)
{
    /// Credits: MarkHC, although slightly modified by me and also documented

    static auto pattern_to_byte = [](const char* pattern)
    {
        /// Prerequisites
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        /// Convert signature into corresponding bytes
        for (auto current = start; current < end; ++current)
        {
            /// Is current byte a wildcard? Simply ignore that that byte later
            if (*current == '?')
            {
                ++current;

                /// Check if following byte is also a wildcard
                if (*current == '?')
                    ++current;

                /// Dummy byte
                bytes.push_back(-1);
            }
            else
            {
                /// Convert character to byte on hexadecimal base
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
    };

    const auto module_handle = GetModuleHandleA(mod);
    if (!module_handle)
        return {};

    /// Get module information to search in the given module
    MODULEINFO module_info;
    GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(module_handle), &module_info, sizeof(MODULEINFO));

    /// The region where we will search for the byte sequence
    const auto image_size = module_info.SizeOfImage;

    /// Check if the image is faulty
    if (!image_size)
        return {};

    /// Convert IDA-Style signature to a byte sequence
    auto pattern_bytes = pattern_to_byte(sig);

    const auto image_bytes = reinterpret_cast<byte*>(module_handle);

    const auto signature_size = pattern_bytes.size();
    const auto signature_bytes = pattern_bytes.data();


    /// Loop through all pages and check the accessable pages
    auto page_information = MEMORY_BASIC_INFORMATION{};
    for (auto current_page = reinterpret_cast<byte*>(module_handle); current_page < reinterpret_cast<byte*>(module_handle + image_size); current_page = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(page_information.BaseAddress) + page_information.RegionSize))
    {
        auto status = VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));

        if (page_information.Protect != PAGE_EXECUTE_READWRITE)
            continue;

        /// Now loop through all bytes and check if the byte sequence matches
        for (auto i = reinterpret_cast<uintptr_t>(page_information.BaseAddress) - reinterpret_cast<uintptr_t>(module_handle); i < page_information.RegionSize; ++i)
        {
            auto byte_sequence_found = true;

            if (i + signature_size == page_information.RegionSize)
            {
                auto status = VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));

                if (page_information.Protect != PAGE_EXECUTE_READ)
                    break;
            }

            /// Go through all bytes from the signature and check if it matches
            for (auto j = 0ul; j < signature_size; ++j)
            {
                if (image_bytes[i + j] != signature_bytes[j] /// Bytes don't match
                    && signature_bytes[j] != -1) /// Byte isn't a wildcard either, WHAT THE HECK
                {
                    byte_sequence_found = false;
                    break;
                }
            }



            /// All good, now return the right address
            if (byte_sequence_found)
                return (uintptr_t(&image_bytes[i]));
        }
    }


    /// Byte sequence wasn't found
    return {};
}

uintptr_t find_ida_sig(const char* mod, const char* sig)
{
    /// Credits: MarkHC, although slightly modified by me and also documented

    static auto pattern_to_byte = [](const char* pattern)
    {
        /// Prerequisites
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        /// Convert signature into corresponding bytes
        for (auto current = start; current < end; ++current)
        {
            /// Is current byte a wildcard? Simply ignore that that byte later
            if (*current == '?')
            {
                ++current;

                /// Check if following byte is also a wildcard
                if (*current == '?')
                    ++current;

                /// Dummy byte
                bytes.push_back(-1);
            }
            else
            {
                /// Convert character to byte on hexadecimal base
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
    };

    const auto module_handle = GetModuleHandleA(mod);
    if (!module_handle)
        return {};

    /// Get module information to search in the given module
    MODULEINFO module_info;
    GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(module_handle), &module_info, sizeof(MODULEINFO));

    /// The region where we will search for the byte sequence
    const auto image_size = module_info.SizeOfImage;

    /// Check if the image is faulty
    if (!image_size)
        return {};

    /// Convert IDA-Style signature to a byte sequence
    auto pattern_bytes = pattern_to_byte(sig);

    const auto image_bytes = reinterpret_cast<byte*>(module_handle);

    const auto signature_size = pattern_bytes.size();
    const auto signature_bytes = pattern_bytes.data();


    /// Loop through all pages and check the accessable pages
    auto page_information = MEMORY_BASIC_INFORMATION{};
    for (auto current_page = reinterpret_cast<byte*>(module_handle); current_page < reinterpret_cast<byte*>(module_handle + image_size); current_page = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(page_information.BaseAddress) + page_information.RegionSize))
    {
        auto status = VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));

        if (page_information.Protect != PAGE_EXECUTE_READWRITE)
            continue;

        /// Now loop through all bytes and check if the byte sequence matches
        for (auto i = reinterpret_cast<uintptr_t>(page_information.BaseAddress) - reinterpret_cast<uintptr_t>(module_handle); i < page_information.RegionSize; ++i)
        {
            auto byte_sequence_found = true;

            if (i + signature_size == page_information.RegionSize)
            {
                auto status = VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));

                if (page_information.Protect != PAGE_EXECUTE_READ)
                    break;
            }

            /// Go through all bytes from the signature and check if it matches
            for (auto j = 0ul; j < signature_size; ++j)
            {
                if (image_bytes[i + j] != signature_bytes[j] /// Bytes don't match
                    && signature_bytes[j] != -1) /// Byte isn't a wildcard either, WHAT THE HECK
                {
                    byte_sequence_found = false;
                    break;
                }
            }



            /// All good, now return the right address
            if (byte_sequence_found)
                return (uintptr_t(&image_bytes[i]));
        }
    }


    /// Byte sequence wasn't found
    return {};
}

uintptr_t memory::find_pattern(uintptr_t range_start, uintptr_t range_end, const char* pattern)
{

    const char* pat = pattern;
    __int64 firstMatch = NULL;
    __int64 pCur = range_start;
    __int64 region_end{};
    MEMORY_BASIC_INFORMATION mbi{};
    SIZE_T size = VirtualQuery((LPCVOID)pCur, &mbi, sizeof(mbi));
    while (sizeof(mbi) == size) {
        if (pCur >= range_end - strlen(pattern))
        {
            break;
        }
        if (!(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READWRITE))) {
            pCur += mbi.RegionSize;
            continue;
        }
        region_end = pCur + mbi.RegionSize;

        if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && !(mbi.Protect & PAGE_GUARD))
        {
            region_end = pCur + mbi.RegionSize;
            while (pCur < region_end)
            {
                if (!*pat)
                    return firstMatch;
                if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
                    if (!firstMatch)
                        firstMatch = pCur;
                    if (!pat[1] || !pat[2])
                        return firstMatch;
                    if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
                        pat += 3;
                    else
                        pat += 2;
                }
                else {
                    if (firstMatch)
                        pCur = firstMatch;
                    pat = pattern;
                    firstMatch = 0;
                }
                pCur++;
            }
        }
    }
    return NULL;


}




Scanner::Scanner(std::ptrdiff_t startAddr, std::ptrdiff_t endAddr, const char* pattern, const char* name, std::ptrdiff_t offset, bool debug)
{
    m_name = name;
    m_debugflag = debug;
    addr = (memory::find_pattern(startAddr, endAddr, pattern) + (offset));
}

Scanner::Scanner(const char* pattern, const char* name, std::ptrdiff_t offset, bool debug) : m_name{ name }, m_debugflag{ debug }
{
    addr = find_ida_sig(0, pattern);
    if (addr)
        addr = addr + (offset);

}

Scanner::Scanner(std::ptrdiff_t address)
{
    addr = address;
}


void nlog(const char* str, ...) {
    va_list ap;
    HWND notepad, edit;
    char buf[256];

    va_start(ap, str);
    vsprintf(buf, str, ap);
    va_end(ap);
    strcat(buf, "");

    notepad = FindWindow(NULL, ("Unbenannt - Editor"));
    if (!notepad)
        notepad = FindWindow(NULL, ("*Unbenannt - Editor"));

    edit = FindWindowEx(notepad, NULL, ("EDIT"), NULL);
    SendMessageA(edit, EM_REPLACESEL, 0, (LPARAM)buf);
}

auto Scanner::validaddr() -> bool
{
    if (is_bad_ptr(addr))
    {
        if (m_debugflag)
        {
            nlog("Failed --- %s : 0\n", m_name);
        }
        return false;
    }

    if (is_valid_ptr(addr))
    {
        if (m_debugflag)
        {
            nlog("Success --- %s : 0x%x\n", m_name, addr - g_data::base);
        }
        return true;
    }
};

auto Scanner::getaddr() -> std::ptrdiff_t {
    if (validaddr()) {
        return addr;
    }
    return NULL;
};

auto Scanner::resolve_call() -> std::ptrdiff_t {
    if (validaddr()) {
        return *(int*)(addr + 1) + addr + 5;
    }
    return NULL;
};

auto Scanner::resolve_lea() -> std::ptrdiff_t {
    if (validaddr()) {
        return *(int*)(addr + 3) + addr + 7;
    }
    return NULL;
};

//works with XOR
auto Scanner::resolve_cmp() -> std::ptrdiff_t {
    if (validaddr()) {
        return *(int*)(addr + 2) + addr + 6;
    }
    return NULL;
};

auto Scanner::vtable_call() -> std::ptrdiff_t {
    if (validaddr()) {

        return *(int*)(addr + 2) + addr + 6;
    }
    return NULL;
};

auto Scanner::resolve_mov() -> std::ptrdiff_t {
    if (validaddr()) {

        return *(int*)(addr + 3) + addr + 7;
    }
    return NULL;
}

auto Scanner::resolve_mov32() -> std::ptrdiff_t {
    if (validaddr()) {

        return *(int*)(addr + 2) + addr + 6;
    }
    return NULL;
}

auto Scanner::resolve_jmp() -> std::ptrdiff_t {
    if (validaddr()) {

        return *(int*)(addr + 2) + addr + 8;
    }
    return NULL;
}

auto Scanner::resolve_jmp2() -> std::ptrdiff_t {
    if (validaddr()) {

        return *(int*)(addr + 1) + addr + 5;
    }
    return NULL;
}


auto resolve_mov(uintptr_t addr) -> std::ptrdiff_t {


    return *(int*)(addr + 3) + addr + 7;

    return NULL;
}
