/*****************************************************************************
 * d3d11_stereo.c: D3D11 convert Frame Sequential to Side By Side
 *****************************************************************************
 * Copyright (C) 2017 Videolabs SAS, Mohammed Danish
 *
 * Authors: Steve Lhomme <robux4@gmail.com>
 *          Mohammed Danish <shaan3@gmail.com>
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
#include <vlc_configuration.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_atomic.h>

#define COBJMACROS
#include <initguid.h>
#include <d3d11.h>

#include "../../video_chroma/d3d11_fmt.h"

struct filter_sys_t
{
    HANDLE     context_mutex;
    picture_t  *p_previous;
};

static picture_t *Filter(filter_t *p_filter, picture_t *p_src)
{
    filter_sys_t *p_sys = p_filter->p_sys;

    picture_sys_t *p_src_sys = ActivePictureSys(p_src);
    /* TODO keep the left eye int p_previous before getting the right eye and return NULL */
    /* TODO if right eye and left eye have proper PTS, copy in p_outpic */

    picture_t *p_outpic = filter_NewPicture( p_filter );
    if( !p_outpic )
    {
        picture_Release( p_src );
        return NULL;
    }

    picture_CopyProperties( p_outpic, p_src );

    if( p_sys->context_mutex != INVALID_HANDLE_VALUE )
        WaitForSingleObjectEx( p_sys->context_mutex, INFINITE, FALSE );

    /* TODO do the texture copying in here */

    if( p_sys->context_mutex  != INVALID_HANDLE_VALUE )
        ReleaseMutex( p_sys->context_mutex );

    picture_Release( p_src );
    return p_outpic;
}

static int Open(vlc_object_t *obj)
{
    filter_t *filter = (filter_t *)obj;
    HRESULT hr;
    int err = VLC_EGENERIC;

    if (filter->fmt_in.video.i_chroma != VLC_CODEC_D3D11_OPAQUE
     && filter->fmt_in.video.i_chroma != VLC_CODEC_D3D11_OPAQUE_10B)
        return VLC_EGENERIC;
    if (filter->fmt_in.video.i_chroma != filter->fmt_out.video.i_chroma)
        return VLC_EGENERIC;
    if (filter->fmt_in.video.multiview_mode != MULTIVIEW_STEREO_FRAME ||
        filter->fmt_out.video.multiview_mode != MULTIVIEW_STEREO_SBS)
        return VLC_EGENERIC;
    if (filter->fmt_out.video.i_width < 2*filter->fmt_in.video.i_width)
        return VLC_EGENERIC;

    picture_t *dst = filter_NewPicture(filter);
    if (dst == NULL)
        return VLC_EGENERIC;
    if (!dst->p_sys)
    {
        msg_Dbg(filter, "D3D11 opaque without a texture");
        goto done;
    }

    filter_sys_t *sys = calloc(1, sizeof(filter_sys_t));
    if (!sys) {
         err = VLC_ENOMEM;
         goto done;
    }

    ID3D11Device *p_device;
    ID3D11DeviceContext_GetDevice(dst->p_sys->context, &p_device);

    HANDLE context_lock = INVALID_HANDLE_VALUE;
    UINT dataSize = sizeof(context_lock);
    hr = ID3D11Device_GetPrivateData(p_device, &GUID_CONTEXT_MUTEX, &dataSize, &context_lock);
    ID3D11Device_Release(p_device);
    if (FAILED(hr))
        msg_Warn(filter, "No mutex found to lock the decoder");
    sys->context_mutex = context_lock;

    filter->pf_video_filter = Filter;
    filter->p_sys = sys;
    err = VLC_SUCCESS;

done:
    picture_Release(dst);
    return err;
}

static void Close(vlc_object_t *obj)
{
    filter_t *filter = (filter_t *)obj;
    filter_sys_t *sys = filter->p_sys;

    if (sys->p_previous)
        picture_Release(sys->p_previous);

    free(sys);
}

vlc_module_begin()
    set_description(N_("Direct3D11 Stereo filter"))
    set_capability("video converter", 10)
    set_category( CAT_VIDEO )
    set_subcategory( SUBCAT_VIDEO_VFILTER )
    set_callbacks(Open, Close)
vlc_module_end()
