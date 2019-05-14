/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_MINIMAL_WINDOWSH_H
#define RAMSES_CAPU_MINIMAL_WINDOWSH_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOGDICAPMASKS
#define NOGDICAPMASKS
#endif
#ifndef NOVIRTUALKEYCODES
#define NOVIRTUALKEYCODES
#endif
#ifndef NOWINMESSAGES
#define NOWINMESSAGES
#endif
#ifndef NOWINSTYLES
#define NOWINSTYLES
#endif
#ifndef NOSYSMETRICS
#define NOSYSMETRICS
#endif
#ifndef NOMENUS
#define NOMENUS
#endif
#ifndef NOICONS
#define NOICONS
#endif
#ifndef NOKEYSTATES
#define NOKEYSTATES
#endif
#ifndef NOSYSCOMMANDS
#define NOSYSCOMMANDS
#endif
#ifndef NORASTEROPS
#define NORASTEROPS
#endif
#ifndef NOSHOWWINDOW
#define NOSHOWWINDOW
#endif
#ifndef OEMRESOURCE
#define OEMRESOURCE
#endif
#ifndef NOATOM
#define NOATOM
#endif
#ifndef NOCLIPBOARD
#define NOCLIPBOARD
#endif
#ifndef NOCOLOR
#define NOCOLOR
#endif
#ifndef NOCTLMGR
#define NOCTLMGR
#endif
#ifndef NODRAWTEXT
#define NODRAWTEXT
#endif
#ifndef NOKERNEL
#define NOKERNEL
#endif
#ifndef NOMB
#define NOMB
#endif
#ifndef NOMEMMGR
#define NOMEMMGR
#endif
#ifndef NOMETAFILE
#define NOMETAFILE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOOPENFILE
#define NOOPENFILE
#endif
#ifndef NOSCROLL
#define NOSCROLL
#endif
#ifndef NOSERVICE
#define NOSERVICE
#endif
#ifndef NOSOUND
#define NOSOUND
#endif
#ifndef NOTEXTMETRIC
#define NOTEXTMETRIC
#endif
#ifndef NOWH
#define NOWH
#endif
#ifndef NOWINOFFSETS
#define NOWINOFFSETS
#endif
#ifndef NOCOMM
#define NOCOMM
#endif
#ifndef NOKANJI
#define NOKANJI
#endif
#ifndef NOHELP
#define NOHELP
#endif
#ifndef NOPROFILER
#define NOPROFILER
#endif
#ifndef NODEFERWINDOWPOS
#define NODEFERWINDOWPOS
#endif
#ifndef NOMCX
#define NOMCX
#endif

#ifndef _WINDOWS_
#include <winsock2.h>
#include <windows.h>
#endif
#undef min
#undef max

#endif
