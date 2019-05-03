// File automatically generated by hookman, **DO NOT MODIFY MANUALLY**
#ifndef _H_HOOKMAN_HOOK_CALLER
#define _H_HOOKMAN_HOOK_CALLER

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <cstdlib>
    #include <malloc.h>
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#include <custom_include1>
#include <custom_include2>

namespace hookman {

template <typename F_TYPE> std::function<F_TYPE> from_c_pointer(uintptr_t p) {
    return std::function<F_TYPE>(reinterpret_cast<F_TYPE *>(p));
}

class HookCaller {
public:
    std::vector<std::function<int(int, double[2])>> friction_factor_impls() {
        return this->_friction_factor_impls;
    }
    std::vector<std::function<int(int, double[2])>> friction_factor_2_impls() {
        return this->_friction_factor_2_impls;
    }

    void append_friction_factor_impl(uintptr_t pointer) {
        this->_friction_factor_impls.push_back(from_c_pointer<int(int, double[2])>(pointer));
    }

    void append_friction_factor_impl(std::function<int(int, double[2])> func) {
        this->_friction_factor_impls.push_back(func);
    }
    void append_friction_factor_2_impl(uintptr_t pointer) {
        this->_friction_factor_2_impls.push_back(from_c_pointer<int(int, double[2])>(pointer));
    }

    void append_friction_factor_2_impl(std::function<int(int, double[2])> func) {
        this->_friction_factor_2_impls.push_back(func);
    }

#if defined(_WIN32)

public:
    ~HookCaller() {
        for (auto handle : this->handles) {
            FreeLibrary(handle);
        }
    }
    void load_impls_from_library(const std::string& utf8_filename) {
        std::wstring w_filename = utf8_to_wstring(utf8_filename);
        auto handle = this->Load_dll(w_filename);
        if (handle == NULL) {
            throw std::runtime_error("Error loading library " + utf8_filename + ": " + std::to_string(GetLastError()));
        }
        this->handles.push_back(handle);

        auto p0 = GetProcAddress(handle, "acme_v1_friction_factor");
        if (p0 != nullptr) {
            this->append_friction_factor_impl((uintptr_t)(p0));
        }

        auto p1 = GetProcAddress(handle, "acme_v1_friction_factor_2");
        if (p1 != nullptr) {
            this->append_friction_factor_2_impl((uintptr_t)(p1));
        }

    }


private:
    std::wstring utf8_to_wstring(const std::string& s) {
        int flags = 0;
        int required_size = MultiByteToWideChar(CP_UTF8, flags, s.c_str(), -1, nullptr, 0);
        std::wstring result;
        if (required_size == 0) {
            return result;
        }
        result.resize(required_size);
        int err = MultiByteToWideChar(CP_UTF8, flags, s.c_str(), -1, &result[0], required_size);
        if (err == 0) {
            // error handling: https://docs.microsoft.com/en-us/windows/desktop/api/stringapiset/nf-stringapiset-multibytetowidechar#return-value
            switch (GetLastError()) {
                case ERROR_INSUFFICIENT_BUFFER: throw std::runtime_error("utf8_to_wstring: ERROR_INSUFFICIENT_BUFFER");
                case ERROR_INVALID_FLAGS: throw std::runtime_error("utf8_to_wstring: ERROR_INVALID_FLAGS");
                case ERROR_INVALID_PARAMETER: throw std::runtime_error("utf8_to_wstring: ERROR_INVALID_PARAMETER");
                case ERROR_NO_UNICODE_TRANSLATION: throw std::runtime_error("utf8_to_wstring: ERROR_NO_UNICODE_TRANSLATION");
                default: throw std::runtime_error("Undefined error: " + std::to_string(GetLastError()));
            }
        }
        return result;
    }


    class PathGuard {
    public:
        PathGuard(std::wstring w_filename)
            : path_env{ get_path() }
        {
            std::wstring::size_type dir_name_size = w_filename.find_last_of(L"/\\");
            std::wstring new_path_env = path_env + L";" + w_filename.substr(0, dir_name_size);
            _wputenv_s(L"PATH", new_path_env.data());
        }

        ~PathGuard() {
            _wputenv_s(L"PATH", path_env.data());
        }

    private:
        static std::wstring get_path() {
            rsize_t _len = 0;
            wchar_t *buf;
            _wdupenv_s(&buf, &_len, L"PATH");
            std::wstring path_env{ buf };
            free(buf);
            return path_env;
        } 

        std::wstring path_env;
    };
    HMODULE Load_dll(const std::wstring& w_filename) {
        // Path Modifier
        PathGuard path_guard{ w_filename };
        // Load library (DLL)
        return LoadLibraryW(w_filename.c_str());
    }


private:
    std::vector<HMODULE> handles;

#elif defined(__linux__)

private:
    std::vector<void*> handles;

public:
    ~HookCaller() {
        for (auto handle : this->handles) {
            dlclose(handle);
        }
    }
    void load_impls_from_library(const std::string& utf8_filename) {
        auto handle = dlopen(utf8_filename.c_str(), RTLD_LAZY);
        if (handle == nullptr) {
            throw std::runtime_error("Error loading library " + utf8_filename + ": dlopen failed");
        }
        this->handles.push_back(handle);

        auto p0 = dlsym(handle, "acme_v1_friction_factor");
        if (p0 != nullptr) {
            this->append_friction_factor_impl((uintptr_t)(p0));
        }

        auto p1 = dlsym(handle, "acme_v1_friction_factor_2");
        if (p1 != nullptr) {
            this->append_friction_factor_2_impl((uintptr_t)(p1));
        }

    }

#else
    #error "unknown platform"
#endif

private:
    std::vector<std::function<int(int, double[2])>> _friction_factor_impls;
    std::vector<std::function<int(int, double[2])>> _friction_factor_2_impls;
};

}  // namespace hookman
#endif // _H_HOOKMAN_HOOK_CALLER
