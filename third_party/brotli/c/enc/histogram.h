/* Copyright 2013 Google Inc. All Rights Reserved.

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

/* Models the histograms of literals, commands and distance codes. */

#ifndef BROTLI_ENC_HISTOGRAM_H_
#define BROTLI_ENC_HISTOGRAM_H_

#if !defined(STARBOARD)
#include <string.h>  /* memset */
#endif

#include "../common/constants.h"
#include "../common/platform.h"
#include <brotli/types.h>
#include "./block_splitter.h"
#include "./command.h"
#include "./context.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define FN(X) X ## Literal
#define DATA_SIZE BROTLI_NUM_LITERAL_SYMBOLS
#define DataType uint8_t
#include "./histogram_inc.h"  /* NOLINT(build/include) */
#undef DataType
#undef DATA_SIZE
#undef FN

#define FN(X) X ## Command
#define DataType uint16_t
#define DATA_SIZE BROTLI_NUM_COMMAND_SYMBOLS
#include "./histogram_inc.h"  /* NOLINT(build/include) */
#undef DATA_SIZE
#undef FN

#define FN(X) X ## Distance
#define DATA_SIZE BROTLI_NUM_DISTANCE_SYMBOLS
#include "./histogram_inc.h"  /* NOLINT(build/include) */
#undef DataType
#undef DATA_SIZE
#undef FN

BROTLI_INTERNAL void BrotliBuildHistogramsWithContext(
    const Command* cmds, const size_t num_commands,
    const BlockSplit* literal_split, const BlockSplit* insert_and_copy_split,
    const BlockSplit* dist_split, const uint8_t* ringbuffer, size_t pos,
    size_t mask, uint8_t prev_byte, uint8_t prev_byte2,
    const ContextType* context_modes, HistogramLiteral* literal_histograms,
    HistogramCommand* insert_and_copy_histograms,
    HistogramDistance* copy_dist_histograms);

#if defined(__cplusplus) || defined(c_plusplus)
}  /* extern "C" */
#endif

#endif  /* BROTLI_ENC_HISTOGRAM_H_ */
