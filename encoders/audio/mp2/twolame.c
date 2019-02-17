/*****************************************************************************
 * twolame.c : twolame encoding functions
 *****************************************************************************
 * Copyright (C) 2010 Open Broadcast Systems Ltd.
 *
 * Authors: Kieran Kunhya <kieran@kunhya.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 ******************************************************************************/

#include "common/common.h"
#include "encoders/audio/audio.h"
#include <twolame.h>
#include <libavutil/fifo.h>
#include <libavresample/avresample.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>

//
struct historical_int64_s
{
	int64_t current;
	int64_t last;
	int64_t diff;
};

__inline__ void historical_int64_init(struct historical_int64_s *s) { memset(s, 0, sizeof(*s)); }
__inline__ void historical_int64_set(struct historical_int64_s *s, int64_t value) { s->last = s->current; s->current = value; s->diff = s->current - s->last; }
__inline__ int64_t historical_int64_get_diff(struct historical_int64_s *s) { return s->diff; }
__inline__ void historical_int64_printf(struct historical_int64_s *s, char *prefix)
{
	printf("%s: curr: %" PRIi64 " last: %" PRIi64 " diff: %" PRIi64 "\n",
		prefix, s->current, s->last, s->diff);
}
//

#define MP2_AUDIO_BUFFER_SIZE 50000

#define LOCAL_DEBUG 0

#define MODULE "[mp2]: "

static void *start_encoder_mp2( void *ptr )
{
#if LOCAL_DEBUG
    printf("%s()\n", __func__);
#endif

    obe_aud_enc_params_t *enc_params = ptr;
    obe_t *h = enc_params->h;
    obe_encoder_t *encoder = enc_params->encoder;
    obe_output_stream_t *stream = enc_params->stream;
    obe_raw_frame_t *raw_frame;
    obe_coded_frame_t *coded_frame;

    /* Interesting 10.8.5 quirk related to the audio clock occasionally being ahead
     * of the video clock. This translates into 'too much' data in the audio fifo.
     * Because we previously used to put the same timestamp on multiple packets, or overly
     * adjust the PTS, things break.
     * The right approach is to accurately track the timestamp based on the head of the fifo,
     * so enable this feature by default.
     * If this is deemed risky, we could always enable it for 480i and 576i only..... but
     * I think we should go ahead, fully retest and just enable this better time calculation
     * mechanism.
     */
    enc_params->use_fifo_head_timing = 1;

    struct historical_int64_s rf_pts;
    historical_int64_init(&rf_pts);

    struct historical_int64_s avfm_pts;
    historical_int64_init(&avfm_pts);

    struct historical_int64_s cf_pts;
    historical_int64_init(&cf_pts);

#if LOCAL_DEBUG
    printf("%s() output_stream_id = %d\n", __func__, encoder->output_stream_id);
#endif

    twolame_options *tl_opts = NULL;
    int output_size, frame_size, linesize; /* Linesize in libavresample terminology is the entire buffer size for packed formats */
    float *audio_buf = NULL;
    uint8_t *output_buf = NULL;
    AVAudioResampleContext *avr = NULL;
    AVFifoBuffer *fifo = NULL;

    int64_t fifoHeadPTS = 0; /* Keeping track of the audio timestamp every time the fifo is empty. */
    int64_t lastCodedFramePTS = 0; /* The codec is burst. We have to keep tabs on the audio pts and adjust to avoid dup pts. */
    int64_t lastOutputFramePTS = 0; /* Last pts we output, we'll comare against future version to warn for discontinuities. */

int64_t headset_bias = 0;
    /* Lock the mutex until we verify parameters */
    pthread_mutex_lock( &encoder->queue.mutex );

    tl_opts = twolame_init();
    if( !tl_opts )
    {
        fprintf( stderr, "[twolame] could not load options" );
        pthread_mutex_unlock( &encoder->queue.mutex );
        goto end;
    }

    /* TODO: setup bitrate reconfig, errors */
    twolame_set_bitrate( tl_opts, stream->bitrate );
    twolame_set_in_samplerate( tl_opts, enc_params->sample_rate );
    twolame_set_out_samplerate( tl_opts, enc_params->sample_rate );
    twolame_set_copyright( tl_opts, 1 );
    twolame_set_original( tl_opts, 1 );
    twolame_set_num_channels( tl_opts, av_get_channel_layout_nb_channels( stream->channel_layout ) );
    twolame_set_error_protection( tl_opts, 1 );
    if( stream->channel_layout == AV_CH_LAYOUT_STEREO )
        twolame_set_mode( tl_opts, stream->mp2_mode-1 );

    twolame_init_params( tl_opts );

    frame_size = twolame_get_framelength( tl_opts ) * enc_params->frames_per_pes;

    encoder->is_ready = 1;
    /* Broadcast because input and muxer can be stuck waiting for encoder */
    pthread_cond_broadcast( &encoder->queue.in_cv );
    pthread_mutex_unlock( &encoder->queue.mutex );

    output_buf = malloc( MP2_AUDIO_BUFFER_SIZE );
    if( !output_buf )
    {
        fprintf( stderr, "Malloc failed\n" );
        goto end;
    }

    avr = avresample_alloc_context();
    if( !avr )
    {
        fprintf( stderr, "Malloc failed\n" );
        goto end;
    }

    av_opt_set_int( avr, "in_channel_layout",   stream->channel_layout,  0 );
    av_opt_set_int( avr, "in_sample_fmt",       enc_params->input_sample_format, 0 );
    av_opt_set_int( avr, "in_sample_rate",      enc_params->sample_rate, 0 );
    av_opt_set_int( avr, "out_channel_layout",  stream->channel_layout, 0 );
    av_opt_set_int( avr, "out_sample_fmt",      AV_SAMPLE_FMT_FLT,   0 );
    av_opt_set_int( avr, "dither_method",       AV_RESAMPLE_DITHER_TRIANGULAR_NS, 0 );

    if( avresample_open( avr ) < 0 )
    {
        fprintf( stderr, "Could not open AVResample\n" );
        goto end;
    }

    /* Setup the output FIFO */
    fifo = av_fifo_alloc( frame_size );
    if( !fifo )
    {
        fprintf( stderr, "Malloc failed\n" );
        goto end;
    }

    while( 1 )
    {
        pthread_mutex_lock( &encoder->queue.mutex );

        while( !encoder->queue.size && !encoder->cancel_thread )
            pthread_cond_wait( &encoder->queue.in_cv, &encoder->queue.mutex );

        if( encoder->cancel_thread )
        {
            pthread_mutex_unlock( &encoder->queue.mutex );
            break;
        }

        raw_frame = encoder->queue.queue[0];
        pthread_mutex_unlock( &encoder->queue.mutex );

        historical_int64_set(&rf_pts, raw_frame->pts);
        //historical_int64_printf(&rf_pts, "  rf_pts");

#if LOCAL_DEBUG
        /* Send any audio to the AC3 frame slicer.
         * Push the buffer starting at the channel containing bitstream, and span 2 channels,
         * we'll get called back with a completely aligned, crc'd and valid AC3 frame.
         */
        printf("%s() output_stream_id = %d linesize = %d, num_samples = %d, num_channels = %d, sample_fmt = %d, raw_frame->input_stream_id = %d\n",
                __func__,
                encoder->output_stream_id,
                raw_frame->audio_frame.linesize,
                raw_frame->audio_frame.num_samples,
                raw_frame->audio_frame.num_channels,
                raw_frame->audio_frame.sample_fmt,
                raw_frame->input_stream_id);
#endif

        /* Allocate the output buffer */
        if( av_samples_alloc( (uint8_t**)&audio_buf, &linesize, av_get_channel_layout_nb_channels( raw_frame->audio_frame.channel_layout ),
                              raw_frame->audio_frame.linesize, AV_SAMPLE_FMT_FLT, 0 ) < 0 )
        {
            syslog( LOG_ERR, "Malloc failed\n" );
            goto end;
        }

        if( avresample_convert( avr, NULL, 0, raw_frame->audio_frame.num_samples, raw_frame->audio_frame.audio_data,
                                raw_frame->audio_frame.linesize, raw_frame->audio_frame.num_samples ) < 0 )
        {
            syslog( LOG_ERR, "[twolame] Sample format conversion failed\n" );
            break;
        }

        avresample_read( avr, (uint8_t**)&audio_buf, avresample_available( avr ) );

        output_size = twolame_encode_buffer_float32_interleaved( tl_opts, audio_buf, raw_frame->audio_frame.num_samples, output_buf, MP2_AUDIO_BUFFER_SIZE );

        if( output_size < 0 )
        {
            syslog( LOG_ERR, "[twolame] Encode failed\n" );
            break;
        }

        free( audio_buf );
        audio_buf = NULL;

        struct avfm_s avfm;
        memcpy(&avfm, &raw_frame->avfm, sizeof(avfm));

        historical_int64_set(&avfm_pts, avfm.audio_pts);
        //historical_int64_printf(&avfm_pts, "avfm_pts");

        raw_frame->release_data( raw_frame );
        raw_frame->release_frame( raw_frame );
        remove_from_queue( &encoder->queue );

        if( av_fifo_realloc2( fifo, av_fifo_size( fifo ) + output_size ) < 0 )
        {
            syslog( LOG_ERR, "Malloc failed\n" );
            break;
        }

        av_fifo_generic_write( fifo, output_buf, output_size, NULL );

        /* If the fifo is empty, and the compressor created some payload,
         * reset the audio timebase to match the current latest audioframe.
         */
        if (output_size > 0 && av_fifo_size(fifo) >= output_size) {
            fifoHeadPTS = avfm.audio_pts;

/* TODO: Cleanup.
 * The audio hardware clock, on startup with 1080i59.95 video, has one of two values with 10.11.4 firmware.
 * Either the audio clocks is the value zero, or its 1801xxx. Reason unknown.
 * For the time being, fudge a bias into the PTS calculations so the output PTS is ALWAYS
 * in the right place - atleast for this specific standard.
 * Its unclean, we need to understand why the audio clock varies and create a reliable fix.
 */
static int headsets = 0;
if (headsets++ == 0 && fifoHeadPTS > 0) {
    headset_bias  = 27000 * 18; /* Add 18ms */
    headset_bias += (27000 / 2); /* Add .5ms */
    headset_bias += (27000 / 4); /* Add .25ms */
    printf("headset bias = %" PRIi64 ", head %" PRIi64 "\n", headset_bias, fifoHeadPTS);
}
        }

        int framesProcessed = 0;
        while( av_fifo_size( fifo ) >= frame_size )
        {
            coded_frame = new_coded_frame( encoder->output_stream_id, frame_size );
            if( !coded_frame )
            {
                syslog( LOG_ERR, "Malloc failed\n" );
                goto end;
            }
            av_fifo_generic_read( fifo, coded_frame->data, frame_size, NULL );
            memcpy(&coded_frame->avfm, &avfm, sizeof(avfm));

            /* 648000 27MHz ticks equates to 24ms.
             * 24ms is the smallest amount of time this audio codec and eject as compressed data.
             * Hence, lowest latency means we get 24ms PES frames.
             * Hence, normal latency is N * pes frames.
             * OBE itself determines what N should be in various latency modes.
             */

            /* In low latency configurations where the framerate (33ms) is larger than the audio interval (24ms),
             * during audio data wrapping conditions we have to prepare two audio output frames for a single video frame.
             * We cannot output two audio frames with the same PTS, we MUST adjust the audio output rate for the
             * second audio frame. So, for low latency, for second frames, bump the audio pts by 24ms.
             * The most obvious case for this fix is anything with a video framerate of > 24ms. IE. this fix
             * is necessary for in i59.94 i50 30p, 24p etc.
             * In no cases should we ever see more than two audio frames per video frame,
             * for this to be untrue, a video frame would have to be atleast 48ms, which is less and 20.8fps.
             * OBE does not support frames as low as 20.8fps.
             */
            framesProcessed++;
            if (h->obe_system == OBE_SYSTEM_TYPE_LOWEST_LATENCY || h->obe_system == OBE_SYSTEM_TYPE_LOW_LATENCY) {
                if (framesProcessed > 1 && enc_params->frames_per_pes == 1) {
                    if (enc_params->use_fifo_head_timing == 1) 
                        fifoHeadPTS += 648000;
                    else
                        coded_frame->avfm.audio_pts += 648000;
                }
            }

            /* In  low latency mode, obe configures a single MP2 frame in every pes (enc_params->frames_per_pes), with a PTS interval of 24ms.
             * In norm latency mode, obe configures        N MP2 frames in every pes, with a PTS interval of N*24ms.
             * Typically, we see 96MS PTS increments in the transport packets, buts its equally valid to see 24ms.
             * Depending on what latency mode we're in, we need to round to the nearest '24ms or N*' interval.
             * Output PTS packets (27MHz) are rounded to nearest 24ms or N*24ms (typically 96ms for norm lat) interval on a 27MHz clock.
             */
            /* The outgoing PTS is rounded and based on recent h/w clock, so massive leaps in
             * the h/w clock are automatically compensated for.
             * 648000 represents number of ticks in 27Mhz clock for 24ms, the smallest MP2 framesize we can deliver.
             * One problem with taking the latest audio pts and using it regardless, is that it can create PTS gaps
             * of 24 * n ms in poor signal conditions (such as when doing 4-5 frame injections). Instead,
             * we'll make a not of the current audio time when the fifo is empty. The fifo is empty every 3-4 audio
             * raw frames. From this, we'll add N *24ms from that fifoHeadPTS time. This keeps the PTS output
             * at a very reliably 24 * n MS under many more rough signal circumstances.
             */

            /* I think the fifo head timing is a better overall audio timing model, and we should consider moving to it
             * in some future release. For the time beings its opt-in.
             */
            int64_t rounded_pts;
            if (enc_params->use_fifo_head_timing == 0)
                rounded_pts = av_rescale(coded_frame->avfm.audio_pts, OBE_CLOCK, 648000 * enc_params->frames_per_pes);
            else
                rounded_pts = av_rescale(fifoHeadPTS, OBE_CLOCK, 648000 * enc_params->frames_per_pes);

            rounded_pts /= OBE_CLOCK;
            rounded_pts *= (648000 * enc_params->frames_per_pes);
rounded_pts += headset_bias;
            coded_frame->pts = rounded_pts;

            coded_frame->pts +=  ((int64_t)stream->audio_offset_ms * 27000LL);
            coded_frame->random_access = 1; /* Every frame output is a random access point */
            coded_frame->type = CF_AUDIO;

            if (enc_params->use_fifo_head_timing) {
                if (coded_frame->pts == lastCodedFramePTS) {
                    coded_frame->pts += (648000 * enc_params->frames_per_pes);
                }
                lastCodedFramePTS = coded_frame->pts;

                historical_int64_set(&cf_pts, coded_frame->pts);
                //historical_int64_printf(&cf_pts, "  cf_pts");

                if (lastOutputFramePTS + (648000 * enc_params->frames_per_pes) != coded_frame->pts) {
                    printf(MODULE "Output PTS discontinuity\n\tShould be %" PRIi64 " was %" PRIi64 " diff %9" PRIi64 " frames_per_pes %d\n",
                        lastOutputFramePTS + (648000 * enc_params->frames_per_pes),
                        coded_frame->pts,
			(lastOutputFramePTS + (648000 * enc_params->frames_per_pes)) - coded_frame->pts,
			enc_params->frames_per_pes);
                }
                lastOutputFramePTS = coded_frame->pts;
            }
if (coded_frame->pts < 0)
  coded_frame->pts = 0;
            add_to_queue( &h->mux_queue, coded_frame );
        }
    }

end:
    if( output_buf )
        free( output_buf );

    if( audio_buf )
        free( audio_buf );

    if( avr )
        avresample_free( &avr );

    if( fifo )
        av_fifo_free( fifo );

    if( tl_opts )
        twolame_close( &tl_opts );
    free( enc_params );

    return NULL;
}

const obe_aud_enc_func_t twolame_encoder = { start_encoder_mp2 };
