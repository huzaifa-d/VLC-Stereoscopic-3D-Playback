/*****************************************************************************
 * dxva2_deinterlace.c: DxVA2 deinterlacing filter
 *****************************************************************************
 * Copyright (C) 2017 Videolabs SAS
 *
 * Authors: Steve Lhomme <robux4@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <assert.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>

#define COBJMACROS
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include "../../video_chroma/d3d9_fmt.h"
#include "../../video_filter/deinterlace/common.h"

struct filter_sys_t
{
    HINSTANCE                      hdecoder_dll;
    /* keep a reference in case the vout is released first */
    HINSTANCE                      d3d9_dll;
    IDirect3DDevice9               *d3ddev;
    IDirectXVideoProcessor         *processor;
    IDirect3DSurface9              *hw_surface;

    DXVA2_VideoProcessorCaps       decoder_caps;

    struct deinterlace_ctx         context;
};

struct filter_mode_t
{
    const char       *psz_mode;
    UINT              i_mode;
    deinterlace_algo  settings;
};
static struct filter_mode_t filter_mode [] = {
    { "blend",   DXVA2_DeinterlaceTech_BOBLineReplicate,
                 { false, false, false, false } },
    { "bob",     DXVA2_DeinterlaceTech_BOBVerticalStretch,
                 { true,  false, false, false } },
    { "x",       DXVA2_DeinterlaceTech_BOBVerticalStretch4Tap,
                 { true, true, false, false } },
    { "ivtc",    DXVA2_DeinterlaceTech_InverseTelecine,
                 { false, true, true, false } },
    { "yadif2x", DXVA2_DeinterlaceTech_PixelAdaptive,
                 { true,  true, false, false } },
};

static void Flush(filter_t *filter)
{
    FlushDeinterlacing(&filter->p_sys->context);
}

static void FillSample( DXVA2_VideoSample *p_sample,
                        const struct deinterlace_ctx *p_context,
                        picture_t *p_pic,
                        const video_format_t *p_fmt,
                        const RECT *p_area,
                        int i_field )
{
    picture_sys_t *p_sys_src = ActivePictureSys(p_pic);

    p_sample->SrcSurface = p_sys_src->surface;
    p_sample->SampleFormat.SampleFormat = p_pic->b_top_field_first ?
                DXVA2_SampleFieldInterleavedEvenFirst :
                DXVA2_SampleFieldInterleavedOddFirst;
    p_sample->Start = 0;
    p_sample->End = GetFieldDuration(p_context, p_fmt, p_pic) * 10;
    p_sample->SampleData = DXVA2_SampleData_RFF_TFF_Present;
    if (!i_field)
        p_sample->SampleData |= DXVA2_SampleData_TFF;
    else
        p_sample->SampleData |= DXVA2_SampleData_RFF;
    p_sample->DstRect = p_sample->SrcRect = *p_area;
    p_sample->PlanarAlpha    = DXVA2_Fixed32OpaqueAlpha();
}

static int RenderPic( filter_t *filter, picture_t *p_outpic, picture_t *src,
                      int order, int i_field )
{
    filter_sys_t *sys = filter->p_sys;
    const int i_samples = sys->decoder_caps.NumBackwardRefSamples + 1 +
                          sys->decoder_caps.NumForwardRefSamples;
    HRESULT hr;
    DXVA2_VideoProcessBltParams params = {0};
    DXVA2_VideoSample samples[i_samples];
    picture_t         *pictures[i_samples];
    D3DSURFACE_DESC srcDesc, dstDesc;
    RECT area;

    picture_t *p_prev = sys->context.pp_history[0];
    picture_t *p_cur  = sys->context.pp_history[1];
    picture_t *p_next = sys->context.pp_history[2];

    picture_sys_t *p_sys_src = ActivePictureSys(src);

    hr = IDirect3DSurface9_GetDesc( p_sys_src->surface, &srcDesc );
    if (unlikely(FAILED(hr)))
        return VLC_EGENERIC;
    hr = IDirect3DSurface9_GetDesc( sys->hw_surface, &dstDesc );
    if (unlikely(FAILED(hr)))
        return VLC_EGENERIC;

    area.top = area.left = 0;
    area.bottom = __MIN(srcDesc.Height, dstDesc.Height);
    area.right  = __MIN(srcDesc.Width,  dstDesc.Width);

    int idx = i_samples - 1;
    if (p_next)
    {
        pictures[idx--] = p_next;
        if (p_cur)
            pictures[idx--] = p_cur;
        if (p_prev)
            pictures[idx--] = p_prev;
    }
    else
        pictures[idx--] = src;
    while (idx >= 0)
        pictures[idx--] = NULL;

    for (idx = 0; idx <= i_samples-1; idx++)
    {
        if (pictures[idx])
            FillSample( &samples[idx], &sys->context, pictures[idx], &filter->fmt_out.video, &area, i_field);
        else
        {
            FillSample( &samples[idx], &sys->context, src, &filter->fmt_out.video, &area, i_field);
            samples[idx].SampleFormat.SampleFormat = DXVA2_SampleUnknown;
        }
    }

    params.TargetFrame = (samples[0].End - samples[0].Start) * order / 2;
    params.TargetRect  = area;
    params.DestData    = 0;
    params.Alpha       = DXVA2_Fixed32OpaqueAlpha();
    params.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
    params.BackgroundColor.Alpha = 0xFFFF;

    hr = IDirectXVideoProcessor_VideoProcessBlt( sys->processor,
                                                 sys->hw_surface,
                                                 &params,
                                                 samples,
                                                 i_samples, NULL );
    if (FAILED(hr))
        return VLC_EGENERIC;

    hr = IDirect3DDevice9_StretchRect( sys->d3ddev,
                                       sys->hw_surface, NULL,
                                       p_outpic->p_sys->surface, NULL,
                                       D3DTEXF_NONE);
    if (FAILED(hr))
        return VLC_EGENERIC;

    return VLC_SUCCESS;
}

static int RenderSinglePic( filter_t *p_filter, picture_t *p_outpic, picture_t *p_pic )
{
    return RenderPic( p_filter, p_outpic, p_pic, 0, 0 );
}

static picture_t *Deinterlace(filter_t *p_filter, picture_t *p_pic)
{
    return DoDeinterlacing( p_filter, &p_filter->p_sys->context, p_pic );
}

static const struct filter_mode_t *GetFilterMode(const char *mode)
{
    if ( mode == NULL || !strcmp( mode, "auto" ) )
        mode = "x";

    for (size_t i=0; i<ARRAY_SIZE(filter_mode); i++)
    {
        if( !strcmp( mode, filter_mode[i].psz_mode ) )
            return &filter_mode[i];
    }

    return NULL;
}

static int Open(vlc_object_t *obj)
{
    filter_t *filter = (filter_t *)obj;
    filter_sys_t *sys = NULL;
    HINSTANCE hdecoder_dll = NULL;
    HINSTANCE d3d9_dll = NULL;
    HRESULT hr;
    picture_t *dst = NULL;
    GUID *processorGUIDs = NULL;
    GUID *processorGUID = NULL;
    IDirectXVideoProcessorService *processor = NULL;

    if (filter->fmt_in.video.i_chroma != VLC_CODEC_D3D9_OPAQUE
     && filter->fmt_in.video.i_chroma != VLC_CODEC_D3D9_OPAQUE_10B)
        return VLC_EGENERIC;
    if (!video_format_IsSimilar(&filter->fmt_in.video, &filter->fmt_out.video))
        return VLC_EGENERIC;

    d3d9_dll = LoadLibrary(TEXT("D3D9.DLL"));
    if (!d3d9_dll)
        goto error;

    hdecoder_dll = LoadLibrary(TEXT("DXVA2.DLL"));
    if (!hdecoder_dll)
        goto error;

    dst = filter_NewPicture(filter);
    if (dst == NULL)
        goto error;

    if (!dst->p_sys)
    {
        msg_Dbg(filter, "D3D11 opaque without a texture");
        goto error;
    }

    sys = calloc(1, sizeof (*sys));
    if (unlikely(sys == NULL))
        goto error;

    HRESULT (WINAPI *CreateVideoService)(IDirect3DDevice9 *,
                                         REFIID riid,
                                         void **ppService);
    CreateVideoService =
      (void *)GetProcAddress(hdecoder_dll, "DXVA2CreateVideoService");
    if (CreateVideoService == NULL)
        goto error;

    hr = IDirect3DSurface9_GetDevice( dst->p_sys->surface, &sys->d3ddev );
    if (FAILED(hr))
        goto error;

    D3DSURFACE_DESC dstDesc;
    hr = IDirect3DSurface9_GetDesc( dst->p_sys->surface, &dstDesc );
    if (unlikely(FAILED(hr)))
        goto error;

    hr = CreateVideoService( sys->d3ddev, &IID_IDirectXVideoProcessorService,
                            (void**)&processor);
    if (FAILED(hr))
        goto error;

    DXVA2_VideoDesc dsc;
    ZeroMemory(&dsc, sizeof(dsc));
    dsc.SampleWidth     = dstDesc.Width;
    dsc.SampleHeight    = dstDesc.Height;
    dsc.Format          = dstDesc.Format;
    if (filter->fmt_in.video.i_frame_rate && filter->fmt_in.video.i_frame_rate_base) {
        dsc.InputSampleFreq.Numerator   = filter->fmt_in.video.i_frame_rate;
        dsc.InputSampleFreq.Denominator = filter->fmt_in.video.i_frame_rate_base;
    } else {
        dsc.InputSampleFreq.Numerator   = 0;
        dsc.InputSampleFreq.Denominator = 0;
    }
    dsc.OutputFrameFreq = dsc.InputSampleFreq;

    DXVA2_ExtendedFormat *pFormat = &dsc.SampleFormat;
    pFormat->SampleFormat = dst->b_top_field_first ?
                DXVA2_SampleFieldInterleavedEvenFirst :
                DXVA2_SampleFieldInterleavedOddFirst;

    UINT count = 0;
    hr = IDirectXVideoProcessorService_GetVideoProcessorDeviceGuids( processor,
                                                                &dsc,
                                                                &count,
                                                                &processorGUIDs);
    if (FAILED(hr))
        goto error;

    char *psz_mode = var_InheritString( filter, "deinterlace-mode" );
    const struct filter_mode_t *p_mode = GetFilterMode(psz_mode);
    if (p_mode == NULL)
    {
        msg_Dbg(filter, "unknown mode %s, trying blend", psz_mode);
        p_mode = GetFilterMode("blend");
    }

    DXVA2_VideoProcessorCaps caps, best_caps;
    unsigned best_score = 0;
    for (UINT i=0; i<count; ++i) {
        hr = IDirectXVideoProcessorService_GetVideoProcessorCaps( processor,
                                                                  processorGUIDs+i,
                                                                  &dsc,
                                                                  dsc.Format,
                                                                  &caps);
        if ( FAILED(hr) || !caps.DeinterlaceTechnology )
            continue;

        unsigned score = (caps.DeinterlaceTechnology & p_mode->i_mode) ? 10 : 1;
        if (best_score < score) {
            best_score = score;
            best_caps = caps;
            processorGUID = processorGUIDs + i;
        }
    }

    if (processorGUID == NULL)
    {
        msg_Dbg(filter, "Could not find a filter to output the required format");
        goto error;
    }

    hr = IDirectXVideoProcessorService_CreateVideoProcessor( processor,
                                                             processorGUID,
                                                             &dsc,
                                                             dsc.Format,
                                                             1,
                                                             &sys->processor );
    if (FAILED(hr))
        goto error;

    hr = IDirectXVideoProcessorService_CreateSurface( processor,
                                                      dstDesc.Width,
                                                      dstDesc.Height,
                                                      0,
                                                      dstDesc.Format,
                                                      D3DPOOL_DEFAULT,
                                                      0,
                                                      DXVA2_VideoProcessorRenderTarget,
                                                      &sys->hw_surface,
                                                      NULL);
    if (FAILED(hr))
        goto error;

    CoTaskMemFree(processorGUIDs);
    picture_Release(dst);
    IDirectXVideoProcessorService_Release(processor);

    sys->hdecoder_dll = hdecoder_dll;
    sys->d3d9_dll     = d3d9_dll;
    sys->decoder_caps = best_caps;

    sys->context.settings = p_mode->settings;
    sys->context.settings.b_use_frame_history = best_caps.NumBackwardRefSamples != 0 ||
                                       best_caps.NumForwardRefSamples  != 0;
    assert(sys->context.settings.b_use_frame_history == p_mode->settings.b_use_frame_history);
    if (sys->context.settings.b_double_rate)
        sys->context.pf_render_ordered = RenderPic;
    else
        sys->context.pf_render_single_pic = RenderSinglePic;

    video_format_t out_fmt;
    GetDeinterlacingOutput( &sys->context, &out_fmt, &filter->fmt_in.video );
    if( !filter->b_allow_fmt_out_change &&
         out_fmt.i_height != filter->fmt_in.video.i_height )
    {
       goto error;
    }

    InitDeinterlacingContext( &sys->context );

    filter->fmt_out.video   = out_fmt;
    filter->pf_video_filter = Deinterlace;
    filter->pf_flush        = Flush;
    filter->p_sys = sys;

    return VLC_SUCCESS;
error:
    CoTaskMemFree(processorGUIDs);
    if (sys && sys->processor)
        IDirectXVideoProcessor_Release( sys->processor );
    if (processor)
        IDirectXVideoProcessorService_Release(processor);
    if (sys && sys->d3ddev)
        IDirect3DDevice9_Release( sys->d3ddev );
    if (hdecoder_dll)
        FreeLibrary(hdecoder_dll);
    if (d3d9_dll)
        FreeLibrary(d3d9_dll);
    if (dst)
        picture_Release(dst);
    free(sys);

    return VLC_EGENERIC;
}

static void Close(vlc_object_t *obj)
{
    filter_t *filter = (filter_t *)obj;
    filter_sys_t *sys = filter->p_sys;

    IDirect3DSurface9_Release( sys->hw_surface );
    IDirectXVideoProcessor_Release( sys->processor );
    IDirect3DDevice9_Release( sys->d3ddev );
    FreeLibrary( sys->hdecoder_dll );
    FreeLibrary( sys->d3d9_dll );

    free(sys);
}

vlc_module_begin()
    set_description(N_("Direct3D9 deinterlacing filter"))
    set_capability("video filter", 0)
    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VFILTER)
    set_callbacks(Open, Close)
    add_shortcut ("deinterlace")
vlc_module_end()
