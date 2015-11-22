/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DOM_BENCHMARK_STAT_NAMES_H_
#define DOM_BENCHMARK_STAT_NAMES_H_

namespace cobalt {
namespace dom {

// These names are referenced by TRACE_EVENT calls within dom code, and
// then also watched for by benchmarks testing that code.  Modifying these
// names will typically also require modifying the database schema that
// the benchmarks are uploaded to.  See ***REMOVED***steel-build-stats-doc.

extern const char kBenchmarkStatUpdateComputedStyles[];
extern const char kBenchmarkStatUpdateMatchingRules[];
extern const char kBenchmarkStatUpdateSelectorTree[];

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_BENCHMARK_STAT_NAMES_H_
