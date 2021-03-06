/*
 *   Copyright (c) 2013 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * DESCRIPTION
 *	Check for the following errors:
 *	1.	EEXIST
 *	2.	EISDIR
 *	3.	ENOTDIR
 *	4.	ENAMETOOLONG
 *	5.	EACCES
 *	6.	EFAULT
 *
 * ALGORITHM
 *	1. Open a file with O_CREAT and O_EXCL, when the file already
 *	   exists. Check the errno for EEXIST
 *
 *	2. Pass a directory as the pathname and request a write access,
 *	   check for errno for EISDIR
 *
 *	3. Specify O_DIRECTORY as a parameter to open and pass a file as the
 *	   pathname, check errno for ENOTDIR
 *
 *	4. Attempt to open() a filename which is more than VFS_MAXNAMLEN, and
 *	   check for errno to be ENAMETOOLONG.
 *
 *	5. Attempt to open a test executable in WRONLY mode,
 *	   open(2) should fail with EACCES.
 *
 *	6. Attempt to pass an invalid pathname with an address pointing outside
 *	   the address space of the process, as the argument to open(), and
 *	   expect to get EFAULT.
 */

#define _GNU_SOURCE		/* for O_DIRECTORY */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

static void setup(void);
static void cleanup(void);

char *TCID = "open08";

static char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static int exp_enos[] = {EEXIST, EISDIR, ENOTDIR, ENAMETOOLONG,
			 EACCES, EFAULT, 0};

static char *bad_addr;

static char filename[40] = "";
static char fname[] = "/bin/cat";
static char bad_file[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";

static struct test_case_t {
	char *fname;
	int flags;
	int error;
} TC[] = {
	{filename, O_CREAT | O_EXCL, EEXIST},
	{"/tmp", O_RDWR, EISDIR},
	{filename, O_DIRECTORY, ENOTDIR},
	{bad_file, O_RDWR, ENAMETOOLONG},
	{fname, O_WRONLY, EACCES},
#if !defined(UCLINUX)
	{(char *)-1, O_CREAT, EFAULT}
#endif
};

int TST_TOTAL = sizeof(TC) / sizeof(TC[0]);

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int i;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(open(TC[i].fname, TC[i].flags,
				  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fildes;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0)
		tst_brkm(TBROK, NULL, "Test must be run as root");

	ltpuser = getpwnam(nobody_uid);
	if (setgid(ltpuser->pw_gid) == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "setgid(%d) failed",
			 ltpuser->pw_gid);
	} else if (setuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "setuid(%d) failed",
			 ltpuser->pw_uid);
	}

	tst_tmpdir();

	sprintf(filename, "open3.%d", getpid());

	fildes = creat(filename, 0600);
	if (fildes == -1)
		tst_brkm(TBROK, cleanup, "Can't creat %s", filename);

	close(fildes);

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK, cleanup, "mmap failed");

	TC[5].fname = bad_addr;
#endif
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();
}
