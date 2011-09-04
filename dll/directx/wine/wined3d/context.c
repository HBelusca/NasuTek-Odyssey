/*
 * Context and render target management in wined3d
 *
 * Copyright 2007-2008 Stefan Dösinger for CodeWeavers
 * Copyright 2009-2011 Henri Verbeet for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include <stdio.h>
#ifdef HAVE_FLOAT_H
# include <float.h>
#endif
#include "wined3d_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d);

static DWORD wined3d_context_tls_idx;

/* FBO helper functions */

/* GL locking is done by the caller */
void context_bind_fbo(struct wined3d_context *context, GLenum target, GLuint *fbo)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;
    GLuint f;

    if (!fbo)
    {
        f = 0;
    }
    else
    {
        if (!*fbo)
        {
            gl_info->fbo_ops.glGenFramebuffers(1, fbo);
            checkGLcall("glGenFramebuffers()");
            TRACE("Created FBO %u.\n", *fbo);
        }
        f = *fbo;
    }

    switch (target)
    {
        case GL_READ_FRAMEBUFFER:
            if (context->fbo_read_binding == f) return;
            context->fbo_read_binding = f;
            break;

        case GL_DRAW_FRAMEBUFFER:
            if (context->fbo_draw_binding == f) return;
            context->fbo_draw_binding = f;
            break;

        case GL_FRAMEBUFFER:
            if (context->fbo_read_binding == f
                    && context->fbo_draw_binding == f) return;
            context->fbo_read_binding = f;
            context->fbo_draw_binding = f;
            break;

        default:
            FIXME("Unhandled target %#x.\n", target);
            break;
    }

    gl_info->fbo_ops.glBindFramebuffer(target, f);
    checkGLcall("glBindFramebuffer()");
}

/* GL locking is done by the caller */
static void context_clean_fbo_attachments(const struct wined3d_gl_info *gl_info, GLenum target)
{
    unsigned int i;

    for (i = 0; i < gl_info->limits.buffers; ++i)
    {
        gl_info->fbo_ops.glFramebufferTexture2D(target, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
        checkGLcall("glFramebufferTexture2D()");
    }
    gl_info->fbo_ops.glFramebufferTexture2D(target, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    checkGLcall("glFramebufferTexture2D()");

    gl_info->fbo_ops.glFramebufferTexture2D(target, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    checkGLcall("glFramebufferTexture2D()");
}

/* GL locking is done by the caller */
static void context_destroy_fbo(struct wined3d_context *context, GLuint *fbo)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;

    context_bind_fbo(context, GL_FRAMEBUFFER, fbo);
    context_clean_fbo_attachments(gl_info, GL_FRAMEBUFFER);
    context_bind_fbo(context, GL_FRAMEBUFFER, NULL);

    gl_info->fbo_ops.glDeleteFramebuffers(1, fbo);
    checkGLcall("glDeleteFramebuffers()");
}

/* GL locking is done by the caller */
static void context_apply_attachment_filter_states(const struct wined3d_context *context,
        struct wined3d_surface *surface, DWORD location)
{
    /* Update base texture states array */
    if (surface->container.type == WINED3D_CONTAINER_TEXTURE)
    {
        struct wined3d_texture *texture = surface->container.u.texture;
        struct wined3d_device *device = surface->resource.device;
        BOOL update_minfilter = FALSE;
        BOOL update_magfilter = FALSE;
        struct gl_texture *gl_tex;

        switch (location)
        {
            case SFLAG_INTEXTURE:
            case SFLAG_INSRGBTEX:
                gl_tex = wined3d_texture_get_gl_texture(texture,
                        context->gl_info, location == SFLAG_INSRGBTEX);
                break;

            default:
                ERR("Unsupported location %s (%#x).\n", debug_surflocation(location), location);
                return;
        }

        if (gl_tex->states[WINED3DTEXSTA_MINFILTER] != WINED3DTEXF_POINT
            || gl_tex->states[WINED3DTEXSTA_MIPFILTER] != WINED3DTEXF_NONE)
        {
            gl_tex->states[WINED3DTEXSTA_MINFILTER] = WINED3DTEXF_POINT;
            gl_tex->states[WINED3DTEXSTA_MIPFILTER] = WINED3DTEXF_NONE;
            update_minfilter = TRUE;
        }

        if (gl_tex->states[WINED3DTEXSTA_MAGFILTER] != WINED3DTEXF_POINT)
        {
            gl_tex->states[WINED3DTEXSTA_MAGFILTER] = WINED3DTEXF_POINT;
            update_magfilter = TRUE;
        }

        if (texture->bind_count)
        {
            WARN("Render targets should not be bound to a sampler\n");
            IWineD3DDeviceImpl_MarkStateDirty(device, STATE_SAMPLER(texture->sampler));
        }

        if (update_minfilter || update_magfilter)
        {
            GLenum target, bind_target;
            GLint old_binding;

            target = surface->texture_target;
            if (target == GL_TEXTURE_2D)
            {
                bind_target = GL_TEXTURE_2D;
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_binding);
            }
            else if (target == GL_TEXTURE_RECTANGLE_ARB)
            {
                bind_target = GL_TEXTURE_RECTANGLE_ARB;
                glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE_ARB, &old_binding);
            }
            else
            {
                bind_target = GL_TEXTURE_CUBE_MAP_ARB;
                glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP_ARB, &old_binding);
            }

            glBindTexture(bind_target, gl_tex->name);
            if (update_minfilter) glTexParameteri(bind_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (update_magfilter) glTexParameteri(bind_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(bind_target, old_binding);
        }

        checkGLcall("apply_attachment_filter_states()");
    }
}

/* GL locking is done by the caller */
void context_attach_depth_stencil_fbo(struct wined3d_context *context,
        GLenum fbo_target, struct wined3d_surface *depth_stencil, BOOL use_render_buffer)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;

    TRACE("Attach depth stencil %p\n", depth_stencil);

    if (depth_stencil)
    {
        DWORD format_flags = depth_stencil->resource.format->flags;

        if (use_render_buffer && depth_stencil->current_renderbuffer)
        {
            if (format_flags & WINED3DFMT_FLAG_DEPTH)
            {
                gl_info->fbo_ops.glFramebufferRenderbuffer(fbo_target, GL_DEPTH_ATTACHMENT,
                        GL_RENDERBUFFER, depth_stencil->current_renderbuffer->id);
                checkGLcall("glFramebufferRenderbuffer()");
            }

            if (format_flags & WINED3DFMT_FLAG_STENCIL)
            {
                gl_info->fbo_ops.glFramebufferRenderbuffer(fbo_target, GL_STENCIL_ATTACHMENT,
                        GL_RENDERBUFFER, depth_stencil->current_renderbuffer->id);
                checkGLcall("glFramebufferRenderbuffer()");
            }
        }
        else
        {
            surface_prepare_texture(depth_stencil, gl_info, FALSE);
            context_apply_attachment_filter_states(context, depth_stencil, SFLAG_INTEXTURE);

            if (format_flags & WINED3DFMT_FLAG_DEPTH)
            {
                gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_DEPTH_ATTACHMENT,
                        depth_stencil->texture_target, depth_stencil->texture_name,
                        depth_stencil->texture_level);
                checkGLcall("glFramebufferTexture2D()");
            }

            if (format_flags & WINED3DFMT_FLAG_STENCIL)
            {
                gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_STENCIL_ATTACHMENT,
                        depth_stencil->texture_target, depth_stencil->texture_name,
                        depth_stencil->texture_level);
                checkGLcall("glFramebufferTexture2D()");
            }
        }

        if (!(format_flags & WINED3DFMT_FLAG_DEPTH))
        {
            gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
            checkGLcall("glFramebufferTexture2D()");
        }

        if (!(format_flags & WINED3DFMT_FLAG_STENCIL))
        {
            gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
            checkGLcall("glFramebufferTexture2D()");
        }
    }
    else
    {
        gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        checkGLcall("glFramebufferTexture2D()");

        gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        checkGLcall("glFramebufferTexture2D()");
    }
}

/* GL locking is done by the caller */
static void context_attach_surface_fbo(const struct wined3d_context *context,
        GLenum fbo_target, DWORD idx, struct wined3d_surface *surface, DWORD location)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;

    TRACE("Attach surface %p to %u\n", surface, idx);

    if (surface && surface->resource.format->id != WINED3DFMT_NULL)
    {
        BOOL srgb;

        switch (location)
        {
            case SFLAG_INTEXTURE:
            case SFLAG_INSRGBTEX:
                srgb = location == SFLAG_INSRGBTEX;
                surface_prepare_texture(surface, gl_info, srgb);
                context_apply_attachment_filter_states(context, surface, location);
                gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_COLOR_ATTACHMENT0 + idx,
                        surface->texture_target, surface_get_texture_name(surface, gl_info, srgb),
                        surface->texture_level);
                break;

            default:
                ERR("Unsupported location %s (%#x).\n", debug_surflocation(location), location);
                break;
        }
        checkGLcall("glFramebufferTexture2D()");
    }
    else
    {
        gl_info->fbo_ops.glFramebufferTexture2D(fbo_target, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_2D, 0, 0);
        checkGLcall("glFramebufferTexture2D()");
    }
}

/* GL locking is done by the caller */
void context_check_fbo_status(struct wined3d_context *context, GLenum target)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;
    GLenum status;

    status = gl_info->fbo_ops.glCheckFramebufferStatus(target);
    if (status == GL_FRAMEBUFFER_COMPLETE)
    {
        TRACE("FBO complete\n");
    }
    else
    {
        struct wined3d_surface *attachment;
        unsigned int i;

        FIXME("FBO status %s (%#x)\n", debug_fbostatus(status), status);

        if (!context->current_fbo)
        {
            ERR("FBO 0 is incomplete, driver bug?\n");
            return;
        }

        FIXME("\tLocation %s (%#x).\n", debug_surflocation(context->current_fbo->location),
                context->current_fbo->location);

        /* Dump the FBO attachments */
        for (i = 0; i < gl_info->limits.buffers; ++i)
        {
            attachment = context->current_fbo->render_targets[i];
            if (attachment)
            {
                FIXME("\tColor attachment %d: (%p) %s %ux%u\n",
                        i, attachment, debug_d3dformat(attachment->resource.format->id),
                        attachment->pow2Width, attachment->pow2Height);
            }
        }
        attachment = context->current_fbo->depth_stencil;
        if (attachment)
        {
            FIXME("\tDepth attachment: (%p) %s %ux%u\n",
                    attachment, debug_d3dformat(attachment->resource.format->id),
                    attachment->pow2Width, attachment->pow2Height);
        }
    }
}

static struct fbo_entry *context_create_fbo_entry(struct wined3d_context *context,
        struct wined3d_surface **render_targets, struct wined3d_surface *depth_stencil, DWORD location)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;
    struct fbo_entry *entry;

    entry = HeapAlloc(GetProcessHeap(), 0, sizeof(*entry));
    entry->render_targets = HeapAlloc(GetProcessHeap(), 0, gl_info->limits.buffers * sizeof(*entry->render_targets));
    memcpy(entry->render_targets, render_targets, gl_info->limits.buffers * sizeof(*entry->render_targets));
    entry->depth_stencil = depth_stencil;
    entry->location = location;
    entry->attached = FALSE;
    entry->id = 0;

    return entry;
}

/* GL locking is done by the caller */
static void context_reuse_fbo_entry(struct wined3d_context *context, GLenum target,
        struct wined3d_surface **render_targets, struct wined3d_surface *depth_stencil,
        DWORD location, struct fbo_entry *entry)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;

    context_bind_fbo(context, target, &entry->id);
    context_clean_fbo_attachments(gl_info, target);

    memcpy(entry->render_targets, render_targets, gl_info->limits.buffers * sizeof(*entry->render_targets));
    entry->depth_stencil = depth_stencil;
    entry->location = location;
    entry->attached = FALSE;
}

/* GL locking is done by the caller */
static void context_destroy_fbo_entry(struct wined3d_context *context, struct fbo_entry *entry)
{
    if (entry->id)
    {
        TRACE("Destroy FBO %d\n", entry->id);
        context_destroy_fbo(context, &entry->id);
    }
    --context->fbo_entry_count;
    list_remove(&entry->entry);
    HeapFree(GetProcessHeap(), 0, entry->render_targets);
    HeapFree(GetProcessHeap(), 0, entry);
}


/* GL locking is done by the caller */
static struct fbo_entry *context_find_fbo_entry(struct wined3d_context *context, GLenum target,
        struct wined3d_surface **render_targets, struct wined3d_surface *depth_stencil, DWORD location)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;
    struct fbo_entry *entry;

    if (depth_stencil && render_targets && render_targets[0])
    {
        if (depth_stencil->resource.width < render_targets[0]->resource.width ||
            depth_stencil->resource.height < render_targets[0]->resource.height)
        {
            WARN("Depth stencil is smaller than the primary color buffer, disabling\n");
            depth_stencil = NULL;
        }
    }

    LIST_FOR_EACH_ENTRY(entry, &context->fbo_list, struct fbo_entry, entry)
    {
        if (!memcmp(entry->render_targets,
                render_targets, gl_info->limits.buffers * sizeof(*entry->render_targets))
                && entry->depth_stencil == depth_stencil && entry->location == location)
        {
            list_remove(&entry->entry);
            list_add_head(&context->fbo_list, &entry->entry);
            return entry;
        }
    }

    if (context->fbo_entry_count < WINED3D_MAX_FBO_ENTRIES)
    {
        entry = context_create_fbo_entry(context, render_targets, depth_stencil, location);
        list_add_head(&context->fbo_list, &entry->entry);
        ++context->fbo_entry_count;
    }
    else
    {
        entry = LIST_ENTRY(list_tail(&context->fbo_list), struct fbo_entry, entry);
        context_reuse_fbo_entry(context, target, render_targets, depth_stencil, location, entry);
        list_remove(&entry->entry);
        list_add_head(&context->fbo_list, &entry->entry);
    }

    return entry;
}

/* GL locking is done by the caller */
static void context_apply_fbo_entry(struct wined3d_context *context, GLenum target, struct fbo_entry *entry)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;
    unsigned int i;

    context_bind_fbo(context, target, &entry->id);

    if (!entry->attached)
    {
        /* Apply render targets */
        for (i = 0; i < gl_info->limits.buffers; ++i)
        {
            context_attach_surface_fbo(context, target, i, entry->render_targets[i], entry->location);
        }

        /* Apply depth targets */
        if (entry->depth_stencil)
            surface_set_compatible_renderbuffer(entry->depth_stencil, entry->render_targets[0]);
        context_attach_depth_stencil_fbo(context, target, entry->depth_stencil, TRUE);

        entry->attached = TRUE;
    }
    else
    {
        for (i = 0; i < gl_info->limits.buffers; ++i)
        {
            if (entry->render_targets[i])
                context_apply_attachment_filter_states(context, entry->render_targets[i], entry->location);
        }
        if (entry->depth_stencil)
            context_apply_attachment_filter_states(context, entry->depth_stencil, SFLAG_INTEXTURE);
    }
}

/* GL locking is done by the caller */
static void context_apply_fbo_state(struct wined3d_context *context, GLenum target,
        struct wined3d_surface **render_targets, struct wined3d_surface *depth_stencil, DWORD location)
{
    struct fbo_entry *entry, *entry2;

    LIST_FOR_EACH_ENTRY_SAFE(entry, entry2, &context->fbo_destroy_list, struct fbo_entry, entry)
    {
        context_destroy_fbo_entry(context, entry);
    }

    if (context->rebind_fbo)
    {
        context_bind_fbo(context, GL_FRAMEBUFFER, NULL);
        context->rebind_fbo = FALSE;
    }

    if (render_targets)
    {
        context->current_fbo = context_find_fbo_entry(context, target, render_targets, depth_stencil, location);
        context_apply_fbo_entry(context, target, context->current_fbo);
    }
    else
    {
        context->current_fbo = NULL;
        context_bind_fbo(context, target, NULL);
    }
}

/* GL locking is done by the caller */
void context_apply_fbo_state_blit(struct wined3d_context *context, GLenum target,
        struct wined3d_surface *render_target, struct wined3d_surface *depth_stencil, DWORD location)
{
    if (location != SFLAG_INDRAWABLE || surface_is_offscreen(render_target))
    {
        UINT clear_size = (context->gl_info->limits.buffers - 1) * sizeof(*context->blit_targets);
        context->blit_targets[0] = render_target;
        if (clear_size) memset(&context->blit_targets[1], 0, clear_size);
        context_apply_fbo_state(context, target, context->blit_targets, depth_stencil, location);
    }
    else
    {
        context_apply_fbo_state(context, target, NULL, NULL, location);
    }
}

/* Context activation is done by the caller. */
void context_alloc_occlusion_query(struct wined3d_context *context, struct wined3d_occlusion_query *query)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;

    if (context->free_occlusion_query_count)
    {
        query->id = context->free_occlusion_queries[--context->free_occlusion_query_count];
    }
    else
    {
        if (gl_info->supported[ARB_OCCLUSION_QUERY])
        {
            ENTER_GL();
            GL_EXTCALL(glGenQueriesARB(1, &query->id));
            checkGLcall("glGenQueriesARB");
            LEAVE_GL();

            TRACE("Allocated occlusion query %u in context %p.\n", query->id, context);
        }
        else
        {
            WARN("Occlusion queries not supported, not allocating query id.\n");
            query->id = 0;
        }
    }

    query->context = context;
    list_add_head(&context->occlusion_queries, &query->entry);
}

void context_free_occlusion_query(struct wined3d_occlusion_query *query)
{
    struct wined3d_context *context = query->context;

    list_remove(&query->entry);
    query->context = NULL;

    if (context->free_occlusion_query_count >= context->free_occlusion_query_size - 1)
    {
        UINT new_size = context->free_occlusion_query_size << 1;
        GLuint *new_data = HeapReAlloc(GetProcessHeap(), 0, context->free_occlusion_queries,
                new_size * sizeof(*context->free_occlusion_queries));

        if (!new_data)
        {
            ERR("Failed to grow free list, leaking query %u in context %p.\n", query->id, context);
            return;
        }

        context->free_occlusion_query_size = new_size;
        context->free_occlusion_queries = new_data;
    }

    context->free_occlusion_queries[context->free_occlusion_query_count++] = query->id;
}

/* Context activation is done by the caller. */
void context_alloc_event_query(struct wined3d_context *context, struct wined3d_event_query *query)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;

    if (context->free_event_query_count)
    {
        query->object = context->free_event_queries[--context->free_event_query_count];
    }
    else
    {
        if (gl_info->supported[ARB_SYNC])
        {
            /* Using ARB_sync, not much to do here. */
            query->object.sync = NULL;
            TRACE("Allocated event query %p in context %p.\n", query->object.sync, context);
        }
        else if (gl_info->supported[APPLE_FENCE])
        {
            ENTER_GL();
            GL_EXTCALL(glGenFencesAPPLE(1, &query->object.id));
            checkGLcall("glGenFencesAPPLE");
            LEAVE_GL();

            TRACE("Allocated event query %u in context %p.\n", query->object.id, context);
        }
        else if(gl_info->supported[NV_FENCE])
        {
            ENTER_GL();
            GL_EXTCALL(glGenFencesNV(1, &query->object.id));
            checkGLcall("glGenFencesNV");
            LEAVE_GL();

            TRACE("Allocated event query %u in context %p.\n", query->object.id, context);
        }
        else
        {
            WARN("Event queries not supported, not allocating query id.\n");
            query->object.id = 0;
        }
    }

    query->context = context;
    list_add_head(&context->event_queries, &query->entry);
}

void context_free_event_query(struct wined3d_event_query *query)
{
    struct wined3d_context *context = query->context;

    list_remove(&query->entry);
    query->context = NULL;

    if (context->free_event_query_count >= context->free_event_query_size - 1)
    {
        UINT new_size = context->free_event_query_size << 1;
        union wined3d_gl_query_object *new_data = HeapReAlloc(GetProcessHeap(), 0, context->free_event_queries,
                new_size * sizeof(*context->free_event_queries));

        if (!new_data)
        {
            ERR("Failed to grow free list, leaking query %u in context %p.\n", query->object.id, context);
            return;
        }

        context->free_event_query_size = new_size;
        context->free_event_queries = new_data;
    }

    context->free_event_queries[context->free_event_query_count++] = query->object;
}

typedef void (context_fbo_entry_func_t)(struct wined3d_context *context, struct fbo_entry *entry);

static void context_enum_surface_fbo_entries(struct wined3d_device *device,
        struct wined3d_surface *surface, context_fbo_entry_func_t *callback)
{
    UINT i;

    for (i = 0; i < device->context_count; ++i)
    {
        struct wined3d_context *context = device->contexts[i];
        const struct wined3d_gl_info *gl_info = context->gl_info;
        struct fbo_entry *entry, *entry2;

        if (context->current_rt == surface) context->current_rt = NULL;

        LIST_FOR_EACH_ENTRY_SAFE(entry, entry2, &context->fbo_list, struct fbo_entry, entry)
        {
            UINT j;

            if (entry->depth_stencil == surface)
            {
                callback(context, entry);
                continue;
            }

            for (j = 0; j < gl_info->limits.buffers; ++j)
            {
                if (entry->render_targets[j] == surface)
                {
                    callback(context, entry);
                    break;
                }
            }
        }
    }
}

static void context_queue_fbo_entry_destruction(struct wined3d_context *context, struct fbo_entry *entry)
{
    list_remove(&entry->entry);
    list_add_head(&context->fbo_destroy_list, &entry->entry);
}

void context_resource_released(struct wined3d_device *device,
        struct wined3d_resource *resource, WINED3DRESOURCETYPE type)
{
    if (!device->d3d_initialized) return;

    switch (type)
    {
        case WINED3DRTYPE_SURFACE:
            context_enum_surface_fbo_entries(device, surface_from_resource(resource),
                    context_queue_fbo_entry_destruction);
            break;

        default:
            break;
    }
}

static void context_detach_fbo_entry(struct wined3d_context *context, struct fbo_entry *entry)
{
    entry->attached = FALSE;
}

void context_resource_unloaded(struct wined3d_device *device,
        struct wined3d_resource *resource, WINED3DRESOURCETYPE type)
{
    switch (type)
    {
        case WINED3DRTYPE_SURFACE:
            context_enum_surface_fbo_entries(device, surface_from_resource(resource),
                    context_detach_fbo_entry);
            break;

        default:
            break;
    }
}

void context_surface_update(struct wined3d_context *context, struct wined3d_surface *surface)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;
    struct fbo_entry *entry = context->current_fbo;
    unsigned int i;

    if (!entry || context->rebind_fbo) return;

    for (i = 0; i < gl_info->limits.buffers; ++i)
    {
        if (surface == entry->render_targets[i])
        {
            TRACE("Updated surface %p is bound as color attachment %u to the current FBO.\n", surface, i);
            context->rebind_fbo = TRUE;
            return;
        }
    }

    if (surface == entry->depth_stencil)
    {
        TRACE("Updated surface %p is bound as depth attachment to the current FBO.\n", surface);
        context->rebind_fbo = TRUE;
    }
}

static BOOL context_set_pixel_format(const struct wined3d_gl_info *gl_info, HDC dc, int format)
{
    int current = GetPixelFormat(dc);

    if (current == format) return TRUE;

    if (!current)
    {
        if (!SetPixelFormat(dc, format, NULL))
        {
            ERR("Failed to set pixel format %d on device context %p, last error %#x.\n",
                    format, dc, GetLastError());
            return FALSE;
        }
        return TRUE;
    }

    /* By default WGL doesn't allow pixel format adjustments but we need it
     * here. For this reason there's a Wine specific wglSetPixelFormat()
     * which allows us to set the pixel format multiple times. Only use it
     * when really needed. */
    if (gl_info->supported[WGL_WINE_PIXEL_FORMAT_PASSTHROUGH])
    {
        if (!GL_EXTCALL(wglSetPixelFormatWINE(dc, format, NULL)))
        {
            ERR("wglSetPixelFormatWINE failed to set pixel format %d on device context %p.\n",
                    format, dc);
            return FALSE;
        }
        return TRUE;
    }

    /* OpenGL doesn't allow pixel format adjustments. Print an error and
     * continue using the old format. There's a big chance that the old
     * format works although with a performance hit and perhaps rendering
     * errors. */
    ERR("Unable to set pixel format %d on device context %p. Already using format %d.\n",
            format, dc, current);
    return TRUE;
}

static BOOL context_set_gl_context(struct wined3d_context *ctx)
{
    struct wined3d_swapchain *swapchain = ctx->swapchain;

    if (!pwglMakeCurrent(ctx->hdc, ctx->glCtx))
    {
        WARN("Failed to make GL context %p current on device context %p, last error %#x.\n",
                ctx->glCtx, ctx->hdc, GetLastError());
        ctx->valid = 0;
        WARN("Trying fallback to the backup window.\n");

        if (!swapchain->backup_dc)
        {
            TRACE("Creating the backup window for swapchain %p.\n", swapchain);
            swapchain->backup_wnd = CreateWindowA(WINED3D_OPENGL_WINDOW_CLASS_NAME, "WineD3D fake window",
                    WS_OVERLAPPEDWINDOW, 10, 10, 10, 10, NULL, NULL, NULL, NULL);
            if (!swapchain->backup_wnd)
            {
                ERR("Failed to create a window.\n");
                goto fail;
            }
            swapchain->backup_dc = GetDC(swapchain->backup_wnd);
            if (!swapchain->backup_dc)
            {
                ERR("Failed to get a DC.\n");
                goto fail;
            }
            if (!context_set_pixel_format(ctx->gl_info, swapchain->backup_dc, ctx->pixel_format))
            {
                ERR("Failed to set pixel format %d on device context %p.\n",
                        ctx->pixel_format, swapchain->backup_dc);
                goto fail;
            }
        }

        if (!pwglMakeCurrent(swapchain->backup_dc, ctx->glCtx))
        {
            ERR("Fallback to backup window (dc %p) failed too, last error %#x.\n",
                    swapchain->backup_dc, GetLastError());
            context_set_current(NULL);
            return FALSE;
        }
    }
    return TRUE;

fail:
    if (swapchain->backup_dc)
    {
        ReleaseDC(swapchain->backup_wnd, swapchain->backup_dc);
        swapchain->backup_dc = NULL;
    }
    if (swapchain->backup_wnd)
    {
        DestroyWindow(swapchain->backup_wnd);
        swapchain->backup_wnd = NULL;
    }
    context_set_current(NULL);
    return FALSE;
}

static void context_restore_gl_context(HDC dc, HGLRC gl_ctx)
{
    if (!pwglMakeCurrent(dc, gl_ctx))
    {
        ERR("Failed to restore GL context %p on device context %p, last error %#x.\n",
                gl_ctx, dc, GetLastError());
        context_set_current(NULL);
    }
}

static void context_update_window(struct wined3d_context *context)
{
    if (context->win_handle == context->swapchain->win_handle)
        return;

    TRACE("Updating context %p window from %p to %p.\n",
            context, context->win_handle, context->swapchain->win_handle);

    if (context->valid)
    {
        /* You'd figure ReleaseDC() would fail if the DC doesn't match the
         * window. However, that's not what actually happens, and there are
         * user32 tests that confirm ReleaseDC() with the wrong window is
         * supposed to succeed. So explicitly check that the DC belongs to
         * the window, since we want to avoid releasing a DC that belongs to
         * some other window if the original window was already destroyed. */
        if (WindowFromDC(context->hdc) != context->win_handle)
        {
            WARN("DC %p does not belong to window %p.\n",
                    context->hdc, context->win_handle);
        }
        else if (!ReleaseDC(context->win_handle, context->hdc))
        {
            ERR("Failed to release device context %p, last error %#x.\n",
                    context->hdc, GetLastError());
        }
    }
    else context->valid = 1;

    context->win_handle = context->swapchain->win_handle;

    if (!(context->hdc = GetDC(context->win_handle)))
    {
        ERR("Failed to get a device context for window %p.\n", context->win_handle);
        goto err;
    }

    if (!context_set_pixel_format(context->gl_info, context->hdc, context->pixel_format))
    {
        ERR("Failed to set pixel format %d on device context %p.\n",
                context->pixel_format, context->hdc);
        goto err;
    }

    context_set_gl_context(context);

    return;

err:
    context->valid = 0;
}

/* Do not call while under the GL lock. */
static void context_destroy_gl_resources(struct wined3d_context *context)
{
    const struct wined3d_gl_info *gl_info = context->gl_info;
    struct wined3d_occlusion_query *occlusion_query;
    struct wined3d_event_query *event_query;
    struct fbo_entry *entry, *entry2;
    HGLRC restore_ctx;
    HDC restore_dc;
    unsigned int i;

    restore_ctx = pwglGetCurrentContext();
    restore_dc = pwglGetCurrentDC();

    context_update_window(context);
    if (context->valid && restore_ctx != context->glCtx)
        context_set_gl_context(context);
    else restore_ctx = NULL;

    ENTER_GL();

    LIST_FOR_EACH_ENTRY(occlusion_query, &context->occlusion_queries, struct wined3d_occlusion_query, entry)
    {
        if (context->valid && gl_info->supported[ARB_OCCLUSION_QUERY])
            GL_EXTCALL(glDeleteQueriesARB(1, &occlusion_query->id));
        occlusion_query->context = NULL;
    }

    LIST_FOR_EACH_ENTRY(event_query, &context->event_queries, struct wined3d_event_query, entry)
    {
        if (context->valid)
        {
            if (gl_info->supported[ARB_SYNC])
            {
                if (event_query->object.sync) GL_EXTCALL(glDeleteSync(event_query->object.sync));
            }
            else if (gl_info->supported[APPLE_FENCE]) GL_EXTCALL(glDeleteFencesAPPLE(1, &event_query->object.id));
            else if (gl_info->supported[NV_FENCE]) GL_EXTCALL(glDeleteFencesNV(1, &event_query->object.id));
        }
        event_query->context = NULL;
    }

    LIST_FOR_EACH_ENTRY_SAFE(entry, entry2, &context->fbo_destroy_list, struct fbo_entry, entry)
    {
        if (!context->valid) entry->id = 0;
        context_destroy_fbo_entry(context, entry);
    }

    LIST_FOR_EACH_ENTRY_SAFE(entry, entry2, &context->fbo_list, struct fbo_entry, entry)
    {
        if (!context->valid) entry->id = 0;
        context_destroy_fbo_entry(context, entry);
    }

    if (context->valid)
    {
        if (context->dst_fbo)
        {
            TRACE("Destroy dst FBO %d\n", context->dst_fbo);
            context_destroy_fbo(context, &context->dst_fbo);
        }
        if (context->dummy_arbfp_prog)
        {
            GL_EXTCALL(glDeleteProgramsARB(1, &context->dummy_arbfp_prog));
        }

        if (gl_info->supported[ARB_OCCLUSION_QUERY])
            GL_EXTCALL(glDeleteQueriesARB(context->free_occlusion_query_count, context->free_occlusion_queries));

        if (gl_info->supported[ARB_SYNC])
        {
            for (i = 0; i < context->free_event_query_count; ++i)
            {
                GL_EXTCALL(glDeleteSync(context->free_event_queries[i].sync));
            }
        }
        else if (gl_info->supported[APPLE_FENCE])
        {
            for (i = 0; i < context->free_event_query_count; ++i)
            {
                GL_EXTCALL(glDeleteFencesAPPLE(1, &context->free_event_queries[i].id));
            }
        }
        else if (gl_info->supported[NV_FENCE])
        {
            for (i = 0; i < context->free_event_query_count; ++i)
            {
                GL_EXTCALL(glDeleteFencesNV(1, &context->free_event_queries[i].id));
            }
        }

        checkGLcall("context cleanup");
    }

    LEAVE_GL();

    HeapFree(GetProcessHeap(), 0, context->free_occlusion_queries);
    HeapFree(GetProcessHeap(), 0, context->free_event_queries);

    if (restore_ctx)
    {
        context_restore_gl_context(restore_dc, restore_ctx);
    }
    else if (pwglGetCurrentContext() && !pwglMakeCurrent(NULL, NULL))
    {
        ERR("Failed to disable GL context.\n");
    }

    ReleaseDC(context->win_handle, context->hdc);

    if (!pwglDeleteContext(context->glCtx))
    {
        DWORD err = GetLastError();
        ERR("wglDeleteContext(%p) failed, last error %#x.\n", context->glCtx, err);
    }
}

DWORD context_get_tls_idx(void)
{
    return wined3d_context_tls_idx;
}

void context_set_tls_idx(DWORD idx)
{
    wined3d_context_tls_idx = idx;
}

struct wined3d_context *context_get_current(void)
{
    return TlsGetValue(wined3d_context_tls_idx);
}

/* Do not call while under the GL lock. */
BOOL context_set_current(struct wined3d_context *ctx)
{
    struct wined3d_context *old = context_get_current();

    if (old == ctx)
    {
        TRACE("Already using D3D context %p.\n", ctx);
        return TRUE;
    }

    if (old)
    {
        if (old->destroyed)
        {
            TRACE("Switching away from destroyed context %p.\n", old);
            context_destroy_gl_resources(old);
            HeapFree(GetProcessHeap(), 0, old);
        }
        else
        {
            old->current = 0;
        }
    }

    if (ctx)
    {
        if (!ctx->valid)
        {
            ERR("Trying to make invalid context %p current\n", ctx);
            return FALSE;
        }

        TRACE("Switching to D3D context %p, GL context %p, device context %p.\n", ctx, ctx->glCtx, ctx->hdc);
        if (!context_set_gl_context(ctx))
            return FALSE;
        ctx->current = 1;
    }
    else if(pwglGetCurrentContext())
    {
        TRACE("Clearing current D3D context.\n");
        if (!pwglMakeCurrent(NULL, NULL))
        {
            DWORD err = GetLastError();
            ERR("Failed to clear current GL context, last error %#x.\n", err);
            TlsSetValue(wined3d_context_tls_idx, NULL);
            return FALSE;
        }
    }

    return TlsSetValue(wined3d_context_tls_idx, ctx);
}

void context_release(struct wined3d_context *context)
{
    TRACE("Releasing context %p, level %u.\n", context, context->level);

    if (WARN_ON(d3d))
    {
        if (!context->level)
            WARN("Context %p is not active.\n", context);
        else if (context != context_get_current())
            WARN("Context %p is not the current context.\n", context);
    }

    if (!--context->level && context->restore_ctx)
    {
        TRACE("Restoring GL context %p on device context %p.\n", context->restore_ctx, context->restore_dc);
        context_restore_gl_context(context->restore_dc, context->restore_ctx);
        context->restore_ctx = NULL;
        context->restore_dc = NULL;
    }
}

static void context_enter(struct wined3d_context *context)
{
    TRACE("Entering context %p, level %u.\n", context, context->level + 1);

    if (!context->level++)
    {
        const struct wined3d_context *current_context = context_get_current();
        HGLRC current_gl = pwglGetCurrentContext();

        if (current_gl && (!current_context || current_context->glCtx != current_gl))
        {
            TRACE("Another GL context (%p on device context %p) is already current.\n",
                    current_gl, pwglGetCurrentDC());
            context->restore_ctx = current_gl;
            context->restore_dc = pwglGetCurrentDC();
        }
    }
}

/*****************************************************************************
 * Context_MarkStateDirty
 *
 * Marks a state in a context dirty. Only one context, opposed to
 * IWineD3DDeviceImpl_MarkStateDirty, which marks the state dirty in all
 * contexts
 *
 * Params:
 *  context: Context to mark the state dirty in
 *  state: State to mark dirty
 *  StateTable: Pointer to the state table in use(for state grouping)
 *
 *****************************************************************************/
static void Context_MarkStateDirty(struct wined3d_context *context, DWORD state, const struct StateEntry *StateTable)
{
    DWORD rep = StateTable[state].representative;
    DWORD idx;
    BYTE shift;

    if (isStateDirty(context, rep)) return;

    context->dirtyArray[context->numDirtyEntries++] = rep;
    idx = rep / (sizeof(*context->isStateDirty) * CHAR_BIT);
    shift = rep & ((sizeof(*context->isStateDirty) * CHAR_BIT) - 1);
    context->isStateDirty[idx] |= (1 << shift);
}

/* This function takes care of WineD3D pixel format selection. */
static int WineD3D_ChoosePixelFormat(struct wined3d_device *device, HDC hdc,
        const struct wined3d_format *color_format, const struct wined3d_format *ds_format,
        BOOL auxBuffers, int numSamples, BOOL findCompatible)
{
    int iPixelFormat=0;
    unsigned int matchtry;
    BYTE redBits, greenBits, blueBits, alphaBits, colorBits;
    BYTE depthBits=0, stencilBits=0;

    static const struct
    {
        BOOL require_aux;
        BOOL exact_alpha;
        BOOL exact_color;
    }
    matches[] =
    {
        /* First, try without alpha match buffers. MacOS supports aux buffers only
         * on A8R8G8B8, and we prefer better offscreen rendering over an alpha match.
         * Then try without aux buffers - this is the most common cause for not
         * finding a pixel format. Also some drivers(the open source ones)
         * only offer 32 bit ARB pixel formats. First try without an exact alpha
         * match, then try without an exact alpha and color match.
         */
        { TRUE,  TRUE,  TRUE  },
        { TRUE,  FALSE, TRUE  },
        { FALSE, TRUE,  TRUE  },
        { FALSE, FALSE, TRUE  },
        { TRUE,  FALSE, FALSE },
        { FALSE, FALSE, FALSE },
    };

    int i = 0;
    int nCfgs = device->adapter->nCfgs;

    TRACE("ColorFormat=%s, DepthStencilFormat=%s, auxBuffers=%d, numSamples=%d, findCompatible=%d\n",
          debug_d3dformat(color_format->id), debug_d3dformat(ds_format->id),
          auxBuffers, numSamples, findCompatible);

    if (!getColorBits(color_format, &redBits, &greenBits, &blueBits, &alphaBits, &colorBits))
    {
        ERR("Unable to get color bits for format %s (%#x)!\n",
                debug_d3dformat(color_format->id), color_format->id);
        return 0;
    }

    getDepthStencilBits(ds_format, &depthBits, &stencilBits);

    for (matchtry = 0; matchtry < (sizeof(matches) / sizeof(*matches)) && !iPixelFormat; ++matchtry)
    {
        for (i = 0; i < nCfgs; ++i)
        {
            const struct wined3d_pixel_format *cfg = &device->adapter->cfgs[i];
            BOOL exactDepthMatch = TRUE;

            /* For now only accept RGBA formats. Perhaps some day we will
             * allow floating point formats for pbuffers. */
            if(cfg->iPixelType != WGL_TYPE_RGBA_ARB)
                continue;

            /* In window mode we need a window drawable format and double buffering. */
            if(!(cfg->windowDrawable && cfg->doubleBuffer))
                continue;

            /* We like to have aux buffers in backbuffer mode */
            if(auxBuffers && !cfg->auxBuffers && matches[matchtry].require_aux)
                continue;

            if(matches[matchtry].exact_color) {
                if(cfg->redSize != redBits)
                    continue;
                if(cfg->greenSize != greenBits)
                    continue;
                if(cfg->blueSize != blueBits)
                    continue;
            } else {
                if(cfg->redSize < redBits)
                    continue;
                if(cfg->greenSize < greenBits)
                    continue;
                if(cfg->blueSize < blueBits)
                    continue;
            }
            if(matches[matchtry].exact_alpha) {
                if(cfg->alphaSize != alphaBits)
                    continue;
            } else {
                if(cfg->alphaSize < alphaBits)
                    continue;
            }

            /* We try to locate a format which matches our requirements exactly. In case of
             * depth it is no problem to emulate 16-bit using e.g. 24-bit, so accept that. */
            if(cfg->depthSize < depthBits)
                continue;
            else if(cfg->depthSize > depthBits)
                exactDepthMatch = FALSE;

            /* In all cases make sure the number of stencil bits matches our requirements
             * even when we don't need stencil because it could affect performance EXCEPT
             * on cards which don't offer depth formats without stencil like the i915 drivers
             * on Linux. */
            if (stencilBits != cfg->stencilSize
                    && !(device->adapter->brokenStencil && stencilBits <= cfg->stencilSize))
                continue;

            /* Check multisampling support */
            if(cfg->numSamples != numSamples)
                continue;

            /* When we have passed all the checks then we have found a format which matches our
             * requirements. Note that we only check for a limit number of capabilities right now,
             * so there can easily be a dozen of pixel formats which appear to be the 'same' but
             * can still differ in things like multisampling, stereo, SRGB and other flags.
             */

            /* Exit the loop as we have found a format :) */
            if(exactDepthMatch) {
                iPixelFormat = cfg->iPixelFormat;
                break;
            } else if(!iPixelFormat) {
                /* In the end we might end up with a format which doesn't exactly match our depth
                 * requirements. Accept the first format we found because formats with higher iPixelFormat
                 * values tend to have more extended capabilities (e.g. multisampling) which we don't need. */
                iPixelFormat = cfg->iPixelFormat;
            }
        }
    }

    /* When findCompatible is set and no suitable format was found, let ChoosePixelFormat choose a pixel format in order not to crash. */
    if(!iPixelFormat && !findCompatible) {
        ERR("Can't find a suitable iPixelFormat\n");
        return FALSE;
    } else if(!iPixelFormat) {
        PIXELFORMATDESCRIPTOR pfd;

        TRACE("Falling back to ChoosePixelFormat as we weren't able to find an exactly matching pixel format\n");
        /* PixelFormat selection */
        ZeroMemory(&pfd, sizeof(pfd));
        pfd.nSize      = sizeof(pfd);
        pfd.nVersion   = 1;
        pfd.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;/*PFD_GENERIC_ACCELERATED*/
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cAlphaBits = alphaBits;
        pfd.cColorBits = colorBits;
        pfd.cDepthBits = depthBits;
        pfd.cStencilBits = stencilBits;
        pfd.iLayerType = PFD_MAIN_PLANE;

        iPixelFormat = ChoosePixelFormat(hdc, &pfd);
        if(!iPixelFormat) {
            /* If this happens something is very wrong as ChoosePixelFormat barely fails */
            ERR("Can't find a suitable iPixelFormat\n");
            return FALSE;
        }
    }

    TRACE("Found iPixelFormat=%d for ColorFormat=%s, DepthStencilFormat=%s\n",
            iPixelFormat, debug_d3dformat(color_format->id), debug_d3dformat(ds_format->id));
    return iPixelFormat;
}

/* Do not call while under the GL lock. */
struct wined3d_context *context_create(struct wined3d_swapchain *swapchain,
        struct wined3d_surface *target, const struct wined3d_format *ds_format)
{
    struct wined3d_device *device = swapchain->device;
    const struct wined3d_gl_info *gl_info = &device->adapter->gl_info;
    const struct wined3d_format *color_format;
    struct wined3d_context *ret;
    PIXELFORMATDESCRIPTOR pfd;
    BOOL auxBuffers = FALSE;
    int numSamples = 0;
    int pixel_format;
    unsigned int s;
    int swap_interval;
    DWORD state;
    HGLRC ctx;
    HDC hdc;

    TRACE("swapchain %p, target %p, window %p.\n", swapchain, target, swapchain->win_handle);

    ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*ret));
    if (!ret)
    {
        ERR("Failed to allocate context memory.\n");
        return NULL;
    }

    if (!(hdc = GetDC(swapchain->win_handle)))
    {
        ERR("Failed to retrieve a device context.\n");
        goto out;
    }

    color_format = target->resource.format;

    /* In case of ORM_BACKBUFFER, make sure to request an alpha component for
     * X4R4G4B4/X8R8G8B8 as we might need it for the backbuffer. */
    if (wined3d_settings.offscreen_rendering_mode == ORM_BACKBUFFER)
    {
        auxBuffers = TRUE;

        if (color_format->id == WINED3DFMT_B4G4R4X4_UNORM)
            color_format = wined3d_get_format(gl_info, WINED3DFMT_B4G4R4A4_UNORM);
        else if (color_format->id == WINED3DFMT_B8G8R8X8_UNORM)
            color_format = wined3d_get_format(gl_info, WINED3DFMT_B8G8R8A8_UNORM);
    }

    /* DirectDraw supports 8bit paletted render targets and these are used by
     * old games like StarCraft and C&C. Most modern hardware doesn't support
     * 8bit natively so we perform some form of 8bit -> 32bit conversion. The
     * conversion (ab)uses the alpha component for storing the palette index.
     * For this reason we require a format with 8bit alpha, so request
     * A8R8G8B8. */
    if (color_format->id == WINED3DFMT_P8_UINT)
        color_format = wined3d_get_format(gl_info, WINED3DFMT_B8G8R8A8_UNORM);

    /* D3D only allows multisampling when SwapEffect is set to WINED3DSWAPEFFECT_DISCARD. */
    if (swapchain->presentParms.MultiSampleType && (swapchain->presentParms.SwapEffect == WINED3DSWAPEFFECT_DISCARD))
    {
        if (!gl_info->supported[ARB_MULTISAMPLE])
            WARN("The application is requesting multisampling without support.\n");
        else
        {
            TRACE("Requesting multisample type %#x.\n", swapchain->presentParms.MultiSampleType);
            numSamples = swapchain->presentParms.MultiSampleType;
        }
    }

    /* Try to find a pixel format which matches our requirements. */
    pixel_format = WineD3D_ChoosePixelFormat(device, hdc, color_format, ds_format,
            auxBuffers, numSamples, FALSE /* findCompatible */);

    /* Try to locate a compatible format if we weren't able to find anything. */
    if (!pixel_format)
    {
        TRACE("Trying to locate a compatible pixel format because an exact match failed.\n");
        pixel_format = WineD3D_ChoosePixelFormat(device, hdc, color_format, ds_format,
                auxBuffers, 0 /* numSamples */, TRUE /* findCompatible */);
    }

    /* If we still don't have a pixel format, something is very wrong as ChoosePixelFormat barely fails */
    if (!pixel_format)
    {
        ERR("Can't find a suitable pixel format.\n");
        goto out;
    }

    DescribePixelFormat(hdc, pixel_format, sizeof(pfd), &pfd);
    if (!context_set_pixel_format(gl_info, hdc, pixel_format))
    {
        ERR("Failed to set pixel format %d on device context %p.\n", pixel_format, hdc);
        goto out;
    }

    ctx = pwglCreateContext(hdc);
    if (device->context_count)
    {
        if (!pwglShareLists(device->contexts[0]->glCtx, ctx))
        {
            DWORD err = GetLastError();
            ERR("wglShareLists(%p, %p) failed, last error %#x.\n",
                    device->contexts[0]->glCtx, ctx, err);
        }
    }

    if(!ctx) {
        ERR("Failed to create a WGL context\n");
        goto out;
    }

    if (!device_context_add(device, ret))
    {
        ERR("Failed to add the newly created context to the context list\n");
        if (!pwglDeleteContext(ctx))
        {
            DWORD err = GetLastError();
            ERR("wglDeleteContext(%p) failed, last error %#x.\n", ctx, err);
        }
        goto out;
    }

    ret->gl_info = gl_info;

    /* Mark all states dirty to force a proper initialization of the states
     * on the first use of the context. */
    for (state = 0; state <= STATE_HIGHEST; ++state)
    {
        if (device->StateTable[state].representative)
            Context_MarkStateDirty(ret, state, device->StateTable);
    }

    ret->swapchain = swapchain;
    ret->current_rt = target;
    ret->tid = GetCurrentThreadId();

    ret->render_offscreen = surface_is_offscreen(target);
    ret->draw_buffer_dirty = TRUE;
    ret->valid = 1;

    ret->glCtx = ctx;
    ret->win_handle = swapchain->win_handle;
    ret->hdc = hdc;
    ret->pixel_format = pixel_format;

    if (device->shader_backend->shader_dirtifyable_constants())
    {
        /* Create the dirty constants array and initialize them to dirty */
        ret->vshader_const_dirty = HeapAlloc(GetProcessHeap(), 0,
                sizeof(*ret->vshader_const_dirty) * device->d3d_vshader_constantF);
        ret->pshader_const_dirty = HeapAlloc(GetProcessHeap(), 0,
                sizeof(*ret->pshader_const_dirty) * device->d3d_pshader_constantF);
        memset(ret->vshader_const_dirty, 1,
               sizeof(*ret->vshader_const_dirty) * device->d3d_vshader_constantF);
        memset(ret->pshader_const_dirty, 1,
                sizeof(*ret->pshader_const_dirty) * device->d3d_pshader_constantF);
    }

    ret->blit_targets = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
            gl_info->limits.buffers * sizeof(*ret->blit_targets));
    if (!ret->blit_targets) goto out;

    ret->draw_buffers = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
            gl_info->limits.buffers * sizeof(*ret->draw_buffers));
    if (!ret->draw_buffers) goto out;

    ret->free_occlusion_query_size = 4;
    ret->free_occlusion_queries = HeapAlloc(GetProcessHeap(), 0,
            ret->free_occlusion_query_size * sizeof(*ret->free_occlusion_queries));
    if (!ret->free_occlusion_queries) goto out;

    list_init(&ret->occlusion_queries);

    ret->free_event_query_size = 4;
    ret->free_event_queries = HeapAlloc(GetProcessHeap(), 0,
            ret->free_event_query_size * sizeof(*ret->free_event_queries));
    if (!ret->free_event_queries) goto out;

    list_init(&ret->event_queries);

    TRACE("Successfully created new context %p\n", ret);

    list_init(&ret->fbo_list);
    list_init(&ret->fbo_destroy_list);

    context_enter(ret);

    /* Set up the context defaults */
    if (!context_set_current(ret))
    {
        ERR("Cannot activate context to set up defaults\n");
        context_release(ret);
        goto out;
    }

    switch (swapchain->presentParms.PresentationInterval)
    {
        case WINED3DPRESENT_INTERVAL_IMMEDIATE:
            swap_interval = 0;
            break;
        case WINED3DPRESENT_INTERVAL_DEFAULT:
        case WINED3DPRESENT_INTERVAL_ONE:
            swap_interval = 1;
            break;
        case WINED3DPRESENT_INTERVAL_TWO:
            swap_interval = 2;
            break;
        case WINED3DPRESENT_INTERVAL_THREE:
            swap_interval = 3;
            break;
        case WINED3DPRESENT_INTERVAL_FOUR:
            swap_interval = 4;
            break;
        default:
            FIXME("Unknown presentation interval %08x\n", swapchain->presentParms.PresentationInterval);
            swap_interval = 1;
    }

    if (gl_info->supported[WGL_EXT_SWAP_CONTROL])
    {
        if (!GL_EXTCALL(wglSwapIntervalEXT(swap_interval)))
            ERR("wglSwapIntervalEXT failed to set swap interval %d for context %p, last error %#x\n",
                swap_interval, ret, GetLastError());
    }

    ENTER_GL();

    glGetIntegerv(GL_AUX_BUFFERS, &ret->aux_buffers);

    TRACE("Setting up the screen\n");
    /* Clear the screen */
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    checkGLcall("glClearColor");
    glClearIndex(0);
    glClearDepth(1);
    glClearStencil(0xffff);

    checkGLcall("glClear");

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    checkGLcall("glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);");

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    checkGLcall("glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);");

    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    checkGLcall("glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);");

    glPixelStorei(GL_PACK_ALIGNMENT, device->surface_alignment);
    checkGLcall("glPixelStorei(GL_PACK_ALIGNMENT, device->surface_alignment);");
    glPixelStorei(GL_UNPACK_ALIGNMENT, device->surface_alignment);
    checkGLcall("glPixelStorei(GL_UNPACK_ALIGNMENT, device->surface_alignment);");

    if (gl_info->supported[APPLE_CLIENT_STORAGE])
    {
        /* Most textures will use client storage if supported. Exceptions are non-native power of 2 textures
         * and textures in DIB sections(due to the memory protection).
         */
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        checkGLcall("glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE)");
    }
    if (gl_info->supported[ARB_VERTEX_BLEND])
    {
        /* Direct3D always uses n-1 weights for n world matrices and uses 1 - sum for the last one
         * this is equal to GL_WEIGHT_SUM_UNITY_ARB. Enabling it doesn't do anything unless
         * GL_VERTEX_BLEND_ARB isn't enabled too
         */
        glEnable(GL_WEIGHT_SUM_UNITY_ARB);
        checkGLcall("glEnable(GL_WEIGHT_SUM_UNITY_ARB)");
    }
    if (gl_info->supported[NV_TEXTURE_SHADER2])
    {
        /* Set up the previous texture input for all shader units. This applies to bump mapping, and in d3d
         * the previous texture where to source the offset from is always unit - 1.
         */
        for (s = 1; s < gl_info->limits.textures; ++s)
        {
            GL_EXTCALL(glActiveTextureARB(GL_TEXTURE0_ARB + s));
            glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + s - 1);
            checkGLcall("glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, ...");
        }
    }
    if (gl_info->supported[ARB_FRAGMENT_PROGRAM])
    {
        /* MacOS(radeon X1600 at least, but most likely others too) refuses to draw if GLSL and ARBFP are
         * enabled, but the currently bound arbfp program is 0. Enabling ARBFP with prog 0 is invalid, but
         * GLSL should bypass this. This causes problems in programs that never use the fixed function pipeline,
         * because the ARBFP extension is enabled by the ARBFP pipeline at context creation, but no program
         * is ever assigned.
         *
         * So make sure a program is assigned to each context. The first real ARBFP use will set a different
         * program and the dummy program is destroyed when the context is destroyed.
         */
        const char *dummy_program =
                "!!ARBfp1.0\n"
                "MOV result.color, fragment.color.primary;\n"
                "END\n";
        GL_EXTCALL(glGenProgramsARB(1, &ret->dummy_arbfp_prog));
        GL_EXTCALL(glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, ret->dummy_arbfp_prog));
        GL_EXTCALL(glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(dummy_program), dummy_program));
    }

    for (s = 0; s < gl_info->limits.point_sprite_units; ++s)
    {
        GL_EXTCALL(glActiveTextureARB(GL_TEXTURE0_ARB + s));
        glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
        checkGLcall("glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE)");
    }

    if (gl_info->supported[ARB_PROVOKING_VERTEX])
    {
        GL_EXTCALL(glProvokingVertex(GL_FIRST_VERTEX_CONVENTION));
    }
    else if (gl_info->supported[EXT_PROVOKING_VERTEX])
    {
        GL_EXTCALL(glProvokingVertexEXT(GL_FIRST_VERTEX_CONVENTION_EXT));
    }
    device->frag_pipe->enable_extension(TRUE);

    LEAVE_GL();

    TRACE("Created context %p.\n", ret);

    return ret;

out:
    HeapFree(GetProcessHeap(), 0, ret->free_event_queries);
    HeapFree(GetProcessHeap(), 0, ret->free_occlusion_queries);
    HeapFree(GetProcessHeap(), 0, ret->draw_buffers);
    HeapFree(GetProcessHeap(), 0, ret->blit_targets);
    HeapFree(GetProcessHeap(), 0, ret->pshader_const_dirty);
    HeapFree(GetProcessHeap(), 0, ret->vshader_const_dirty);
    HeapFree(GetProcessHeap(), 0, ret);
    return NULL;
}

/* Do not call while under the GL lock. */
void context_destroy(struct wined3d_device *device, struct wined3d_context *context)
{
    BOOL destroy;

    TRACE("Destroying ctx %p\n", context);

    if (context->tid == GetCurrentThreadId() || !context->current)
    {
        context_destroy_gl_resources(context);
        TlsSetValue(wined3d_context_tls_idx, NULL);
        destroy = TRUE;
    }
    else
    {
        context->destroyed = 1;
        destroy = FALSE;
    }

    HeapFree(GetProcessHeap(), 0, context->draw_buffers);
    HeapFree(GetProcessHeap(), 0, context->blit_targets);
    HeapFree(GetProcessHeap(), 0, context->vshader_const_dirty);
    HeapFree(GetProcessHeap(), 0, context->pshader_const_dirty);
    device_context_remove(device, context);
    if (destroy) HeapFree(GetProcessHeap(), 0, context);
}

/* GL locking is done by the caller */
static inline void set_blit_dimension(UINT width, UINT height) {
    glMatrixMode(GL_PROJECTION);
    checkGLcall("glMatrixMode(GL_PROJECTION)");
    glLoadIdentity();
    checkGLcall("glLoadIdentity()");
    glOrtho(0, width, 0, height, 0.0, -1.0);
    checkGLcall("glOrtho");
    glViewport(0, 0, width, height);
    checkGLcall("glViewport");
}

/*****************************************************************************
 * SetupForBlit
 *
 * Sets up a context for DirectDraw blitting.
 * All texture units are disabled, texture unit 0 is set as current unit
 * fog, lighting, blending, alpha test, z test, scissor test, culling disabled
 * color writing enabled for all channels
 * register combiners disabled, shaders disabled
 * world matrix is set to identity, texture matrix 0 too
 * projection matrix is setup for drawing screen coordinates
 *
 * Params:
 *  This: Device to activate the context for
 *  context: Context to setup
 *
 *****************************************************************************/
/* Context activation is done by the caller. */
static void SetupForBlit(struct wined3d_device *device, struct wined3d_context *context)
{
    int i;
    const struct StateEntry *StateTable = device->StateTable;
    const struct wined3d_gl_info *gl_info = context->gl_info;
    UINT width = context->current_rt->resource.width;
    UINT height = context->current_rt->resource.height;
    DWORD sampler;

    TRACE("Setting up context %p for blitting\n", context);
    if(context->last_was_blit) {
        if(context->blit_w != width || context->blit_h != height) {
            ENTER_GL();
            set_blit_dimension(width, height);
            LEAVE_GL();
            context->blit_w = width; context->blit_h = height;
            /* No need to dirtify here, the states are still dirtified because they weren't
             * applied since the last SetupForBlit call. Otherwise last_was_blit would not
             * be set
             */
        }
        TRACE("Context is already set up for blitting, nothing to do\n");
        return;
    }
    context->last_was_blit = TRUE;

    /* TODO: Use a display list */

    /* Disable shaders */
    ENTER_GL();
    device->shader_backend->shader_select(context, FALSE, FALSE);
    LEAVE_GL();

    Context_MarkStateDirty(context, STATE_VSHADER, StateTable);
    Context_MarkStateDirty(context, STATE_PIXELSHADER, StateTable);

    /* Call ENTER_GL() once for all gl calls below. In theory we should not call
     * helper functions in between gl calls. This function is full of Context_MarkStateDirty
     * which can safely be called from here, we only lock once instead locking/unlocking
     * after each GL call.
     */
    ENTER_GL();

    /* Disable all textures. The caller can then bind a texture it wants to blit
     * from
     *
     * The blitting code uses (for now) the fixed function pipeline, so make sure to reset all fixed
     * function texture unit. No need to care for higher samplers
     */
    for (i = gl_info->limits.textures - 1; i > 0 ; --i)
    {
        sampler = device->rev_tex_unit_map[i];
        GL_EXTCALL(glActiveTextureARB(GL_TEXTURE0_ARB + i));
        checkGLcall("glActiveTextureARB");

        if (gl_info->supported[ARB_TEXTURE_CUBE_MAP])
        {
            glDisable(GL_TEXTURE_CUBE_MAP_ARB);
            checkGLcall("glDisable GL_TEXTURE_CUBE_MAP_ARB");
        }
        glDisable(GL_TEXTURE_3D);
        checkGLcall("glDisable GL_TEXTURE_3D");
        if (gl_info->supported[ARB_TEXTURE_RECTANGLE])
        {
            glDisable(GL_TEXTURE_RECTANGLE_ARB);
            checkGLcall("glDisable GL_TEXTURE_RECTANGLE_ARB");
        }
        glDisable(GL_TEXTURE_2D);
        checkGLcall("glDisable GL_TEXTURE_2D");

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        checkGLcall("glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);");

        if (sampler != WINED3D_UNMAPPED_STAGE)
        {
            if (sampler < MAX_TEXTURES) {
                Context_MarkStateDirty(context, STATE_TEXTURESTAGE(sampler, WINED3DTSS_COLOROP), StateTable);
            }
            Context_MarkStateDirty(context, STATE_SAMPLER(sampler), StateTable);
        }
    }
    GL_EXTCALL(glActiveTextureARB(GL_TEXTURE0_ARB));
    checkGLcall("glActiveTextureARB");

    sampler = device->rev_tex_unit_map[0];

    if (gl_info->supported[ARB_TEXTURE_CUBE_MAP])
    {
        glDisable(GL_TEXTURE_CUBE_MAP_ARB);
        checkGLcall("glDisable GL_TEXTURE_CUBE_MAP_ARB");
    }
    glDisable(GL_TEXTURE_3D);
    checkGLcall("glDisable GL_TEXTURE_3D");
    if (gl_info->supported[ARB_TEXTURE_RECTANGLE])
    {
        glDisable(GL_TEXTURE_RECTANGLE_ARB);
        checkGLcall("glDisable GL_TEXTURE_RECTANGLE_ARB");
    }
    glDisable(GL_TEXTURE_2D);
    checkGLcall("glDisable GL_TEXTURE_2D");

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glMatrixMode(GL_TEXTURE);
    checkGLcall("glMatrixMode(GL_TEXTURE)");
    glLoadIdentity();
    checkGLcall("glLoadIdentity()");

    if (gl_info->supported[EXT_TEXTURE_LOD_BIAS])
    {
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT,
                  GL_TEXTURE_LOD_BIAS_EXT,
                  0.0f);
        checkGLcall("glTexEnvf GL_TEXTURE_LOD_BIAS_EXT ...");
    }

    if (sampler != WINED3D_UNMAPPED_STAGE)
    {
        if (sampler < MAX_TEXTURES) {
            Context_MarkStateDirty(context, STATE_TRANSFORM(WINED3DTS_TEXTURE0 + sampler), StateTable);
            Context_MarkStateDirty(context, STATE_TEXTURESTAGE(sampler, WINED3DTSS_COLOROP), StateTable);
        }
        Context_MarkStateDirty(context, STATE_SAMPLER(sampler), StateTable);
    }

    /* Other misc states */
    glDisable(GL_ALPHA_TEST);
    checkGLcall("glDisable(GL_ALPHA_TEST)");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_ALPHATESTENABLE), StateTable);
    glDisable(GL_LIGHTING);
    checkGLcall("glDisable GL_LIGHTING");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_LIGHTING), StateTable);
    glDisable(GL_DEPTH_TEST);
    checkGLcall("glDisable GL_DEPTH_TEST");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_ZENABLE), StateTable);
    glDisableWINE(GL_FOG);
    checkGLcall("glDisable GL_FOG");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_FOGENABLE), StateTable);
    glDisable(GL_BLEND);
    checkGLcall("glDisable GL_BLEND");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_ALPHABLENDENABLE), StateTable);
    glDisable(GL_CULL_FACE);
    checkGLcall("glDisable GL_CULL_FACE");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_CULLMODE), StateTable);
    glDisable(GL_STENCIL_TEST);
    checkGLcall("glDisable GL_STENCIL_TEST");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_STENCILENABLE), StateTable);
    glDisable(GL_SCISSOR_TEST);
    checkGLcall("glDisable GL_SCISSOR_TEST");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_SCISSORTESTENABLE), StateTable);
    if (gl_info->supported[ARB_POINT_SPRITE])
    {
        glDisable(GL_POINT_SPRITE_ARB);
        checkGLcall("glDisable GL_POINT_SPRITE_ARB");
        Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_POINTSPRITEENABLE), StateTable);
    }
    glColorMask(GL_TRUE, GL_TRUE,GL_TRUE,GL_TRUE);
    checkGLcall("glColorMask");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_COLORWRITEENABLE), StateTable);
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_COLORWRITEENABLE1), StateTable);
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_COLORWRITEENABLE2), StateTable);
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_COLORWRITEENABLE3), StateTable);
    if (gl_info->supported[EXT_SECONDARY_COLOR])
    {
        glDisable(GL_COLOR_SUM_EXT);
        Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_SPECULARENABLE), StateTable);
        checkGLcall("glDisable(GL_COLOR_SUM_EXT)");
    }

    /* Setup transforms */
    glMatrixMode(GL_MODELVIEW);
    checkGLcall("glMatrixMode(GL_MODELVIEW)");
    glLoadIdentity();
    checkGLcall("glLoadIdentity()");
    Context_MarkStateDirty(context, STATE_TRANSFORM(WINED3DTS_WORLDMATRIX(0)), StateTable);

    context->last_was_rhw = TRUE;
    Context_MarkStateDirty(context, STATE_VDECL, StateTable); /* because of last_was_rhw = TRUE */

    glDisable(GL_CLIP_PLANE0); checkGLcall("glDisable(clip plane 0)");
    glDisable(GL_CLIP_PLANE1); checkGLcall("glDisable(clip plane 1)");
    glDisable(GL_CLIP_PLANE2); checkGLcall("glDisable(clip plane 2)");
    glDisable(GL_CLIP_PLANE3); checkGLcall("glDisable(clip plane 3)");
    glDisable(GL_CLIP_PLANE4); checkGLcall("glDisable(clip plane 4)");
    glDisable(GL_CLIP_PLANE5); checkGLcall("glDisable(clip plane 5)");
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_CLIPPING), StateTable);

    set_blit_dimension(width, height);
    device->frag_pipe->enable_extension(FALSE);

    LEAVE_GL();

    context->blit_w = width; context->blit_h = height;
    Context_MarkStateDirty(context, STATE_VIEWPORT, StateTable);
    Context_MarkStateDirty(context, STATE_TRANSFORM(WINED3DTS_PROJECTION), StateTable);
}

/* Do not call while under the GL lock. */
static struct wined3d_context *FindContext(struct wined3d_device *device, struct wined3d_surface *target)
{
    struct wined3d_context *current_context = context_get_current();
    struct wined3d_context *context;

    if (current_context && current_context->destroyed) current_context = NULL;

    if (!target)
    {
        if (current_context
                && current_context->current_rt
                && current_context->swapchain->device == device)
        {
            target = current_context->current_rt;
        }
        else
        {
            struct wined3d_swapchain *swapchain = device->swapchains[0];
            if (swapchain->back_buffers) target = swapchain->back_buffers[0];
            else target = swapchain->front_buffer;
        }
    }

    if (current_context && current_context->current_rt == target)
    {
        context_update_window(current_context);
        return current_context;
    }

    if (target->container.type == WINED3D_CONTAINER_SWAPCHAIN)
    {
        TRACE("Rendering onscreen\n");

        context = swapchain_get_context(target->container.u.swapchain);
    }
    else
    {
        TRACE("Rendering offscreen\n");

        /* Stay with the current context if possible. Otherwise use the
         * context for the primary swapchain. */
        if (current_context && current_context->swapchain->device == device)
            context = current_context;
        else
            context = swapchain_get_context(device->swapchains[0]);
    }

    context_update_window(context);

    return context;
}

/* Context activation and GL locking are done by the caller. */
static void context_apply_draw_buffers(struct wined3d_context *context, DWORD rt_mask, struct wined3d_surface **rts)
{
    if (!rt_mask)
    {
        glDrawBuffer(GL_NONE);
        checkGLcall("glDrawBuffer()");
    }
    else if (!surface_is_offscreen(rts[0]))
    {
        glDrawBuffer(surface_get_gl_buffer(rts[0]));
        checkGLcall("glDrawBuffer()");
    }
    else
    {
        if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
        {
            const struct wined3d_gl_info *gl_info = context->gl_info;
            unsigned int i = 0;

            while (rt_mask)
            {
                if ((rt_mask & 1) && rts[i] && rts[i]->resource.format->id != WINED3DFMT_NULL)
                    context->draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
                else
                    context->draw_buffers[i] = GL_NONE;

                rt_mask >>= 1;
                ++i;
            }

            if (gl_info->supported[ARB_DRAW_BUFFERS])
            {
                GL_EXTCALL(glDrawBuffersARB(i, context->draw_buffers));
                checkGLcall("glDrawBuffers()");
            }
            else
            {
                glDrawBuffer(context->draw_buffers[0]);
                checkGLcall("glDrawBuffer()");
            }
        }
        else
        {
            glDrawBuffer(rts[0]->resource.device->offscreenBuffer);
            checkGLcall("glDrawBuffer()");
        }
    }
}

/* GL locking is done by the caller. */
void context_set_draw_buffer(struct wined3d_context *context, GLenum buffer)
{
    glDrawBuffer(buffer);
    checkGLcall("glDrawBuffer()");
    context->draw_buffer_dirty = TRUE;
}

static inline void context_set_render_offscreen(struct wined3d_context *context, const struct StateEntry *StateTable,
        BOOL offscreen)
{
    if (context->render_offscreen == offscreen) return;

    Context_MarkStateDirty(context, STATE_POINTSPRITECOORDORIGIN, StateTable);
    Context_MarkStateDirty(context, STATE_TRANSFORM(WINED3DTS_PROJECTION), StateTable);
    Context_MarkStateDirty(context, STATE_VIEWPORT, StateTable);
    Context_MarkStateDirty(context, STATE_SCISSORRECT, StateTable);
    Context_MarkStateDirty(context, STATE_FRONTFACE, StateTable);
    context->render_offscreen = offscreen;
}

static BOOL match_depth_stencil_format(const struct wined3d_format *existing,
        const struct wined3d_format *required)
{
    BYTE existing_depth, existing_stencil, required_depth, required_stencil;

    if (existing == required) return TRUE;
    if ((existing->flags & WINED3DFMT_FLAG_FLOAT) != (required->flags & WINED3DFMT_FLAG_FLOAT)) return FALSE;

    getDepthStencilBits(existing, &existing_depth, &existing_stencil);
    getDepthStencilBits(required, &required_depth, &required_stencil);

    if(existing_depth < required_depth) return FALSE;
    /* If stencil bits are used the exact amount is required - otherwise wrapping
     * won't work correctly */
    if(required_stencil && required_stencil != existing_stencil) return FALSE;
    return TRUE;
}

/* The caller provides a context */
static void context_validate_onscreen_formats(struct wined3d_device *device,
        struct wined3d_context *context, struct wined3d_surface *depth_stencil)
{
    /* Onscreen surfaces are always in a swapchain */
    struct wined3d_swapchain *swapchain = context->current_rt->container.u.swapchain;

    if (context->render_offscreen || !depth_stencil) return;
    if (match_depth_stencil_format(swapchain->ds_format, depth_stencil->resource.format)) return;

    /* TODO: If the requested format would satisfy the needs of the existing one(reverse match),
     * or no onscreen depth buffer was created, the OpenGL drawable could be changed to the new
     * format. */
    WARN("Depth stencil format is not supported by WGL, rendering the backbuffer in an FBO\n");

    /* The currently active context is the necessary context to access the swapchain's onscreen buffers */
    surface_load_location(context->current_rt, SFLAG_INTEXTURE, NULL);
    swapchain->render_to_fbo = TRUE;
    context_set_render_offscreen(context, device->StateTable, TRUE);
}

/* Context activation is done by the caller. */
void context_apply_blit_state(struct wined3d_context *context, struct wined3d_device *device)
{
    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
    {
        context_validate_onscreen_formats(device, context, NULL);

        if (context->render_offscreen)
        {
            surface_internal_preload(context->current_rt, SRGB_RGB);

            ENTER_GL();
            context_apply_fbo_state_blit(context, GL_FRAMEBUFFER, context->current_rt, NULL, SFLAG_INTEXTURE);
            LEAVE_GL();
        }
        else
        {
            ENTER_GL();
            context_bind_fbo(context, GL_FRAMEBUFFER, NULL);
            LEAVE_GL();
        }

        context->draw_buffer_dirty = TRUE;
    }

    ENTER_GL();
    if (context->draw_buffer_dirty)
    {
        context_apply_draw_buffers(context, 1, &context->current_rt);
        if (wined3d_settings.offscreen_rendering_mode != ORM_FBO)
            context->draw_buffer_dirty = FALSE;
    }

    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
    {
        context_check_fbo_status(context, GL_FRAMEBUFFER);
    }
    LEAVE_GL();

    SetupForBlit(device, context);
}

static BOOL context_validate_rt_config(UINT rt_count,
        struct wined3d_surface **rts, struct wined3d_surface *ds)
{
    unsigned int i;

    if (ds) return TRUE;

    for (i = 0; i < rt_count; ++i)
    {
        if (rts[i] && rts[i]->resource.format->id != WINED3DFMT_NULL)
            return TRUE;
    }

    WARN("Invalid render target config, need at least one attachment.\n");
    return FALSE;
}

/* Context activation is done by the caller. */
BOOL context_apply_clear_state(struct wined3d_context *context, struct wined3d_device *device,
        UINT rt_count, struct wined3d_surface **rts, struct wined3d_surface *depth_stencil)
{
    const struct StateEntry *state_table = device->StateTable;
    DWORD rt_mask = 0;
    UINT i;

    if (!context_validate_rt_config(rt_count, rts, depth_stencil))
        return FALSE;


    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
    {
        context_validate_onscreen_formats(device, context, depth_stencil);

        ENTER_GL();

        if (!rt_count || surface_is_offscreen(rts[0]))
        {
            for (i = 0; i < rt_count; ++i)
            {
                context->blit_targets[i] = rts[i];
                rt_mask |= (1 << i);
            }
            while (i < context->gl_info->limits.buffers)
            {
                context->blit_targets[i] = NULL;
                ++i;
            }
            context_apply_fbo_state(context, GL_FRAMEBUFFER, context->blit_targets, depth_stencil, SFLAG_INTEXTURE);
            glReadBuffer(GL_NONE);
            checkGLcall("glReadBuffer");
        }
        else
        {
            context_apply_fbo_state(context, GL_FRAMEBUFFER, NULL, NULL, SFLAG_INDRAWABLE);
            rt_mask = 1;
        }

        LEAVE_GL();
    }
    else
    {
        rt_mask = 1;
    }

    ENTER_GL();
    context_apply_draw_buffers(context, rt_mask, rts);
    context->draw_buffer_dirty = TRUE;

    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
    {
        context_check_fbo_status(context, GL_FRAMEBUFFER);
    }

    if (context->last_was_blit)
    {
        device->frag_pipe->enable_extension(TRUE);
    }

    /* Blending and clearing should be orthogonal, but tests on the nvidia
     * driver show that disabling blending when clearing improves the clearing
     * performance incredibly. */
    glDisable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);
    checkGLcall("glEnable GL_SCISSOR_TEST");
    LEAVE_GL();

    context->last_was_blit = FALSE;
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_ALPHABLENDENABLE), state_table);
    Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_SCISSORTESTENABLE), state_table);
    Context_MarkStateDirty(context, STATE_SCISSORRECT, state_table);

    return TRUE;
}

/* Context activation is done by the caller. */
BOOL context_apply_draw_state(struct wined3d_context *context, struct wined3d_device *device)
{
    const struct StateEntry *state_table = device->StateTable;
    const struct wined3d_fb_state *fb = &device->fb;
    unsigned int i;

    if (!context_validate_rt_config(context->gl_info->limits.buffers,
            fb->render_targets, fb->depth_stencil))
        return FALSE;

    /* Preload resources before FBO setup. Texture preload in particular may
     * result in changes to the current FBO, due to using e.g. FBO blits for
     * updating a resource location. */
    device_update_tex_unit_map(device);
    device_preload_textures(device);
    if (isStateDirty(context, STATE_VDECL))
        device_update_stream_info(device, context->gl_info);

    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
    {
        context_validate_onscreen_formats(device, context, fb->depth_stencil);

        if (!context->render_offscreen)
        {
            ENTER_GL();
            context_apply_fbo_state(context, GL_FRAMEBUFFER, NULL, NULL, SFLAG_INDRAWABLE);
            LEAVE_GL();
        }
        else
        {
            ENTER_GL();
            context_apply_fbo_state(context, GL_FRAMEBUFFER, fb->render_targets, fb->depth_stencil, SFLAG_INTEXTURE);
            glReadBuffer(GL_NONE);
            checkGLcall("glReadBuffer");
            LEAVE_GL();
        }
    }

    ENTER_GL();
    if (context->draw_buffer_dirty)
    {
        const struct wined3d_shader *ps = device->stateBlock->state.pixel_shader;
        DWORD rt_mask = ps ? ps->reg_maps.rt_mask : 1;

        rt_mask &= device->valid_rt_mask;
        context_apply_draw_buffers(context, rt_mask, fb->render_targets);
        context->draw_buffer_dirty = FALSE;
    }

    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
    {
        context_check_fbo_status(context, GL_FRAMEBUFFER);
    }

    if (context->last_was_blit)
    {
        device->frag_pipe->enable_extension(TRUE);
    }

    for (i = 0; i < context->numDirtyEntries; ++i)
    {
        DWORD rep = context->dirtyArray[i];
        DWORD idx = rep / (sizeof(*context->isStateDirty) * CHAR_BIT);
        BYTE shift = rep & ((sizeof(*context->isStateDirty) * CHAR_BIT) - 1);
        context->isStateDirty[idx] &= ~(1 << shift);
        state_table[rep].apply(rep, device->stateBlock, context);
    }
    LEAVE_GL();
    context->numDirtyEntries = 0; /* This makes the whole list clean */
    context->last_was_blit = FALSE;

    return TRUE;
}

static void context_setup_target(struct wined3d_device *device,
        struct wined3d_context *context, struct wined3d_surface *target)
{
    BOOL old_render_offscreen = context->render_offscreen, render_offscreen;
    const struct StateEntry *StateTable = device->StateTable;

    if (!target) return;
    render_offscreen = surface_is_offscreen(target);
    if (context->current_rt == target && render_offscreen == old_render_offscreen) return;

    /* To compensate the lack of format switching with some offscreen rendering methods and on onscreen buffers
     * the alpha blend state changes with different render target formats. */
    if (!context->current_rt)
    {
        Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_ALPHABLENDENABLE), StateTable);
    }
    else
    {
        const struct wined3d_format *old = context->current_rt->resource.format;
        const struct wined3d_format *new = target->resource.format;

        if (old->id != new->id)
        {
            /* Disable blending when the alpha mask has changed and when a format doesn't support blending. */
            if ((old->alpha_mask && !new->alpha_mask) || (!old->alpha_mask && new->alpha_mask)
                    || !(new->flags & WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING))
            {
                Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_ALPHABLENDENABLE), StateTable);
            }
            /* Update sRGB writing when switching between formats that do/do not support sRGB writing */
            if ((old->flags & WINED3DFMT_FLAG_SRGB_WRITE) != (new->flags & WINED3DFMT_FLAG_SRGB_WRITE))
            {
                Context_MarkStateDirty(context, STATE_RENDER(WINED3DRS_SRGBWRITEENABLE), StateTable);
            }
        }

        /* When switching away from an offscreen render target, and we're not
         * using FBOs, we have to read the drawable into the texture. This is
         * done via PreLoad (and SFLAG_INDRAWABLE set on the surface). There
         * are some things that need care though. PreLoad needs a GL context,
         * and FindContext is called before the context is activated. It also
         * has to be called with the old rendertarget active, otherwise a
         * wrong drawable is read. */
        if (wined3d_settings.offscreen_rendering_mode != ORM_FBO
                && old_render_offscreen && context->current_rt != target)
        {
            /* Read the back buffer of the old drawable into the destination texture. */
            if (context->current_rt->texture_name_srgb)
                surface_internal_preload(context->current_rt, SRGB_SRGB);
            surface_internal_preload(context->current_rt, SRGB_RGB);
            surface_modify_location(context->current_rt, SFLAG_INDRAWABLE, FALSE);
        }
    }

    context->draw_buffer_dirty = TRUE;
    context->current_rt = target;
    context_set_render_offscreen(context, StateTable, render_offscreen);
}

/* Do not call while under the GL lock. */
struct wined3d_context *context_acquire(struct wined3d_device *device, struct wined3d_surface *target)
{
    struct wined3d_context *current_context = context_get_current();
    struct wined3d_context *context;

    TRACE("device %p, target %p.\n", device, target);

    context = FindContext(device, target);
    context_setup_target(device, context, target);
    context_enter(context);
    if (!context->valid) return context;

    if (context != current_context)
    {
        if (!context_set_current(context))
        {
            ERR("Failed to activate the new context.\n");
        }
        else
        {
            ENTER_GL();
            device->frag_pipe->enable_extension(!context->last_was_blit);
            LEAVE_GL();
        }

        if (context->vshader_const_dirty)
        {
            memset(context->vshader_const_dirty, 1,
                    sizeof(*context->vshader_const_dirty) * device->d3d_vshader_constantF);
            device->highest_dirty_vs_const = device->d3d_vshader_constantF;
        }
        if (context->pshader_const_dirty)
        {
            memset(context->pshader_const_dirty, 1,
                   sizeof(*context->pshader_const_dirty) * device->d3d_pshader_constantF);
            device->highest_dirty_ps_const = device->d3d_pshader_constantF;
        }
    }
    else if (context->restore_ctx)
    {
        context_set_gl_context(context);
    }

    return context;
}
