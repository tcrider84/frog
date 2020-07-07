/*
 * ntoskrnl.exe implementation
 *
 * Copyright (C) 2007 Alexandre Julliard
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

#ifndef __WINE_NTOSKRNL_PRIVATE_H
#define __WINE_NTOSKRNL_PRIVATE_H

#include "wine/asm.h"
#include "wine/list.h"

static inline LPCSTR debugstr_us( const UNICODE_STRING *us )
{
    if (!us) return "<null>";
    return debugstr_wn( us->Buffer, us->Length / sizeof(WCHAR) );
}

#define TO_USER(x) (typeof(x)) ((ULONG_PTR)x & 0x0000ffffffffffff)
#define TO_KRNL(x) (PVOID) (x ? (ULONG_PTR)x | 0xffff000000000000 : (ULONG_PTR)x)

typedef void (*kernel_struct_accessed)(void *base, DWORD offset, BOOL write, void *ip);

void *register_kernel_struct(void *obj, unsigned int size, kernel_struct_accessed callback);
void forget_kernel_struct(void *obj);

void flush_emulated_memory(void);

int suspend_all_other_threads(void);
void resume_system_threads(void);


struct _OBJECT_TYPE
{
    const WCHAR *name;            /* object type name used for type validation */
    void *(*constructor)(HANDLE); /* used for creating an object from server handle */
    void (*release)(void*);       /* called when the last reference is released */
};

struct _EPROCESS
{
    DISPATCHER_HEADER header;
    /* padding to require a 32-bit displacement */
    CHAR padding[0x200 - sizeof(DISPATCHER_HEADER)];
    PVOID handle_table;
    PROCESS_BASIC_INFORMATION info;
    CHAR image_file_name[16];
    /* TODO: we should store a section object here instead */
    PFILE_OBJECT file_object;
    PVOID section_base_address;
    LONGLONG create_time;
    BOOL wow64;
};
//356
struct _KTHREAD
{
    DISPATCHER_HEADER header;
    /* padding to require a 32-bit displacement */
    CHAR padding[356 - sizeof(DISPATCHER_HEADER)];
    BYTE bruh;
    KPROCESSOR_MODE prev_mode;
    PEPROCESS process;
    CLIENT_ID id;
    unsigned int critical_region;
    LIST_ENTRY ApcListHead[2];
    CRITICAL_SECTION apc_cs;
    HANDLE apc_event;
    HANDLE imposter_thread;
    struct list system_thread_entry;
    PVOID user_input_copy, user_output_copy;
    PBYTE user_input, user_output;
    BOOL bruhther;
    CHAR more_padding[750];
};

struct _ETHREAD
{
    struct _KTHREAD kthread;
};

void *alloc_kernel_object( POBJECT_TYPE type, HANDLE handle, SIZE_T size, LONG ref ) DECLSPEC_HIDDEN;
NTSTATUS kernel_object_from_handle( HANDLE handle, POBJECT_TYPE type, void **ret ) DECLSPEC_HIDDEN;

extern POBJECT_TYPE ExEventObjectType;
extern POBJECT_TYPE ExSemaphoreObjectType;
extern POBJECT_TYPE IoDeviceObjectType;
extern POBJECT_TYPE IoDriverObjectType;
extern POBJECT_TYPE IoFileObjectType;
extern POBJECT_TYPE PsProcessType;
extern POBJECT_TYPE PsThreadType;
extern POBJECT_TYPE SeTokenObjectType;

#define DECLARE_CRITICAL_SECTION(cs) \
    static CRITICAL_SECTION cs; \
    static CRITICAL_SECTION_DEBUG cs##_debug = \
    { 0, 0, &cs, { &cs##_debug.ProcessLocksList, &cs##_debug.ProcessLocksList }, \
      0, 0, { (DWORD_PTR)(__FILE__ ": " # cs) }}; \
    static CRITICAL_SECTION cs = { &cs##_debug, -1, 0, 0, 0, 0 };

void ObReferenceObject( void *obj ) DECLSPEC_HIDDEN;

void pnp_manager_enumerate_root_devices( const WCHAR *driver_name ) DECLSPEC_HIDDEN;
void pnp_manager_start(void) DECLSPEC_HIDDEN;
void pnp_manager_stop(void) DECLSPEC_HIDDEN;

void disk_driver_start(void) DECLSPEC_HIDDEN;
void disk_driver_stop(void) DECLSPEC_HIDDEN;

static const WCHAR servicesW[] = {'\\','R','e','g','i','s','t','r','y',
                                  '\\','M','a','c','h','i','n','e',
                                  '\\','S','y','s','t','e','m',
                                  '\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t',
                                  '\\','S','e','r','v','i','c','e','s',
                                  '\\',0};

struct wine_device
{
    DEVICE_OBJECT device_obj;
    DEVICE_RELATIONS *children;
};

extern DWORD64 eac_base;
#endif
