/*
 * Library notification functions test program
 *
 * Copyright (C) 2006-2020, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <file_stream.h>
#include <types.h>

#if defined( HAVE_STDLIB_H ) || defined( WINAPI )
#include <stdlib.h>
#endif

#include "ewf_test_libewf.h"
#include "ewf_test_macros.h"
#include "ewf_test_unused.h"

/* Tests the libewf_notify_set_verbose function
 * Returns 1 if successful or 0 if not
 */
int ewf_test_notify_set_verbose(
     void )
{
	/* Test invocation of function only
	 */
	libewf_notify_set_verbose(
	 0 );

	return( 1 );
}

/* Tests the libewf_notify_set_stream function
 * Returns 1 if successful or 0 if not
 */
int ewf_test_notify_set_stream(
     void )
{
	/* Test invocation of function only
	 */
	libewf_notify_set_stream(
	 NULL,
	 NULL );

	return( 1 );
}

/* Tests the libewf_notify_stream_open function
 * Returns 1 if successful or 0 if not
 */
int ewf_test_notify_stream_open(
     void )
{
	/* Test invocation of function only
	 */
	libewf_notify_stream_open(
	 NULL,
	 NULL );

	return( 1 );
}

/* Tests the libewf_notify_stream_close function
 * Returns 1 if successful or 0 if not
 */
int ewf_test_notify_stream_close(
     void )
{
	/* Test invocation of function only
	 */
	libewf_notify_stream_close(
	 NULL );

	return( 1 );
}

/* The main program
 */
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
int wmain(
     int argc EWF_TEST_ATTRIBUTE_UNUSED,
     wchar_t * const argv[] EWF_TEST_ATTRIBUTE_UNUSED )
#else
int main(
     int argc EWF_TEST_ATTRIBUTE_UNUSED,
     char * const argv[] EWF_TEST_ATTRIBUTE_UNUSED )
#endif
{
	EWF_TEST_UNREFERENCED_PARAMETER( argc )
	EWF_TEST_UNREFERENCED_PARAMETER( argv )

	EWF_TEST_RUN(
	 "libewf_notify_set_verbose",
	 ewf_test_notify_set_verbose )

	EWF_TEST_RUN(
	 "libewf_notify_set_stream",
	 ewf_test_notify_set_stream )

	EWF_TEST_RUN(
	 "libewf_notify_stream_open",
	 ewf_test_notify_stream_open )

	EWF_TEST_RUN(
	 "libewf_notify_stream_close",
	 ewf_test_notify_stream_close )

	return( EXIT_SUCCESS );

on_error:
	return( EXIT_FAILURE );
}

