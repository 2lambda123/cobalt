// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "bindings/modules/v8/ModuleBindingsInitializer.h"

#include "bindings/core/v8/ModuleProxy.h"
#include "bindings/core/v8/V8PerIsolateData.h"
#include "core/dom/ExecutionContext.h"
#include "modules/indexeddb/IDBPendingTransactionMonitor.h"

namespace blink {

// initPartialInterfacesInModules is generated by
// generate_init_partial_interfaces.py.
void initPartialInterfacesInModules();

static void didLeaveScriptContextForModule(v8::Isolate* isolate)
{
    // Indexed DB requires that transactions are created with an internal |active| flag
    // set to true, but the flag becomes false when control returns to the event loop.
    V8PerIsolateData::from(isolate)->ensureIDBPendingTransactionMonitor()->deactivateNewTransactions();
}

void ModuleBindingsInitializer::init()
{
    ModuleProxy::moduleProxy().registerDidLeaveScriptContextForRecursionScope(didLeaveScriptContextForModule);
    initPartialInterfacesInModules();
}

} // namespace blink
