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

#define __STDC_FORMAT_MACROS
#include "config.h"
#include "ewk_tiled_matrix.h"

#include "ewk_tiled_backing_store.h"
#include "ewk_tiled_private.h"
#include <Eina.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h> // XXX remove me later
#include <stdlib.h>
#include <string.h>

struct _Ewk_Tile_Matrix {
    Eina_Matrixsparse* matrix;
    Ewk_Tile_Unused_Cache* tilieUnusedCache;
    Evas_Colorspace cspace;
    struct {
        void (*callback)(void* data, Ewk_Tile* tile, const Eina_Rectangle* update);
        void* data;
    } render;
    unsigned int frozen;
    Eina_List* updates;
    struct {
        Evas_Coord width, height;
    } tile;
#ifdef DEBUG_MEM_LEAKS
    struct {
        struct {
            uint64_t allocated, freed;
        } tiles, bytes;
    } stats;
#endif
};

#ifdef DEBUG_MEM_LEAKS
static uint64_t tiles_leaked = 0;
static uint64_t bytes_leaked = 0;
#endif

/* called when matrixsparse is resized or freed */
static void _ewk_tile_matrix_cell_free(void* userData, void* cellData)
{
    Ewk_Tile_Matrix* tileMatrix = static_cast<Ewk_Tile_Matrix*>(userData);
    Ewk_Tile* tile = static_cast<Ewk_Tile*>(cellData);

    if (!tile)
        return;

    ewk_tile_unused_cache_freeze(tileMatrix->tilieUnusedCache);

    if (tile->updates || tile->stats.full_update)
        tileMatrix->updates = eina_list_remove(tileMatrix->updates, tile);

    if (tile->visible)
        ERR("freeing cell that is visible, leaking tile %p", tile);
    else {
        if (!ewk_tile_unused_cache_tile_get(tileMatrix->tilieUnusedCache, tile))
            ERR("tile %p was not in cache %p? leaking...", tile, tileMatrix->tilieUnusedCache);
        else {
            DBG("tile cell does not exist anymore, free it %p", tile);
#ifdef DEBUG_MEM_LEAKS
            tileMatrix->stats.bytes.freed += tile->bytes;
            tileMatrix->stats.tiles.freed++;
#endif
            ewk_tile_free(tile);
        }
    }

    ewk_tile_unused_cache_thaw(tileMatrix->tilieUnusedCache);
}

/* called when cache of unused tile is flushed */
static void _ewk_tile_matrix_tile_free(void* data, Ewk_Tile* tile)
{
    Ewk_Tile_Matrix* tileMatrix = static_cast<Ewk_Tile_Matrix*>(data);
    Eina_Matrixsparse_Cell* cell;

    if (!eina_matrixsparse_cell_idx_get(tileMatrix->matrix, tile->row, tile->col, &cell)) {
        ERR("removing tile %p that was not in the matrix? Leaking...", tile);
        return;
    }

    if (!cell) {
        ERR("removing tile %p that was not in the matrix? Leaking...", tile);
        return;
    }

    if (tile->updates || tile->stats.full_update)
        tileMatrix->updates = eina_list_remove(tileMatrix->updates, tile);

    /* set to null to avoid double free */
    eina_matrixsparse_cell_data_replace(cell, 0, 0);
    eina_matrixsparse_cell_clear(cell);

    if (EINA_UNLIKELY(!!tile->visible)) {
        ERR("cache of unused tiles requesting deletion of used tile %p? "
            "Leaking...", tile);
        return;
    }

#ifdef DEBUG_MEM_LEAKS
    tileMatrix->stats.bytes.freed += tile->bytes;
    tileMatrix->stats.tiles.freed++;
#endif

    ewk_tile_free(tile);
}

/**
 * Creates a new matrix of tiles.
 *
 * The tile matrix is responsible for keeping tiles around and
 * providing fast access to them. One can use it to retrieve new or
 * existing tiles and give them back, allowing them to be
 * freed/replaced by the cache.
 *
 * @param tuc cache of unused tiles or @c 0 to create one
 *        automatically.
 * @param columns number of columns in the matrix.
 * @param rows number of rows in the matrix.
 * @param cspace the color space used to create tiles in this matrix.
 * @param render_cb function used to render given tile update.
 * @param render_data context to give back to @a render_cb.
 *
 * @return newly allocated instance on success, @c 0 on failure.
 */
Ewk_Tile_Matrix* ewk_tile_matrix_new(Ewk_Tile_Unused_Cache* tileUnusedCache, unsigned long columns, unsigned long rows, Evas_Colorspace colorSpace, void (*renderCallback)(void* data, Ewk_Tile* tile, const Eina_Rectangle* update), const void* renderData)
{
    Ewk_Tile_Matrix* tileMatrix = static_cast<Ewk_Tile_Matrix*>(calloc(1, sizeof(Ewk_Tile_Matrix)));
    if (!tileMatrix)
        return 0;

    tileMatrix->matrix = eina_matrixsparse_new(rows, columns, _ewk_tile_matrix_cell_free, tileMatrix);
    if (!tileMatrix->matrix) {
        ERR("could not create sparse matrix.");
        free(tileMatrix);
        return 0;
    }

    if (tileUnusedCache)
        tileMatrix->tilieUnusedCache = ewk_tile_unused_cache_ref(tileUnusedCache);
    else {
        tileMatrix->tilieUnusedCache = ewk_tile_unused_cache_new(40960000);
        if (!tileMatrix->tilieUnusedCache) {
            ERR("no cache of unused tile!");
            eina_matrixsparse_free(tileMatrix->matrix);
            free(tileMatrix);
            return 0;
        }
    }

    tileMatrix->cspace = colorSpace;
    tileMatrix->render.callback = renderCallback;
    tileMatrix->render.data = (void*)renderData;
    tileMatrix->tile.width = DEFAULT_TILE_W;
    tileMatrix->tile.height = DEFAULT_TILE_H;

    return tileMatrix;
}

/**
 * Destroys tiles matrix, releasing its resources.
 *
 * The cache instance is unreferenced, possibly freeing it.
 */
void ewk_tile_matrix_free(Ewk_Tile_Matrix* tileMatrix)
{
#ifdef DEBUG_MEM_LEAKS
    uint64_t tiles, bytes;
#endif

    EINA_SAFETY_ON_NULL_RETURN(tileMatrix);
    ewk_tile_unused_cache_freeze(tileMatrix->tilieUnusedCache);

    eina_matrixsparse_free(tileMatrix->matrix);

    ewk_tile_unused_cache_thaw(tileMatrix->tilieUnusedCache);
    ewk_tile_unused_cache_unref(tileMatrix->tilieUnusedCache);

#ifdef DEBUG_MEM_LEAKS
    tiles = tileMatrix->stats.tiles.allocated - tileMatrix->stats.tiles.freed;
    bytes = tileMatrix->stats.bytes.allocated - tileMatrix->stats.bytes.freed;

    tiles_leaked += tiles;
    bytes_leaked += bytes;

    if (tiles || bytes)
        ERR("tiled matrix leaked: tiles[+%" PRIu64 ",-%" PRIu64 ":%" PRIu64 "] "
            "bytes[+%" PRIu64 ",-%" PRIu64 ":%" PRIu64 "]",
            tileMatrix->stats.tiles.allocated, tileMatrix->stats.tiles.freed, tiles,
            tileMatrix->stats.bytes.allocated, tileMatrix->stats.bytes.freed, bytes);
    else if (tiles_leaked || bytes_leaked)
        WRN("tiled matrix had no leaks: tiles[+%" PRIu64 ",-%" PRIu64 "] "
            "bytes[+%" PRIu64 ",-%" PRIu64 "], but some other leaked "
            "%" PRIu64 " tiles (%" PRIu64 " bytes)",
            tileMatrix->stats.tiles.allocated, tileMatrix->stats.tiles.freed,
            tileMatrix->stats.bytes.allocated, tileMatrix->stats.bytes.freed,
            tiles_leaked, bytes_leaked);
    else
        INF("tiled matrix had no leaks: tiles[+%" PRIu64 ",-%" PRIu64 "] "
            "bytes[+%" PRIu64 ",-%" PRIu64 "]",
            tileMatrix->stats.tiles.allocated, tileMatrix->stats.tiles.freed,
            tileMatrix->stats.bytes.allocated, tileMatrix->stats.bytes.freed);
#endif

    free(tileMatrix);
}

/**
 * Resize matrix to given number of rows and columns.
 */
void ewk_tile_matrix_resize(Ewk_Tile_Matrix* tileMatrix, unsigned long cols, unsigned long rows)
{
    EINA_SAFETY_ON_NULL_RETURN(tileMatrix);
    eina_matrixsparse_size_set(tileMatrix->matrix, rows, cols);
}

/**
 * Get the cache of unused tiles in use by given matrix.
 *
 * No reference is taken to the cache.
 */
Ewk_Tile_Unused_Cache* ewk_tile_matrix_unused_cache_get(const Ewk_Tile_Matrix* tileMatrix)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(tileMatrix, 0);
    return tileMatrix->tilieUnusedCache;
}

/**
 * Get the exact tile for the given position and zoom.
 *
 * If the tile.widthas unused then it's removed from the cache.
 *
 * After usage, please give it back using
 * ewk_tile_matrix_tile_put(). If you just want to check if it exists,
 * then use ewk_tile_matrix_tile_exact_exists().
 *
 * @param tileMatrix the tile matrix to get tile from.
 * @param column the column number.
 * @param row the row number.
 * @param zoom the exact zoom to use.
 *
 * @return The tile instance or @c 0 if none is found. If the tile
 *         was in the unused cache it will be @b removed (thus
 *         considered used) and one should give it back with
 *         ewk_tile_matrix_tile_put() afterwards.
 *
 * @see ewk_tile_matrix_tile_exact_get()
 */
Ewk_Tile* ewk_tile_matrix_tile_exact_get(Ewk_Tile_Matrix* tileMatrix, unsigned long column, unsigned long row, float zoom)
{
    Ewk_Tile* tile = static_cast<Ewk_Tile*>(eina_matrixsparse_data_idx_get(tileMatrix->matrix, row, column));
    if (!tile)
        return 0;

    if (tile->zoom == zoom)
        goto end;

end:
    if (!tile->visible) {
        if (!ewk_tile_unused_cache_tile_get(tileMatrix->tilieUnusedCache, tile))
            WRN("Ewk_Tile was unused but not in cache? bug!");
    }

    return tile;
}

/**
 * Checks if tile of given zoom exists in matrix.
 *
 * @param tileMatrix the tile matrix to check tile existence.
 * @param column the column number.
 * @param row the row number.
 * @param zoom the exact zoom to query.
 *
 * @return @c EINA_TRUE if found, @c EINA_FALSE otherwise.
 *
 * @see ewk_tile_matrix_tile_exact_get()
 */
Eina_Bool ewk_tile_matrix_tile_exact_exists(Ewk_Tile_Matrix* tileMatrix, unsigned long column, unsigned long row, float zoom)
{
    if (!eina_matrixsparse_data_idx_get(tileMatrix->matrix, row, column))
        return EINA_FALSE;

    return EINA_TRUE;
}

/**
 * Create a new tile at given position and zoom level in the matrix.
 *
 * The newly created tile is considered in use and not put into cache
 * of unused tiles. After it is used one should call
 * ewk_tile_matrix_tile_put() to give it back to matrix.
 *
 * @param tileMatrix the tile matrix to create tile on.
 * @param column the column number.
 * @param row the row number.
 * @param zoom the level to create tile, used to determine tile size.
 */
Ewk_Tile* ewk_tile_matrix_tile_new(Ewk_Tile_Matrix* tileMatrix, Evas* canvas, unsigned long column, unsigned long row, float zoom)
{
    Evas_Coord tileWidth, tileHeight;
    Ewk_Tile* tile;

    EINA_SAFETY_ON_NULL_RETURN_VAL(tileMatrix, 0);
    EINA_SAFETY_ON_FALSE_RETURN_VAL(zoom > 0.0, 0);

    tileWidth = tileMatrix->tile.width;
    tileHeight = tileMatrix->tile.height;

    tile = ewk_tile_new(canvas, tileWidth, tileHeight, zoom, tileMatrix->cspace);
    if (!tile) {
        ERR("could not create tile %dx%d at %f, cspace=%d", tileWidth, tileHeight, (double)zoom, tileMatrix->cspace);
        return 0;
    }

    if (!eina_matrixsparse_data_idx_set(tileMatrix->matrix, row, column, tile)) {
        ERR("could not set matrix cell, row/col outside matrix dimensions!");
        ewk_tile_free(tile);
        return 0;
    }

    tile->col = column;
    tile->row = row;
    tile->x = column * tileWidth;
    tile->y = row * tileHeight;

    cairo_translate(tile->cairo, -tile->x, -tile->y);

    tile->stats.full_update = EINA_TRUE;
    tileMatrix->updates = eina_list_append(tileMatrix->updates, tile);

#ifdef DEBUG_MEM_LEAKS
    tileMatrix->stats.bytes.allocated += tile->bytes;
    tileMatrix->stats.tiles.allocated++;
#endif

    return tile;
}

/**
 * Gives back the tile to the tile matrix.
 *
 * This will report the tile is no longer in use by the one that got
 * it with ewk_tile_matrix_tile_exact_get().
 *
 * Any previous reference to tile should be released
 * (ewk_tile.heightide()) before calling this function, so it will
 * be known if it is not visibile anymore and thus can be put into the
 * unused cache.
 *
 * @param tileMatrix the tile matrix to return tile to.
 * @param t the tile instance to return, must @b not be @c 0.
 * @param last_used time in which tile.widthas last used.
 *
 * @return #EINA_TRUE on success or #EINA_FALSE on failure.
 */
Eina_Bool ewk_tile_matrix_tile_put(Ewk_Tile_Matrix* tileMatrix, Ewk_Tile* tile, double lastUsed)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(tileMatrix, EINA_FALSE);
    EINA_SAFETY_ON_NULL_RETURN_VAL(tile, EINA_FALSE);

    if (tile->visible)
        return EINA_TRUE;

    tile->stats.last_used = lastUsed;
    return ewk_tile_unused_cache_tile_put(tileMatrix->tilieUnusedCache, tile, _ewk_tile_matrix_tile_free, tileMatrix);
}

Eina_Bool ewk_tile_matrix_tile_update(Ewk_Tile_Matrix* tileMatrix, unsigned long col, unsigned long row, const Eina_Rectangle* update)
{
    Eina_Rectangle newUpdate;
    EINA_SAFETY_ON_NULL_RETURN_VAL(tileMatrix, EINA_FALSE);
    EINA_SAFETY_ON_NULL_RETURN_VAL(update, EINA_FALSE);

    memcpy(&newUpdate, update, sizeof(newUpdate));
    // check update is valid, otherwise return EINA_FALSE
    if (update->x < 0 || update->y < 0 || update->w <= 0 || update->h <= 0) {
        ERR("invalid update region.");
        return EINA_FALSE;
    }

    if (update->x + update->w - 1 >= tileMatrix->tile.width)
        newUpdate.w = tileMatrix->tile.width - update->x;
    if (update->y + update->h - 1 >= tileMatrix->tile.height)
        newUpdate.h = tileMatrix->tile.height - update->y;

    Ewk_Tile* tile = static_cast<Ewk_Tile*>(eina_matrixsparse_data_idx_get(tileMatrix->matrix, row, col));
    if (!tile)
        return EINA_TRUE;

    if (!tile->updates && !tile->stats.full_update)
        tileMatrix->updates = eina_list_append(tileMatrix->updates, tile);
    ewk_tile_update_area(tile, &newUpdate);

    return EINA_TRUE;
}

Eina_Bool ewk_tile_matrix_tile_update_full(Ewk_Tile_Matrix* tileMatrix, unsigned long column, unsigned long row)
{
    Eina_Matrixsparse_Cell* cell;
    EINA_SAFETY_ON_NULL_RETURN_VAL(tileMatrix, EINA_FALSE);

    if (!eina_matrixsparse_cell_idx_get(tileMatrix->matrix, row, column, &cell))
        return EINA_FALSE;

    if (!cell)
        return EINA_TRUE;

    Ewk_Tile* tile = static_cast<Ewk_Tile*>(eina_matrixsparse_cell_data_get(cell));
    if (!tile) {
        CRITICAL("matrix cell with no tile!");
        return EINA_TRUE;
    }

    if (!tile->updates && !tile->stats.full_update)
        tileMatrix->updates = eina_list_append(tileMatrix->updates, tile);
    ewk_tile_update_full(tile);

    return EINA_TRUE;
}

void ewk_tile_matrix_tile_updates_clear(Ewk_Tile_Matrix* tileMatrix, Ewk_Tile* tile)
{
    EINA_SAFETY_ON_NULL_RETURN(tileMatrix);
    if (!tile->updates && !tile->stats.full_update)
        return;
    ewk_tile_updates_clear(tile);
    tileMatrix->updates = eina_list_remove(tileMatrix->updates, tile);
}

static Eina_Bool _ewk_tile_matrix_slicer_setup(Ewk_Tile_Matrix* tileMatrix, const Eina_Rectangle* area, float zoom, Eina_Tile_Grid_Slicer* slicer)
{
    unsigned long rows, cols;
    Evas_Coord x, y, width, height, tileWidth, tileHeight;

    if (area->w <= 0 || area->h <= 0) {
        WRN("invalid area region: %d,%d+%dx%d.",
            area->x, area->y, area->w, area->h);
        return EINA_FALSE;
    }

    x = area->x;
    y = area->y;
    width = area->w;
    height = area->h;

    tileWidth = tileMatrix->tile.width;
    tileHeight = tileMatrix->tile.height;

    // cropping area region to fit matrix
    eina_matrixsparse_size_get(tileMatrix->matrix, &rows, &cols);
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }

    if (y + height - 1 > (long)(rows * tileHeight))
        height = rows * tileHeight - y;
    if (x + width - 1 > (long)(cols * tileWidth))
        width = cols * tileWidth - x;

    return eina_tile_grid_slicer_setup(slicer, x, y, width, height, tileWidth, tileHeight);
}


Eina_Bool ewk_tile_matrix_update(Ewk_Tile_Matrix* tileMatrix, const Eina_Rectangle* update, float zoom)
{
    const Eina_Tile_Grid_Info* info;
    Eina_Tile_Grid_Slicer slicer;
    EINA_SAFETY_ON_NULL_RETURN_VAL(tileMatrix, EINA_FALSE);
    EINA_SAFETY_ON_NULL_RETURN_VAL(update, EINA_FALSE);

    if (update->w < 1 || update->h < 1) {
        DBG("Why we get updates with empty areas? %d,%d+%dx%d at zoom %f",
            update->x, update->y, update->w, update->h, zoom);
        return EINA_TRUE;
    }

    if (!_ewk_tile_matrix_slicer_setup(tileMatrix, update, zoom, &slicer)) {
        ERR("Could not setup slicer for update %d,%d+%dx%d at zoom %f",
            update->x, update->y, update->w, update->h, zoom);
        return EINA_FALSE;
    }

    while (eina_tile_grid_slicer_next(&slicer, &info)) {
        unsigned long col, row;
        col = info->col;
        row = info->row;

        Ewk_Tile* tile = static_cast<Ewk_Tile*>(eina_matrixsparse_data_idx_get(tileMatrix->matrix, row, col));
        if (!tile)
            continue;

        if (!tile->updates && !tile->stats.full_update)
            tileMatrix->updates = eina_list_append(tileMatrix->updates, tile);
        if (info->full)
            ewk_tile_update_full(tile);
        else
            ewk_tile_update_area(tile, &info->rect);
    }


    return EINA_TRUE;
}

void ewk_tile_matrix_updates_process(Ewk_Tile_Matrix* tileMatrix)
{
    Eina_List* list, *listNext;
    void* item;
    EINA_SAFETY_ON_NULL_RETURN(tileMatrix);

    // process updates, unflag tiles
    EINA_LIST_FOREACH_SAFE(tileMatrix->updates, list, listNext, item) {
        Ewk_Tile* tile = static_cast<Ewk_Tile*>(item);
        ewk_tile_updates_process(tile, tileMatrix->render.callback, tileMatrix->render.data);
        if (tile->visible) {
            ewk_tile_updates_clear(tile);
            tileMatrix->updates = eina_list_remove_list(tileMatrix->updates, list);
        }
    }
}

void ewk_tile_matrix_updates_clear(Ewk_Tile_Matrix* tileMatrix)
{
    void* item;
    EINA_SAFETY_ON_NULL_RETURN(tileMatrix);

    EINA_LIST_FREE(tileMatrix->updates, item)
        ewk_tile_updates_clear(static_cast<Ewk_Tile*>(item));
    tileMatrix->updates = 0;
}

// remove me later!
void ewk_tile_matrix_dbg(const Ewk_Tile_Matrix* tileMatrix)
{
    Eina_Iterator* iterator = eina_matrixsparse_iterator_complete_new(tileMatrix->matrix);
    Eina_Matrixsparse_Cell* cell;
    Eina_Bool last_empty = EINA_FALSE;

#ifdef DEBUG_MEM_LEAKS
    printf("Ewk_Tile Matrix: tiles[+%" PRIu64 ",-%" PRIu64 ":%" PRIu64 "] "
           "bytes[+%" PRIu64 ",-%" PRIu64 ":%" PRIu64 "]\n",
           tileMatrix->stats.tiles.allocated, tileMatrix->stats.tiles.freed,
           tileMatrix->stats.tiles.allocated - tileMatrix->stats.tiles.freed,
           tileMatrix->stats.bytes.allocated, tileMatrix->stats.bytes.freed,
           tileMatrix->stats.bytes.allocated - tileMatrix->stats.bytes.freed);
#else
    printf("Ewk_Tile Matrix:\n");
#endif

    EINA_ITERATOR_FOREACH(iterator, cell) {
        unsigned long row, column;
        eina_matrixsparse_cell_position_get(cell, &row, &column);

        Ewk_Tile* tile = static_cast<Ewk_Tile*>(eina_matrixsparse_cell_data_get(cell));
        if (!tile) {
            if (!last_empty) {
                last_empty = EINA_TRUE;
                printf("Empty:");
            }
            printf(" [%lu,%lu]", column, row);
        } else {
            if (last_empty) {
                last_empty = EINA_FALSE;
                printf("\n");
            }
            printf("%3lu,%3lu %10p:", column, row, tile);
            printf(" [%3lu,%3lu + %dx%d @ %0.3f]%c", tile->col, tile->row, tile->width, tile->height, tile->zoom, tile->visible ? '*' : ' ');
            printf("\n");
        }
    }
    if (last_empty)
        printf("\n");
    eina_iterator_free(iterator);

    ewk_tile_unused_cache_dbg(tileMatrix->tilieUnusedCache);
}

/**
 * Freeze matrix to not do maintenance tasks.
 *
 * Maintenance tasks optimize usage, but maybe we know we should hold
 * on them until we do the last operation, in this case we freeze
 * while operating and then thaw when we're done.
 *
 * @see ewk_tile_matrix_thaw()
 */
void ewk_tile_matrix_freeze(Ewk_Tile_Matrix* tileMatrix)
{
    EINA_SAFETY_ON_NULL_RETURN(tileMatrix);
    if (!tileMatrix->frozen)
        ewk_tile_unused_cache_freeze(tileMatrix->tilieUnusedCache);
    tileMatrix->frozen++;
}

/**
 * Unfreezes maintenance tasks.
 *
 * If this is the last counterpart of freeze, then maintenance tasks
 * will run immediately.
 */
void ewk_tile_matrix_thaw(Ewk_Tile_Matrix* tileMatrix)
{
    EINA_SAFETY_ON_NULL_RETURN(tileMatrix);
    if (!tileMatrix->frozen) {
        ERR("thawing more than freezing!");
        return;
    }

    tileMatrix->frozen--;
    if (!tileMatrix->frozen)
        ewk_tile_unused_cache_thaw(tileMatrix->tilieUnusedCache);
}
