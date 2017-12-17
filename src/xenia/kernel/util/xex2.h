/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_UTIL_XEX2_H_
#define XENIA_KERNEL_UTIL_XEX2_H_

#include "xenia/kernel/util/xex2_info.h"
#include "xenia/memory.h"

namespace xe {}  // namespace xe

typedef struct {
  int reserved;
} xe_xex2_options_t;

struct xe_xex2;
typedef struct xe_xex2* xe_xex2_ref;

typedef struct {
  uint32_t ordinal;
  uint32_t value_address;  // address to place value
  uint32_t thunk_address;  // NULL or address of thunk
} xe_xex2_import_info_t;

enum xe_pe_section_flags_e : uint32_t {
  kXEPESectionContainsCode = 0x00000020,
  kXEPESectionContainsDataInit = 0x00000040,
  kXEPESectionContainsDataUninit = 0x00000080,
  kXEPESectionMemoryExecute = 0x20000000,
  kXEPESectionMemoryRead = 0x40000000,
  kXEPESectionMemoryWrite = 0x80000000,
};

class PESection {
 public:
  char name[9];  // 8 + 1 for \0
  uint32_t raw_address;
  uint32_t raw_size;
  uint32_t address;
  uint32_t size;
  uint32_t flags;  // kXEPESection*
};

struct PEExport {
  const char* name;
  uint32_t ordinal;

  uint64_t addr;  // Function address
};

xe_xex2_ref xe_xex2_load(xe::Memory* memory, const void* addr,
                         const size_t length, xe_xex2_options_t options);
void xe_xex2_dealloc(xe_xex2_ref xex);

const xe_xex2_header_t* xe_xex2_get_header(xe_xex2_ref xex);
const PESection* xe_xex2_get_pe_section(xe_xex2_ref xex, const char* name);

int xe_xex2_get_import_infos(xe_xex2_ref xex,
                             const xe_xex2_import_library_t* library,
                             xe_xex2_import_info_t** out_import_infos,
                             size_t* out_import_info_count);

uint32_t xe_xex2_lookup_export(xe_xex2_ref xex, const char* name);
uint32_t xe_xex2_lookup_export(xe_xex2_ref xex, uint16_t ordinal);

#endif  // XENIA_KERNEL_UTIL_XEX2_H_
