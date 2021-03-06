/*
 * Demuxer for avisynth
 * Copyright (c) 2005 Gianluigi Tiesi <sherpya@netfarm.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"

#include "stream.h"
#include "demuxer.h"
#include "stheader.h"

#include "wine/windef.h"

#ifdef WIN32_LOADER
#include "ldt_keeper.h"
#endif

#include "demux_avs.h"

#define MAX_AVS_SIZE    16 * 1024 /* 16k should be enough */
#undef ENABLE_AUDIO

HMODULE WINAPI LoadLibraryA(LPCSTR);
FARPROC WINAPI GetProcAddress(HMODULE,LPCSTR);
int     WINAPI FreeLibrary(HMODULE);

typedef WINAPI AVS_ScriptEnvironment* (*imp_avs_create_script_environment)(int version);
typedef WINAPI AVS_Value (*imp_avs_invoke)(AVS_ScriptEnvironment *, const char * name, AVS_Value args, const char** arg_names);
typedef WINAPI const AVS_VideoInfo *(*imp_avs_get_video_info)(AVS_Clip *);
typedef WINAPI AVS_Clip* (*imp_avs_take_clip)(AVS_Value, AVS_ScriptEnvironment *);
typedef WINAPI AVS_VideoFrame* (*imp_avs_get_frame)(AVS_Clip *, int n);
typedef WINAPI void (*imp_avs_release_video_frame)(AVS_VideoFrame *);
#ifdef ENABLE_AUDIO
//typedef WINAPI int (*imp_avs_get_audio)(AVS_Clip *, void * buf, uint64_t start, uint64_t count); 
typedef WINAPI int (*imp_avs_get_audio)(AVS_ScriptEnvironment *, void * buf, uint64_t start, uint64_t count); 
#endif

#define Q(string) # string
#define IMPORT_FUNC(x) \
    AVS->x = ( imp_##x ) GetProcAddress(AVS->dll, Q(x)); \
    if (!AVS->x) { mp_msg(MSGT_DEMUX,MSGL_V,"AVS: failed to load "Q(x)"()\n"); return 0; }

typedef struct tagAVS
{
    AVS_ScriptEnvironment *avs_env;
    AVS_Value handler;
    AVS_Clip *clip;
    const AVS_VideoInfo *video_info;
    HMODULE dll;
    int frameno;
    int init;
    
    imp_avs_create_script_environment avs_create_script_environment;
    imp_avs_invoke avs_invoke;
    imp_avs_get_video_info avs_get_video_info;
    imp_avs_take_clip avs_take_clip;
    imp_avs_get_frame avs_get_frame;
    imp_avs_release_video_frame avs_release_video_frame;
#ifdef ENABLE_AUDIO
    imp_avs_get_audio avs_get_audio;
#endif
} AVS_T;

AVS_T *initAVS(const char *filename)
{   
    AVS_T *AVS = (AVS_T *) malloc (sizeof(AVS_T));
    AVS_Value arg0 = avs_new_value_string(filename);
    AVS_Value args = avs_new_value_array(&arg0, 1);
    
    memset(AVS, 0, sizeof(AVS_T));

#ifdef WIN32_LOADER
    Setup_LDT_Keeper();
#endif
    
    AVS->dll = LoadLibraryA("avisynth.dll");
    if(!AVS->dll)
    {
        mp_msg(MSGT_DEMUX ,MSGL_V, "AVS: failed to load avisynth.dll\n");
        return NULL;
    }
    
    /* Dynamic import of needed stuff from avisynth.dll */
    IMPORT_FUNC(avs_create_script_environment);
    IMPORT_FUNC(avs_create_script_environment);
    IMPORT_FUNC(avs_invoke);
    IMPORT_FUNC(avs_get_video_info);
    IMPORT_FUNC(avs_take_clip);
    IMPORT_FUNC(avs_get_frame);
    IMPORT_FUNC(avs_release_video_frame);
#ifdef ENABLE_AUDIO
    IMPORT_FUNC(avs_get_audio);
#endif
    
    AVS->avs_env = AVS->avs_create_script_environment(AVISYNTH_INTERFACE_VERSION);
    if (!AVS->avs_env)
    {
        mp_msg(MSGT_DEMUX, MSGL_V, "AVS: avs_create_script_environment failed\n");
        return NULL;
    }
    

    AVS->handler = AVS->avs_invoke(AVS->avs_env, "Import", args, 0);
    
    if (avs_is_error(AVS->handler))
    {
        mp_msg(MSGT_DEMUX, MSGL_V, "AVS: Avisynth error: %s\n", avs_as_string(AVS->handler));
        return NULL;
    }

    if (!avs_is_clip(AVS->handler))
    {
        mp_msg(MSGT_DEMUX, MSGL_V, "AVS: Avisynth doesn't return a clip\n");
        return NULL;
    }
    
    return AVS;
}

/* Implement RGB MODES ?? */
#if 0
static __inline int get_mmioFOURCC(const AVS_VideoInfo *v)
{
    if (avs_is_rgb(v)) return mmioFOURCC(8, 'R', 'G', 'B');
    if (avs_is_rgb24(v)) return mmioFOURCC(24, 'R', 'G', 'B');
    if (avs_is_rgb32(v)) return mmioFOURCC(32, 'R', 'G', 'B');
    if (avs_is_yv12(v)) return mmioFOURCC('Y', 'V', '1', '2');
    if (avs_is_yuy(v)) return mmioFOURCC('Y', 'U', 'Y', ' ');
    if (avs_is_yuy2(v)) return mmioFOURCC('Y', 'U', 'Y', '2');    
    return 0;
}
#endif

int demux_avs_fill_buffer(demuxer_t *demuxer)
{
    AVS_VideoFrame *curr_frame;
    demux_packet_t *dp = NULL;
    AVS_T *AVS = (AVS_T *) demuxer->priv;

    demux_stream_t *d_video=demuxer->video;
    sh_video_t *sh_video=d_video->sh;

#ifdef ENABLE_AUDIO
    demux_stream_t *d_audio=demuxer->audio;
    sh_audio_t *sh_audio=d_audio->sh;
#endif
    
    if (AVS->video_info->num_frames < AVS->frameno) return 0; // EOF
    
    curr_frame = AVS->avs_get_frame(AVS->clip, AVS->frameno);
    if (!curr_frame)
    {
        mp_msg(MSGT_DEMUX, MSGL_V, "AVS: error getting frame -- EOF??\n");
        return 0;
    }

    if (avs_has_video(AVS->video_info))
    {
        dp = new_demux_packet(curr_frame->vfb->data_size);
        sh_video->num_frames_decoded++;
        sh_video->num_frames++;

        d_video->pts=AVS->frameno / sh_video->fps; // OSD

        memcpy(dp->buffer, curr_frame->vfb->data + curr_frame->offset, curr_frame->vfb->data_size);
        ds_add_packet(demuxer->video, dp);

    }
    
#ifdef ENABLE_AUDIO
    /* Audio */
    if (avs_has_audio(AVS->video_info))
    {
        int l = sh_audio->wf->nAvgBytesPerSec;
        dp = new_demux_packet(l);
        
        if (AVS->avs_get_audio(AVS->avs_env, dp->buffer, AVS->frameno*sh_video->fps*l, l))
        {
            mp_msg(MSGT_DEMUX, MSGL_V, "AVS: avs_get_audio() failed\n");
            return 0;
        }
        ds_add_packet(demuxer->audio, dp);
    }
#endif
    
    AVS->frameno++;
    AVS->avs_release_video_frame(curr_frame);
    return 1;
}

int demux_open_avs(demuxer_t* demuxer)
{
    sh_video_t *sh_video = NULL;
#ifdef ENABLE_AUDIO
    sh_audio_t *sh_audio = NULL;
#endif
    int found = 0;
    AVS_T *AVS = (AVS_T *) demuxer->priv;
    AVS->frameno = 0;

    mp_msg(MSGT_DEMUX, MSGL_V, "AVS: demux_open_avs()\n");
    demuxer->seekable = 1;

    AVS->clip = AVS->avs_take_clip(AVS->handler, AVS->avs_env);
    if(!AVS->clip)
    {
        mp_msg(MSGT_DEMUX, MSGL_V, "AVS: avs_take_clip() failed\n");
        return 0;
    }

    AVS->video_info = AVS->avs_get_video_info(AVS->clip);
    if (!AVS->video_info)
    {
        mp_msg(MSGT_DEMUX, MSGL_V, "AVS: avs_get_video_info() call failed\n");
        return 0;
    }
    
    if (!avs_is_yv12(AVS->video_info))
    {
        AVS->handler = AVS->avs_invoke(AVS->avs_env, "ConvertToYV12", avs_new_value_array(&AVS->handler, 1), 0);
        if (avs_is_error(AVS->handler))
        {
            mp_msg(MSGT_DEMUX, MSGL_V, "AVS: Cannot convert input video to YV12: %s\n", avs_as_string(AVS->handler));
            return 0;
        }
        
        AVS->clip = AVS->avs_take_clip(AVS->handler, AVS->avs_env);
        
        if(!AVS->clip)
        {
            mp_msg(MSGT_DEMUX, MSGL_V, "AVS: avs_take_clip() failed\n");
            return 0;
        }

        AVS->video_info = AVS->avs_get_video_info(AVS->clip);
        if (!AVS->video_info)
        {
            mp_msg(MSGT_DEMUX, MSGL_V, "AVS: avs_get_video_info() call failed\n");
            return 0;
        }
    }
    
    // TODO check field-based ??

    /* Video */  
    if (avs_has_video(AVS->video_info))
    {
        found = 1;
        sh_video = new_sh_video(demuxer, 0);
        
        demuxer->video->sh = sh_video;
        sh_video->ds = demuxer->video;
        
        sh_video->disp_w = AVS->video_info->width;
        sh_video->disp_h = AVS->video_info->height;
        
        //sh_video->format = get_mmioFOURCC(AVS->video_info);
        sh_video->format = mmioFOURCC('Y', 'V', '1', '2');
        sh_video->fps = (float) ((float) AVS->video_info->fps_numerator / (float) AVS->video_info->fps_denominator);
        sh_video->frametime = 1.0 / sh_video->fps;
        
        sh_video->bih = (BITMAPINFOHEADER*) malloc(sizeof(BITMAPINFOHEADER) + (256 * 4));
        sh_video->bih->biCompression = sh_video->format;
        sh_video->bih->biBitCount = avs_bits_per_pixel(AVS->video_info);
        //sh_video->bih->biPlanes = 2;
        
        sh_video->bih->biWidth = AVS->video_info->width;
        sh_video->bih->biHeight = AVS->video_info->height;
        sh_video->num_frames = 0;
        sh_video->num_frames_decoded = 0;
    }
    
#ifdef ENABLE_AUDIO
    /* Audio */
    if (avs_has_audio(AVS->video_info))
    {
        found = 1;
        mp_msg(MSGT_DEMUX, MSGL_V, "AVS: Clip has audio -> Channels = %d - Freq = %d\n", AVS->video_info->nchannels, AVS->video_info->audio_samples_per_second);

        sh_audio = new_sh_audio(demuxer, 0);
        demuxer->audio->sh = sh_audio;
        sh_audio->ds = demuxer->audio;
        
        sh_audio->wf = (WAVEFORMATEX*) malloc(sizeof(WAVEFORMATEX));
        sh_audio->wf->wFormatTag = sh_audio->format = 0x1;
        sh_audio->wf->nChannels = sh_audio->channels = AVS->video_info->nchannels;
        sh_audio->wf->nSamplesPerSec = sh_audio->samplerate = AVS->video_info->audio_samples_per_second;
        sh_audio->wf->nAvgBytesPerSec = AVS->video_info->audio_samples_per_second * 4;
        sh_audio->wf->nBlockAlign = 4;
        sh_audio->wf->wBitsPerSample = sh_audio->samplesize = 16; // AVS->video_info->sample_type ??
        sh_audio->wf->cbSize = 0;
        sh_audio->i_bps = sh_audio->wf->nAvgBytesPerSec;
        sh_audio->o_bps = sh_audio->wf->nAvgBytesPerSec;
    }
#endif

    AVS->init = 1;
    return found;
}

int demux_avs_control(demuxer_t *demuxer, int cmd, void *arg)
{   
    demux_stream_t *d_video=demuxer->video;
    sh_video_t *sh_video=d_video->sh;
    AVS_T *AVS = (AVS_T *) demuxer->priv;

    switch(cmd)
    {
        case DEMUXER_CTRL_GET_TIME_LENGTH:
        {
            if (!AVS->video_info->num_frames) return DEMUXER_CTRL_DONTKNOW;
            *((unsigned long *)arg) = AVS->video_info->num_frames / sh_video->fps;
            return DEMUXER_CTRL_OK;
        }
        case DEMUXER_CTRL_GET_PERCENT_POS:
        {
            if (!AVS->video_info->num_frames) return DEMUXER_CTRL_DONTKNOW;
            *((int *)arg) = (int) (AVS->frameno * 100 / AVS->video_info->num_frames);
            return DEMUXER_CTRL_OK;
        }
    default:
        return DEMUXER_CTRL_NOTIMPL;
    }
}

void demux_close_avs(demuxer_t* demuxer)
{
    AVS_T *AVS = (AVS_T *) demuxer->priv;
    // TODO release_clip?
    if (AVS)
    {
        if (AVS->dll)
        {
            mp_msg(MSGT_DEMUX, MSGL_V, "AVS: Unloading avisynth.dll\n");
            FreeLibrary(AVS->dll);
        }
        free(AVS);
    }
}

void demux_seek_avs(demuxer_t *demuxer, float rel_seek_secs,int flags)
{
    demux_stream_t *d_video=demuxer->video;
    sh_video_t *sh_video=d_video->sh;
    AVS_T *AVS = (AVS_T *) demuxer->priv;
    int video_pos=AVS->frameno;
    
    //mp_msg(MSGT_DEMUX, MSGL_V, "AVS: seek rel_seek_secs = %f - flags = %x\n", rel_seek_secs, flags);
    
    // seek absolute
    if (flags&1) video_pos=0;

    video_pos += (rel_seek_secs * sh_video->fps);
    if (video_pos < 0) video_pos = 0;
    if (video_pos > AVS->video_info->num_frames) video_pos = AVS->video_info->num_frames;
        
    AVS->frameno = video_pos;
    sh_video->num_frames_decoded = video_pos;
    sh_video->num_frames = video_pos;
    d_video->pts=AVS->frameno / sh_video->fps; // OSD
}

int avs_check_file(demuxer_t *demuxer, const char *filename)
{
    mp_msg(MSGT_DEMUX, MSGL_V, "AVS: avs_check_file - attempting to open file %s\n", filename);

    if (!filename) return 0;
    
    /* Avoid crazy memory eating when passing an mpg stream */
    if (demuxer->movi_end > MAX_AVS_SIZE)
    {
        mp_msg(MSGT_DEMUX,MSGL_V, "AVS: File is too big, aborting...\n");
        return 0;
    }
    
    demuxer->priv = initAVS(filename);
    
    if (demuxer->priv)
    {
        mp_msg(MSGT_DEMUX,MSGL_V, "AVS: Init Ok\n");
        return 1;
    }
    mp_msg(MSGT_DEMUX,MSGL_V, "AVS: Init failed\n");
    return 0;
}
