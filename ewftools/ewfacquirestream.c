/*
 * ewfacquirestream
 * Reads data from a stdin and writes it in EWF format
 *
 * Copyright (c) 2006-2008, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the creator, related organisations, nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER, COMPANY AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "../libewf/libewf_includes.h"

#include <errno.h>
#include <stdio.h>

#if defined( HAVE_SYS_IOCTL_H )
#include <sys/ioctl.h>
#endif

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "../libewf/libewf_char.h"
#include "../libewf/libewf_common.h"
#include "../libewf/libewf_notify.h"
#include "../libewf/libewf_string.h"

#include "ewfcommon.h"
#include "ewfgetopt.h"
#include "ewfglob.h"
#include "ewfinput.h"
#include "ewfoutput.h"
#include "ewfsignal.h"
#include "ewfstring.h"

/* Prints the executable usage information
 */
void usage( void )
{
	fprintf( stdout, "Usage: ewfacquirestream [ -b amount_of_sectors ] [ -c compression_type ] [ -C case_number ] [ -d digest_type ] [ -D description ]\n" );
	fprintf( stdout, "                        [ -e examiner_name ] [ -E evidence_number ] [ -f format ] [ -m media_type ] [ -M volume_type ] [ -N notes ]\n" );
	fprintf( stdout, "                        [ -S segment_file_size ] [ -t target ] [ -hsvVw ]\n\n" );

	fprintf( stdout, "\tReads data from stdin\n\n" );

	fprintf( stdout, "\t-b: specify the amount of sectors to read at once (per chunk), options: 64 (default),\n" );
	fprintf( stdout, "\t    128, 256, 512, 1024, 2048, 4096, 8192, 16384 or 32768\n" );
	fprintf( stdout, "\t-c: specify the compression type, options: none (is default), empty_block, fast, best\n" );
	fprintf( stdout, "\t-C: specify the case number (default is case_number).\n" );
	fprintf( stdout, "\t-d: calculate additional digest (hash) types besides md5, options: sha1\n" );
	fprintf( stdout, "\t-D: specify the description (default is description).\n" );
	fprintf( stdout, "\t-e: specify the examiner name (default is examiner_name).\n" );
	fprintf( stdout, "\t-E: specify the evidence number (default is evidence_number).\n" );
	fprintf( stdout, "\t-f: specify the EWF file format to write to, options: ftk, encase2, encase3, encase4,\n" );
	fprintf( stdout, "\t    encase5 (is default), encase6, linen5, linen6, ewfx\n" );
	fprintf( stdout, "\t-h: shows this help\n" );
	fprintf( stdout, "\t-m: specify the media type, options: fixed (is default), removable\n" );
	fprintf( stdout, "\t-M: specify the volume type, options: logical, physical (is default)\n" );
	fprintf( stdout, "\t-N: specify the notes (default is notes).\n" );
	fprintf( stdout, "\t-s: swap byte pairs of the media data (from AB to BA)\n" );
	fprintf( stdout, "\t    (use this for big to little endian conversion and vice versa)\n" );
	fprintf( stdout, "\t-S: specify the segment file size in kibibytes (KiB) (default is %" PRIu32 ")\n",
	 (uint32_t) ( EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE / 1024 ) );
	fprintf( stdout, "\t    (minimum is %" PRIu32 ", maximum is %" PRIu64 " for encase6 format and %" PRIu32 " for other formats)\n",
	 (uint32_t) ( EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE / 1024 ),
	 (uint64_t) ( EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT / 1024 ),
	 (uint32_t) ( EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT / 1024 ) );
	fprintf( stdout, "\t-t: specify the target file (without extension) to write to (default is stream)\n" );
	fprintf( stdout, "\t-v: verbose output to stderr\n" );
	fprintf( stdout, "\t-V: print version\n" );
	fprintf( stdout, "\t-w: wipe sectors on read error (mimic EnCase like behavior)\n" );
}

/* The main program
 */
#if defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
#if defined( HAVE_UUID_UUID_H ) && defined( HAVE_LIBUUID )
	uint8_t guid[ 16 ];
#endif
	CHAR_T *filenames[ 1 ]                     = { _S_CHAR_T( "stream" ) };

	LIBEWF_HANDLE *handle                      = NULL;
	libewf_char_t *calculated_md5_hash_string  = NULL;
	libewf_char_t *calculated_sha1_hash_string = NULL;
	libewf_char_t *case_number                 = NULL;
	libewf_char_t *description                 = NULL;
	libewf_char_t *evidence_number             = NULL;
	libewf_char_t *examiner_name               = NULL;
	libewf_char_t *notes                       = NULL;
	libewf_char_t *acquiry_operating_system    = NULL;
	libewf_char_t *acquiry_software_version    = NULL;
	libewf_char_t *program                     = _S_LIBEWF_CHAR( "ewfacquirestream" );
	CHAR_T *option_case_number                 = NULL;
	CHAR_T *option_description                 = NULL;
	CHAR_T *option_examiner_name               = NULL;
	CHAR_T *option_evidence_number             = NULL;
	CHAR_T *option_notes                       = NULL;
	CHAR_T *time_string                        = NULL;
	CHAR_T *end_of_string                      = NULL;
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
        CHAR_T *error_string                       = NULL;
#endif
	void *callback                             = &ewfoutput_stream_process_status_fprint;

	INT_T option                               = 0;
	size_t string_length                       = 0;
	time_t timestamp_start                     = 0;
	time_t timestamp_end                       = 0;
	int64_t write_count                        = 0;
	uint64_t segment_file_size                 = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;
	uint64_t acquiry_offset                    = 0;
	uint64_t acquiry_size                      = 0;
	uint64_t sectors_per_chunk                 = 64;
	uint32_t sector_error_granularity          = 64;
	int8_t compression_level                   = LIBEWF_COMPRESSION_NONE;
	uint8_t media_type                         = LIBEWF_MEDIA_TYPE_FIXED;
	uint8_t volume_type                        = LIBEWF_VOLUME_TYPE_PHYSICAL;
	uint8_t compress_empty_block               = 0;
	uint8_t libewf_format                      = LIBEWF_FORMAT_ENCASE5;
	uint8_t wipe_chunk_on_error                = 0;
	uint8_t read_error_retry                   = 2;
	uint8_t swap_byte_pairs                    = 0;
	uint8_t seek_on_error                      = 0;
	uint8_t calculate_md5                      = 1;
	uint8_t calculate_sha1                     = 0;
	uint8_t verbose                            = 0;

	ewfsignal_initialize();

	ewfoutput_version_fprint( stdout, program );

	while( ( option = ewfgetopt( argc, argv, _S_CHAR_T( "b:c:C:d:D:e:E:f:hm:M:N:sS:t:vVw" ) ) ) != (INT_T) -1 )
	{
		switch( option )
		{
			case (INT_T) '?':
			default:
				fprintf( stderr, "Invalid argument: %" PRIs "\n", argv[ optind ] );

				usage();

				return( EXIT_FAILURE );

			case (INT_T) 'b':
				sectors_per_chunk = ewfinput_determine_sectors_per_chunk_char_t( optarg );

				if( sectors_per_chunk == 0 )
				{
					fprintf( stderr, "Unsuported amount of sectors per chunk defaulting to 64.\n" );

					sectors_per_chunk = 64;
				}
				break;

			case (INT_T) 'c':
				if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "empty_block" ), 11 ) == 0 )
				{
					compress_empty_block = 1;
				}
				else
				{
					compression_level = ewfinput_determine_compression_level_char_t( optarg );
				
					if( compression_level <= -1 )
					{
						fprintf( stderr, "Unsupported compression type defaulting to none.\n" );

						compression_level = LIBEWF_COMPRESSION_NONE;
					}
				}
				break;

			case (INT_T) 'C':
				option_case_number = optarg;

				break;

			case (INT_T) 'd':
				if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "sha1" ), 4 ) == 0 )
				{
					calculate_sha1 = 1;
				}
				else
				{
					fprintf( stderr, "Unsupported digest type.\n" );
				}
				break;

			case (INT_T) 'D':
				option_description = optarg;

				break;

			case (INT_T) 'e':
				option_examiner_name = optarg;

				break;

			case (INT_T) 'E':
				option_evidence_number = optarg;

				break;

			case (INT_T) 'f':
				libewf_format = ewfinput_determine_libewf_format_char_t( optarg );

				if( ( libewf_format == 0 )
				 || ( libewf_format == LIBEWF_FORMAT_EWF )
				 || ( libewf_format == LIBEWF_FORMAT_SMART ) )
				{
					fprintf( stderr, "Unsupported EWF file format type defaulting to encase5.\n" );

					libewf_format = LIBEWF_FORMAT_ENCASE5;
				}
				break;

			case (INT_T) 'h':
				usage();

				return( EXIT_SUCCESS );

			case (INT_T) 'm':
				if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "fixed" ), 5 ) == 0 )
				{
					media_type = LIBEWF_MEDIA_TYPE_FIXED;
				}
				else if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "removable" ), 9 ) == 0 )
				{
					media_type = LIBEWF_MEDIA_TYPE_REMOVABLE;
				}
				else
				{
					fprintf( stderr, "Unsupported media type defaulting to fixed.\n" );
				}
				break;

			case (INT_T) 'M':
				if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "logical" ), 7 ) == 0 )
				{
					volume_type = LIBEWF_VOLUME_TYPE_LOGICAL;
				}
				else if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "physical" ), 8 ) == 0 )
				{
					volume_type = LIBEWF_VOLUME_TYPE_PHYSICAL;
				}
				else
				{
					fprintf( stderr, "Unsupported volume type defaulting to logical.\n" );
				}
				break;

			case (INT_T) 'N':
				option_notes = optarg;

				break;

			case (INT_T) 'q':

				break;

			case (INT_T) 's':
				swap_byte_pairs = 1;

				break;

			case (INT_T) 'S':
				string_length      = CHAR_T_LENGTH( optarg );
				end_of_string      = &optarg[ string_length - 1 ];
				segment_file_size  = (uint64_t) CHAR_T_TOLONG( optarg, &end_of_string, 0 );
				segment_file_size *= 1024;

				if( ( segment_file_size < EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE )
				 || ( ( libewf_format == LIBEWF_FORMAT_ENCASE6 )
				  && ( segment_file_size >= (int64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
				 || ( ( libewf_format != LIBEWF_FORMAT_ENCASE6 )
				  && ( segment_file_size >= (int64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT ) ) )
				{
					fprintf( stderr, "Unsupported segment file size defaulting to %" PRIu32 ".\n",
					 (uint32_t) EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE );

					segment_file_size = (int64_t) EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;
				}
				break;

			case (INT_T) 't':
				filenames[ 0 ] = optarg;

				break;

			case (INT_T) 'v':
				verbose = 1;

				break;

			case (INT_T) 'V':
				ewfoutput_copyright_fprint( stdout );

				return( EXIT_SUCCESS );

			case (INT_T) 'w':
				wipe_chunk_on_error = 1;

				break;
		}
	}
	libewf_set_notify_values( stderr, verbose );

	if( option_case_number != NULL )
	{
		string_length = CHAR_T_LENGTH( option_case_number );

		if( string_length > 0 )
		{
			string_length += 1;
			case_number    = (libewf_char_t *) libewf_common_alloc(
			                                    sizeof( libewf_char_t ) * string_length );

			if( case_number == NULL )
			{
				fprintf( stderr, "Unable to create case number string.\n" );

				return( EXIT_FAILURE );
			}
			if( ewfstring_copy_libewf_char_from_char_t( case_number, option_case_number, string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set case number string.\n" );

				return( EXIT_FAILURE );
			}
		}
	}
	if( option_description != NULL )
	{
		string_length = CHAR_T_LENGTH( option_description );

		if( string_length > 0 )
		{
			string_length += 1;
			description    = (libewf_char_t *) libewf_common_alloc(
			                                    sizeof( libewf_char_t ) * string_length );

			if( description == NULL )
			{
				fprintf( stderr, "Unable to create description string.\n" );

				return( EXIT_FAILURE );
			}
			if( ewfstring_copy_libewf_char_from_char_t( description, option_description, string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set description string.\n" );

				return( EXIT_FAILURE );
			}
		}
	}
	if( option_examiner_name != NULL )
	{
		string_length = CHAR_T_LENGTH( option_examiner_name );

		if( string_length > 0 )
		{
			string_length += 1;
			examiner_name  = (libewf_char_t *) libewf_common_alloc(
			                                    sizeof( libewf_char_t ) * string_length );

			if( examiner_name == NULL )
			{
				fprintf( stderr, "Unable to create examiner name string.\n" );

				return( EXIT_FAILURE );
			}
			if( ewfstring_copy_libewf_char_from_char_t( examiner_name, option_examiner_name, string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set examiner name string.\n" );

				return( EXIT_FAILURE );
			}
		}
	}
	if( option_evidence_number != NULL )
	{
		string_length = CHAR_T_LENGTH( option_evidence_number );

		if( string_length > 0 )
		{
			string_length  += 1;
			evidence_number = (libewf_char_t *) libewf_common_alloc(
			                                     sizeof( libewf_char_t ) * string_length );

			if( evidence_number == NULL )
			{
				fprintf( stderr, "Unable to create evidence number string.\n" );

				return( EXIT_FAILURE );
			}
			if( ewfstring_copy_libewf_char_from_char_t( evidence_number, option_evidence_number, string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set evidence number string.\n" );

				return( EXIT_FAILURE );
			}
		}
	}
	if( option_notes != NULL )
	{
		string_length = CHAR_T_LENGTH( option_notes );

		if( string_length > 0 )
		{
			string_length += 1;
			notes          = (libewf_char_t *) libewf_common_alloc(
			                                    sizeof( libewf_char_t ) * string_length );

			if( notes == NULL )
			{
				fprintf( stderr, "Unable to create notes string.\n" );

				return( EXIT_FAILURE );
			}
			if( ewfstring_copy_libewf_char_from_char_t( notes, option_notes, string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set notes string.\n" );

				return( EXIT_FAILURE );
			}
		}
	}
	acquiry_operating_system = ewfcommon_determine_operating_system();
	acquiry_software_version = LIBEWF_VERSION;

	fprintf( stdout, "Using the following acquiry parameters:\n" );

	ewfoutput_acquiry_parameters_fprint(
	 stdout,
	 filenames[ 0 ],
	 case_number,
	 description,
	 evidence_number,
	 examiner_name,
	 notes,
	 media_type,
	 volume_type,
	 compression_level,
	 compress_empty_block,
	 libewf_format,
	 (off64_t) acquiry_offset,
	 (size64_t) acquiry_size,
	 (size64_t) segment_file_size,
	 (uint32_t) sectors_per_chunk,
	 sector_error_granularity,
	 read_error_retry,
	 wipe_chunk_on_error );

	handle = libewf_open( (CHAR_T * const *) filenames, 1, LIBEWF_OPEN_WRITE );

	if( handle == NULL )
	{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
		if( errno != 0 )
		{
			error_string = ewfstring_strerror( errno );
		}
		if( error_string != NULL )
		{
			fprintf( stderr, "Unable to open EWF file(s) with failure: %" PRIs ".\n",
			 error_string );

			libewf_common_free( error_string );
		}
		else
		{
			fprintf( stderr, "Unable to create EWF file(s).\n" );
		}
#else
		fprintf( stderr, "Unable to create EWF file(s).\n" );
#endif

		return( EXIT_FAILURE );
	}
	if( case_number == NULL )
	{
		string_length = 0;
	}
	else
	{
		string_length = libewf_string_length( case_number );
	}
	if( libewf_set_header_value_case_number(
	     handle,
	     case_number,
	     string_length ) != 1 )
	{
		fprintf( stderr, "Unable to set header value case number in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	libewf_common_free( case_number );

	if( description == NULL )
	{
		string_length = 0;
	}
	else
	{
		string_length = libewf_string_length( description );
	}
	if( libewf_set_header_value_description(
	     handle,
	     description,
	     string_length ) != 1 )
	{
		fprintf( stderr, "Unable to set header value description in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	libewf_common_free( description );

	if( examiner_name == NULL )
	{
		string_length = 0;
	}
	else
	{
		string_length = libewf_string_length( examiner_name );
	}
	if( libewf_set_header_value_examiner_name(
	     handle,
	     examiner_name,
	     string_length ) != 1 )
	{
		fprintf( stderr, "Unable to set header value examiner name in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	libewf_common_free( examiner_name );

	if( evidence_number == NULL )
	{
		string_length = 0;
	}
	else
	{
		string_length = libewf_string_length( evidence_number );
	}
	if( libewf_set_header_value_evidence_number(
	     handle,
	     evidence_number,
	     string_length ) != 1 )
	{
		fprintf( stderr, "Unable to set header value evidence number in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	libewf_common_free( evidence_number );

	if( notes == NULL )
	{
		string_length = 0;
	}
	else
	{
		string_length = libewf_string_length( notes );
	}
	if( libewf_set_header_value_notes(
	     handle,
	     notes,
	     string_length ) != 1 )
	{
		fprintf( stderr, "Unable to set header value notes in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	libewf_common_free( notes );

	/* Acquiry date, system date and compression type will be generated automatically when set to NULL
	 */
	if( acquiry_operating_system != NULL )
	{
		if( libewf_set_header_value_acquiry_operating_system(
		     handle,
		     acquiry_operating_system,
		     libewf_string_length( acquiry_operating_system ) ) != 1 )
		{
			fprintf( stderr, "Unable to set header value acquiry operating system in handle.\n" );

			if( libewf_close( handle ) != 0 )
			{
				fprintf( stderr, "Unable to close EWF file(s).\n" );
			}
			return( EXIT_FAILURE );
		}
		libewf_common_free( acquiry_operating_system );
	}
	if( libewf_set_header_value(
	     handle,
	     _S_LIBEWF_CHAR( "acquiry_software" ),
	     program,
	     16 ) != 1 )
	{
		fprintf( stderr, "Unable to set header value acquiry software in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	if( libewf_set_header_value_acquiry_software_version(
	     handle,
	     acquiry_software_version,
	     libewf_string_length( acquiry_software_version ) ) != 1 )
	{
		fprintf( stderr, "Unable to set header value acquiry software version number in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	/* Format needs to be set before segment file size
	 */
	if( libewf_set_format(
	     handle,
	     libewf_format ) != 1 )
	{
		fprintf( stderr, "Unable to set format in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	if( libewf_set_segment_file_size(
	     handle,
	     (size64_t) segment_file_size ) != 1 )
	{
		fprintf( stderr, "Unable to set segment file size in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	if( libewf_set_media_type(
	     handle,
	     media_type ) != 1 )
	{
		fprintf( stderr, "Unable to set media type in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	if( libewf_set_volume_type(
	     handle,
	     volume_type ) != 1 )
	{
		fprintf( stderr, "Unable to set volume type in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	if( libewf_set_compression_values(
	     handle,
	     compression_level,
	     compress_empty_block ) != 1 )
	{
		fprintf( stderr, "Unable to set compression values in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
#if defined(HAVE_UUID_UUID_H) && defined(HAVE_LIBUUID)
	/* Add a system GUID if necessary
	 */
	if( ewfcommon_determine_guid(
	     guid,
	     libewf_format ) != 1 )
	{
		fprintf( stderr, "Unable to create GUID.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
	if( libewf_set_guid(
	     handle,
	     guid,
	     16 ) != 1 )
	{
		fprintf( stderr, "Unable to set GUID in handle.\n" );

		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		return( EXIT_FAILURE );
	}
#endif
	if( calculate_md5 == 1 )
	{
		calculated_md5_hash_string = (libewf_char_t *) libewf_common_alloc(
		                                                sizeof( libewf_char_t ) * EWFSTRING_DIGEST_HASH_LENGTH_MD5 );

		if( calculated_md5_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated MD5 hash string.\n" );

			if( libewf_close( handle ) != 0 )
			{
				fprintf( stderr, "Unable to close EWF file(s).\n" );
			}
			return( EXIT_FAILURE );
		}
	}
	if( calculate_sha1 == 1 )
	{
		calculated_sha1_hash_string = (libewf_char_t *) libewf_common_alloc(
		                                                 sizeof( libewf_char_t ) * EWFSTRING_DIGEST_HASH_LENGTH_SHA1 );

		if( calculated_sha1_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated SHA1 hash string.\n" );

			libewf_common_free( calculated_md5_hash_string );

			if( libewf_close( handle ) != 0 )
			{
				fprintf( stderr, "Unable to close EWF file(s).\n" );
			}
			return( EXIT_FAILURE );
		}
	}
	/* Start acquiring data
	 */
	timestamp_start = time( NULL );
	time_string     = libewf_common_ctime( &timestamp_start );

	if( time_string != NULL )
	{
		fprintf( stdout, "Acquiry started at: %" PRIs "\n", time_string );

		libewf_common_free( time_string );
	}
	else
	{
		fprintf( stdout, "Acquiry started.\n" );
	}
	if( callback != NULL )
	{
		ewfoutput_process_status_initialize( stdout, _S_LIBEWF_CHAR( "acquired" ), timestamp_start );
	}
	fprintf( stdout, "This could take a while.\n\n" );

	write_count = ewfcommon_write_from_file_descriptor(
	               handle,
	               0,
	               acquiry_size,
	               acquiry_offset,
	               (uint32_t) sectors_per_chunk,
	               512,
	               read_error_retry,
	               sector_error_granularity,
	               wipe_chunk_on_error,
	               seek_on_error,
	               calculate_md5,
	               calculated_md5_hash_string,
	               EWFSTRING_DIGEST_HASH_LENGTH_MD5,
	               calculate_sha1,
	               calculated_sha1_hash_string,
	               EWFSTRING_DIGEST_HASH_LENGTH_SHA1,
	               swap_byte_pairs,
	               callback );

	/* Done acquiring data
	 */
	timestamp_end = time( NULL );
	time_string   = libewf_common_ctime( &timestamp_end );

	if( write_count <= -1 )
	{
		if( time_string != NULL )
		{
			fprintf( stdout, "Acquiry failed at: %" PRIs "\n", time_string );

			libewf_common_free( time_string );
		}
		else
		{
			fprintf( stdout, "Acquiry failed.\n" );
		}
		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		if( calculate_md5 == 1 )
		{
			libewf_common_free( calculated_md5_hash_string );
		}
		if( calculate_sha1 == 1 )
		{
			libewf_common_free( calculated_sha1_hash_string );
		}
		return( EXIT_FAILURE );
	}
	if( time_string != NULL )
	{
		fprintf( stdout, "Acquiry completed at: %" PRIs "\n", time_string );

		libewf_common_free( time_string );
	}
	else
	{
		fprintf( stdout, "Acquiry completed.\n" );
	}
	ewfoutput_process_summary_fprint( stdout, _S_LIBEWF_CHAR( "Written" ), write_count, timestamp_start, timestamp_end );

	fprintf( stdout, "\n" );

	ewfoutput_acquiry_errors_fprint( stdout, handle );

	if( libewf_close( handle ) != 0 )
	{
		fprintf( stderr, "Unable to close EWF file(s).\n" );

		if( calculate_md5 == 1 )
		{
			libewf_common_free( calculated_md5_hash_string );
		}
		if( calculate_sha1 == 1 )
		{
			libewf_common_free( calculated_sha1_hash_string );
		}
		return( EXIT_FAILURE );
	}
	if( calculate_md5 == 1 )
	{
		fprintf( stdout, "MD5 hash calculated over data: %" PRIs_EWF "\n", calculated_md5_hash_string );

		libewf_common_free( calculated_md5_hash_string );
	}
	if( calculate_sha1 == 1 )
	{
		fprintf( stdout, "SHA1 hash calculated over data:\t%" PRIs_EWF "\n", calculated_sha1_hash_string );

		libewf_common_free( calculated_sha1_hash_string );
	}
	return( EXIT_SUCCESS );
}

