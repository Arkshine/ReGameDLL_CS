/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/

#ifndef _MEMORY_H
#define _MEMORY_H

#include "osconfig.h"


#define MAX_PATTERN 128


struct Section;
struct Section
{
	Section			*next;
	size_t			start;
	size_t			size;
	size_t			end;
	int				protection;
	int				inode;
	char			filename[_MAX_FNAME];
	int				namelen;
};

struct Module
{
	size_t			base;
	size_t			size;
	size_t			end;
	size_t			handle;

#ifdef WIN32
	Section codeSection;
#endif
};

struct Address
{
	// Keeps offset for SWDS on application start; during HookEngine() an real address is written here.
	size_t			originalAddress;
	const char		*symbolName;
	size_t			address;
	int				symbolIndex;
};

struct FunctionHook
{
	// Keeps offset for SWDS on application start; during HookEngine() an real address is written here.
	size_t			originalAddress;
	const char		*symbolName;
	size_t			handlerFunc;
	int				symbolIndex;
	bool			bIsHooked;
};

struct VirtualTableRef
{
	size_t			originalAddress;
	const char		*symbolName;
	size_t			size;
};

struct AddressRef
{
	// Keeps offset for SWDS on application start; during HookEngine() an real address is written here.
	size_t			originalAddress;
	const char		*symbolName;
	size_t			addressRef;
	int				symbolIndex;
	bool			reverse;
};

class CFuncAddr {
public:
	uint32_t Addr;
	CFuncAddr *Next;

	void *operator new(size_t size){
		return malloc(size * sizeof(CFuncAddr));
	}

	void operator delete(void *cPoint) {
		free(cPoint);
	}

	CFuncAddr(uint32_t addr) {
		Addr = addr;
		Next = NULL;
	}

	~CFuncAddr() {
		if (Next) {
			delete Next;
			Next = NULL;
		}
	}
};

bool FindModuleByAddress(size_t addr, Module *module);
bool FindModuleByName(const char *moduleName, Module *module);

void ProcessModuleData(Module *module);

size_t ConvertHexString(const char *srcHexString, unsigned char *outBuffer, size_t bufferSize);
size_t MemoryFindForward(size_t start, size_t end, const unsigned char *pattern, const unsigned char *mask, size_t len);
size_t MemoryFindBackward(size_t start, size_t end, const unsigned char *pattern, const unsigned char *mask, size_t len);
size_t MemoryFindRefForwardPrefix8(size_t start, size_t end, size_t refAddress, uint8_t prefixValue, bool relative);

uint32_t HookDWord(size_t addr, uint32_t newDWord);
void ExchangeMemoryBytes(size_t origAddr, size_t dataAddr, uint32_t size);

bool GetAddress(Module *module, Address *addr, size_t baseOffset);
bool HookFunction(Module *module, FunctionHook *hook);
void HookFunctionCall(void* hookWhat, void* hookAddr);
bool HIDDEN FindDataRef(Module *module, AddressRef *ref);

#ifdef WIN32
void FindAllCalls(Section* section, CFuncAddr** calls, uint32_t findRefsTo);
#endif

#if defined(_WIN32) && !defined(REGAMEDLL_UNIT_TESTS)

const char *stripClass(const char *str);
void VirtualTableInit(void *ptr, const char *baseClass = NULL);
void HIDDEN GetAddressVtableByClassname(const char *szClassName, const int iOffset = 0, bool bCreate = true);

#endif // _WIN32 && REGAMEDLL_UNIT_TESTS

#endif // _MEMORY_H
