/*****************************************************************************
 * x264.c : x264 encoding functions
 *****************************************************************************
 * Copyright (C) 2018 LTN
 *
 * Authors: Kieran Kunhya <kieran@kunhya.com>
 * Authors: Steven Toth <stoth@ltnglobal.com>
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
#include "encoders/video/video.h"
#include <libavutil/mathematics.h>
#include <x265.h>

#define LOCAL_DEBUG 0

#define MESSAGE_PREFIX "[x265]:"

#if SEI_TIMESTAMPING
#define SEI_BIT_DELIMITER 0x81 /* Marker to prevent 21 consequtive zeros, its illegal. */
static const unsigned char ltn_uuid_sei_timestamp[] =
{
    0x59, 0x96, 0xFF, 0x28, 0x17, 0xCA, 0x41, 0x96, 0x8D, 0xE3, 0xE5, 0x3F, 0xE2, 0xF9, 0x92, 0xAE
};
#endif

struct context_s
{
	obe_vid_enc_params_t *enc_params;
	obe_t *h;
	obe_encoder_t *encoder;

	/* */
	x265_encoder *hevc_encoder;
	x265_param   *hevc_params;
	x265_picture *hevc_picture_in;
	x265_picture *hevc_picture_out;

	uint32_t      i_nal;
	x265_nal     *hevc_nals;

	uint64_t      raw_frame_count;
};

struct userdata_s
{
	struct avfm_s avfm;
};

struct userdata_s *userdata_calloc()
{
	struct userdata_s *ud = calloc(1, sizeof(*ud));
	return ud;
}

int userdata_set(struct userdata_s *ud, struct avfm_s *s)
{
	memcpy(&ud->avfm, s, sizeof(struct avfm_s));
	return 0;
}

void userdata_free(struct userdata_s *ud)
{
	memset(ud, 0, sizeof(*ud));
	free(ud);
}

const char *sliceTypeLookup(uint32_t type)
{
	switch(type) {
	case X265_TYPE_AUTO: return "X265_TYPE_AUTO";
	case X265_TYPE_IDR:  return "X265_TYPE_IDR";
	case X265_TYPE_I:    return "X265_TYPE_I";
	case X265_TYPE_P:    return "X265_TYPE_P";
	case X265_TYPE_BREF: return "X265_TYPE_BREF";
	case X265_TYPE_B:    return "X265_TYPE_B";
	default:             return "UNKNOWN";
	}
}

/* Convert a obe_raw_frame_t into a x264_picture_t struct.
 * Incoming frame is colorspace YUV420P.
 */
static int convert_obe_to_x265_pic(struct context_s *ctx, x265_picture *p, struct userdata_s *ud, obe_raw_frame_t *rf)
{
	obe_image_t *img = &rf->img;
	int count = 0, idx = 0;

	x265_picture_init(ctx->hevc_params, p);

	p->sliceType = X265_TYPE_AUTO;
	p->bitDepth = 8;
	p->stride[0] = img->stride[0];
	p->stride[1] = img->stride[1]; // >> x265_cli_csps[p->colorSpace].width[1];
	p->stride[2] = img->stride[2]; // >> x265_cli_csps[p->colorSpace].width[2];

	for (int i = 0; i < 3; i++) {
		p->stride[i] = img->stride[i];
		p->planes[i] = img->plane[i];
	}

	p->colorSpace = img->csp == PIX_FMT_YUV422P || img->csp == PIX_FMT_YUV422P10 ? X265_CSP_I422 : X265_CSP_I420;
#ifdef HIGH_BIT_DEPTH
	p->colorSpace |= X265_CSP_HIGH_DEPTH;
#endif

	for (int i = 0; i < rf->num_user_data; i++) {
		/* Only give correctly formatted data to the encoder */
		if (rf->user_data[i].type == USER_DATA_AVC_REGISTERED_ITU_T35 ||
			rf->user_data[i].type == USER_DATA_AVC_UNREGISTERED) {
			count++;
		}
	}

#if SEI_TIMESTAMPING
	/* Create space for unregister data, containing before and after timestamps. */
	count += 1;
#endif

	p->userSEI.numPayloads = count;

	if (p->userSEI.numPayloads) {
		p->userSEI.payloads = malloc(p->userSEI.numPayloads * sizeof(*p->userSEI.payloads));
		if (!p->userSEI.payloads)
			return -1;

		for (int i = 0; i < rf->num_user_data; i++) {
			/* Only give correctly formatted data to the encoder */

			if (rf->user_data[i].type == USER_DATA_AVC_REGISTERED_ITU_T35 || rf->user_data[i].type == USER_DATA_AVC_UNREGISTERED) {
				p->userSEI.payloads[idx].payloadType = rf->user_data[i].type;
				p->userSEI.payloads[idx].payloadSize = rf->user_data[i].len;
				p->userSEI.payloads[idx].payload = rf->user_data[i].data;
				idx++;
			} else {
				syslog(LOG_WARNING, MESSAGE_PREFIX " Invalid user data presented to encoder - type %i\n", rf->user_data[i].type);
				free(rf->user_data[i].data);
			}
			/* Set the pointer to NULL so only x264 can free the data if necessary */
			rf->user_data[i].data = NULL;
		}
	} else if (rf->num_user_data) {
		for (int i = 0; i < rf->num_user_data; i++) {
			syslog(LOG_WARNING, MESSAGE_PREFIX " Invalid user data presented to encoder - type %i\n", rf->user_data[i].type);
			free(rf->user_data[i].data);
		}
	}

#if SEI_TIMESTAMPING
	x265_sei_payload *x;
	int i;

	/* Start time - Always the last SEI */
	static uint32_t framecount = 0;
	x = &p->userSEI.payloads[count - 1];
	x->payloadType = USER_DATA_AVC_UNREGISTERED;
	x->payloadSize = sizeof(ltn_uuid_sei_timestamp) + 32 + 10;
	x->payload = calloc(1, x->payloadSize);

	struct timeval tv;
	gettimeofday(&tv, NULL);
	/* Format of LTN_SEI_TAG_START_TIME record:
	 * All records are big endian.
	 * 4 byte header + 20 byte payload.
	 * TT TT TT TT : 16 byte UUID
	 * FC FC FC FC : incrementing frame counter.
	 * HS HS HS HS : time received from hardware seconds (timeval.ts_sec).
	 * HU HU HU HU : time received from hardware useconds (timeval.ts_usec).
	 * SS SS SS SS : time send to compressor seconds (timeval.ts_sec).
	 * SU SU SU SU : time send to compressor useconds (timeval.ts_usec).
	 * ES ES ES ES : time exit from compressor seconds (timeval.ts_sec).
	 * EU EU EU EU : time exit from compressor useconds (timeval.ts_usec).
	 */

	memcpy(x->payload, ltn_uuid_sei_timestamp, sizeof(ltn_uuid_sei_timestamp));
	i = sizeof(ltn_uuid_sei_timestamp);

	x->payload[i++] = (framecount >> 24) & 0xff;
	x->payload[i++] = (framecount >> 16) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;
	x->payload[i++] = (framecount >>  8) & 0xff;
	x->payload[i++] = (framecount >>  0) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;

	unsigned int sec = avfm_get_hw_received_tv_sec(&rf->avfm);
	unsigned int usec = avfm_get_hw_received_tv_usec(&rf->avfm);
	x->payload[i++] = (sec >> 24) & 0xff;
	x->payload[i++] = (sec >> 16) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;
	x->payload[i++] = (sec >>  8) & 0xff;
	x->payload[i++] = (sec >>  0) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;
	x->payload[i++] = (usec >> 24) & 0xff;
	x->payload[i++] = (usec >> 16) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;
	x->payload[i++] = (usec >>  8) & 0xff;
	x->payload[i++] = (usec >>  0) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;

	x->payload[i++] = (tv.tv_sec >> 24) & 0xff;
	x->payload[i++] = (tv.tv_sec >> 16) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;
	x->payload[i++] = (tv.tv_sec >>  8) & 0xff;
	x->payload[i++] = (tv.tv_sec >>  0) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;
	x->payload[i++] = (tv.tv_usec >> 24) & 0xff;
	x->payload[i++] = (tv.tv_usec >> 16) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;
	x->payload[i++] = (tv.tv_usec >>  8) & 0xff;
	x->payload[i++] = (tv.tv_usec >>  0) & 0xff;
	x->payload[i++] = SEI_BIT_DELIMITER;

	/* The remaining 8 bytes (time exit from compressor fields)
	 * will be filled when the frame exists the compressor. */
	framecount++;
#endif

	return 0;
}

/* OBE will pass us a AVC struct initially. Pull out any important pieces
 * and pass those to x265.
 */
static void *x265_start_encoder( void *ptr )
{
	struct context_s ectx, *ctx = &ectx;
	memset(ctx, 0, sizeof(*ctx));

	ctx->enc_params = ptr;
	ctx->h = ctx->enc_params->h;
	ctx->encoder = ctx->enc_params->encoder;

	ctx->hevc_params = x265_param_alloc();
	if (!ctx->hevc_params) {
		fprintf(stderr, MESSAGE_PREFIX " failed to allocate params\n");
		goto out1;
	}

	x265_param_default(ctx->hevc_params);

	int ret = x265_param_default_preset(ctx->hevc_params, "ultrafast", "zerolatency");
	if (ret < 0) {
		fprintf(stderr, MESSAGE_PREFIX " failed to set default params\n");
		goto out1;
	}

//	ctx->hevc_params->fpsDenom = ctx->enc_params->avc_param.i_fps_den;
//	ctx->hevc_params->fpsNum = ctx->enc_params->avc_param.i_fps_num;

#if 0
                avc_param->rc.i_vbv_max_bitrate = obe_otoi( vbv_maxrate, 0 );
                avc_param->rc.i_vbv_buffer_size = obe_otoi( vbv_bufsize, 0 );
                avc_param->rc.i_bitrate         = obe_otoi( bitrate, 0 );
                avc_param->i_keyint_max        = obe_otoi( keyint, avc_param->i_keyint_max );
                avc_param->rc.i_lookahead      = obe_otoi( lookahead, avc_param->rc.i_lookahead );
                avc_param->i_threads           = obe_otoi( threads, avc_param->i_threads );
#endif

	char val[64];
	x265_param_parse(ctx->hevc_params, "input-res", "1280x720");

	ctx->hevc_params->internalCsp = X265_CSP_I420;
	x265_param_parse(ctx->hevc_params, "repeat-headers", "1");

	sprintf(&val[0], "%.3f", (float)ctx->enc_params->avc_param.i_fps_num / (float)ctx->enc_params->avc_param.i_fps_den); 
	x265_param_parse(ctx->hevc_params, "fps", val);

	sprintf(&val[0], "%d",ctx->enc_params->avc_param.i_keyint_max);
	x265_param_parse(ctx->hevc_params, "keyint", val);

	sprintf(&val[0], "%d", ctx->enc_params->avc_param.rc.i_vbv_buffer_size);
	x265_param_parse(ctx->hevc_params, "vbv-bufsize", val);

	sprintf(&val[0], "%d", ctx->enc_params->avc_param.rc.i_vbv_max_bitrate);
	x265_param_parse(ctx->hevc_params, "vbv-maxrate", val);

	/* 0 Is preferred, which is 'autodetect' */
	sprintf(&val[0], "%d", ctx->enc_params->avc_param.i_threads);
	x265_param_parse(ctx->hevc_params, "frame-threads", val);

//	x265_param_parse(ctx->hevc_params, "rc-lookahead", "4");
//	x265_param_parse(ctx->hevc_params, "vbv-minrate", "6000");
	sprintf(&val[0], "%d", ctx->enc_params->avc_param.rc.i_bitrate);
	x265_param_parse(ctx->hevc_params, "bitrate", val);

	ctx->hevc_picture_in = x265_picture_alloc();
	if (!ctx->hevc_picture_in) {
		fprintf(stderr, MESSAGE_PREFIX " failed to allocate picture\n");
		goto out2;
	}

	ctx->hevc_picture_out = x265_picture_alloc();
	if (!ctx->hevc_picture_out) {
		fprintf(stderr, MESSAGE_PREFIX " failed to allocate picture\n");
		goto out3;
	}

	/* Fix this? its AVC specific. */
	ctx->encoder->encoder_params = malloc(sizeof(ctx->enc_params->avc_param) );
	if (!ctx->encoder->encoder_params) {
		pthread_mutex_unlock(&ctx->encoder->queue.mutex);
		fprintf(stderr, MESSAGE_PREFIX " failed to allocate encoder params\n");
		goto out4;
	}
	memcpy(ctx->encoder->encoder_params, &ctx->enc_params->avc_param, sizeof(ctx->enc_params->avc_param));

	ctx->hevc_encoder = x265_encoder_open(ctx->hevc_params);
	if (!ctx->hevc_encoder) {
		pthread_mutex_unlock(&ctx->encoder->queue.mutex);
		fprintf(stderr, MESSAGE_PREFIX " failed to open encoder\n");
		goto out5;
	}

	/* Lock the mutex until we verify and fetch new parameters */
	pthread_mutex_lock(&ctx->encoder->queue.mutex);

	ctx->encoder->is_ready = 1;

	int64_t frame_duration = av_rescale_q( 1, (AVRational){ ctx->enc_params->avc_param.i_fps_den, ctx->enc_params->avc_param.i_fps_num}, (AVRational){ 1, OBE_CLOCK } );
	printf("frame_duration = %" PRIi64 "\n", frame_duration);
	//buffer_duration = frame_duration * ctx->enc_params->avc_param.sc.i_buffer_size;

	/* Wake up the muxer */
	pthread_cond_broadcast(&ctx->encoder->queue.in_cv);
	pthread_mutex_unlock(&ctx->encoder->queue.mutex);

	while (1) {
		pthread_mutex_lock(&ctx->encoder->queue.mutex);

		while (!ctx->encoder->queue.size && !ctx->encoder->cancel_thread) {
			pthread_cond_wait(&ctx->encoder->queue.in_cv, &ctx->encoder->queue.mutex);
		}

		if (ctx->encoder->cancel_thread) {
			pthread_mutex_unlock(&ctx->encoder->queue.mutex);
			break;
		}

		/* Reset the speedcontrol buffer if the source has dropped frames. Otherwise speedcontrol
		 * stays in an underflow state and is locked to the fastest preset.
		 */
		pthread_mutex_lock(&ctx->h->drop_mutex);
		if (ctx->h->video_encoder_drop) {
			pthread_mutex_lock(&ctx->h->enc_smoothing_queue.mutex);
			ctx->h->enc_smoothing_buffer_complete = 0;
			pthread_mutex_unlock(&ctx->h->enc_smoothing_queue.mutex);
#if 0
			fprintf(stderr, MESSAGE_PREFIX " Speedcontrol reset\n");
			x264_speedcontrol_sync( s, enc_params->avc_param.sc.i_buffer_size, enc_params->avc_param.sc.f_buffer_init, 0 );
#endif
			ctx->h->video_encoder_drop = 0;
		}
		pthread_mutex_unlock(&ctx->h->drop_mutex);

		obe_raw_frame_t *rf = ctx->encoder->queue.queue[0];
		ctx->raw_frame_count++;
		pthread_mutex_unlock(&ctx->encoder->queue.mutex);

#if LOCAL_DEBUG
		//printf(MESSAGE_PREFIX " popped a raw frame[%" PRIu64 "] -- pts %" PRIi64 "\n", ctx->raw_frame_count, rf->avfm.audio_pts);
#endif

		struct userdata_s *ud = userdata_calloc();

		/* convert obe_frame_t into x264 friendly struct.
		 * Bundle up and incoming SEI etc, into the userdata context
		 */
		if (convert_obe_to_x265_pic(ctx, ctx->hevc_picture_in, ud, rf) < 0) {
			fprintf(stderr, MESSAGE_PREFIX " pic prepare failed\n");
			break;
		}

		ctx->hevc_picture_in->userData = ud;

		/* Cache the upstream timing information in userdata. */
		userdata_set(ud, &rf->avfm);

		/* If the AFD has changed, then change the SAR. x264 will write the SAR at the next keyframe
		 * TODO: allow user to force keyframes in order to be frame accurate.
		 */
		if (rf->sar_width != ctx->enc_params->avc_param.vui.i_sar_width ||
			rf->sar_height != ctx->enc_params->avc_param.vui.i_sar_height) {

			ctx->enc_params->avc_param.vui.i_sar_width  = rf->sar_width;
			ctx->enc_params->avc_param.vui.i_sar_height = rf->sar_height;
//			pic.param = &enc_params->avc_param;

		}
#if 0
        /* Update speedcontrol based on the system state */
        if( h->obe_system == OBE_SYSTEM_TYPE_GENERIC )
        {
            pthread_mutex_lock( &h->enc_smoothing_queue.mutex );
            if( h->enc_smoothing_buffer_complete )
            {
                /* Wait until a frame is sent out. */
                while( !h->enc_smoothing_last_exit_time )
                    pthread_cond_wait( &h->enc_smoothing_queue.out_cv, &h->enc_smoothing_queue.mutex );

                /* time elapsed since last frame was removed */
                int64_t last_frame_delta = get_input_clock_in_mpeg_ticks( h ) - h->enc_smoothing_last_exit_time;

                int64_t buffer_duration;
                float buffer_fill;
                if( h->enc_smoothing_queue.size )
                {
                    obe_coded_frame_t *first_frame, *last_frame;
                    first_frame = h->enc_smoothing_queue.queue[0];
                    last_frame = h->enc_smoothing_queue.queue[h->enc_smoothing_queue.size-1];
                    int64_t frame_durations = last_frame->real_dts - first_frame->real_dts + frame_duration;
                    buffer_fill = (float)(frame_durations - last_frame_delta)/buffer_duration;
                }
                else
                    buffer_fill = (float)(-1 * last_frame_delta)/buffer_duration;

#if X264_BUILD < 148
                x264_speedcontrol_sync( s, buffer_fill, enc_params->avc_param.sc.i_buffer_size, 1 );
#endif
            }

            pthread_mutex_unlock( &h->enc_smoothing_queue.mutex );
        }
#endif

		int leave = 0;
		while (!leave) { 
			/* Compress PIC to NALS. */
			/* Once the pipeline is completely full, x265_encoder_encode() will block until the next output picture is complete. */

			if (rf) {
				ctx->hevc_picture_in->pts = rf->avfm.audio_pts;
				ret = x265_encoder_encode(ctx->hevc_encoder, &ctx->hevc_nals, &ctx->i_nal, ctx->hevc_picture_in, ctx->hevc_picture_out);
			}

#if SEI_TIMESTAMPING
			/* Walk through each of the NALS and insert current time into any LTN sei timestamp frames we find. */
			for (int m = 0; m < ctx->i_nal; m++) {
				if (ctx->hevc_nals[m].type == 39 &&
					memcmp(&ctx->hevc_nals[m].payload[23], ltn_uuid_sei_timestamp, sizeof(ltn_uuid_sei_timestamp)) == 0)
				{
					struct timeval tv;
					gettimeofday(&tv, NULL);
					unsigned char *p = ctx->hevc_nals[m].payload;

					/* Add the time exit from compressor seconds/useconds. */
					int i = 69;
					p[i++] = (tv.tv_sec >> 24) & 0x0ff;
					p[i++] = (tv.tv_sec >> 16) & 0x0ff;
					p[i++] = SEI_BIT_DELIMITER;
					p[i++] = (tv.tv_sec >>  8) & 0x0ff;
					p[i++] = (tv.tv_sec >>  0) & 0x0ff;
					p[i++] = SEI_BIT_DELIMITER;
					p[i++] = (tv.tv_usec >> 24) & 0x0ff;
					p[i++] = (tv.tv_usec >> 16) & 0x0ff;
					p[i++] = SEI_BIT_DELIMITER;
					p[i++] = (tv.tv_usec >>  8) & 0x0ff;
					p[i++] = (tv.tv_usec >>  0) & 0x0ff;
					p[i++] = SEI_BIT_DELIMITER;
				}
			}
#endif

			int64_t arrival_time = 0;
			if (rf) {
				arrival_time = rf->arrival_time;
				rf->release_data(rf);
				rf->release_frame(rf);
				remove_from_queue(&ctx->encoder->queue);
				rf = 0;
			}

			if (ret < 0) {
				fprintf(stderr, MESSAGE_PREFIX " picture encode failed, ret = %d\n", ret);
				break;
			}

			if (ret == 0) {
				//fprintf(stderr, MESSAGE_PREFIX " ret = %d\n", ret);
				leave = 1;
				continue;
			}

			if (ret > 0) {
				for (int z = 0; z < ctx->i_nal; z++) {
					obe_coded_frame_t *cf = new_coded_frame(ctx->encoder->output_stream_id, ctx->hevc_nals[z].sizeBytes);
					if (!cf) {
						fprintf(stderr, MESSAGE_PREFIX " unable to alloc a new coded frame\n");
						break;
					}
#if LOCAL_DEBUG
					printf(MESSAGE_PREFIX " acquired %7d nals bytes (%d nals), pts = %12" PRIi64 " dts = %12" PRIi64 ", ret = %d, ",
						ctx->hevc_nals[z].sizeBytes, ctx->i_nal - z,
						ctx->hevc_picture_out->pts,
						ctx->hevc_picture_out->dts,
						ret);
					printf("poc %8d  sliceType %d [%s]\n",
						ctx->hevc_picture_out->poc, ctx->hevc_picture_out->sliceType,
						sliceTypeLookup(ctx->hevc_picture_out->sliceType));
#endif
					/* Prep the frame. */
#if 0
static FILE *fh = NULL;
if (fh == NULL)
  fh = fopen("/tmp/hevc.nals", "wb");

if (fh)
  fwrite(ctx->hevc_nals[z].payload, 1, ctx->hevc_nals[z].sizeBytes, fh);
#endif

					struct userdata_s *out_ud = ctx->hevc_picture_out->userData; 
					if (out_ud) {
						/* Make sure we push the original hardware timing into the new frame. */
						memcpy(&cf->avfm, &out_ud->avfm, sizeof(struct avfm_s));
						free(ctx->hevc_picture_out->userData);
						ctx->hevc_picture_out->userData = 0;

						cf->pts = out_ud->avfm.audio_pts;
					} else {
						//fprintf(stderr, MESSAGE_PREFIX " missing pic out userData\n");
					}

					memcpy(cf->data, ctx->hevc_nals[z].payload, ctx->hevc_nals[z].sizeBytes);
					cf->len                      = ctx->hevc_nals[z].sizeBytes;
					cf->type                     = CF_VIDEO;
					cf->pts                      = ctx->hevc_picture_out->pts + 45000;
					cf->real_pts                 = ctx->hevc_picture_out->pts + 45000;
					cf->real_dts                 = ctx->hevc_picture_out->dts;
					cf->cpb_initial_arrival_time = cf->real_pts;
					cf->cpb_final_arrival_time   = cf->real_pts + 45000;

#if 0
// X264 specific, I don't think we need to do this for HEVC
            cf->pts = coded_frame->avfm.audio_pts;

            /* The audio and video clocks jump with different intervals when the cable
             * is disconnected, suggestedint a BM firmware bug.
             * We'll use the audio clock regardless, for both audio and video compressors.
             */
            int64_t new_dts  = avfm->audio_pts + 24299700 - abs(cf->real_dts - cf->real_pts) + (2 * 450450);

            /* We need to userstand, for this temporal frame, how much it varies from the dts. */
            int64_t pts_diff = cf->real_dts - cf->real_pts;

            /* Construct a new PTS based on the hardware DTS and the PTS offset difference. */
            int64_t new_pts  = new_dts - pts_diff;

            cf->real_dts = new_dts;
            cf->real_pts = new_pts;
            cf->cpb_initial_arrival_time = new_dts;
            cf->cpb_final_arrival_time   = new_dts + abs(pic_out.hrd_timing.cpb_final_arrival_time - pic_out.hrd_timing.cpb_final_arrival_time);

            cpb_removal_time = cf->real_pts; /* Only used for manually eyeballing the video output clock. */

#endif
#if 0
			printf(MESSAGE_PREFIX " real_pts:%" PRIi64 " real_dts:%" PRIi64 " (%.3f %.3f)\n",
				cf->real_pts, cf->real_dts,
				ctx->hevc_picture_out->hrd_timing.dpb_output_time, pic_out.hrd_timing.cpb_removal_time);
#endif

					cf->priority = IS_X265_TYPE_I(ctx->hevc_picture_out->sliceType);
					cf->random_access = IS_X265_TYPE_I(ctx->hevc_picture_out->sliceType);

					if (ctx->h->obe_system == OBE_SYSTEM_TYPE_LOWEST_LATENCY || ctx->h->obe_system == OBE_SYSTEM_TYPE_LOW_LATENCY) {
						cf->arrival_time = arrival_time;
						add_to_queue(&ctx->h->mux_queue, cf);
						//printf(MESSAGE_PREFIX " Encode Latency %"PRIi64" \n", obe_mdate() - cf->arrival_time);
					} else {
						add_to_queue(&ctx->h->enc_smoothing_queue, cf);
					}
				} /* For each NAL */
			} /* if nal_bytes > 0 */

			leave = 1;
		} /* While ! leave */

	} /* While (1) */

	if (ctx->hevc_encoder)
		x265_encoder_close(ctx->hevc_encoder);

out5:
	free(ctx->enc_params);

out4:
	if (ctx->hevc_picture_out)
		x265_picture_free(ctx->hevc_picture_out);
out3:
	if (ctx->hevc_picture_in)
		x265_picture_free(ctx->hevc_picture_in);
out2:
	if (ctx->hevc_params)
		x265_param_free(ctx->hevc_params);
out1:
	x265_cleanup();

	return NULL;
}

const obe_vid_enc_func_t x265_obe_encoder = { x265_start_encoder };