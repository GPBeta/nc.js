
/***************************************************************
 * Name:      os.cpp
 * Purpose:   Code for Node-CEF OS Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-14
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

#define _WINSOCKAPI_    // stops windows.h including winsock.h

/// ----------------------------------------------------------------------------
/// headers
/// ----------------------------------------------------------------------------

#include "ncjs/constants.h"
#include "ncjs/module.h"

#include <uv.h>
#include <sstream>

// Add Windows fallback.
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 256
#endif  // MAXHOSTNAMELEN

namespace ncjs {

/// ============================================================================
/// implementation
/// ============================================================================

/// ----------------------------------------------------------------------------
/// ModuleOS
/// ----------------------------------------------------------------------------

class ModuleOS : public JsObjecT<ModuleOS> {

    // os.hostname()
    NCJS_OBJECT_FUNCTION(GetHostname)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        char buf[MAXHOSTNAMELEN + 1];

        if (gethostname(buf, sizeof(buf))) {
#ifdef __POSIX__
            int errorno = errno;
#else  // __MINGW32__
            int errorno = WSAGetLastError();
#endif  // __POSIX__
            return Environment::ErrorException(errorno, "gethostname", NULL, except);
        }
        buf[sizeof(buf) - 1] = '\0';

        retval = CefV8Value::CreateString(buf);
    }

    // os.type()
    NCJS_OBJECT_FUNCTION(GetOSType)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const char* rval;

#ifdef __POSIX__
        struct utsname info;
        if (uname(&info) < 0) {
            return SysCallException(errno, NCJS_REFTEXT("uname"), except);
        }
        rval = info.sysname;
#else  // __MINGW32__
        rval = "Windows_NT";
#endif  // __POSIX__

        retval = CefV8Value::CreateString(rval);
    }

    // os.release()
    NCJS_OBJECT_FUNCTION(GetOSRelease)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const char* rval;

#ifdef __POSIX__
        struct utsname info;
        if (uname(&info) < 0) {
            return SysCallException(errno, NCJS_REFTEXT("uname"), except);
        }
        rval = info.release;
#else  // Windows
        char release[256];
        OSVERSIONINFOW info;

        info.dwOSVersionInfoSize = sizeof(info);

        // Don't complain that GetVersionEx is deprecated; there is no alternative.
#pragma warning(suppress : 4996)
        if (GetVersionExW(&info) == 0)
            return;

        sprintf(release,
            "%d.%d.%d",
            static_cast<int>(info.dwMajorVersion),
            static_cast<int>(info.dwMinorVersion),
            static_cast<int>(info.dwBuildNumber));
        rval = release;
#endif  // __POSIX__

        retval = CefV8Value::CreateString(rval);
    }

    // os.cpus()
    NCJS_OBJECT_FUNCTION(GetCPUs)(CefRefPtr<CefV8Value> object, 
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        uv_cpu_info_t* cpu_infos;
        int count, i;
    
        int err = uv_cpu_info(&cpu_infos, &count);
        if (err)
            return;

		CefRefPtr<CefV8Value> cpus = CefV8Value::CreateArray(count);
        for (i = 0; i < count; i++) {
            using namespace consts;

            uv_cpu_info_t* ci = cpu_infos + i;
    
            CefRefPtr<CefV8Value> times_info = CefV8Value::CreateObject(NULL);
            times_info->SetValue(str_user, CefV8Value::CreateDouble(double(ci->cpu_times.user)), V8_PROPERTY_ATTRIBUTE_NONE);
            times_info->SetValue(str_nice, CefV8Value::CreateDouble(double(ci->cpu_times.nice)), V8_PROPERTY_ATTRIBUTE_NONE);
            times_info->SetValue(str_sys,  CefV8Value::CreateDouble(double(ci->cpu_times.sys)),  V8_PROPERTY_ATTRIBUTE_NONE);
            times_info->SetValue(str_idle, CefV8Value::CreateDouble(double(ci->cpu_times.idle)), V8_PROPERTY_ATTRIBUTE_NONE);
            times_info->SetValue(str_irq,  CefV8Value::CreateDouble(double(ci->cpu_times.irq)),  V8_PROPERTY_ATTRIBUTE_NONE);
    
            CefRefPtr<CefV8Value> cpu_info = CefV8Value::CreateObject(NULL);
			cpu_info->SetValue(str_model, CefV8Value::CreateString(ci->model),         V8_PROPERTY_ATTRIBUTE_NONE);
            cpu_info->SetValue(str_speed, CefV8Value::CreateDouble(double(ci->speed)), V8_PROPERTY_ATTRIBUTE_NONE);
            cpu_info->SetValue(str_times, times_info,                                  V8_PROPERTY_ATTRIBUTE_NONE);
    
            cpus->SetValue(i, cpu_info);
        }
    
        uv_free_cpu_info(cpu_infos, count);
        retval = cpus;
    }

    // os.freemem()
    NCJS_OBJECT_FUNCTION(GetFreeMemory)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        unsigned long long rval = uv_get_free_memory();

        if (rval == unsigned long long(-1))
            return;

        retval = CefV8Value::CreateDouble(double(rval));
    }

    // os.freemem()
    NCJS_OBJECT_FUNCTION(GetTotalMemory)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        unsigned long long rval = uv_get_total_memory();

        if (rval == unsigned long long(-1))
            return;

        retval = CefV8Value::CreateDouble(double(rval));
    }

    // os.uptime()
    NCJS_OBJECT_FUNCTION(GetUptime)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
		double uptime;
		int err = uv_uptime(&uptime);
		if (err == 0)
			retval = CefV8Value::CreateDouble(uptime);
    }

    // os.loadavg()
    NCJS_OBJECT_FUNCTION(GetLoadAvg)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
		double loadavg[3];
		uv_loadavg(loadavg);
        CefRefPtr<CefV8Value> loads = CefV8Value::CreateArray(3);
        loads->SetValue(0, CefV8Value::CreateDouble(loadavg[0]));
        loads->SetValue(1, CefV8Value::CreateDouble(loadavg[1]));
        loads->SetValue(2, CefV8Value::CreateDouble(loadavg[2]));

		retval = loads;
    }

	NCJS_OBJECT_FUNCTION(GetInterfaceAddresses)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
		uv_interface_address_t* interfaces;
		int count, i;
		char ip[INET6_ADDRSTRLEN];
		char netmask[INET6_ADDRSTRLEN];
		char mac[18];
		CefString name;
		CefRefPtr<CefV8Value> o, family, ifarr;
		CefRefPtr<CefV8Value> ret = CefV8Value::CreateObject(NULL);
		int err = uv_interface_addresses(&interfaces, &count);

		if (err == UV_ENOSYS) {
			retval = ret;
			return;
		} else if (err) {
            return Environment::UvException(err, "uv_interface_addresses", except);
		}

        CefRefPtr<CefV8Value> STR_IPV4 = CefV8Value::CreateString(consts::str_IPv4);
		CefRefPtr<CefV8Value> STR_IPV6 = CefV8Value::CreateString(consts::str_IPv6);
		CefRefPtr<CefV8Value> STR_UNKN = CefV8Value::CreateString(consts::str_unknown);

        for (i = 0; i < count; i++) {
            const char* const raw_name = interfaces[i].name;

            // On Windows, the interface name is the UTF8-encoded friendly name and may
            // contain non-ASCII characters.  On UNIX, it's just a binary string with
            // no particular encoding but we treat it as a one-byte Latin-1 string.
            name = raw_name;

            if (ret->HasValue(name)) {
                ifarr = ret->GetValue(name);
            } else {
				ifarr = CefV8Value::CreateArray(0);
				ret->SetValue(name, ifarr, V8_PROPERTY_ATTRIBUTE_NONE);
            }

#pragma warning(suppress : 4996)
			sprintf(mac,
			"%02x:%02x:%02x:%02x:%02x:%02x",
			static_cast<unsigned char>(interfaces[i].phys_addr[0]),
			static_cast<unsigned char>(interfaces[i].phys_addr[1]),
			static_cast<unsigned char>(interfaces[i].phys_addr[2]),
			static_cast<unsigned char>(interfaces[i].phys_addr[3]),
			static_cast<unsigned char>(interfaces[i].phys_addr[4]),
			static_cast<unsigned char>(interfaces[i].phys_addr[5]));

			if (interfaces[i].address.address4.sin_family == AF_INET) {
				uv_ip4_name(&interfaces[i].address.address4, ip, sizeof(ip));
				uv_ip4_name(&interfaces[i].netmask.netmask4, netmask, sizeof(netmask));
				family = STR_IPV4;
			} else if (interfaces[i].address.address4.sin_family == AF_INET6) {
				uv_ip6_name(&interfaces[i].address.address6, ip, sizeof(ip));
				uv_ip6_name(&interfaces[i].netmask.netmask6, netmask, sizeof(netmask));
				family = STR_IPV6;
			} else {
				strncpy(ip, "<unknown sa family>", INET6_ADDRSTRLEN);
				family = STR_UNKN;
			}

			o = CefV8Value::CreateObject(NULL);
			o->SetValue(consts::str_address, CefV8Value::CreateString(ip), V8_PROPERTY_ATTRIBUTE_NONE);
			o->SetValue(consts::str_netmask, CefV8Value::CreateString(netmask), V8_PROPERTY_ATTRIBUTE_NONE);
			o->SetValue(consts::str_family, family, V8_PROPERTY_ATTRIBUTE_NONE);
			o->SetValue(consts::str_mac, CefV8Value::CreateString(mac), V8_PROPERTY_ATTRIBUTE_NONE);

			if (interfaces[i].address.address4.sin_family == AF_INET6) {
				uint32_t scopeid = interfaces[i].address.address6.sin6_scope_id;
				o->SetValue(consts::str_scopeid, CefV8Value::CreateUInt(scopeid), V8_PROPERTY_ATTRIBUTE_NONE);
			}

			const bool internal = interfaces[i].is_internal ? true : false;
			o->SetValue(consts::str_internal, CefV8Value::CreateBool(internal), V8_PROPERTY_ATTRIBUTE_NONE);

			ifarr->SetValue(ifarr->GetArrayLength(), o);
        }

		uv_free_interface_addresses(interfaces, count);
		retval = ret;
	}

	// os.homedir()
    NCJS_OBJECT_FUNCTION(GetHomeDirectory)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
		char buf[MAX_PATH];

		size_t len = sizeof(buf);
		const int err = uv_os_homedir(buf, &len);

		if (err)
			return Environment::UvException(err, "uv_os_homedir", except);

		retval = CefV8Value::CreateString(buf);
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_FUNCTION("getHostname",           GetHostname)
        NCJS_MAP_OBJECT_FUNCTION("getOSType",             GetOSType)
        NCJS_MAP_OBJECT_FUNCTION("getOSRelease",          GetOSRelease)
        NCJS_MAP_OBJECT_FUNCTION("getCPUs",               GetCPUs)
        NCJS_MAP_OBJECT_FUNCTION("getFreeMemory",         GetFreeMemory)
        NCJS_MAP_OBJECT_FUNCTION("getTotalMemory",        GetTotalMemory)
        NCJS_MAP_OBJECT_FUNCTION("getUptime",             GetUptime)
        NCJS_MAP_OBJECT_FUNCTION("getLoadAvg",            GetLoadAvg)
        NCJS_MAP_OBJECT_FUNCTION("getHomeDirectory",      GetHomeDirectory)
		NCJS_MAP_OBJECT_FUNCTION("getInterfaceAddresses", GetInterfaceAddresses)
        NCJS_MAP_OBJECT(Bool,    "isBigEndian",           Environment::IsBE())
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(os, ModuleOS);

} // ncjs