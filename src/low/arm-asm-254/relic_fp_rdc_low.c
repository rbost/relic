/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (C) 2007-2019 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or modify it under the
 * terms of the version 2.1 (or later) of the GNU Lesser General Public License
 * as published by the Free Software Foundation; or version 2.0 of the Apache
 * License as published by the Apache Software Foundation. See the LICENSE files
 * for more details.
 *
 * RELIC is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the LICENSE files for more details.
 *
 * You should have received a copy of the GNU Lesser General Public or the
 * Apache License along with RELIC. If not, see <https://www.gnu.org/licenses/>
 * or <https://www.apache.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of the low-level prime field modular reduction functions.
 *
 * @ingroup fp
 */

#include "relic_core.h"
#include "relic_fp.h"
#include "relic_fp_low.h"
#include "relic_bn_low.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fp_rdcs_low(dig_t *c, const dig_t *a, const dig_t *m) {
	relic_align dig_t q[2 * FP_DIGS], _q[2 * FP_DIGS];
	relic_align dig_t _r[2 * FP_DIGS], r[2 * FP_DIGS], t[2 * FP_DIGS];
	int *sform, len;
	int first, i, j, b0, d0, b1, d1;

	sform = fp_prime_get_sps(&len);

	SPLIT(b0, d0, FP_BITS, DIG_LOG);
	first = (d0) + (b0 == 0 ? 0 : 1);

	/* q = floor(a/b^k) */
	dv_zero(q, 2 * FP_DIGS);
	bn_rshd_low(q, a, 2 * FP_DIGS, d0);
	if (b0 > 0) {
		bn_rshb_low(q, q, 2 * FP_DIGS, b0);
	}

	/* r = a - qb^k. */
	dv_copy(r, a, first);
	if (b0 > 0) {
		r[first - 1] &= MASK(b0);
	}

	while (!fp_is_zero(q)) {
		dv_zero(_q, 2 * FP_DIGS);
		for (i = len - 1; i > 0; i--) {
			j = (sform[i] < 0 ? -sform[i] : sform[i]);
			SPLIT(b1, d1, j, DIG_LOG);
			dv_zero(t, 2 * FP_DIGS);
			bn_lshd_low(t, q, FP_DIGS, d1);
			if (b1 > 0) {
				bn_lshb_low(t, t, 2 * FP_DIGS, b1);
			}
			if (sform[i] > 0) {
				bn_subn_low(_q, _q, t, 2 * FP_DIGS);
			} else {
				bn_addn_low(_q, _q, t, 2 * FP_DIGS);
			}
		}
		if (sform[0] > 0) {
			bn_subn_low(_q, _q, q, 2 * FP_DIGS);
		} else {
			bn_addn_low(_q, _q, q, 2 * FP_DIGS);
		}
		bn_rshd_low(q, _q, 2 * FP_DIGS, d0);
		if (b0 > 0) {
			bn_rshb_low(q, q, 2 * FP_DIGS, b0);
		}

		dv_copy(_r, _q, first);
		if (b0 > 0) {
			_r[first - 1] &= MASK(b0);
		}
		fp_add(r, r, _r);
	}
	while (dv_cmp(r, m, FP_DIGS) != CMP_LT) {
		fp_subn_low(r, r, m);
	}
	fp_copy(c, r);
}
