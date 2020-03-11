/*
 * Checksum functions
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
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined( _LIBEWF_CHECKSUM_H )
#define _LIBEWF_CHECKSUM_H

#include <common.h>
#include <types.h>

#include "libewf_deflate.h"
#include "libewf_libcerror.h"

#if defined( __cplusplus )
extern "C" {
#endif

#if defined( HAVE_ADLER32 ) && ( defined( HAVE_ZLIB ) || defined( ZLIB_DLL ) )

int libewf_checksum_calculate_adler32(
     uint32_t *checksum_value,
     const uint8_t *buffer,
     size_t size,
     uint32_t initial_value,
     libcerror_error_t **error );

#else
#define libewf_checksum_calculate_adler32( checksum_value, buffer, size, initial_value, error ) \
	libewf_deflate_calculate_adler32( checksum_value, buffer, size, initial_value, error )

#endif /* defined( HAVE_ADLER32 ) && ( defined( HAVE_ZLIB ) || defined( ZLIB_DLL ) ) */

#if defined( __cplusplus )
}
#endif

#endif /* !defined( _LIBEWF_CHECKSUM_H ) */

