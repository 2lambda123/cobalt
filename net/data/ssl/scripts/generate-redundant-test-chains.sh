#!/bin/sh

# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script generates two chains of test certificates:
#
#     1. A (end-entity) -> B -> C -> D (self-signed root)
#     2. A (end-entity) -> B -> C2 (self-signed root)
#
# in which A, B, C, and D have distinct keypairs. C2 is a self-signed root
# certificate that uses the same keypair as C.
#
# We use these cert chains in
# SSLClientSocketTest.VerifyReturnChainProperlyOrdered to ensure that
# SSLInfo objects see the certificate chain as validated rather than as
# served by the server. The server serves chain 1. The client has C2, NOT D,
# installed as a trusted root. Therefore, the chain will validate as chain
# 2, even though the server served chain 1.

try () {
  echo "$@"
  $@ || exit 1
}

generate_key_command () {
  case "$1" in
    rsa)
      echo genrsa
      ;;
    *)
      exit 1
  esac
}

try rm -rf out
try mkdir out

echo Create the serial number files.
serial=100
for i in B C C2 D
do
  try echo $serial > out/$i-serial
  serial=$(expr $serial + 1)
done

echo Generate the keys.
try openssl genrsa -out out/A.key 2048
try openssl genrsa -out out/B.key 2048
try openssl genrsa -out out/C.key 2048
try openssl genrsa -out out/D.key 2048

echo Generate the D CSR.
CA_COMMON_NAME="D Root CA" \
  CA_DIR=out \
  CA_NAME=req_env_dn \
  KEY_SIZE=2048 \
  ALGO=rsa \
  CERT_TYPE=root \
  TYPE=D CERTIFICATE=D \
  try openssl req \
    -new \
    -key out/D.key \
    -out out/D.csr \
    -config redundant-ca.cnf

echo D signs itself.
CA_COMMON_NAME="D Root CA" \
  CA_DIR=out \
  CA_NAME=req_env_dn \
  try openssl x509 \
    -req -days 3650 \
    -in out/D.csr \
    -extensions ca_cert \
    -signkey out/D.key \
    -out out/D.pem

echo Generate the C2 root CSR.
CA_COMMON_NAME="C CA" \
  CA_DIR=out \
  CA_NAME=req_env_dn \
  KEY_SIZE=2048 \
  ALGO=rsa \
  CERT_TYPE=root \
  TYPE=C2 CERTIFICATE=C2 \
  try openssl req \
    -new \
    -key out/C.key \
    -out out/C2.csr \
    -config redundant-ca.cnf

echo C2 signs itself.
CA_COMMON_NAME="C CA" \
  CA_DIR=out \
  CA_NAME=req_env_dn \
  try openssl x509 \
    -req -days 3650 \
    -in out/C2.csr \
    -extensions ca_cert \
    -signkey out/C.key \
    -out out/C2.pem

echo Generate the B and C intermediaries\' CSRs.
for i in B C
do
  name="$i Intermediate CA"
  CA_COMMON_NAME="$i CA" \
    CA_DIR=out \
    CA_NAME=req_env_dn \
    KEY_SIZE=2048 \
    ALGO=rsa \
    CERT_TYPE=root \
    TYPE=$i CERTIFICATE=$i \
    try openssl req \
      -new \
      -key out/$i.key \
      -out out/$i.csr \
      -config redundant-ca.cnf
done

echo D signs the C intermediate.
# Make sure the signer's DB file exists.
touch out/D-index.txt
CA_COMMON_NAME="D Root CA" \
  CA_DIR=out \
  CA_NAME=req_env_dn \
  KEY_SIZE=2048 \
  ALGO=rsa \
  CERT_TYPE=root \
  TYPE=D CERTIFICATE=D \
  try openssl ca \
    -batch \
    -extensions ca_cert \
    -in out/C.csr \
    -out out/C.pem \
    -config redundant-ca.cnf

echo C signs the B intermediate.
touch out/C-index.txt
CA_COMMON_NAME="C CA" \
  CA_DIR=out \
  CA_NAME=req_env_dn \
  KEY_SIZE=2048 \
  ALGO=rsa \
  CERT_TYPE=root \
  TYPE=C CERTIFICATE=C \
  try openssl ca \
    -batch \
    -extensions ca_cert \
    -in out/B.csr \
    -out out/B.pem \
    -config redundant-ca.cnf

echo Generate the A end-entity CSR.
try openssl req \
  -new \
  -key out/A.key \
  -out out/A.csr \
  -config ee.cnf

echo B signs A.
touch out/B-index.txt
CA_COMMON_NAME="B CA" \
  CA_DIR=out \
  CA_NAME=req_env_dn \
  KEY_SIZE=$signer_key_size \
  ALGO=$signer_algo \
  CERT_TYPE=intermediate \
  TYPE=B CERTIFICATE=B \
  try openssl ca \
    -batch \
    -extensions user_cert \
    -in out/A.csr \
    -out out/A.pem \
    -config redundant-ca.cnf

echo Create redundant-server-chain.pem
cat out/A.key out/A.pem out/B.pem out/C.pem out/D.pem \
    > redundant-server-chain.pem

echo Create redundant-validated-chain.pem
cat out/A.key out/A.pem out/B.pem out/C2.pem > redundant-validated-chain.pem

echo Create redundant-validated-chain-root.pem
cp out/C2.pem redundant-validated-chain-root.pem

