/*****************************************************************************
 * box_default.h
 *****************************************************************************
 * Copyright (C) 2017 L-SMASH project
 *
 * Authors: Yusuke Nakamura <muken.the.vfrmaniac@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *****************************************************************************/

/* This file is available under an ISC license. */

#define ALLOCATE_BOX( box_name ) \
    (isom_##box_name##_t *)allocate_box_by_default( &isom_##box_name##_box_default, \
                                                    sizeof(isom_##box_name##_box_default) )

#define  DEFINE_BOX_DEFAULT_CONSTANT( box_name )                            \
    extern const isom_##box_name##_t isom_##box_name##_box_default;         \
    static inline isom_##box_name##_t *isom_non_existing_##box_name( void ) \
    {                                                                       \
        return (isom_##box_name##_t *)&isom_##box_name##_box_default;       \
    }

DEFINE_BOX_DEFAULT_CONSTANT( dummy )
DEFINE_BOX_DEFAULT_CONSTANT( unknown )
DEFINE_BOX_DEFAULT_CONSTANT( ftyp )
DEFINE_BOX_DEFAULT_CONSTANT( ctab )
DEFINE_BOX_DEFAULT_CONSTANT( tkhd )
DEFINE_BOX_DEFAULT_CONSTANT( clef )
DEFINE_BOX_DEFAULT_CONSTANT( prof )
DEFINE_BOX_DEFAULT_CONSTANT( enof )
DEFINE_BOX_DEFAULT_CONSTANT( tapt )
DEFINE_BOX_DEFAULT_CONSTANT( elst )
DEFINE_BOX_DEFAULT_CONSTANT( edts )
DEFINE_BOX_DEFAULT_CONSTANT( tref_type )
DEFINE_BOX_DEFAULT_CONSTANT( tref )
DEFINE_BOX_DEFAULT_CONSTANT( mdhd )
DEFINE_BOX_DEFAULT_CONSTANT( hdlr )
DEFINE_BOX_DEFAULT_CONSTANT( vmhd )
DEFINE_BOX_DEFAULT_CONSTANT( smhd )
DEFINE_BOX_DEFAULT_CONSTANT( hmhd )
DEFINE_BOX_DEFAULT_CONSTANT( nmhd )
DEFINE_BOX_DEFAULT_CONSTANT( gmin )
DEFINE_BOX_DEFAULT_CONSTANT( text )
DEFINE_BOX_DEFAULT_CONSTANT( gmhd )
DEFINE_BOX_DEFAULT_CONSTANT( dref_entry )
DEFINE_BOX_DEFAULT_CONSTANT( dref )
DEFINE_BOX_DEFAULT_CONSTANT( dinf )
DEFINE_BOX_DEFAULT_CONSTANT( esds )
DEFINE_BOX_DEFAULT_CONSTANT( dovi )
DEFINE_BOX_DEFAULT_CONSTANT( btrt )
DEFINE_BOX_DEFAULT_CONSTANT( glbl )
DEFINE_BOX_DEFAULT_CONSTANT( clap )
DEFINE_BOX_DEFAULT_CONSTANT( pasp )
DEFINE_BOX_DEFAULT_CONSTANT( colr )
DEFINE_BOX_DEFAULT_CONSTANT( gama )
DEFINE_BOX_DEFAULT_CONSTANT( fiel )
DEFINE_BOX_DEFAULT_CONSTANT( clli )
DEFINE_BOX_DEFAULT_CONSTANT( mdcv )
DEFINE_BOX_DEFAULT_CONSTANT( cspc )
DEFINE_BOX_DEFAULT_CONSTANT( sgbt )
DEFINE_BOX_DEFAULT_CONSTANT( stsl )
DEFINE_BOX_DEFAULT_CONSTANT( sample_entry )
DEFINE_BOX_DEFAULT_CONSTANT( mp4s_entry )
DEFINE_BOX_DEFAULT_CONSTANT( visual_entry )
DEFINE_BOX_DEFAULT_CONSTANT( frma )
DEFINE_BOX_DEFAULT_CONSTANT( enda )
DEFINE_BOX_DEFAULT_CONSTANT( mp4a )
DEFINE_BOX_DEFAULT_CONSTANT( terminator )
DEFINE_BOX_DEFAULT_CONSTANT( wave )
DEFINE_BOX_DEFAULT_CONSTANT( chan )
DEFINE_BOX_DEFAULT_CONSTANT( srat )
DEFINE_BOX_DEFAULT_CONSTANT( audio_entry )
DEFINE_BOX_DEFAULT_CONSTANT( tims )
DEFINE_BOX_DEFAULT_CONSTANT( tsro )
DEFINE_BOX_DEFAULT_CONSTANT( tssy )
DEFINE_BOX_DEFAULT_CONSTANT( hint_entry )
DEFINE_BOX_DEFAULT_CONSTANT( metadata_entry )
DEFINE_BOX_DEFAULT_CONSTANT( qt_text_entry )
DEFINE_BOX_DEFAULT_CONSTANT( ftab )
DEFINE_BOX_DEFAULT_CONSTANT( tx3g_entry )
DEFINE_BOX_DEFAULT_CONSTANT( SA3D )
DEFINE_BOX_DEFAULT_CONSTANT( st3d )
DEFINE_BOX_DEFAULT_CONSTANT( sv3d )
DEFINE_BOX_DEFAULT_CONSTANT( svhd )
DEFINE_BOX_DEFAULT_CONSTANT( proj )
DEFINE_BOX_DEFAULT_CONSTANT( prhd )
DEFINE_BOX_DEFAULT_CONSTANT( equi )
DEFINE_BOX_DEFAULT_CONSTANT( cbmp )
DEFINE_BOX_DEFAULT_CONSTANT( vexu )
DEFINE_BOX_DEFAULT_CONSTANT( eyes )
DEFINE_BOX_DEFAULT_CONSTANT( must )
DEFINE_BOX_DEFAULT_CONSTANT( stri )
DEFINE_BOX_DEFAULT_CONSTANT( hero )
DEFINE_BOX_DEFAULT_CONSTANT( stsd )
DEFINE_BOX_DEFAULT_CONSTANT( stts )
DEFINE_BOX_DEFAULT_CONSTANT( ctts )
DEFINE_BOX_DEFAULT_CONSTANT( cslg )
DEFINE_BOX_DEFAULT_CONSTANT( stsz )
DEFINE_BOX_DEFAULT_CONSTANT( stz2 )
DEFINE_BOX_DEFAULT_CONSTANT( stss )
DEFINE_BOX_DEFAULT_CONSTANT( stps )
DEFINE_BOX_DEFAULT_CONSTANT( sdtp )
DEFINE_BOX_DEFAULT_CONSTANT( stsc )
DEFINE_BOX_DEFAULT_CONSTANT( stco )
DEFINE_BOX_DEFAULT_CONSTANT( sgpd )
DEFINE_BOX_DEFAULT_CONSTANT( sbgp )
DEFINE_BOX_DEFAULT_CONSTANT( stbl )
DEFINE_BOX_DEFAULT_CONSTANT( minf )
DEFINE_BOX_DEFAULT_CONSTANT( mdia )
DEFINE_BOX_DEFAULT_CONSTANT( mvhd )
DEFINE_BOX_DEFAULT_CONSTANT( iods )
DEFINE_BOX_DEFAULT_CONSTANT( mdat )
DEFINE_BOX_DEFAULT_CONSTANT( skip )
DEFINE_BOX_DEFAULT_CONSTANT( chpl )
DEFINE_BOX_DEFAULT_CONSTANT( keys )
DEFINE_BOX_DEFAULT_CONSTANT( mean )
DEFINE_BOX_DEFAULT_CONSTANT( name )
DEFINE_BOX_DEFAULT_CONSTANT( data )
DEFINE_BOX_DEFAULT_CONSTANT( metaitem )
DEFINE_BOX_DEFAULT_CONSTANT( ilst )
DEFINE_BOX_DEFAULT_CONSTANT( meta )
DEFINE_BOX_DEFAULT_CONSTANT( WLOC )
DEFINE_BOX_DEFAULT_CONSTANT( LOOP )
DEFINE_BOX_DEFAULT_CONSTANT( SelO )
DEFINE_BOX_DEFAULT_CONSTANT( AllF )
DEFINE_BOX_DEFAULT_CONSTANT( hnti )
DEFINE_BOX_DEFAULT_CONSTANT( rtp )
DEFINE_BOX_DEFAULT_CONSTANT( sdp )
DEFINE_BOX_DEFAULT_CONSTANT( cprt )
DEFINE_BOX_DEFAULT_CONSTANT( udta )
DEFINE_BOX_DEFAULT_CONSTANT( mehd )
DEFINE_BOX_DEFAULT_CONSTANT( trex )
DEFINE_BOX_DEFAULT_CONSTANT( mvex )
DEFINE_BOX_DEFAULT_CONSTANT( mfhd )
DEFINE_BOX_DEFAULT_CONSTANT( tfhd )
DEFINE_BOX_DEFAULT_CONSTANT( tfdt )
DEFINE_BOX_DEFAULT_CONSTANT( trun )
DEFINE_BOX_DEFAULT_CONSTANT( traf )
DEFINE_BOX_DEFAULT_CONSTANT( moof )
DEFINE_BOX_DEFAULT_CONSTANT( tfra )
DEFINE_BOX_DEFAULT_CONSTANT( mfro )
DEFINE_BOX_DEFAULT_CONSTANT( mfra )
DEFINE_BOX_DEFAULT_CONSTANT( trak )
DEFINE_BOX_DEFAULT_CONSTANT( moov )
DEFINE_BOX_DEFAULT_CONSTANT( styp )
DEFINE_BOX_DEFAULT_CONSTANT( sidx )
DEFINE_BOX_DEFAULT_CONSTANT( emsg )
DEFINE_BOX_DEFAULT_CONSTANT( file_abstract )
DEFINE_BOX_DEFAULT_CONSTANT( root_abstract )

void *allocate_box_by_default
(
    const void  *nonexist_ptr,
    const size_t data_type_size
);
