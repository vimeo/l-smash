/*****************************************************************************
 * av1_obu.c
 *****************************************************************************
 * Copyright (C) 2020 L-SMASH Project
 *
 * Authors: Derek Buitenhuis <derek.buitenhuis@gmail.com>
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

#include "common/internal.h" /* must be placed first */

#include "codecs/av1.h"

#include <inttypes.h>
#include <string.h>

#define OBU_SEQUENCE_HEADER 1
#define OBU_TEMPORAL_DELIMITER 2
#define OBU_FRAME_HEADER 3
#define OBU_TILE_GROUP 4
#define OBU_METADATA 5
#define OBU_FRAME 6
#define OBU_REDUNDANT_FRAME_HEADER 7
#define OBU_PADDING 15

/* GIANT HACK BECAUSE I WAS LAZY - CONVERT TO BS */
static uint64_t obu_av1_leb128_buf
(
    uint8_t *buf,
    uint32_t *consumed
)
{
    uint64_t value = 0;
    *consumed = 0;

    for( uint64_t i = 0; i < 8; i++ )
    {
        uint8_t b = buf[*consumed];
        value |= ((uint64_t)(b & 0x7F)) << (i * 7);
        (*consumed)++;
        if ((b & 0x80) != 0x80)
            break;
    }

    return value;
}

static uint64_t obu_av1_leb128
(
    lsmash_bs_t *bs,
    uint32_t offset,
    uint32_t *consumed
)
{
    uint64_t value = 0;
    *consumed = 0;

    for( uint64_t i = 0; i < 8; i++ )
    {
        uint8_t b = lsmash_bs_show_byte( bs, offset + (*consumed) );
        value |= ((uint64_t)(b & 0x7F)) << (i * 7);
        (*consumed)++;
        if ((b & 0x80) != 0x80)
            break;
    }

    return value;
}

static uint32_t obu_av1_vlc
(
    lsmash_bits_t *bits
)
{
    uint32_t leadingzeroes = 0;
    while( leadingzeroes < 32 )
    {
        int b = (int) lsmash_bits_get( bits, 1 );
        if( b != 0 )
            break;
        leadingzeroes++;
    }
    assert( leadingzeroes != 32 ); /* lazy... */
    uint32_t val = lsmash_bits_get( bits, leadingzeroes );
    return val + ((1 << leadingzeroes) - 1);
}

static void obu_parse_color_config( lsmash_bits_t *bits, lsmash_av1_specific_parameters_t *param )
{
    int BitDepth = 8;

    param->high_bitdepth = (uint8_t) lsmash_bits_get( bits, 1 );
    if( param->seq_profile == 2 && param->high_bitdepth )
    {
        param->twelve_bit = (uint8_t) lsmash_bits_get( bits, 1 );
        BitDepth = param->twelve_bit ? 12 : 10;
    }
    else if( param->seq_profile <= 2)
    {
        BitDepth = param->high_bitdepth ? 10 : 8;
    }
    if( param->seq_profile == 1 )
        param->monochrome = 0;
    else
        param->monochrome = (uint8_t) lsmash_bits_get( bits, 1 );
    /* NumPlanes = param->monochrome ? 1 : 3; */
    int color_description_present_flag = (int) lsmash_bits_get( bits, 1 );
    uint8_t color_primaries;
    uint8_t transfer_characteristics;
    uint8_t matrix_coefficients;
    if( color_description_present_flag )
    {
        color_primaries = (int) lsmash_bits_get( bits, 8 );
        transfer_characteristics = (int) lsmash_bits_get( bits, 8 );
        matrix_coefficients = (int) lsmash_bits_get( bits, 8 );
    }
    else
    {
        /* Unspecified */
        color_primaries = 2;
        transfer_characteristics = 2;
        matrix_coefficients = 2;
    }
    if( param->monochrome )
    {
        /* color_range */
        lsmash_bits_get( bits, 1 );
        param->chroma_subsampling_x = 1;
        param->chroma_subsampling_y = 1;
        param->chroma_sample_position = LSMASH_AV1_CSP_UNKNOWN;
        /* separate_uv_delta_q = 0 */
        return;
    }
    else if( color_primaries == 1 && transfer_characteristics == 13 && matrix_coefficients == 0 )
    {
        /* color_range = 1 */
        param->chroma_subsampling_x = 0;
        param->chroma_subsampling_y = 0;
    }
    else
    {
        /* color_range */
        lsmash_bits_get( bits, 1 );
        if( param->seq_profile == 0 )
        {
            param->chroma_subsampling_x = 1;
            param->chroma_subsampling_y = 1;
        }
        else if( param->seq_profile == 1 )
        {
            param->chroma_subsampling_x = 0;
            param->chroma_subsampling_y = 0;
        }
        else
        {
            if( BitDepth == 12 )
            {
                param->chroma_subsampling_x = lsmash_bits_get( bits, 1 );
                if( param->chroma_subsampling_x )
                    param->chroma_subsampling_y = lsmash_bits_get( bits, 1 );
                else
                    param->chroma_subsampling_y = 0;
            }
            else
            {
                param->chroma_subsampling_x = 1;
                param->chroma_subsampling_y = 0;
            }
        }
        if( param->chroma_subsampling_x && param->chroma_subsampling_y )
            param->chroma_sample_position =  lsmash_bits_get( bits, 2 );
    }
    /* separate_uv_delta_q */
}

static int obu_parse_seq_header( uint8_t *obubuf, uint32_t obusize, lsmash_av1_specific_parameters_t *param )
{
    lsmash_bits_t *bits = lsmash_bits_adhoc_create();
    if( !bits )
        return -1;

    int ret = lsmash_bits_import_data( bits, obubuf, obusize );
    if( ret < 0 )
    {
        lsmash_bits_adhoc_cleanup( bits );
        return -1;
    }

    int decoder_model_info_present_flag = 0;
    int buffer_delay_length_minus_1 = 0;

    param->seq_profile = (uint8_t) lsmash_bits_get( bits, 3 );

    int still_picture = (int) lsmash_bits_get( bits, 1 );
    int reduced_still_picture_header = (int) lsmash_bits_get( bits, 1 );
    if( still_picture || reduced_still_picture_header )
    {
        lsmash_bits_adhoc_cleanup( bits );
        assert(0);
        return -1;
    }

    /* Skip timing info. */
    int timing_info_present_flag = lsmash_bits_get( bits, 1 );
    if( timing_info_present_flag )
    {
        /* num_units_in_display_tick */
        lsmash_bits_get( bits, 16 );
        lsmash_bits_get( bits, 16 );
        /* time_scale */
        lsmash_bits_get( bits, 16 );
        lsmash_bits_get( bits, 16 );
        int equal_picture_interval = (int) lsmash_bits_get( bits, 1 );
        if( equal_picture_interval )
            obu_av1_vlc(bits);
        decoder_model_info_present_flag = (int) lsmash_bits_get( bits, 1 );
        if( decoder_model_info_present_flag ) {
            buffer_delay_length_minus_1 = (int) lsmash_bits_get( bits, 5 );
            /* num_units_in_decoding_tick */
            lsmash_bits_get( bits, 16 );
            lsmash_bits_get( bits, 16 );
            /* buffer_removal_time_length_minus_1 */
            lsmash_bits_get( bits, 5 );
            /* frame_presentation_time_length_minus_1 */
            lsmash_bits_get( bits, 5 );
        }
    }
    param->initial_presentation_delay_present = (uint8_t) lsmash_bits_get( bits, 1 );
    int operating_points_cnt_minus_1 = (int) lsmash_bits_get( bits, 5 );
    for( int i = 0; i <= operating_points_cnt_minus_1; i++ )
    {
        /* operating_point_idc */
        lsmash_bits_get( bits, 12 );
        uint8_t seq_level_idx = (uint8_t) lsmash_bits_get( bits, 5 );
        if( i == 0 )
            param->seq_level_idx_0 = seq_level_idx;
        if( seq_level_idx > 7 )
        {
            uint8_t seq_tier = (uint8_t) lsmash_bits_get( bits, 1 );
            if( i == 0 )
                param->seq_tier_0 = seq_tier;
        }
        if( decoder_model_info_present_flag )
        {
            int decoder_model_present_for_this_op = (int) lsmash_bits_get( bits, 1 );
            if( decoder_model_present_for_this_op )
            {
                /* decoder_buffer_delay */
                lsmash_bits_get( bits, buffer_delay_length_minus_1 + 1 );
                /* encoder_buffer_delay */
                lsmash_bits_get( bits, buffer_delay_length_minus_1 + 1 );
                /* low_delay_mode_flag */
                lsmash_bits_get( bits, 1 );
            }
        }
    }
    if( param->initial_presentation_delay_present )
    {
        int initial_display_delay_present_for_this_op = (int) lsmash_bits_get( bits, 1 );
        if( initial_display_delay_present_for_this_op )
        {
            param->initial_presentation_delay_minus_one = (uint8_t) lsmash_bits_get( bits, 4 );
        }
    }
    int frame_width_bits_minus_1 = (int) lsmash_bits_get( bits, 4 );
    int frame_height_bits_minus_1 = (int) lsmash_bits_get( bits, 4 );
    /* max_frame_width_minus_1 */
    lsmash_bits_get( bits, frame_width_bits_minus_1 + 1 );
    /* max_frame_height_minus_1 */
    lsmash_bits_get( bits, frame_height_bits_minus_1 + 1 );

    /* Always read because we check for reduced_still_picture_header already */
    int frame_id_numbers_present_flag = (int) lsmash_bits_get( bits, 1 );
    if( frame_id_numbers_present_flag )
    {
        /* delta_frame_id_length_minus_2 */
        lsmash_bits_get( bits, 4 );
        /* additional_frame_id_length_minus_1 */
        lsmash_bits_get( bits, 3 );
    }

    /* use_128x128_superblock */
    lsmash_bits_get( bits, 1 );
    /* enable_filter_intra */
    lsmash_bits_get( bits, 1 );
    /* enable_intra_edge_filter */
    lsmash_bits_get( bits, 1 );

    /* Ditto to above */
    /* enable_interintra_compound */
    lsmash_bits_get( bits, 1 );
    /* enable_masked_compound */
    lsmash_bits_get( bits, 1 );
    /* enable_warped_motion */
    lsmash_bits_get( bits, 1 );
    /* enable_dual_filter */
    lsmash_bits_get( bits, 1 );
    int enable_order_hint = (int) lsmash_bits_get( bits, 1 );
    if( enable_order_hint )
    {
        /* enable_jnt_comp */
        lsmash_bits_get( bits, 1 );
        /* enable_jnt_comp */
        lsmash_bits_get( bits, 1 );
    }
    int seq_choose_screen_content_tools = (int) lsmash_bits_get( bits, 1 );
    int seq_force_screen_content_tools = 0;
    if( seq_choose_screen_content_tools )
        seq_force_screen_content_tools = 2;
    else
        seq_force_screen_content_tools = (int) lsmash_bits_get( bits, 1 );
    if( seq_force_screen_content_tools > 0 )
    {
        int seq_choose_integer_mv = (int) lsmash_bits_get( bits, 1 );
        if( seq_choose_integer_mv )
        {
            /* seq_force_integer_mv */
             lsmash_bits_get( bits, 1 );
        }
    }
    if( enable_order_hint )
    {
        /* order_hint_bits_minus_1 */
        lsmash_bits_get( bits, 3 );
    }

    /* enable_superres */
    lsmash_bits_get( bits, 1 );
    /* enable_cdef */
    lsmash_bits_get( bits, 1 );
    /* enable_restoration */
    lsmash_bits_get( bits, 1 );

    obu_parse_color_config( bits, param );

    /* film_grain_params_present */

    lsmash_bits_adhoc_cleanup( bits );

    return 0;
}

lsmash_av1_specific_parameters_t *obu_av1_parse_seq_header
(
    lsmash_bs_t *bs,
    uint32_t length,
    uint32_t offset
)
{
    lsmash_av1_specific_parameters_t *param = lsmash_malloc_zero(sizeof(lsmash_av1_specific_parameters_t));
    if ( !param )
        return NULL;

    uint32_t off = 0;

    while( off < length )
    {
        uint8_t temp8 = lsmash_bs_show_byte( bs, off + offset );
        uint8_t obutype = (temp8 & 0x78) >> 3;
        int extension = (((temp8 & 0x04) >> 2) == 1);
        int hassize = (((temp8 & 0x02) >> 1) == 1);

        off++;
        if( extension )
            off++;
        if( !hassize )
            continue;

        uint32_t consumed = 0;
        uint64_t obusize = obu_av1_leb128( bs, off + offset, &consumed );
        off += consumed;
        assert ( obusize <= UINT32_MAX );

        switch( obutype )
        {
            case OBU_SEQUENCE_HEADER:
            {
                uint32_t headersize = consumed + extension + 1;
                uint8_t *obubuf = lsmash_malloc( obusize + headersize );
                if( !obubuf ) {
                    av1_destruct_specific_data( param );
                    return NULL;
                }
                for( uint32_t i = 0; i < obusize + headersize; i++ )
                    obubuf[i] = lsmash_bs_show_byte( bs, off + offset + i - headersize );
                off += obusize;

                int ret = obu_parse_seq_header( obubuf + headersize, obusize, param );
                if( ret < 0 ) {
                    lsmash_free(obubuf);
                    av1_destruct_specific_data( param );
                    return NULL;
                }

                uint32_t oldpos = param->configOBUs.sz;
                param->configOBUs.sz += obusize + headersize;
                uint8_t *newdata = lsmash_realloc( param->configOBUs.data, param->configOBUs.sz );
                if( !newdata ) {
                    lsmash_free(obubuf);
                    av1_destruct_specific_data( param );
                    return NULL;
                }
                param->configOBUs.data = newdata;
                memcpy( param->configOBUs.data + oldpos, obubuf, obusize + headersize );
                lsmash_free( obubuf );

                break;
            }
            case OBU_METADATA:
                printf("TODO: NEED TO COPY THIS INTO CONFIGOBUS\n");
                assert(0);
            default:
                off += obusize;
                break;
        }
    }

    return param;
}

static int include_obu(uint8_t obutype)
{
    return obutype == OBU_SEQUENCE_HEADER ||
           obutype == OBU_FRAME_HEADER ||
           obutype == OBU_TILE_GROUP ||
           obutype == OBU_METADATA ||
           obutype == OBU_FRAME;
}

static int obu_av1_parse_uncompressed_frame_type
(
    uint8_t *obubuf,
    uint32_t obusize
)
{
    lsmash_bits_t *bits = lsmash_bits_adhoc_create();
    if( !bits )
        return 0;

    int ret = lsmash_bits_import_data( bits, obubuf, obusize );
    if( ret < 0 )
    {
        lsmash_bits_adhoc_cleanup( bits );
        return 0;
    }

    /* SeenFrameHeader should be zero once we call this func. */
    /* uncompressed_header() */
    /* reduced_still_picture_header will be zero because we fail if we spot it anyway... HACK. */

    int show_existing_frame = lsmash_bits_get( bits, 1 );
    if( !show_existing_frame )
    {
        int frame_type = lsmash_bits_get( bits, 2 );
        lsmash_bits_adhoc_cleanup( bits );
        return ( frame_type == 0 ); /* KEY_FRAME */
    }

    lsmash_bits_adhoc_cleanup( bits );

    return 0;
}

uint8_t *obu_av1_assemble_sample
(
    uint8_t *packetbuf,
    uint32_t length,
    uint32_t *samplelength,
    int *issync
)
{
    uint8_t *samplebuf = NULL;
    *samplelength = 0;
    *issync = 0;
    uint32_t offset = 0;
    int seen_seq_header = 0;

    while( offset < length )
    {
        uint8_t temp8 = packetbuf[offset];
        uint8_t obutype = (temp8 & 0x78) >> 3;
        int extension = (((temp8 & 0x04) >> 2) == 1);
        int hassize = (((temp8 & 0x02) >> 1) == 1);

        offset++;
        if( extension )
            offset++;
        if( !hassize )
        {
            if( include_obu(obutype) )
            {
                uint8_t *newbuf = lsmash_realloc( samplebuf, (*samplelength) + extension + 1 );
                if( !newbuf )
                {
                    lsmash_free( samplebuf );
                    return NULL;
                }
                samplebuf = newbuf;
                memcpy(samplebuf + (*samplelength), &packetbuf[offset - (extension + 1)], extension + 1);
                (*samplelength) += extension + 1;
            }
            continue;
        }

        uint32_t consumed = 0;
        uint64_t obusize = obu_av1_leb128_buf(&packetbuf[offset], &consumed);
        offset += consumed;

        if( !include_obu(obutype) )
        {
            offset += obusize;
            continue;
        }

        if( obutype == OBU_SEQUENCE_HEADER )
        {
            seen_seq_header = 1;
        }
        else if( obutype == OBU_FRAME_HEADER && seen_seq_header ) /* spec requires sync samples to have the seq header first */
        {
            *issync = obu_av1_parse_uncompressed_frame_type( &packetbuf[offset], obusize );
        }

        uint32_t total_size = consumed + obusize + extension + 1;
        uint8_t *newbuf = lsmash_realloc( samplebuf, (*samplelength) + total_size );
        if( !newbuf )
        {
            lsmash_free( samplebuf );
            return NULL;
        }
        samplebuf = newbuf;
        memcpy(samplebuf + (*samplelength), &packetbuf[offset - consumed - extension - 1], total_size);
        offset += obusize;
        (*samplelength) += total_size;
    }

    return samplebuf;
}
