/*      -*- OpenSAF  -*-
 *
 * Copyright Ericsson AB 2017 - All Rights Reserved.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 */

#include "base/hash.h"
#include <dlfcn.h>
#include <pthread.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "base/logtrace.h"
#include "base/osaf_utility.h"

namespace {

class HashLibrary {
 public:
  struct Context {
    uint64_t data[26];
    unsigned data2[2];
  };

  static void Initialize();

  static HashLibrary* instance() { return instance_; }
  int Init(Context* context) const { return init_function_(context); }
  int Update(Context* context, const void* data, size_t size) const {
    return update_function_(context, data, size);
  }
  int Final(unsigned char* result, Context* context) const {
    return final_function_(result, context);
  }

 private:
  using InitFunction = int(Context*);
  using UpdateFunction = int(Context*, const void*, size_t);
  using FinalFunction = int(unsigned char*, Context*);

  HashLibrary();
  static void* OpenLibrary();
  static void* GetSymbol(void* handle, int bits, const char* function,
                         void* fallback);
  static int FallbackInitFunction(Context* context);
  static int FallbackUpdateFunction(Context* context, const void* data,
                                    size_t size);
  static int FallbackFinalFunction(unsigned char* result, Context* context);
  static void PthreadOnceInitRoutine();
  void* handle_;
  InitFunction* init_function_;
  UpdateFunction* update_function_;
  FinalFunction* final_function_;
  static pthread_once_t once_control_;
  static HashLibrary* instance_;
};

pthread_once_t HashLibrary::once_control_ = PTHREAD_ONCE_INIT;
HashLibrary* HashLibrary::instance_ = nullptr;

const char* const kSslLibs[] = {
#include "osaf/ssl_libs.cc"
};

HashLibrary::HashLibrary()
    : handle_{OpenLibrary()},
      init_function_{reinterpret_cast<InitFunction*>(
          GetSymbol(handle_, 512, "Init",
                    reinterpret_cast<void*>(FallbackInitFunction)))},
      update_function_{reinterpret_cast<UpdateFunction*>(
          GetSymbol(handle_, 512, "Update",
                    reinterpret_cast<void*>(FallbackUpdateFunction)))},
      final_function_{reinterpret_cast<FinalFunction*>(
          GetSymbol(handle_, 512, "Final",
                    reinterpret_cast<void*>(FallbackFinalFunction)))} {}

void* HashLibrary::OpenLibrary() {
  void* handle = nullptr;
  for (int i = 0;
       handle == nullptr && i != sizeof(kSslLibs) / sizeof(kSslLibs[0]); ++i) {
    handle = dlopen(kSslLibs[i], RTLD_LAZY);
  }
  if (handle == nullptr) LOG_ER("Could not open ssl library");
  return handle;
}

void* HashLibrary::GetSymbol(void* handle, int bits, const char* function,
                             void* fallback) {
  void* symbol = nullptr;
  if (handle != nullptr) {
    char buf[32];
    int result = snprintf(buf, sizeof(buf), "SHA%d_%s", bits, function);
    if (result >= 0 && static_cast<size_t>(result) < sizeof(buf)) {
      symbol = dlsym(handle, buf);
      if (symbol == nullptr) {
        const char* error = dlerror();
        if (error == nullptr) error = "";
        LOG_ER("Could not find symbol %s: %s", buf, error);
      }
    } else {
      LOG_ER("Symbol name too large");
    }
  }
  return symbol != nullptr ? symbol : fallback;
}

int HashLibrary::FallbackInitFunction(Context*) { return 1; }

int HashLibrary::FallbackUpdateFunction(Context*, const void*, size_t) {
  return 1;
}

int HashLibrary::FallbackFinalFunction(unsigned char* result, Context*) {
  memset(result, 0, 64);
  return 1;
}

void HashLibrary::Initialize() {
  int result = pthread_once(&once_control_, PthreadOnceInitRoutine);
  if (result != 0) osaf_abort(result);
}

void HashLibrary::PthreadOnceInitRoutine() {
  assert(instance_ == nullptr);
  instance_ = new HashLibrary();
  if (instance_ == nullptr) osaf_abort(0);
}
}

namespace base {

const char kHashFunctionAlphabet[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};

void InitializeHashFunction() { HashLibrary::Initialize(); }

std::string Hash(const std::string& message) {
  HashLibrary::Context context;
  HashLibrary* lib = HashLibrary::instance();
  lib->Init(&context);
  context.data[0] += UINT64_C(0x96f78fac128be92b);
  context.data[1] += UINT64_C(0x202b002c69f03634);
  context.data[2] += UINT64_C(0x473aef07a340f237);
  context.data[3] += UINT64_C(0x4746024456ec7df0);
  context.data[4] += UINT64_C(0x209b3f0619762c29);
  context.data[5] += UINT64_C(0x6569267c8fb4c21d);
  context.data[6] += UINT64_C(0x4aa747ffd7996d81);
  context.data[7] += UINT64_C(0xc8b19fc2c59a8106);
  lib->Update(&context, message.data(), message.size());
  unsigned char result[64];
  lib->Final(result, &context);
  std::string encoded;
  encoded.reserve(32);
  for (int i = 0; i != 24; i += 3) {
    uint64_t a = (result[i] << 16) | (result[i + 1] << 8) | result[i + 2];
    encoded.push_back(kHashFunctionAlphabet[a >> 18]);
    encoded.push_back(kHashFunctionAlphabet[(a >> 12) & 63]);
    encoded.push_back(kHashFunctionAlphabet[(a >> 6) & 63]);
    encoded.push_back(kHashFunctionAlphabet[a & 63]);
  }
  return encoded;
}

}  // namespace base
