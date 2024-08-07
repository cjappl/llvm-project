//===-- Linux implementation of the mtx_destroy function ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "src/threads/mtx_destroy.h"
#include "src/__support/common.h"
#include "src/__support/macros/config.h"
#include "src/__support/threads/mutex.h"

#include <threads.h> // For mtx_t definition.

namespace LIBC_NAMESPACE_DECL {

LLVM_LIBC_FUNCTION(void, mtx_destroy, (mtx_t *)) {}

} // namespace LIBC_NAMESPACE_DECL
