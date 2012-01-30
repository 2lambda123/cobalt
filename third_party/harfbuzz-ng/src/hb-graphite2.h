/*
 * Copyright (C) 2011  Martin Hosken
 * Copyright (C) 2011  SIL International
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef HB_GRAPHITE2_H
#define HB_GRAPHITE2_H

#include "hb-common.h"
#include "hb-shape.h"

HB_BEGIN_DECLS


#define HB_GRAPHITE_TAG_Silf HB_TAG('S','i','l','f')

hb_bool_t
hb_graphite_shape (hb_font_t          *font,
		   hb_buffer_t        *buffer,
		   const hb_feature_t *features,
		   unsigned int        num_features,
		   const char * const *shaper_options);

HB_END_DECLS

#endif /* HB_GRAPHITE2_H */
