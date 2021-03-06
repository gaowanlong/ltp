/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"

static void setup(void);
static void cleanup(void);

char *TCID = "open03";
int TST_TOTAL = 1;

static int exp_enos[] = { 0, 0 };

static char fname[255];
static int fd;

int main(int ac, char **av)
{
	int lc;
	char *msg;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		TEST(open(fname, O_RDWR | O_CREAT, 0700));
		fd = TEST_RETURN;

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL | TTERRNO,
				 "open(%s,O_RDWR|O_CREAT,0700) failed", fname);
		} else {
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS,
					 "open(%s, O_RDWR|O_CREAT,0700) returned %ld",
					 fname, TEST_RETURN);
			}

			if (close(fd) == -1)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "close(%s) failed", fname);
			else if (unlink(fname) == -1)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "unlink(%s) failed", fname);
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "tfile_%d", getpid());
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();
}
