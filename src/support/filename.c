/*-
 * Copyright (c) 2014-2015 MongoDB, Inc.
 * Copyright (c) 2008-2014 WiredTiger, Inc.
 *	All rights reserved.
 *
 * See the file LICENSE for redistribution information.
 */

#include "wt_internal.h"

/*
 * __wt_filename --
 *	Build a file name in a scratch buffer, automatically calculate the
 *	length of the file name.
 */
int
__wt_filename(WT_SESSION_IMPL *session, const char *name, char **path)
{
	return (__wt_nfilename(session, name, strlen(name), path));
}

/*
 * __wt_nfilename --
 *	Build a file name in a scratch buffer.  If the name is already an
 *	absolute path duplicate it, otherwise generate a path relative to the
 *	connection home directory.
 */
int
__wt_nfilename(
    WT_SESSION_IMPL *session, const char *name, size_t namelen, char **path)
{
	WT_CONNECTION_IMPL *conn;
	size_t len;
	char *buf;

	conn = S2C(session);
	*path = NULL;

	if (__wt_absolute_path(name))
		WT_RET(__wt_strndup(session, name, namelen, path));
	else {
		len = strlen(conn->home) + 1 + namelen + 1;
		WT_RET(__wt_calloc(session, 1, len, &buf));
		snprintf(buf, len, "%s%s%.*s",
		    conn->home, __wt_path_separator(), (int)namelen, name);
		*path = buf;
	}

	return (0);
}

/*
 * __wt_remove_if_exists --
 *	Remove a file if it exists.
 */
int
__wt_remove_if_exists(WT_SESSION_IMPL *session, const char *name)
{
	int exist;

	WT_RET(__wt_exist(session, name, &exist));
	if (exist)
		WT_RET(__wt_remove(session, name));
	return (0);
}

/*
 * __wt_fp_open --
 *	Open a FILE handle.
 */
int
__wt_fp_open(WT_SESSION_IMPL *session, const char *name, FILE **fpp)
{
	WT_DECL_RET;
	char *path;

	WT_RET(__wt_filename(session, name, &path));

	if ((*fpp = fopen(path, "w")) == NULL)
		ret = __wt_errno();

	__wt_free(session, path);

	return (ret);
}

/*
 * __wt_fp_close --
 *	Close a FILE handle.
 */
int
__wt_fp_close(WT_SESSION_IMPL *session, FILE **fpp)
{
	WT_DECL_RET;

	WT_UNUSED(session);

	/* Close the handle (which implicitly flushes the file to disk). */
	if (*fpp != NULL) {
		if (fclose(*fpp) != 0)
			ret = __wt_errno();
		*fpp = NULL;
	}

	return (ret);
}
