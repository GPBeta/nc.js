
/***************************************************************
 * Name:      constants.cpp
 * Purpose:   Code for Node-CEF Constants Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-24
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

#include "ncjs/module.h"
#include "ncjs/string.h"

#include <uv.h>

#include <errno.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#if HAVE_OPENSSL
#include <openssl/ec.h>
#include <openssl/ssl.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif  // !OPENSSL_NO_ENGINE
#endif

namespace ncjs {

/// ============================================================================
/// implementation
/// ============================================================================

#if HAVE_OPENSSL
const char* default_cipher_list = DEFAULT_CIPHER_LIST_CORE;
#endif

void DefineErrnoConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Context> ctx,
                          CefRefPtr<CefV8Value> target)
{
#ifdef E2BIG
  NCJS_PROPERTY_CONST_AUTO(Double, target, E2BIG);
#endif

#ifdef EACCES
  NCJS_PROPERTY_CONST_AUTO(Double, target, EACCES);
#endif

#ifdef EADDRINUSE
  NCJS_PROPERTY_CONST_AUTO(Double, target, EADDRINUSE);
#endif

#ifdef EADDRNOTAVAIL
  NCJS_PROPERTY_CONST_AUTO(Double, target, EADDRNOTAVAIL);
#endif

#ifdef EAFNOSUPPORT
  NCJS_PROPERTY_CONST_AUTO(Double, target, EAFNOSUPPORT);
#endif

#ifdef EAGAIN
  NCJS_PROPERTY_CONST_AUTO(Double, target, EAGAIN);
#endif

#ifdef EALREADY
  NCJS_PROPERTY_CONST_AUTO(Double, target, EALREADY);
#endif

#ifdef EBADF
  NCJS_PROPERTY_CONST_AUTO(Double, target, EBADF);
#endif

#ifdef EBADMSG
  NCJS_PROPERTY_CONST_AUTO(Double, target, EBADMSG);
#endif

#ifdef EBUSY
  NCJS_PROPERTY_CONST_AUTO(Double, target, EBUSY);
#endif

#ifdef ECANCELED
  NCJS_PROPERTY_CONST_AUTO(Double, target, ECANCELED);
#endif

#ifdef ECHILD
  NCJS_PROPERTY_CONST_AUTO(Double, target, ECHILD);
#endif

#ifdef ECONNABORTED
  NCJS_PROPERTY_CONST_AUTO(Double, target, ECONNABORTED);
#endif

#ifdef ECONNREFUSED
  NCJS_PROPERTY_CONST_AUTO(Double, target, ECONNREFUSED);
#endif

#ifdef ECONNRESET
  NCJS_PROPERTY_CONST_AUTO(Double, target, ECONNRESET);
#endif

#ifdef EDEADLK
  NCJS_PROPERTY_CONST_AUTO(Double, target, EDEADLK);
#endif

#ifdef EDESTADDRREQ
  NCJS_PROPERTY_CONST_AUTO(Double, target, EDESTADDRREQ);
#endif

#ifdef EDOM
  NCJS_PROPERTY_CONST_AUTO(Double, target, EDOM);
#endif

#ifdef EDQUOT
  NCJS_PROPERTY_CONST_AUTO(Double, target, EDQUOT);
#endif

#ifdef EEXIST
  NCJS_PROPERTY_CONST_AUTO(Double, target, EEXIST);
#endif

#ifdef EFAULT
  NCJS_PROPERTY_CONST_AUTO(Double, target, EFAULT);
#endif

#ifdef EFBIG
  NCJS_PROPERTY_CONST_AUTO(Double, target, EFBIG);
#endif

#ifdef EHOSTUNREACH
  NCJS_PROPERTY_CONST_AUTO(Double, target, EHOSTUNREACH);
#endif

#ifdef EIDRM
  NCJS_PROPERTY_CONST_AUTO(Double, target, EIDRM);
#endif

#ifdef EILSEQ
  NCJS_PROPERTY_CONST_AUTO(Double, target, EILSEQ);
#endif

#ifdef EINPROGRESS
  NCJS_PROPERTY_CONST_AUTO(Double, target, EINPROGRESS);
#endif

#ifdef EINTR
  NCJS_PROPERTY_CONST_AUTO(Double, target, EINTR);
#endif

#ifdef EINVAL
  NCJS_PROPERTY_CONST_AUTO(Double, target, EINVAL);
#endif

#ifdef EIO
  NCJS_PROPERTY_CONST_AUTO(Double, target, EIO);
#endif

#ifdef EISCONN
  NCJS_PROPERTY_CONST_AUTO(Double, target, EISCONN);
#endif

#ifdef EISDIR
  NCJS_PROPERTY_CONST_AUTO(Double, target, EISDIR);
#endif

#ifdef ELOOP
  NCJS_PROPERTY_CONST_AUTO(Double, target, ELOOP);
#endif

#ifdef EMFILE
  NCJS_PROPERTY_CONST_AUTO(Double, target, EMFILE);
#endif

#ifdef EMLINK
  NCJS_PROPERTY_CONST_AUTO(Double, target, EMLINK);
#endif

#ifdef EMSGSIZE
  NCJS_PROPERTY_CONST_AUTO(Double, target, EMSGSIZE);
#endif

#ifdef EMULTIHOP
  NCJS_PROPERTY_CONST_AUTO(Double, target, EMULTIHOP);
#endif

#ifdef ENAMETOOLONG
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENAMETOOLONG);
#endif

#ifdef ENETDOWN
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENETDOWN);
#endif

#ifdef ENETRESET
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENETRESET);
#endif

#ifdef ENETUNREACH
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENETUNREACH);
#endif

#ifdef ENFILE
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENFILE);
#endif

#ifdef ENOBUFS
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOBUFS);
#endif

#ifdef ENODATA
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENODATA);
#endif

#ifdef ENODEV
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENODEV);
#endif

#ifdef ENOENT
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOENT);
#endif

#ifdef ENOEXEC
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOEXEC);
#endif

#ifdef ENOLCK
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOLCK);
#endif

#ifdef ENOLINK
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOLINK);
#endif

#ifdef ENOMEM
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOMEM);
#endif

#ifdef ENOMSG
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOMSG);
#endif

#ifdef ENOPROTOOPT
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOPROTOOPT);
#endif

#ifdef ENOSPC
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOSPC);
#endif

#ifdef ENOSR
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOSR);
#endif

#ifdef ENOSTR
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOSTR);
#endif

#ifdef ENOSYS
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOSYS);
#endif

#ifdef ENOTCONN
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOTCONN);
#endif

#ifdef ENOTDIR
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOTDIR);
#endif

#ifdef ENOTEMPTY
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOTEMPTY);
#endif

#ifdef ENOTSOCK
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOTSOCK);
#endif

#ifdef ENOTSUP
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOTSUP);
#endif

#ifdef ENOTTY
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENOTTY);
#endif

#ifdef ENXIO
  NCJS_PROPERTY_CONST_AUTO(Double, target, ENXIO);
#endif

#ifdef EOPNOTSUPP
  NCJS_PROPERTY_CONST_AUTO(Double, target, EOPNOTSUPP);
#endif

#ifdef EOVERFLOW
  NCJS_PROPERTY_CONST_AUTO(Double, target, EOVERFLOW);
#endif

#ifdef EPERM
  NCJS_PROPERTY_CONST_AUTO(Double, target, EPERM);
#endif

#ifdef EPIPE
  NCJS_PROPERTY_CONST_AUTO(Double, target, EPIPE);
#endif

#ifdef EPROTO
  NCJS_PROPERTY_CONST_AUTO(Double, target, EPROTO);
#endif

#ifdef EPROTONOSUPPORT
  NCJS_PROPERTY_CONST_AUTO(Double, target, EPROTONOSUPPORT);
#endif

#ifdef EPROTOTYPE
  NCJS_PROPERTY_CONST_AUTO(Double, target, EPROTOTYPE);
#endif

#ifdef ERANGE
  NCJS_PROPERTY_CONST_AUTO(Double, target, ERANGE);
#endif

#ifdef EROFS
  NCJS_PROPERTY_CONST_AUTO(Double, target, EROFS);
#endif

#ifdef ESPIPE
  NCJS_PROPERTY_CONST_AUTO(Double, target, ESPIPE);
#endif

#ifdef ESRCH
  NCJS_PROPERTY_CONST_AUTO(Double, target, ESRCH);
#endif

#ifdef ESTALE
  NCJS_PROPERTY_CONST_AUTO(Double, target, ESTALE);
#endif

#ifdef ETIME
  NCJS_PROPERTY_CONST_AUTO(Double, target, ETIME);
#endif

#ifdef ETIMEDOUT
  NCJS_PROPERTY_CONST_AUTO(Double, target, ETIMEDOUT);
#endif

#ifdef ETXTBSY
  NCJS_PROPERTY_CONST_AUTO(Double, target, ETXTBSY);
#endif

#ifdef EWOULDBLOCK
  NCJS_PROPERTY_CONST_AUTO(Double, target, EWOULDBLOCK);
#endif

#ifdef EXDEV
  NCJS_PROPERTY_CONST_AUTO(Double, target, EXDEV);
#endif
}

void DefineWindowsErrorConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Context> ctx,
                                 CefRefPtr<CefV8Value> target)
{
#ifdef WSAEINTR
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEINTR);
#endif

#ifdef WSAEBADF
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEBADF);
#endif

#ifdef WSAEACCES
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEACCES);
#endif

#ifdef WSAEFAULT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEFAULT);
#endif

#ifdef WSAEINVAL
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEINVAL);
#endif

#ifdef WSAEMFILE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEMFILE);
#endif

#ifdef WSAEWOULDBLOCK
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEWOULDBLOCK);
#endif

#ifdef WSAEINPROGRESS
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEINPROGRESS);
#endif

#ifdef WSAEALREADY
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEALREADY);
#endif

#ifdef WSAENOTSOCK
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENOTSOCK);
#endif

#ifdef WSAEDESTADDRREQ
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEDESTADDRREQ);
#endif

#ifdef WSAEMSGSIZE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEMSGSIZE);
#endif

#ifdef WSAEPROTOTYPE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEPROTOTYPE);
#endif

#ifdef WSAENOPROTOOPT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENOPROTOOPT);
#endif

#ifdef WSAEPROTONOSUPPORT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEPROTONOSUPPORT);
#endif

#ifdef WSAESOCKTNOSUPPORT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAESOCKTNOSUPPORT);
#endif

#ifdef WSAEOPNOTSUPP
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEOPNOTSUPP);
#endif

#ifdef WSAEPFNOSUPPORT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEPFNOSUPPORT);
#endif

#ifdef WSAEAFNOSUPPORT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEAFNOSUPPORT);
#endif

#ifdef WSAEADDRINUSE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEADDRINUSE);
#endif

#ifdef WSAEADDRNOTAVAIL
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEADDRNOTAVAIL);
#endif

#ifdef WSAENETDOWN
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENETDOWN);
#endif

#ifdef WSAENETUNREACH
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENETUNREACH);
#endif

#ifdef WSAENETRESET
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENETRESET);
#endif

#ifdef WSAECONNABORTED
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAECONNABORTED);
#endif

#ifdef WSAECONNRESET
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAECONNRESET);
#endif

#ifdef WSAENOBUFS
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENOBUFS);
#endif

#ifdef WSAEISCONN
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEISCONN);
#endif

#ifdef WSAENOTCONN
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENOTCONN);
#endif

#ifdef WSAESHUTDOWN
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAESHUTDOWN);
#endif

#ifdef WSAETOOMANYREFS
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAETOOMANYREFS);
#endif

#ifdef WSAETIMEDOUT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAETIMEDOUT);
#endif

#ifdef WSAECONNREFUSED
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAECONNREFUSED);
#endif

#ifdef WSAELOOP
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAELOOP);
#endif

#ifdef WSAENAMETOOLONG
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENAMETOOLONG);
#endif

#ifdef WSAEHOSTDOWN
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEHOSTDOWN);
#endif

#ifdef WSAEHOSTUNREACH
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEHOSTUNREACH);
#endif

#ifdef WSAENOTEMPTY
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENOTEMPTY);
#endif

#ifdef WSAEPROCLIM
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEPROCLIM);
#endif

#ifdef WSAEUSERS
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEUSERS);
#endif

#ifdef WSAEDQUOT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEDQUOT);
#endif

#ifdef WSAESTALE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAESTALE);
#endif

#ifdef WSAEREMOTE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEREMOTE);
#endif

#ifdef WSASYSNOTREADY
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSASYSNOTREADY);
#endif

#ifdef WSAVERNOTSUPPORTED
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAVERNOTSUPPORTED);
#endif

#ifdef WSANOTINITIALISED
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSANOTINITIALISED);
#endif

#ifdef WSAEDISCON
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEDISCON);
#endif

#ifdef WSAENOMORE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAENOMORE);
#endif

#ifdef WSAECANCELLED
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAECANCELLED);
#endif

#ifdef WSAEINVALIDPROCTABLE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEINVALIDPROCTABLE);
#endif

#ifdef WSAEINVALIDPROVIDER
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEINVALIDPROVIDER);
#endif

#ifdef WSAEPROVIDERFAILEDINIT
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEPROVIDERFAILEDINIT);
#endif

#ifdef WSASYSCALLFAILURE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSASYSCALLFAILURE);
#endif

#ifdef WSASERVICE_NOT_FOUND
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSASERVICE_NOT_FOUND);
#endif

#ifdef WSATYPE_NOT_FOUND
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSATYPE_NOT_FOUND);
#endif

#ifdef WSA_E_NO_MORE
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSA_E_NO_MORE);
#endif

#ifdef WSA_E_CANCELLED
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSA_E_CANCELLED);
#endif

#ifdef WSAEREFUSED
  NCJS_PROPERTY_CONST_AUTO(Double, target, WSAEREFUSED);
#endif
}

void DefineSignalConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Context> ctx,
                           CefRefPtr<CefV8Value> target)
{
#ifdef SIGHUP
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGHUP);
#endif

#ifdef SIGINT
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGINT);
#endif

#ifdef SIGQUIT
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGQUIT);
#endif

#ifdef SIGILL
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGILL);
#endif

#ifdef SIGTRAP
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGTRAP);
#endif

#ifdef SIGABRT
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGABRT);
#endif

#ifdef SIGIOT
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGIOT);
#endif

#ifdef SIGBUS
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGBUS);
#endif

#ifdef SIGFPE
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGFPE);
#endif

#ifdef SIGKILL
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGKILL);
#endif

#ifdef SIGUSR1
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGUSR1);
#endif

#ifdef SIGSEGV
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGSEGV);
#endif

#ifdef SIGUSR2
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGUSR2);
#endif

#ifdef SIGPIPE
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGPIPE);
#endif

#ifdef SIGALRM
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGALRM);
#endif

  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGTERM);

#ifdef SIGCHLD
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGCHLD);
#endif

#ifdef SIGSTKFLT
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGSTKFLT);
#endif


#ifdef SIGCONT
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGCONT);
#endif

#ifdef SIGSTOP
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGSTOP);
#endif

#ifdef SIGTSTP
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGTSTP);
#endif

#ifdef SIGBREAK
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGBREAK);
#endif

#ifdef SIGTTIN
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGTTIN);
#endif

#ifdef SIGTTOU
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGTTOU);
#endif

#ifdef SIGURG
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGURG);
#endif

#ifdef SIGXCPU
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGXCPU);
#endif

#ifdef SIGXFSZ
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGXFSZ);
#endif

#ifdef SIGVTALRM
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGVTALRM);
#endif

#ifdef SIGPROF
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGPROF);
#endif

#ifdef SIGWINCH
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGWINCH);
#endif

#ifdef SIGIO
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGIO);
#endif

#ifdef SIGPOLL
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGPOLL);
#endif

#ifdef SIGLOST
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGLOST);
#endif

#ifdef SIGPWR
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGPWR);
#endif

#ifdef SIGSYS
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGSYS);
#endif

#ifdef SIGUNUSED
  NCJS_PROPERTY_CONST_AUTO(Double, target, SIGUNUSED);
#endif
}

void DefineOpenSSLConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Context> ctx,
                            CefRefPtr<CefV8Value> target)
{
#ifdef SSL_OP_ALL
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_ALL);
#endif

#ifdef SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION);
#endif

#ifdef SSL_OP_CIPHER_SERVER_PREFERENCE
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_CIPHER_SERVER_PREFERENCE);
#endif

#ifdef SSL_OP_CISCO_ANYCONNECT
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_CISCO_ANYCONNECT);
#endif

#ifdef SSL_OP_COOKIE_EXCHANGE
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_COOKIE_EXCHANGE);
#endif

#ifdef SSL_OP_CRYPTOPRO_TLSEXT_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_CRYPTOPRO_TLSEXT_BUG);
#endif

#ifdef SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);
#endif

#ifdef SSL_OP_EPHEMERAL_RSA
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_EPHEMERAL_RSA);
#endif

#ifdef SSL_OP_LEGACY_SERVER_CONNECT
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_LEGACY_SERVER_CONNECT);
#endif

#ifdef SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER);
#endif

#ifdef SSL_OP_MICROSOFT_SESS_ID_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_MICROSOFT_SESS_ID_BUG);
#endif

#ifdef SSL_OP_MSIE_SSLV2_RSA_PADDING
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_MSIE_SSLV2_RSA_PADDING);
#endif

#ifdef SSL_OP_NETSCAPE_CA_DN_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NETSCAPE_CA_DN_BUG);
#endif

#ifdef SSL_OP_NETSCAPE_CHALLENGE_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NETSCAPE_CHALLENGE_BUG);
#endif

#ifdef SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG);
#endif

#ifdef SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG);
#endif

#ifdef SSL_OP_NO_COMPRESSION
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_COMPRESSION);
#endif

#ifdef SSL_OP_NO_QUERY_MTU
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_QUERY_MTU);
#endif

#ifdef SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
#endif

#ifdef SSL_OP_NO_SSLv2
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_SSLv2);
#endif

#ifdef SSL_OP_NO_SSLv3
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_SSLv3);
#endif

#ifdef SSL_OP_NO_TICKET
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_TICKET);
#endif

#ifdef SSL_OP_NO_TLSv1
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_TLSv1);
#endif

#ifdef SSL_OP_NO_TLSv1_1
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_TLSv1_1);
#endif

#ifdef SSL_OP_NO_TLSv1_2
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_NO_TLSv1_2);
#endif

#ifdef SSL_OP_PKCS1_CHECK_1
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_PKCS1_CHECK_1);
#endif

#ifdef SSL_OP_PKCS1_CHECK_2
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_PKCS1_CHECK_2);
#endif

#ifdef SSL_OP_SINGLE_DH_USE
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_SINGLE_DH_USE);
#endif

#ifdef SSL_OP_SINGLE_ECDH_USE
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_SINGLE_ECDH_USE);
#endif

#ifdef SSL_OP_SSLEAY_080_CLIENT_DH_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_SSLEAY_080_CLIENT_DH_BUG);
#endif

#ifdef SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG);
#endif

#ifdef SSL_OP_TLS_BLOCK_PADDING_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_TLS_BLOCK_PADDING_BUG);
#endif

#ifdef SSL_OP_TLS_D5_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_TLS_D5_BUG);
#endif

#ifdef SSL_OP_TLS_ROLLBACK_BUG
    NCJS_PROPERTY_CONST_AUTO(Double, target, SSL_OP_TLS_ROLLBACK_BUG);
#endif

# ifndef OPENSSL_NO_ENGINE

# ifdef ENGINE_METHOD_DSA
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_DSA);
# endif

# ifdef ENGINE_METHOD_DH
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_DH);
# endif

# ifdef ENGINE_METHOD_RAND
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_RAND);
# endif

# ifdef ENGINE_METHOD_ECDH
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_ECDH);
# endif

# ifdef ENGINE_METHOD_ECDSA
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_ECDSA);
# endif

# ifdef ENGINE_METHOD_CIPHERS
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_CIPHERS);
# endif

# ifdef ENGINE_METHOD_DIGESTS
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_DIGESTS);
# endif

# ifdef ENGINE_METHOD_STORE
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_STORE);
# endif

# ifdef ENGINE_METHOD_PKEY_METHS
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_PKEY_METHS);
# endif

# ifdef ENGINE_METHOD_PKEY_ASN1_METHS
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_PKEY_ASN1_METHS);
# endif

# ifdef ENGINE_METHOD_ALL
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_ALL);
# endif

# ifdef ENGINE_METHOD_NONE
    NCJS_PROPERTY_CONST_AUTO(Double, target, ENGINE_METHOD_NONE);
# endif

# endif  // !OPENSSL_NO_ENGINE

#ifdef DH_CHECK_P_NOT_SAFE_PRIME
    NCJS_PROPERTY_CONST_AUTO(Double, target, DH_CHECK_P_NOT_SAFE_PRIME);
#endif

#ifdef DH_CHECK_P_NOT_PRIME
    NCJS_PROPERTY_CONST_AUTO(Double, target, DH_CHECK_P_NOT_PRIME);
#endif

#ifdef DH_UNABLE_TO_CHECK_GENERATOR
    NCJS_PROPERTY_CONST_AUTO(Double, target, DH_UNABLE_TO_CHECK_GENERATOR);
#endif

#ifdef DH_NOT_SUITABLE_GENERATOR
    NCJS_PROPERTY_CONST_AUTO(Double, target, DH_NOT_SUITABLE_GENERATOR);
#endif

#ifdef OPENSSL_NPN_NEGOTIATED
#define NPN_ENABLED 1
    NCJS_PROPERTY_CONST_AUTO(Double, target, NPN_ENABLED);
#endif

#ifdef RSA_PKCS1_PADDING
    NCJS_PROPERTY_CONST_AUTO(Double, target, RSA_PKCS1_PADDING);
#endif

#ifdef RSA_SSLV23_PADDING
    NCJS_PROPERTY_CONST_AUTO(Double, target, RSA_SSLV23_PADDING);
#endif

#ifdef RSA_NO_PADDING
    NCJS_PROPERTY_CONST_AUTO(Double, target, RSA_NO_PADDING);
#endif

#ifdef RSA_PKCS1_OAEP_PADDING
    NCJS_PROPERTY_CONST_AUTO(Double, target, RSA_PKCS1_OAEP_PADDING);
#endif

#ifdef RSA_X931_PADDING
    NCJS_PROPERTY_CONST_AUTO(Double, target, RSA_X931_PADDING);
#endif

#ifdef RSA_PKCS1_PSS_PADDING
    NCJS_PROPERTY_CONST_AUTO(Double, target, RSA_PKCS1_PSS_PADDING);
#endif

#if HAVE_OPENSSL
  // NOTE: These are not defines
  NCJS_PROPERTY_CONST_AUTO(Double, target, POINT_CONVERSION_COMPRESSED);

  NCJS_PROPERTY_CONST_AUTO(Double, target, POINT_CONVERSION_UNCOMPRESSED);

  NCJS_PROPERTY_CONST_AUTO(Double, target, POINT_CONVERSION_HYBRID);
#endif
}

void DefineSystemConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Context> ctx,
                           CefRefPtr<CefV8Value> target)
{
  // file access modes
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_RDONLY);
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_WRONLY);
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_RDWR);

  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFMT);
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFREG);
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFDIR);
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFCHR);
#ifdef S_IFBLK
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFBLK);
#endif

#ifdef S_IFIFO
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFIFO);
#endif

#ifdef S_IFLNK
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFLNK);
#endif

#ifdef S_IFSOCK
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IFSOCK);
#endif

#ifdef O_CREAT
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_CREAT);
#endif

#ifdef O_EXCL
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_EXCL);
#endif

#ifdef O_NOCTTY
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_NOCTTY);
#endif

#ifdef O_TRUNC
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_TRUNC);
#endif

#ifdef O_APPEND
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_APPEND);
#endif

#ifdef O_DIRECTORY
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_DIRECTORY);
#endif

#ifdef O_EXCL
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_EXCL);
#endif

#ifdef O_NOFOLLOW
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_NOFOLLOW);
#endif

#ifdef O_SYNC
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_SYNC);
#endif

#ifdef O_SYMLINK
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_SYMLINK);
#endif

#ifdef O_DIRECT
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_DIRECT);
#endif

#ifdef O_NONBLOCK
  NCJS_PROPERTY_CONST_AUTO(Double, target, O_NONBLOCK);
#endif

#ifdef S_IRWXU
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IRWXU);
#endif

#ifdef S_IRUSR
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IRUSR);
#endif

#ifdef S_IWUSR
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IWUSR);
#endif

#ifdef S_IXUSR
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IXUSR);
#endif

#ifdef S_IRWXG
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IRWXG);
#endif

#ifdef S_IRGRP
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IRGRP);
#endif

#ifdef S_IWGRP
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IWGRP);
#endif

#ifdef S_IXGRP
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IXGRP);
#endif

#ifdef S_IRWXO
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IRWXO);
#endif

#ifdef S_IROTH
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IROTH);
#endif

#ifdef S_IWOTH
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IWOTH);
#endif

#ifdef S_IXOTH
  NCJS_PROPERTY_CONST_AUTO(Double, target, S_IXOTH);
#endif

#ifdef F_OK
  NCJS_PROPERTY_CONST_AUTO(Double, target, F_OK);
#endif

#ifdef R_OK
  NCJS_PROPERTY_CONST_AUTO(Double, target, R_OK);
#endif

#ifdef W_OK
  NCJS_PROPERTY_CONST_AUTO(Double, target, W_OK);
#endif

#ifdef X_OK
  NCJS_PROPERTY_CONST_AUTO(Double, target, X_OK);
#endif
}

void DefineUVConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Context> ctx,
                       CefRefPtr<CefV8Value> target)
{
  NCJS_PROPERTY_CONST_AUTO(Double, target, UV_UDP_REUSEADDR);
}

void DefineCryptoConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Context> ctx,
                           CefRefPtr<CefV8Value> target)
{
#if HAVE_OPENSSL
  NCJS_PROPERTY_CONST_AUTO(String, target,
                              NCJS_REFTEXT("defaultCoreCipherList"),
                              DEFAULT_CIPHER_LIST_CORE);
  NCJS_PROPERTY_CONST_AUTO(String, target,
                              NCJS_REFTEXT("defaultCipherList"),
                              default_cipher_list);
#endif
}

/// ----------------------------------------------------------------------------
/// ModuleConstants
/// ----------------------------------------------------------------------------

class ModuleConstants : public JsObjecT<ModuleConstants> {

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_EXTRA(DefineErrnoConstants)
        NCJS_MAP_OBJECT_EXTRA(DefineWindowsErrorConstants)
        NCJS_MAP_OBJECT_EXTRA(DefineSignalConstants)
        NCJS_MAP_OBJECT_EXTRA(DefineOpenSSLConstants)
        NCJS_MAP_OBJECT_EXTRA(DefineSystemConstants)
        NCJS_MAP_OBJECT_EXTRA(DefineUVConstants)
        NCJS_MAP_OBJECT_EXTRA(DefineCryptoConstants)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(constants, ModuleConstants);

} // ncjs