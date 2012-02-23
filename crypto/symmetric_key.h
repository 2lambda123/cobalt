// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CRYPTO_SYMMETRIC_KEY_H_
#define CRYPTO_SYMMETRIC_KEY_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "crypto/crypto_export.h"

#if defined(USE_NSS)
#include "crypto/scoped_nss_types.h"
#elif defined(OS_MACOSX)
#include <Security/cssmtype.h>
#elif defined(OS_WIN)
#include "crypto/scoped_capi_types.h"
#endif

namespace crypto {

// Wraps a platform-specific symmetric key and allows it to be held in a
// scoped_ptr.
class CRYPTO_EXPORT SymmetricKey {
 public:
  // Defines the algorithm that a key will be used with. See also
  // classs Encrptor.
  enum Algorithm {
    AES,
    HMAC_SHA1,
  };

  virtual ~SymmetricKey();

  // Generates a random key suitable to be used with |algorithm| and of
  // |key_size_in_bits| bits. |key_size_in_bits| must be a multiple of 8.
  // The caller is responsible for deleting the returned SymmetricKey.
  static SymmetricKey* GenerateRandomKey(Algorithm algorithm,
                                         size_t key_size_in_bits);

  // Derives a key from the supplied password and salt using PBKDF2, suitable
  // for use with specified |algorithm|. Note |algorithm| is not the algorithm
  // used to derive the key from the password. |key_size_in_bits| must be a
  // multiple of 8. The caller is responsible for deleting the returned
  // SymmetricKey.
  static SymmetricKey* DeriveKeyFromPassword(Algorithm algorithm,
                                             const std::string& password,
                                             const std::string& salt,
                                             size_t iterations,
                                             size_t key_size_in_bits);

  // Imports an array of key bytes in |raw_key|. This key may have been
  // generated by GenerateRandomKey or DeriveKeyFromPassword and exported with
  // GetRawKey, or via another compatible method. The key must be of suitable
  // size for use with |algorithm|. The caller owns the returned SymmetricKey.
  static SymmetricKey* Import(Algorithm algorithm, const std::string& raw_key);

#if defined(USE_OPENSSL)
  const std::string& key() { return key_; }
#elif defined(USE_NSS)
  PK11SymKey* key() const { return key_.get(); }
#elif defined(OS_MACOSX)
  CSSM_DATA cssm_data() const;
#elif defined(OS_WIN)
  HCRYPTKEY key() const { return key_.get(); }
#endif

  // Extracts the raw key from the platform specific data.
  // Warning: |raw_key| holds the raw key as bytes and thus must be handled
  // carefully.
  bool GetRawKey(std::string* raw_key);

#if defined(OS_CHROMEOS)
  // Creates symmetric key from NSS key. Takes over the ownership of |key|.
  static SymmetricKey* CreateFromKey(PK11SymKey* key);
#endif

 private:
#if defined(USE_OPENSSL)
  SymmetricKey() {}
  std::string key_;
#elif defined(USE_NSS)
  explicit SymmetricKey(PK11SymKey* key);
  ScopedPK11SymKey key_;
#elif defined(OS_MACOSX)
  SymmetricKey(const void* key_data, size_t key_size_in_bits);
  std::string key_;
#elif defined(OS_WIN)
  SymmetricKey(HCRYPTPROV provider, HCRYPTKEY key,
               const void* key_data, size_t key_size_in_bytes);

  ScopedHCRYPTPROV provider_;
  ScopedHCRYPTKEY key_;

  // Contains the raw key, if it is known during initialization and when it
  // is likely that the associated |provider_| will be unable to export the
  // |key_|. This is the case of HMAC keys when the key size exceeds 16 bytes
  // when using the default RSA provider.
  // TODO(rsleevi): See if KP_EFFECTIVE_KEYLEN is the reason why CryptExportKey
  // fails with NTE_BAD_KEY/NTE_BAD_LEN
  std::string raw_key_;
#endif

  DISALLOW_COPY_AND_ASSIGN(SymmetricKey);
};

}  // namespace crypto

#endif  // CRYPTO_SYMMETRIC_KEY_H_
