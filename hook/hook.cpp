/*
 *  Copyright (c) 2020, Rockchip Electronics Co., Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <unordered_map>
#include <string>

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <xf86drm.h>
#include <sys/mman.h>

#ifdef HAS_GBM
#include <gbm.h>
#endif

#ifdef HAS_EGL
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#ifdef HAS_X11
#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#endif

#ifdef HAS_VULKAN
#include <vulkan/vulkan.h>
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

#ifndef DRM_FORMAT_MOD_INVALID
#define DRM_FORMAT_MOD_INVALID ((1ULL<<56) - 1)
#endif

extern "C" {

/* A stub symbol to ensure that the hook library would not be removed as unused */
int mali_injected = 0;

/* Override libmali symbols */

#ifdef HAS_GBM
#ifndef HAS_gbm_bo_map
static int (* _gbm_device_get_fd)(struct gbm_device *gbm) = NULL;
#endif
static struct gbm_surface * (* _gbm_surface_create)(struct gbm_device *, uint32_t, uint32_t, uint32_t, uint32_t) = NULL;
#ifdef HAS_gbm_surface_create_with_modifiers
static struct gbm_surface *(* _gbm_surface_create_with_modifiers) (struct gbm_device *gbm, uint32_t width, uint32_t height, uint32_t format, const uint64_t *modifiers, const unsigned int count);
#endif
static struct gbm_bo * (* _gbm_bo_create)(struct gbm_device *, uint32_t, uint32_t, uint32_t, uint32_t) = NULL;
#ifdef HAS_gbm_bo_create_with_modifiers
static struct gbm_bo * (* _gbm_bo_create_with_modifiers)(struct gbm_device *gbm, uint32_t width, uint32_t height, uint32_t format, const uint64_t *modifiers, const unsigned int count) = NULL;
#endif
#ifdef HAS_gbm_bo_get_modifier
static uint64_t (* _gbm_bo_get_modifier)(struct gbm_bo *bo) = NULL;
#endif
#ifndef HAS_gbm_bo_map
static uint32_t (* _gbm_bo_get_width)(struct gbm_bo *bo) = NULL;
#endif
#if !defined(HAS_gbm_bo_map) || !defined(HAS_gbm_bo_unmap)
static uint32_t (* _gbm_bo_get_height)(struct gbm_bo *bo) = NULL;
#endif
#if !defined(HAS_gbm_bo_map) || !defined(HAS_gbm_bo_unmap) || \
   !defined(HAS_gbm_bo_get_stride_for_plane)
static uint32_t (* _gbm_bo_get_stride)(struct gbm_bo *bo) = NULL;
#endif
#ifndef HAS_gbm_bo_get_bpp
static uint32_t (* _gbm_bo_get_format)(struct gbm_bo *bo) = NULL;
#endif
#ifndef HAS_gbm_bo_map
static uint32_t (* _gbm_bo_get_bpp)(struct gbm_bo *bo) = NULL;
static struct gbm_device * (* _gbm_bo_get_device)(struct gbm_bo *bo) = NULL;
#endif
#if !defined(HAS_gbm_bo_map) || !defined(HAS_gbm_bo_get_handle_for_plane)
static union gbm_bo_handle (* _gbm_bo_get_handle)(struct gbm_bo *bo) = NULL;
#endif
#ifndef HAS_gbm_bo_get_fd_for_plane
static int (* _gbm_bo_get_fd)(struct gbm_bo *bo) = NULL;
#endif
#endif

#ifdef HAS_EGL
static PFNEGLGETCURRENTSURFACEPROC _eglGetCurrentSurface = NULL;
static PFNEGLGETDISPLAYPROC _eglGetDisplay = NULL;
static PFNEGLGETPROCADDRESSPROC _eglGetProcAddress = NULL;
static PFNEGLGETPLATFORMDISPLAYPROC _eglGetPlatformDisplay = NULL;
static PFNEGLCHOOSECONFIGPROC _eglChooseConfig = NULL;
static PFNEGLGETPLATFORMDISPLAYEXTPROC _eglGetPlatformDisplayEXT = NULL;
static PFNEGLCREATEPIXMAPSURFACEPROC _eglCreatePixmapSurface = NULL;
static PFNEGLCREATEWINDOWSURFACEPROC _eglCreateWindowSurface = NULL;
static EGLBoolean (* _eglDestroySurface)(EGLDisplay dpy, EGLSurface surface) = NULL;
static PFNEGLMAKECURRENTPROC _eglMakeCurrent = NULL;
#endif

#ifdef HAS_VULKAN
static PFN_vkVoidFunction (* _vk_icdGetInstanceProcAddr)(VkInstance instance, const char* pName) = NULL;
static PFN_vkVoidFunction (* _vk_icdGetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName) = NULL;
#endif

#define MALI_SYMBOL(func) { #func, (void **)(&_ ## func), }
static struct {
   const char *func;
   void **symbol;
} mali_symbols[] = {
#ifdef HAS_GBM
#ifndef HAS_gbm_bo_map
   MALI_SYMBOL(gbm_device_get_fd),
#endif
   MALI_SYMBOL(gbm_surface_create),
#ifdef HAS_gbm_surface_create_with_modifiers
   MALI_SYMBOL(gbm_surface_create_with_modifiers),
#endif
   MALI_SYMBOL(gbm_bo_create),
#ifdef HAS_gbm_bo_create_with_modifiers
   MALI_SYMBOL(gbm_bo_create_with_modifiers),
#endif
#ifdef HAS_gbm_bo_get_modifier
   MALI_SYMBOL(gbm_bo_get_modifier),
#endif
#ifndef HAS_gbm_bo_map
   MALI_SYMBOL(gbm_bo_get_width),
#endif
#if !defined(HAS_gbm_bo_map) || !defined(HAS_gbm_bo_unmap)
   MALI_SYMBOL(gbm_bo_get_height),
#endif
#if !defined(HAS_gbm_bo_map) || !defined(HAS_gbm_bo_unmap) || \
   !defined(HAS_gbm_bo_get_stride_for_plane)
   MALI_SYMBOL(gbm_bo_get_stride),
#endif
#ifndef HAS_gbm_bo_get_bpp
   MALI_SYMBOL(gbm_bo_get_format),
#endif
#ifndef HAS_gbm_bo_map
   MALI_SYMBOL(gbm_bo_get_bpp),
   MALI_SYMBOL(gbm_bo_get_device),
#endif
#if !defined(HAS_gbm_bo_map) || !defined(HAS_gbm_bo_get_handle_for_plane)
   MALI_SYMBOL(gbm_bo_get_handle),
#endif
#ifndef HAS_gbm_bo_get_fd_for_plane
   MALI_SYMBOL(gbm_bo_get_fd),
#endif
#endif
#ifdef HAS_EGL
   MALI_SYMBOL(eglGetCurrentSurface),
   MALI_SYMBOL(eglGetDisplay),
   MALI_SYMBOL(eglGetProcAddress),
   MALI_SYMBOL(eglChooseConfig),
   MALI_SYMBOL(eglCreatePixmapSurface),
   MALI_SYMBOL(eglCreateWindowSurface),
   MALI_SYMBOL(eglDestroySurface),
   MALI_SYMBOL(eglMakeCurrent),
#endif
#ifdef HAS_VULKAN
   MALI_SYMBOL(vk_icdGetInstanceProcAddr),
   MALI_SYMBOL(vk_icdGetPhysicalDeviceProcAddr),
#endif
};

__attribute__((constructor)) static void
load_mali_symbols(void)
{
   void *handle, *symbol;
   unsigned int i;

   /* The libmali should be already loaded */
   handle = dlopen(LIBMALI_SO, RTLD_LAZY | RTLD_NOLOAD);
   if (!handle) {
      /* Should not reach here */
      fprintf(stderr, "[MALI-HOOK] FATAL: dlopen(" LIBMALI_SO ") failed(%s)\n",
              dlerror());
      exit(-1);
   }

   for (i = 0; i < ARRAY_SIZE(mali_symbols); i++) {
      const char *func = mali_symbols[i].func;

      /* Clear error */
      dlerror();

      symbol = dlsym(handle, func);
      if (!symbol) {
         /* Should not reach here */
         fprintf(stderr, "[MALI-HOOK] FATAL: " LIBMALI_SO
                 " dlsym(%s) failed(%s)\n", func, dlerror());
         exit(-1);
      }

      *mali_symbols[i].symbol = symbol;
   }

   dlclose(handle);

#ifdef HAS_EGL
   _eglGetPlatformDisplay =
      (PFNEGLGETPLATFORMDISPLAYPROC)_eglGetProcAddress("eglGetPlatformDisplay");
   _eglGetPlatformDisplayEXT =
      (PFNEGLGETPLATFORMDISPLAYEXTPROC)_eglGetProcAddress("eglGetPlatformDisplayEXT");
#endif
}

#ifdef HAS_GBM

/* Implement new GBM APIs */

__attribute__((unused)) static inline bool
can_ignore_modifiers(const uint64_t *modifiers,
                     const unsigned int count)
{
   for (unsigned int i = 0; i < count; i++) {
      if (modifiers[i] == DRM_FORMAT_MOD_LINEAR ||
          modifiers[i] == DRM_FORMAT_MOD_INVALID) {
         return true;
      }
   }

   return !count;
}

#ifndef HAS_gbm_bo_get_offset
uint32_t
gbm_bo_get_offset(struct gbm_bo *bo, int plane)
{
   return 0;
}
#endif

#ifndef HAS_gbm_bo_get_plane_count
int
gbm_bo_get_plane_count(struct gbm_bo *bo)
{
   return 1;
}
#endif

#ifndef HAS_gbm_bo_get_stride_for_plane
uint32_t
gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane)
{
   if (plane)
      return 0;

   return _gbm_bo_get_stride(bo);
}
#endif

#ifndef HAS_gbm_bo_get_fd_for_plane
int
gbm_bo_get_fd_for_plane(struct gbm_bo *bo, int plane)
{
   if (plane)
      return -1;

   return _gbm_bo_get_fd(bo);
}
#endif

#ifndef HAS_gbm_bo_get_handle_for_plane
union gbm_bo_handle
gbm_bo_get_handle_for_plane(struct gbm_bo *bo, int plane)
{
   union gbm_bo_handle ret;
   ret.s32 = -1;

   if (plane)
      return ret;

   return _gbm_bo_get_handle(bo);
}
#endif

#ifndef HAS_gbm_device_get_format_modifier_plane_count
int
gbm_device_get_format_modifier_plane_count(struct gbm_device *gbm,
                                           uint32_t format,
                                           uint64_t modifier)
{
   return can_ignore_modifiers(&modifier, 1) ? 1 : 0;
}
#endif

#ifndef HAS_gbm_bo_create_with_modifiers2
struct gbm_bo *
gbm_bo_create_with_modifiers2(struct gbm_device *gbm,
                              uint32_t width, uint32_t height,
                              uint32_t format,
                              const uint64_t *modifiers,
                              const unsigned int count,
                              uint32_t flags)
{
#ifdef HAS_gbm_bo_create_with_modifiers
   /* flags ignored */
   return _gbm_bo_create_with_modifiers(gbm, width, height, format,
                                        modifiers, count);
#else
   if (!can_ignore_modifiers(modifiers, count))
      return NULL;

   return gbm_bo_create(gbm, width, height, format, flags);
#endif
}
#endif

#ifndef HAS_gbm_bo_create_with_modifiers
struct gbm_bo *
gbm_bo_create_with_modifiers(struct gbm_device *gbm,
                             uint32_t width, uint32_t height,
                             uint32_t format,
                             const uint64_t *modifiers,
                             const unsigned int count)
{
   return gbm_bo_create_with_modifiers2(gbm, width, height, format,
                                        modifiers, count, GBM_BO_USE_SCANOUT);
}
#endif

#ifndef HAS_gbm_surface_create_with_modifiers2
struct gbm_surface *
gbm_surface_create_with_modifiers2(struct gbm_device *gbm,
                                   uint32_t width, uint32_t height,
                                   uint32_t format,
                                   const uint64_t *modifiers,
                                   const unsigned int count,
                                   uint32_t flags)
{
#ifdef HAS_gbm_surface_create_with_modifiers
   /* flags ignored */
   return _gbm_surface_create_with_modifiers(gbm, width, height, format,
                                             modifiers, count);
#else
   if (!can_ignore_modifiers(modifiers, count))
      return NULL;

   return gbm_surface_create(gbm, width, height, format, 0);
#endif
}
#endif

#ifndef HAS_gbm_surface_create_with_modifiers
struct gbm_surface *
gbm_surface_create_with_modifiers(struct gbm_device *gbm,
                                  uint32_t width, uint32_t height,
                                  uint32_t format,
                                  const uint64_t *modifiers,
                                  const unsigned int count)
{
   return gbm_surface_create_with_modifiers2(gbm, width, height, format,
                                             modifiers, count,
                                             GBM_BO_USE_SCANOUT);
}
#endif

#ifndef HAS_gbm_bo_map
void *
gbm_bo_map(struct gbm_bo *bo,
           uint32_t x, uint32_t y, uint32_t width, uint32_t height,
           uint32_t flags, uint32_t *stride, void **map_data)
{
   struct drm_mode_map_dumb arg;
   struct gbm_device *gbm_dev;
   void *map;
   int fd, ret;

   if (!bo || !map_data || width <= 0 || width > _gbm_bo_get_width(bo) ||
       height <= 0 || height > _gbm_bo_get_height(bo)) {
      errno = EINVAL;
      return MAP_FAILED;
   }

   gbm_dev = _gbm_bo_get_device(bo);
   if (!gbm_dev)
      return MAP_FAILED;

   fd = _gbm_device_get_fd(gbm_dev);
   if (fd < 0)
      return MAP_FAILED;

   memset(&arg, 0, sizeof(arg));
   arg.handle = _gbm_bo_get_handle(bo).u32;
   ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
   if (ret)
      return MAP_FAILED;

   map = mmap(NULL, _gbm_bo_get_stride(bo) * _gbm_bo_get_height(bo),
              PROT_READ | PROT_WRITE, MAP_SHARED, fd, arg.offset);
   if (map == MAP_FAILED)
      return map;

   *map_data = map;

   if (stride)
      *stride = _gbm_bo_get_stride(bo);

   return map + y * _gbm_bo_get_stride(bo) + x * (_gbm_bo_get_bpp(bo) >> 3);
}
#endif

#ifndef HAS_gbm_bo_unmap
void
gbm_bo_unmap(struct gbm_bo *bo, void *map_data)
{
   if (map_data)
      munmap(map_data, _gbm_bo_get_stride(bo) * _gbm_bo_get_height(bo));
}
#endif

/* From mesa3d mesa-23.1.3-1 : src/gbm/main/gbm.c */
#ifndef HAS_gbm_bo_get_bpp
uint32_t
gbm_bo_get_bpp(struct gbm_bo *bo)
{
   switch (_gbm_bo_get_format(bo)) {
   default:
      return 0;
   case GBM_FORMAT_C8:
   case GBM_FORMAT_R8:
   case GBM_FORMAT_RGB332:
   case GBM_FORMAT_BGR233:
      return 8;
   case GBM_FORMAT_R16:
   case GBM_FORMAT_GR88:
   case GBM_FORMAT_XRGB4444:
   case GBM_FORMAT_XBGR4444:
   case GBM_FORMAT_RGBX4444:
   case GBM_FORMAT_BGRX4444:
   case GBM_FORMAT_ARGB4444:
   case GBM_FORMAT_ABGR4444:
   case GBM_FORMAT_RGBA4444:
   case GBM_FORMAT_BGRA4444:
   case GBM_FORMAT_XRGB1555:
   case GBM_FORMAT_XBGR1555:
   case GBM_FORMAT_RGBX5551:
   case GBM_FORMAT_BGRX5551:
   case GBM_FORMAT_ARGB1555:
   case GBM_FORMAT_ABGR1555:
   case GBM_FORMAT_RGBA5551:
   case GBM_FORMAT_BGRA5551:
   case GBM_FORMAT_RGB565:
   case GBM_FORMAT_BGR565:
      return 16;
   case GBM_FORMAT_RGB888:
   case GBM_FORMAT_BGR888:
      return 24;
   case GBM_FORMAT_RG1616:
   case GBM_FORMAT_GR1616:
   case GBM_FORMAT_XRGB8888:
   case GBM_FORMAT_XBGR8888:
   case GBM_FORMAT_RGBX8888:
   case GBM_FORMAT_BGRX8888:
   case GBM_FORMAT_ARGB8888:
   case GBM_FORMAT_ABGR8888:
   case GBM_FORMAT_RGBA8888:
   case GBM_FORMAT_BGRA8888:
   case GBM_FORMAT_XRGB2101010:
   case GBM_FORMAT_XBGR2101010:
   case GBM_FORMAT_RGBX1010102:
   case GBM_FORMAT_BGRX1010102:
   case GBM_FORMAT_ARGB2101010:
   case GBM_FORMAT_ABGR2101010:
   case GBM_FORMAT_RGBA1010102:
   case GBM_FORMAT_BGRA1010102:
      return 32;
   case GBM_FORMAT_XBGR16161616:
   case GBM_FORMAT_ABGR16161616:
   case GBM_FORMAT_XBGR16161616F:
   case GBM_FORMAT_ABGR16161616F:
      return 64;
   }
}
#endif

/* From mesa3d mesa-23.1.3-1 : src/gbm/main/gbm.c */
#ifndef HAS_gbm_format_get_name
static uint32_t
gbm_format_canonicalize(uint32_t gbm_format)
{
   switch (gbm_format) {
   case GBM_BO_FORMAT_XRGB8888:
      return GBM_FORMAT_XRGB8888;
   case GBM_BO_FORMAT_ARGB8888:
      return GBM_FORMAT_ARGB8888;
   default:
      return gbm_format;
   }
}

char *
gbm_format_get_name(uint32_t gbm_format, struct gbm_format_name_desc *desc)
{
   gbm_format = gbm_format_canonicalize(gbm_format);

   desc->name[0] = gbm_format;
   desc->name[1] = gbm_format >> 8;
   desc->name[2] = gbm_format >> 16;
   desc->name[3] = gbm_format >> 24;
   desc->name[4] = 0;

   return desc->name;
}
#endif

/* Wrappers for invalid modifier */

uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo)
{
#ifdef HAS_gbm_bo_get_modifier
   uint64_t modifier = _gbm_bo_get_modifier(bo);
   if (modifier != DRM_FORMAT_MOD_INVALID)
      return modifier;
#endif
   return DRM_FORMAT_MOD_LINEAR;
}

/* Wrappers for unsupported flags */

struct gbm_surface *
gbm_surface_create(struct gbm_device *gbm,
                   uint32_t width, uint32_t height,
                   uint32_t format, uint32_t flags)
{
   struct gbm_surface *surface;

   surface = _gbm_surface_create(gbm, width, height, format, flags);
   if (surface)
      return surface;

   flags &= GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
   return _gbm_surface_create(gbm, width, height, format, flags);
}

struct gbm_bo *
gbm_bo_create(struct gbm_device *gbm,
              uint32_t width, uint32_t height,
              uint32_t format, uint32_t flags)
{
   struct gbm_bo *bo;

   bo = _gbm_bo_create(gbm, width, height, format, flags);
   if (bo)
      return bo;

   flags &= GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING |
      GBM_BO_USE_WRITE | GBM_BO_USE_CURSOR_64X64;
   return _gbm_bo_create(gbm, width, height, format, flags);
}

#endif // HAS_GBM

#ifdef HAS_EGL
#ifdef HAS_X11

/* Hacked displays (should not be much) */
#define MAX_X11_DISPLAY 32
static Display *_x11_displays[MAX_X11_DISPLAY] = { NULL, };

static pthread_mutex_t _x11_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline int
force_x11_threads(void)
{
   return !getenv("MALI_X11_NO_FORCE_THREADS");
}

__attribute__((constructor)) static void
init_x11_threads(void)
{
   if (force_x11_threads())
      XInitThreads();
}

__attribute__((destructor)) static void
cleanup_x11_display(void)
{
   int i;

   for (i = 0; i < MAX_X11_DISPLAY; i++) {
      Display *display = _x11_displays[i];
      if (display)
         XCloseDisplay(display);
   }
}

static Display *
fixup_x11_display(Display *display)
{
   int i;

   if (!force_x11_threads())
      return display;

   if (!display || display->lock_fns)
      return display;

   pthread_mutex_lock(&_x11_mutex);
   /* Create a new threaded display */
   display = XOpenDisplay(DisplayString(display));

   for (i = 0; i < MAX_X11_DISPLAY; i++) {
      if (!_x11_displays[i]) {
         _x11_displays[i] = display;
         break;
      }
   }
   pthread_mutex_unlock(&_x11_mutex);

   return display;
}

#endif // HAS_X11

/* Override EGL symbols */

#ifdef HAS_X11

EGLAPI EGLDisplay EGLAPIENTRY
eglGetPlatformDisplayEXT (EGLenum platform, void *native_display, const EGLint *attrib_list)
{
   if (!_eglGetPlatformDisplayEXT)
      return EGL_NO_DISPLAY;

   if (platform == EGL_PLATFORM_X11_KHR && native_display) {
      native_display = (void *)fixup_x11_display((Display *)native_display);
      if (!native_display)
         return EGL_NO_DISPLAY;
   }

   return _eglGetPlatformDisplayEXT(platform, native_display, attrib_list);
}

#endif // HAS_X11

EGLAPI EGLDisplay EGLAPIENTRY
eglGetDisplay (EGLNativeDisplayType display_id)
{
   const char *type = getenv("MALI_DEFAULT_WINSYS");

   // HACK: For chromium angle with in-process-gpu.
   if (getenv("MALI_FORCE_DEFAULT_DISPLAY") &&
       display_id != EGL_DEFAULT_DISPLAY) {
      fprintf(stderr, "[MALI-HOOK] WARN: Native display(%p) ignored!\n",
              display_id);
      display_id = EGL_DEFAULT_DISPLAY;
   }

#ifdef HAS_GBM
   if (type && !strcmp(type, "gbm"))
      return eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, display_id, NULL);
#endif

#ifdef HAS_WAYLAND
   if (type && !strcmp(type, "wayland"))
      return eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_EXT, display_id, NULL);
#endif

#ifdef HAS_X11
   /* Use X11 by default when avaiable */
   return eglGetPlatformDisplay(EGL_PLATFORM_X11_KHR, display_id, NULL);
#else
   return _eglGetDisplay(display_id);
#endif
}

/* Export for EGL 1.5 */

#define GET_PROC_ADDR(v, n) v = (typeof(v))_eglGetProcAddress(n)

/* From mesa3d mesa-23.1.3-1 : src/egl/main/egldisplay.h */
static inline size_t
_eglNumAttribs(const EGLAttrib *attribs)
{
   size_t len = 0;

   if (attribs) {
      while (attribs[len] != EGL_NONE)
         len += 2;
      len++;
   }
   return len;
}

/* From mesa3d mesa-23.1.3-1 : src/egl/main/eglapi.c */
static EGLint *
_eglConvertAttribsToInt(const EGLAttrib *attr_list)
{
   size_t size = _eglNumAttribs(attr_list);
   EGLint *int_attribs = NULL;

   /* Convert attributes from EGLAttrib[] to EGLint[] */
   if (size) {
      int_attribs = (EGLint *)calloc(size, sizeof(int_attribs[0]));
      if (!int_attribs)
         return NULL;

      for (size_t i = 0; i < size; i++)
         int_attribs[i] = attr_list[i];
   }
   return int_attribs;
}

EGLAPI EGLDisplay EGLAPIENTRY
eglGetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list)
{
   if (_eglGetPlatformDisplayEXT) {
      EGLint *int_attribs = _eglConvertAttribsToInt(attrib_list);
      if (!int_attribs == !attrib_list) {
         EGLDisplay display =
            _eglGetPlatformDisplayEXT(platform, native_display, int_attribs);
         free(int_attribs);
         return display;
      }
   }

   if (!_eglGetPlatformDisplay)
      return EGL_NO_DISPLAY;

#ifdef HAS_X11
   if (platform == EGL_PLATFORM_X11_KHR && native_display) {
      native_display = (void *)fixup_x11_display((Display *)native_display);
      if (!native_display)
         return EGL_NO_DISPLAY;
   }
#endif

   return _eglGetPlatformDisplay(platform, native_display, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list)
{
   PFNEGLCREATEPLATFORMWINDOWSURFACEPROC create_platform_window_surface;

   GET_PROC_ADDR(create_platform_window_surface,
                 "eglCreatePlatformWindowSurface");
   if (!create_platform_window_surface) {
      EGLint *int_attribs = _eglConvertAttribsToInt(attrib_list);
      if (!int_attribs == !attrib_list) {
         EGLSurface surface =
            _eglCreateWindowSurface(dpy, config, native_window, int_attribs);
         free(int_attribs);
         return surface;
      }
   }

   return create_platform_window_surface(dpy, config, native_window, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list)
{
   PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC create_platform_pixmap_surface;

   GET_PROC_ADDR(create_platform_pixmap_surface,
                 "eglCreatePlatformPixmapSurface");
   if (!create_platform_pixmap_surface) {
      EGLint *int_attribs = _eglConvertAttribsToInt(attrib_list);
      if (!int_attribs == !attrib_list) {
         EGLSurface surface =
            _eglCreatePixmapSurface(dpy, config, (gbm_bo *)native_pixmap, int_attribs);
         free(int_attribs);
         return surface;
      }
   }

   return create_platform_pixmap_surface(dpy, config, native_pixmap, attrib_list);
}

/* HACK: Unset current surface before destroying it */

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
   if (_eglGetCurrentSurface(EGL_DRAW) == surface ||
       _eglGetCurrentSurface(EGL_READ) == surface)
      _eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   return _eglDestroySurface(dpy, surface);
}

/* HACK: Fixup EGL_OPENGL_BIT */

EGLBoolean eglChooseConfig (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
#define MAX_EGL_ATTRS 1024
   EGLint list[MAX_EGL_ATTRS];
   int i = 0;

   if (!attrib_list)
      return _eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);

   while (attrib_list[i] != EGL_NONE) {
      if (i > MAX_EGL_ATTRS - 2)
         return EGL_FALSE;

      list[i] = attrib_list[i];
      list[i + 1] = attrib_list[i + 1];

      if (list[i] == EGL_RENDERABLE_TYPE && list[i + 1] == EGL_OPENGL_BIT)
         list[i + 1] = EGL_OPENGL_ES_BIT;

      i += 2;
   }
   list[i] = EGL_NONE;

   return _eglChooseConfig(dpy, list, configs, config_size, num_config);
}

/* Override proc addesses */

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char *procname)
{
   if (!procname)
      return NULL;

   if (!strcmp(procname, __func__))
      return (__eglMustCastToProperFunctionPointerType)eglGetProcAddress;

   if (!strcmp(procname, "eglGetDisplay"))
      return (__eglMustCastToProperFunctionPointerType)eglGetDisplay;

   if (!strcmp(procname, "eglGetPlatformDisplay")) {
      if (!_eglGetPlatformDisplay && !_eglGetPlatformDisplayEXT)
         return NULL;
      return (__eglMustCastToProperFunctionPointerType)eglGetPlatformDisplay;
   }

#ifdef HAS_X11
   if (!strcmp(procname, "eglGetPlatformDisplayEXT")) {
      if (!_eglGetPlatformDisplayEXT)
         return NULL;
      return (__eglMustCastToProperFunctionPointerType)eglGetPlatformDisplayEXT;
   }
#endif

   if (!strcmp(procname, "eglChooseConfig"))
      return (__eglMustCastToProperFunctionPointerType)eglChooseConfig;

   if (!strcmp(procname, "eglCreatePlatformWindowSurface"))
      return (__eglMustCastToProperFunctionPointerType)eglCreatePlatformWindowSurface;

   if (!strcmp(procname, "eglCreatePlatformPixmapSurface"))
      return (__eglMustCastToProperFunctionPointerType)eglCreatePlatformPixmapSurface;

   if (!strcmp(procname, "eglDestroySurface"))
      return (__eglMustCastToProperFunctionPointerType)eglDestroySurface;

   return _eglGetProcAddress(procname);
}

#endif // HAS_EGL

#ifdef HAS_VULKAN

static inline uint32_t PackRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((a << 24) | (b << 16) | (g << 8) | r);
}

void DecompressBlockDXT1(uint32_t x, uint32_t y, uint32_t width, const uint8_t *blockStorage, uint32_t *image) {
    uint16_t color0 = *(uint16_t *)(blockStorage);
    uint16_t color1 = *(uint16_t *)(blockStorage + 2);

    uint32_t temp;

    temp = (color0 >> 11) * 255 + 16;
    uint8_t r0 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    uint8_t g0 = (uint8_t)((temp/64 + temp)/64);
    temp = (color0 & 0x001F) * 255 + 16;
    uint8_t b0 = (uint8_t)((temp/32 + temp)/32);

    temp = (color1 >> 11) * 255 + 16;
    uint8_t r1 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    uint8_t g1 = (uint8_t)((temp/64 + temp)/64);
    temp = (color1 & 0x001F) * 255 + 16;
    uint8_t b1 = (uint8_t)((temp/32 + temp)/32);

    uint32_t code = *(uint32_t *)(blockStorage + 4);

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            uint32_t finalColor = 0;
            uint8_t positionCode = (code >> 2*(4*j+i)) & 0x03;

            if (color0 > color1)
                switch (positionCode) {
                case 0:
                    finalColor = PackRGBA(r0, g0, b0, 255);
                    break;
                case 1:
                    finalColor = PackRGBA(r1, g1, b1, 255);
                    break;
                case 2:
                    finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, 255);
                    break;
                case 3:
                    finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, 255);
                    break;
                }
            else
                switch (positionCode) {
                case 0:
                    finalColor = PackRGBA(r0, g0, b0, 255);
                    break;
                case 1:
                    finalColor = PackRGBA(r1, g1, b1, 255);
                    break;
                case 2:
                    finalColor = PackRGBA((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, 255);
                    break;
                case 3:
                    finalColor = PackRGBA(0, 0, 0, 255);
                    break;
                }

            if (x + i < width)
                image[(y + j)*width + (x + i)] = finalColor;
        }
    }
}

void BlockDecompressImageDXT1(uint32_t width, uint32_t height, const uint8_t *blockStorage, uint32_t *image) {
    uint32_t blockCountX = (width + 3) / 4;
    uint32_t blockCountY = (height + 3) / 4;
    for (uint32_t j = 0; j < blockCountY; j++) {
        for (uint32_t i = 0; i < blockCountX; i++)
            DecompressBlockDXT1(i*4, j*4, width, blockStorage + i * 8, image);
        blockStorage += blockCountX * 8;
    }
}

void DecompressBlockDXT5(uint32_t x, uint32_t y, uint32_t width, const uint8_t *blockStorage, uint32_t *image) {
    uint8_t alpha0 = *(uint8_t *)(blockStorage);
    uint8_t alpha1 = *(uint8_t *)(blockStorage + 1);

    const uint8_t *bits = blockStorage + 2;
    uint32_t alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
    uint16_t alphaCode2 = bits[0] | (bits[1] << 8);

    uint16_t color0 = *(uint16_t *)(blockStorage + 8);
    uint16_t color1 = *(uint16_t *)(blockStorage + 10);

    uint32_t temp;

    temp = (color0 >> 11) * 255 + 16;
    uint8_t r0 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    uint8_t g0 = (uint8_t)((temp/64 + temp)/64);
    temp = (color0 & 0x001F) * 255 + 16;
    uint8_t b0 = (uint8_t)((temp/32 + temp)/32);

    temp = (color1 >> 11) * 255 + 16;
    uint8_t r1 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    uint8_t g1 = (uint8_t)((temp/64 + temp)/64);
    temp = (color1 & 0x001F) * 255 + 16;
    uint8_t b1 = (uint8_t)((temp/32 + temp)/32);

    uint32_t code = *(uint32_t *)(blockStorage + 12);

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            int alphaCodeIndex = 3*(4*j+i);
            int alphaCode;

            if (alphaCodeIndex <= 12)
                alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
            else if (alphaCodeIndex == 15)
                alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
            else // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
                alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;

            uint8_t finalAlpha;
            if (alphaCode == 0)
                finalAlpha = alpha0;
            else if (alphaCode == 1)
                finalAlpha = alpha1;
            else if (alpha0 > alpha1)
                finalAlpha = ((8-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/7;
            else if (alphaCode == 6)
                finalAlpha = 0;
            else if (alphaCode == 7)
                finalAlpha = 255;
            else
                finalAlpha = ((6-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/5;

            uint8_t colorCode = (code >> 2*(4*j+i)) & 0x03;

            uint32_t finalColor;
            switch (colorCode) {
                case 0:
                    finalColor = PackRGBA(r0, g0, b0, finalAlpha);
                    break;
                case 1:
                    finalColor = PackRGBA(r1, g1, b1, finalAlpha);
                    break;
                case 2:
                    finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, finalAlpha);
                    break;
                case 3:
                    finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, finalAlpha);
                    break;
            }

            if (x + i < width)
                image[(y + j)*width + (x + i)] = finalColor;
        }
    }
}

void BlockDecompressImageDXT5(uint32_t width, uint32_t height, const uint8_t *blockStorage, uint32_t *image) {
    uint32_t blockCountX = (width + 3) / 4;
    uint32_t blockCountY = (height + 3) / 4;
    for (uint32_t j = 0; j < blockCountY; j++) {
        for (uint32_t i = 0; i < blockCountX; i++)
            DecompressBlockDXT5(i*4, j*4, width, blockStorage + i * 16, image);
        blockStorage += blockCountX * 16;
    }
}

// #define VK_LOG(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define VK_LOG(fmt, ...)

static PFN_vkGetDeviceProcAddr _vkGetDeviceProcAddr = NULL;
static PFN_vkCmdCopyBufferToImage _vkCmdCopyBufferToImage = NULL;
static PFN_vkMapMemory _vkMapMemory = NULL;
static PFN_vkUnmapMemory _vkUnmapMemory = NULL;
static PFN_vkCreateBuffer _vkCreateBuffer = NULL;
static PFN_vkGetBufferMemoryRequirements _vkGetBufferMemoryRequirements = NULL;
static PFN_vkAllocateMemory _vkAllocateMemory = NULL;
static PFN_vkBindBufferMemory _vkBindBufferMemory = NULL;
static PFN_vkCreateImage _vkCreateImage = NULL;
static PFN_vkGetPhysicalDeviceMemoryProperties _vkGetPhysicalDeviceMemoryProperties = NULL;
static PFN_vkDestroyBuffer _vkDestroyBuffer = NULL;
static PFN_vkFreeMemory _vkFreeMemory = NULL;
static PFN_vkCreateImageView _vkCreateImageView = NULL;
static PFN_vkDestroyImage _vkDestroyImage = NULL;

typedef struct {
   VkDeviceMemory memory;
   size_t memoryOffset;
} buffer_data;

std::unordered_map<VkDeviceMemory, uint8_t *> memory_map;
std::unordered_map<VkBuffer, buffer_data> buffer_map;
std::unordered_map<VkImage, VkFormat> image_map;

static VkDevice _device = NULL;
VkPhysicalDeviceMemoryProperties _memProperties;

void my_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
   _vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
   _memProperties = *pMemoryProperties;
}

void my_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
   auto it = image_map.find(dstImage);
   if (it == image_map.end()) {
      _vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
      return;
   }

   buffer_data &buf = buffer_map[srcBuffer];
   uint8_t *src_data = memory_map[buf.memory] + buf.memoryOffset;

   size_t imageSize = 0;
   VkBufferImageCopy region[regionCount];
   for (uint32_t i = 0; i < regionCount; i++) {
      region[i] = pRegions[i];
      region[i].bufferOffset = imageSize;
      imageSize += region[i].imageExtent.width * region[i].imageExtent.height * 4;
   }

   VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = imageSize,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
   };
   VkBuffer _buffer;
   if (_vkCreateBuffer(_device, &bufferInfo, NULL, &_buffer) != VK_SUCCESS)
      fprintf(stderr, "Failed to create buffer\n");

   VkMemoryRequirements memRequirements;
   _vkGetBufferMemoryRequirements(_device, _buffer, &memRequirements);

   VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
   VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size
   };
   for (uint32_t i = 0; i < _memProperties.memoryTypeCount; i++)
      if ((memRequirements.memoryTypeBits & (1 << i)) && (_memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
         allocInfo.memoryTypeIndex = i;
         break;
      }

   VkDeviceMemory _bufferMemory;
   if (_vkAllocateMemory(_device, &allocInfo, NULL, &_bufferMemory) != VK_SUCCESS)
      fprintf(stderr, "Failed to allocate memory\n");
   if (_vkBindBufferMemory(_device, _buffer, _bufferMemory, 0) != VK_SUCCESS)
      fprintf(stderr, "Failed to bind buffer memory\n");

   uint8_t* _dst_data = NULL;
   if (_vkMapMemory(_device, _bufferMemory, 0, imageSize, 0, (void **)&_dst_data) != VK_SUCCESS)
      fprintf(stderr, "Failed to map memory\n");

   for (uint32_t i = 0; i < regionCount; i++) {
      uint32_t width = region[i].imageExtent.width, height = region[i].imageExtent.height;
      uint8_t *src = src_data + pRegions[i].bufferOffset;
      uint8_t *dst = _dst_data + region[i].bufferOffset;
      if (it->second == VK_FORMAT_BC1_RGBA_UNORM_BLOCK || it->second == VK_FORMAT_BC1_RGBA_SRGB_BLOCK)
         BlockDecompressImageDXT1(width, height, src, (uint32_t *)dst);
      else
         BlockDecompressImageDXT5(width, height, src, (uint32_t *)dst);
   }

   _vkCmdCopyBufferToImage(commandBuffer, _buffer, dstImage, dstImageLayout, regionCount, region);
   _vkUnmapMemory(_device, _bufferMemory);
   _vkDestroyBuffer(_device, _buffer, NULL);
   _vkFreeMemory(_device, _bufferMemory, NULL);
}

VkResult my_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
   VkResult result = _vkMapMemory(device, memory, offset, size, flags, ppData);
   memory_map[memory] = (uint8_t *)*ppData;
   return result;
}

void my_vkUnmapMemory(VkDevice device, VkDeviceMemory memory) {
   memory_map.erase(memory);
   _vkUnmapMemory(device, memory);
}

VkResult my_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
   return _vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

void my_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
   _vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

VkResult my_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
   return  _vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

VkResult my_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
   buffer_map[buffer] = { .memory = memory, .memoryOffset = memoryOffset };
   return  _vkBindBufferMemory(device, buffer, memory, memoryOffset);
}

VkResult my_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
   _device = device;
   VkImageCreateInfo myImageCreateInfo = *pCreateInfo;
   switch (pCreateInfo->format) {
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	 myImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
         break;
      case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	 myImageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
         break;
      case VK_FORMAT_BC3_UNORM_BLOCK:
         myImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
         break;
      case VK_FORMAT_BC3_SRGB_BLOCK:
	 myImageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
         break;
      default:
         break;
   }
   VkResult result = _vkCreateImage(device, &myImageCreateInfo, pAllocator, pImage);
   if (myImageCreateInfo.format != pCreateInfo->format)
      image_map[*pImage] = pCreateInfo->format;
   return result;
}

void my_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
   image_map.erase(image);
   _vkDestroyImage(device, image, pAllocator);
}

VkResult my_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
   VkImageViewCreateInfo myImageViewCreateInfo = *pCreateInfo;
   switch (pCreateInfo->format) {
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	 myImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
         break;
      case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	 myImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
         break;
      case VK_FORMAT_BC3_UNORM_BLOCK:
         myImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
         break;
      case VK_FORMAT_BC3_SRGB_BLOCK:
	 myImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
         break;
      default:
         break;
   }
   return _vkCreateImageView(device, &myImageViewCreateInfo, pAllocator, pView);
}

void my_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
   buffer_map.erase(buffer);
   _vkDestroyBuffer(device, buffer, pAllocator);
}

void my_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
   _vkFreeMemory(device, memory, pAllocator);
}

PFN_vkVoidFunction my_vkGetDeviceProcAddr(VkDevice device, const char* pName);

static std::unordered_map<std::string, std::pair<PFN_vkVoidFunction, PFN_vkVoidFunction*>> _remap;

static void maybe_init() {
   if (_remap.size() != 0) return;

#define ADD_VK_SYM(func) _remap[#func] = std::make_pair((PFN_vkVoidFunction)my_ ## func, (PFN_vkVoidFunction*)&_ ##func)

ADD_VK_SYM(vkGetDeviceProcAddr);
ADD_VK_SYM(vkCmdCopyBufferToImage);
ADD_VK_SYM(vkMapMemory);
ADD_VK_SYM(vkUnmapMemory);
ADD_VK_SYM(vkCreateBuffer);
ADD_VK_SYM(vkGetBufferMemoryRequirements);
ADD_VK_SYM(vkAllocateMemory);
ADD_VK_SYM(vkBindBufferMemory);
ADD_VK_SYM(vkCreateImage);
ADD_VK_SYM(vkGetPhysicalDeviceMemoryProperties);
ADD_VK_SYM(vkDestroyBuffer);
ADD_VK_SYM(vkFreeMemory);
ADD_VK_SYM(vkCreateImageView);
ADD_VK_SYM(vkDestroyImage);
}

PFN_vkVoidFunction myGetProcAddr(PFN_vkVoidFunction addr, const char* pName) {
   maybe_init();
   auto it = _remap.find(pName);
   if (it != _remap.end()) {
      *it->second.second = addr;
      return it->second.first;
   }
   return addr;
}

PFN_vkVoidFunction my_vkGetDeviceProcAddr(VkDevice device, const char* pName) {
   PFN_vkVoidFunction addr = _vkGetDeviceProcAddr(device, pName);
   return myGetProcAddr(addr, pName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetPhysicalDeviceProcAddr(VkInstance instance, const char* pName) {
   PFN_vkVoidFunction addr = _vk_icdGetPhysicalDeviceProcAddr(instance, pName);
   return myGetProcAddr(addr, pName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
   PFN_vkVoidFunction addr = _vk_icdGetInstanceProcAddr(instance, pName);
   return myGetProcAddr(addr, pName);
}
#endif

}
