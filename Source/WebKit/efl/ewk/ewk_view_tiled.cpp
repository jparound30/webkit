/*
    Copyright (C) 2009-2010 Samsung Electronics
    Copyright (C) 2009-2010 ProFUSION embedded systems

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "ewk_view.h"

#include "ewk_logging.h"
#include "ewk_private.h"

#include <Evas.h>
#include <eina_safety_checks.h>
#include <ewk_tiled_backing_store.h>

static Ewk_View_Smart_Class _parent_sc = EWK_VIEW_SMART_CLASS_INIT_NULL;

static Eina_Bool _ewk_view_tiled_render_cb(void* data, Ewk_Tile* tile, const Eina_Rectangle* area)
{
    Ewk_View_Private_Data* priv = static_cast<Ewk_View_Private_Data*>(data);
    Eina_Rectangle rect = {area->x + tile->x, area->y + tile->y, area->w, area->h};

    return ewk_view_paint_contents(priv, tile->cairo, &rect);
}

static void* _ewk_view_tiled_updates_process_pre(void* data, Evas_Object* ewkTile)
{
    Ewk_View_Private_Data* priv = static_cast<Ewk_View_Private_Data*>(data);
    ewk_view_layout_if_needed_recursive(priv);
    return 0;
}

static Evas_Object* _ewk_view_tiled_smart_backing_store_add(Ewk_View_Smart_Data* smartData)
{
    Evas_Object* bs = ewk_tiled_backing_store_add(smartData->base.evas);
    ewk_tiled_backing_store_render_cb_set
        (bs, _ewk_view_tiled_render_cb, smartData->_priv);
    ewk_tiled_backing_store_updates_process_pre_set
        (bs, _ewk_view_tiled_updates_process_pre, smartData->_priv);
    return bs;
}

static void
_ewk_view_tiled_contents_size_changed_cb(void* data, Evas_Object* ewkTile, void* eventInfo)
{
    Evas_Coord* size = static_cast<Evas_Coord*>(eventInfo);
    Ewk_View_Smart_Data* smartData = static_cast<Ewk_View_Smart_Data*>(data);

    ewk_tiled_backing_store_contents_resize
        (smartData->backing_store, size[0], size[1]);
}

static void _ewk_view_tiled_smart_add(Evas_Object* ewkTile)
{
    Ewk_View_Smart_Data* sd;

    _parent_sc.sc.add(ewkTile);

    sd = static_cast<Ewk_View_Smart_Data*>(evas_object_smart_data_get(ewkTile));
    if (!sd)
        return;

    evas_object_smart_callback_add(
        sd->main_frame, "contents,size,changed",
        _ewk_view_tiled_contents_size_changed_cb, sd);
    ewk_frame_paint_full_set(sd->main_frame, EINA_TRUE);
}

static Eina_Bool _ewk_view_tiled_smart_scrolls_process(Ewk_View_Smart_Data* smartData)
{
    const Ewk_Scroll_Request* sr;
    const Ewk_Scroll_Request* sr_end;
    size_t count;
    Evas_Coord vw, vh;

    ewk_frame_contents_size_get(smartData->main_frame, &vw, &vh);

    sr = ewk_view_scroll_requests_get(smartData->_priv, &count);
    sr_end = sr + count;
    for (; sr < sr_end; sr++) {
        if (sr->main_scroll)
            ewk_tiled_backing_store_scroll_full_offset_add
                (smartData->backing_store, sr->dx, sr->dy);
        else {
            Evas_Coord sx, sy, sw, sh;

            sx = sr->x;
            sy = sr->y;
            sw = sr->w;
            sh = sr->h;

            if (abs(sr->dx) >= sw || abs(sr->dy) >= sh) {
                /* doubt webkit would be so     stupid... */
                DBG("full page scroll %+03d,%+03d. convert to repaint %d,%d + %dx%d",
                    sr->dx, sr->dy, sx, sy, sw, sh);
                ewk_view_repaint_add(smartData->_priv, sx, sy, sw, sh);
                continue;
            }

            if (sx + sw > vw)
                sw = vw - sx;
            if (sy + sh > vh)
                sh = vh - sy;

            if (sw < 0)
                sw = 0;
            if (sh < 0)
                sh = 0;

            if (!sw || !sh)
                continue;

            sx -= abs(sr->dx);
            sy -= abs(sr->dy);
            sw += abs(sr->dx);
            sh += abs(sr->dy);
            ewk_view_repaint_add(smartData->_priv, sx, sy, sw, sh);
            INF("using repaint for inner frame scolling!");
        }
    }

    return EINA_TRUE;
}

static Eina_Bool _ewk_view_tiled_smart_repaints_process(Ewk_View_Smart_Data* smartData)
{
    const Eina_Rectangle* pr, * pr_end;
    size_t count;
    int sx, sy;

    ewk_frame_scroll_pos_get(smartData->main_frame, &sx, &sy);

    pr = ewk_view_repaints_get(smartData->_priv, &count);
    pr_end = pr + count;
    for (; pr < pr_end; pr++) {
        Eina_Rectangle r;
        r.x = pr->x + sx;
        r.y = pr->y + sy;
        r.w = pr->w;
        r.h = pr->h;
        ewk_tiled_backing_store_update(smartData->backing_store, &r);
    }
    ewk_tiled_backing_store_updates_process(smartData->backing_store);

    return EINA_TRUE;
}

static Eina_Bool _ewk_view_tiled_smart_contents_resize(Ewk_View_Smart_Data* smartData, int width, int height)
{
    ewk_tiled_backing_store_contents_resize(smartData->backing_store, width, height);
    return EINA_TRUE;
}

static Eina_Bool _ewk_view_tiled_smart_zoom_set(Ewk_View_Smart_Data* smartData, float zoom, Evas_Coord centerX, Evas_Coord centerY)
{
    Evas_Coord x, y, w, h;
    Eina_Bool r;
    r = ewk_tiled_backing_store_zoom_set(smartData->backing_store,
                                         &zoom, centerX, centerY, &x, &y);
    if (!r)
        return r;
    ewk_tiled_backing_store_disabled_update_set(smartData->backing_store, EINA_TRUE);
    r = _parent_sc.zoom_set(smartData, zoom, centerX, centerY);
    ewk_frame_scroll_set(smartData->main_frame, -x, -y);
    ewk_frame_scroll_size_get(smartData->main_frame, &w, &h);
    ewk_tiled_backing_store_fix_offsets(smartData->backing_store, w, h);
    ewk_view_scrolls_process(smartData);
    evas_object_smart_calculate(smartData->backing_store);
    evas_object_smart_calculate(smartData->self);
    ewk_tiled_backing_store_disabled_update_set(smartData->backing_store, EINA_FALSE);
    return r;
}

static Eina_Bool _ewk_view_tiled_smart_zoom_weak_set(Ewk_View_Smart_Data* smartData, float zoom, Evas_Coord centerX, Evas_Coord centerY)
{
    return ewk_tiled_backing_store_zoom_weak_set(smartData->backing_store, zoom, centerX, centerY);
}

static void _ewk_view_tiled_smart_zoom_weak_smooth_scale_set(Ewk_View_Smart_Data* smartData, Eina_Bool smoothScale)
{
    ewk_tiled_backing_store_zoom_weak_smooth_scale_set(smartData->backing_store, smoothScale);
}

static void _ewk_view_tiled_smart_flush(Ewk_View_Smart_Data* smartData)
{
    ewk_tiled_backing_store_flush(smartData->backing_store);
    _parent_sc.flush(smartData);
}

static Eina_Bool _ewk_view_tiled_smart_pre_render_region(Ewk_View_Smart_Data* smartData, Evas_Coord x, Evas_Coord y, Evas_Coord width, Evas_Coord height, float zoom)
{
    return ewk_tiled_backing_store_pre_render_region
               (smartData->backing_store, x, y, width, height, zoom);
}

static Eina_Bool _ewk_view_tiled_smart_pre_render_relative_radius(Ewk_View_Smart_Data* smartData, unsigned int n, float zoom)
{
    return ewk_tiled_backing_store_pre_render_relative_radius
               (smartData->backing_store, n, zoom);
}

static void _ewk_view_tiled_smart_pre_render_cancel(Ewk_View_Smart_Data* smartData)
{
    ewk_tiled_backing_store_pre_render_cancel(smartData->backing_store);
}

static Eina_Bool _ewk_view_tiled_smart_disable_render(Ewk_View_Smart_Data* smartData)
{
    return ewk_tiled_backing_store_disable_render(smartData->backing_store);
}

static Eina_Bool _ewk_view_tiled_smart_enable_render(Ewk_View_Smart_Data* smartData)
{
    return ewk_tiled_backing_store_enable_render(smartData->backing_store);
}

Eina_Bool ewk_view_tiled_smart_set(Ewk_View_Smart_Class* api)
{
    if (!ewk_view_base_smart_set(api))
        return EINA_FALSE;

    if (EINA_UNLIKELY(!_parent_sc.sc.add))
        ewk_view_base_smart_set(&_parent_sc);

    api->sc.add = _ewk_view_tiled_smart_add;

    api->backing_store_add = _ewk_view_tiled_smart_backing_store_add;
    api->scrolls_process = _ewk_view_tiled_smart_scrolls_process;
    api->repaints_process = _ewk_view_tiled_smart_repaints_process;
    api->contents_resize = _ewk_view_tiled_smart_contents_resize;
    api->zoom_set = _ewk_view_tiled_smart_zoom_set;
    api->zoom_weak_set = _ewk_view_tiled_smart_zoom_weak_set;
    api->zoom_weak_smooth_scale_set = _ewk_view_tiled_smart_zoom_weak_smooth_scale_set;
    api->flush = _ewk_view_tiled_smart_flush;
    api->pre_render_region = _ewk_view_tiled_smart_pre_render_region;
    api->pre_render_relative_radius = _ewk_view_tiled_smart_pre_render_relative_radius;
    api->pre_render_cancel = _ewk_view_tiled_smart_pre_render_cancel;
    api->disable_render = _ewk_view_tiled_smart_disable_render;
    api->enable_render = _ewk_view_tiled_smart_enable_render;
    return EINA_TRUE;
}

static inline Evas_Smart* _ewk_view_tiled_smart_class_new(void)
{
    static Ewk_View_Smart_Class api = EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION("EWK_View_Tiled");
    static Evas_Smart* smart = 0;

    if (EINA_UNLIKELY(!smart)) {
        ewk_view_tiled_smart_set(&api);
        smart = evas_smart_class_new(&api.sc);
    }

    return smart;
}

Evas_Object* ewk_view_tiled_add(Evas* canvas)
{
    return evas_object_smart_add(canvas, _ewk_view_tiled_smart_class_new());
}

Ewk_Tile_Unused_Cache* ewk_view_tiled_unused_cache_get(const Evas_Object* ewkTile)
{
    Ewk_View_Smart_Data* sd = ewk_view_smart_data_get(ewkTile);
    EINA_SAFETY_ON_NULL_RETURN_VAL(sd, 0);
    return ewk_tiled_backing_store_tile_unused_cache_get(sd->backing_store);
}

void ewk_view_tiled_unused_cache_set(Evas_Object* ewkTile, Ewk_Tile_Unused_Cache* cache)
{
    Ewk_View_Smart_Data* sd = ewk_view_smart_data_get(ewkTile);
    EINA_SAFETY_ON_NULL_RETURN(sd);
    ewk_tiled_backing_store_tile_unused_cache_set(sd->backing_store, cache);
}
