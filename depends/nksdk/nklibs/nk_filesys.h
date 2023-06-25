#ifndef NK_FILESYS_H__ /*/////////////////////////////////////////////////////*/
#define NK_FILESYS_H__

// If you define NK_SINGLE_UNIT we know you are doing a single compilation unit
// build and we can just include the implementation straight away without needing
// the NK_xxx_IMPLEMENTATION define to be specified for each of the libraries.
#if defined(NK_SINGLE_UNIT) && !defined(NK_FILESYS_IMPLEMENTATION)
#define NK_FILESYS_IMPLEMENTATION
#endif // NK_SINGLE_UNIT && !NK_FILESYS_IMPLEMENTATION

#include "nk_define.h"

#include <stdarg.h>

NK_ENUM(nkFileReadMode, nkS32)
{
    nkFileReadMode_Text,
    nkFileReadMode_Binary
};

typedef struct nkFileContent
{
    void* data;
    nkU64 size; // Does not include null-terminator even when loaded with nkFileReadMode_Text!
}
nkFileContent;

typedef struct nkFileList
{
    nkChar** items;
    nkU64    size;   // Number of slots allocated.
    nkU64    length; // Number of slots filled.
}
nkFileList;

NK_ENUM(nkPathListFlags, nkU32)
{
    nkPathListFlags_None      = (     0),
    nkPathListFlags_Recursive = (1 << 0),
    nkPathListFlags_Files     = (1 << 1),
    nkPathListFlags_Paths     = (1 << 2),
    nkPathListFlags_All       = (    -1)
};

NKAPI nkBool  nk_read_file_content       (nkFileContent* content, const nkChar* file_name, nkFileReadMode mode);
NKAPI nkBool  nk_write_file_content      (nkFileContent* content, const nkChar* file_name);
NKAPI void    nk_free_file_content       (nkFileContent* content);
NKAPI nkChar* nk_get_exec_path           (void); // Result must be freed by the caller!
NKAPI nkChar* nk_get_data_path           (void); // Result must be freed by the caller!
NKAPI nkBool  nk_list_path_content       (const nkChar* path_name, nkPathListFlags flags, nkFileList* file_list);
NKAPI void    nk_free_path_content       (nkFileList* file_list);
NKAPI nkBool  nk_create_file             (const nkChar* file_name);
NKAPI nkBool  nk_create_path             (const nkChar* path_name);
NKAPI nkBool  nk_delete_file             (const nkChar* file_name);
NKAPI nkBool  nk_delete_path             (const nkChar* path_name);
NKAPI nkBool  nk_rename_file             (const nkChar* old_file, const nkChar* new_file);
NKAPI nkBool  nk_rename_path             (const nkChar* old_path, const nkChar* new_path);
NKAPI nkBool  nk_copy_file               (const nkChar* src_file, const nkChar* dst_file);
NKAPI nkBool  nk_move_file               (const nkChar* src_file, const nkChar* dst_file);
NKAPI nkU64   nk_file_size               (const nkChar* file_name);
NKAPI nkBool  nk_file_exists             (const nkChar* file_name);
NKAPI nkBool  nk_path_exists             (const nkChar* path_name);
NKAPI nkBool  nk_is_file                 (const nkChar* file_name);
NKAPI nkBool  nk_is_path                 (const nkChar* path_name);
NKAPI nkBool  nk_is_path_absolute        (const nkChar* path_name);
NKAPI nkBool  nk_is_path_relative        (const nkChar* path_name);
// Path manipulation helpers (result must be freed by the caller)!
NKAPI nkChar* nk_strip_file_path         (const nkChar* file_name); // "some/file/name.txt" => "name.txt"
NKAPI nkChar* nk_strip_file_ext          (const nkChar* file_name); // "some/file/name.txt" => "some/file/name"
NKAPI nkChar* nk_strip_file_name         (const nkChar* file_name); // "some/file/name.txt" => "some/file/"
NKAPI nkChar* nk_strip_file_path_and_ext (const nkChar* file_name); // "some/file/name.txt" => "name"
NKAPI nkChar* nk_strip_file_path_and_name(const nkChar* file_name); // "some/file/name.txt" => ".txt"
NKAPI nkChar* nk_fixup_path              (const nkChar* path_name); // Converts all slashes to forward and appends a trailing slash if necessary.
NKAPI nkChar* nk_make_path_absolute      (const nkChar* path_name);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

#ifdef NK_FILESYS_IMPLEMENTATION /*///////////////////////////////////////////*/

// @Todo: Custom memory allocators.

#if defined(NK_OS_WIN32) /* ------------------------------------------------- */

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <stdlib.h>
#include <string.h>

// @Temporary: We should require these to be properly linked?
#pragma comment(lib, "shell32")
#pragma comment(lib, "shlwapi")

NKINTERNAL nkChar* nk__allocate_path_buffer(nkU64* max_size)
{
    NK_ASSERT(max_size);
    nkChar* buffer = NK_MALLOC_TYPES(nkChar, MAX_PATH);
    (*max_size) = MAX_PATH;
    return buffer;
}

NKINTERNAL nkU64 nk__get_file_size(HANDLE file)
{
    LARGE_INTEGER file_size = NK_ZERO_MEM;
    GetFileSizeEx(file, &file_size);
    return file_size.QuadPart;
}

NKAPI nkBool nk_read_file_content(nkFileContent* content, const nkChar* file_name, nkFileReadMode mode)
{
    NK_ASSERT(content);

    HANDLE file = CreateFileA(file_name, GENERIC_READ, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE) return NK_FALSE;

    nkBool success = NK_FALSE;

    nkU64 bytes_to_read = nk__get_file_size(file);

    content->size = bytes_to_read;
    content->data = NK_MALLOC_BYTES(content->size + ((mode == nkFileReadMode_Text) ? 1 : 0));
    if(content->data)
    {
        success = NK_CAST(nkBool, ReadFile(file, content->data, NK_CAST(DWORD,bytes_to_read), NULL, NULL));
        if(success && (mode == nkFileReadMode_Text))
            NK_CAST(nkChar*,content->data)[bytes_to_read] = '\0';
    }

    CloseHandle(file);

    return success;
}

NKAPI nkBool nk_write_file_content(nkFileContent* content, const nkChar* file_name)
{
    NK_ASSERT(content);

    HANDLE file = CreateFileA(file_name, GENERIC_WRITE, 0,
        NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE) return NK_FALSE;

    nkBool success = NK_CAST(nkBool, WriteFile(file, content->data, NK_CAST(DWORD,content->size), NULL, NULL));

    CloseHandle(file);

    return success;
}

NKAPI void nk_free_file_content(nkFileContent* content)
{
    NK_ASSERT(content);
    NK_FREE(content->data);
    content->size = 0;
    content->data = NULL;
}

NKAPI nkChar* nk_get_exec_path(void)
{
    nkU64 path_size = 0;
    nkChar* path = nk__allocate_path_buffer(&path_size);
    if(!path) return NULL;

    GetModuleFileNameA(NULL, path, NK_CAST(DWORD,path_size));

    nkChar* clean_path = nk_fixup_path(path);
    NK_FREE(path);
    if(!clean_path) return NULL;

    nkChar* stripped_path = nk_strip_file_name(clean_path);
    NK_FREE(clean_path);
    if(!stripped_path) return NULL;

    return stripped_path;
}

NKAPI nkChar* nk_get_data_path(void)
{
    nkChar* path = NULL;

    // We use a temporary buffer because the function requires MAX_PATH.
    nkChar temp_buffer[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, temp_buffer)))
    {
        nkU64 end = strlen(temp_buffer);
        if(end+1 < MAX_PATH) // Add a trailing slash if we have space.
        {
            temp_buffer[end+0] = '/';
            temp_buffer[end+1] = '\0';
        }
        path = nk_fixup_path(temp_buffer);
    }

    return path;
}

NKAPI nkBool nk_list_path_content(const nkChar* path_name, nkPathListFlags flags, nkFileList* file_list)
{
    NK_ASSERT(file_list);

    nkChar* clean_path_name = nk_fixup_path(path_name);
    if(!clean_path_name) return NK_FALSE;

    nkChar* find_path_name = NK_MALLOC_TYPES(nkChar, strlen(clean_path_name)+2);
    if(!find_path_name) return NK_FALSE;
    strcpy(find_path_name, clean_path_name);
    nkU64 find_path_name_length = strlen(find_path_name);

    find_path_name[find_path_name_length-1] = '\\';
    find_path_name[find_path_name_length  ] = '*';
    find_path_name[find_path_name_length+1] = '\0';

    // Gets the first file/dir in the directory.
    WIN32_FIND_DATAA file_data = NK_ZERO_MEM;
    HANDLE find_file = FindFirstFileA(find_path_name, &file_data);

    if(find_file != INVALID_HANDLE_VALUE)
    {
        do
        {
            // Prevents listing self and parent dir in content.
            const nkChar* item_name = file_data.cFileName;
            if((strcmp(item_name, ".") != 0) && (strcmp(item_name, "..") != 0))
            {
                nkU64 item_length = strlen(clean_path_name) + strlen(item_name) + 1;
                nkChar* full_item_name = NK_MALLOC_TYPES(nkChar, item_length);
                if(!full_item_name)
                {
                    break; // Failed.
                }

                strcpy(full_item_name, clean_path_name);
                strcat(full_item_name, item_name);

                // Determine if the caller wants this item added to the list.
                nkBool should_add_item = NK_FALSE;
                if((nk_is_path(full_item_name) && NK_CHECK_FLAGS(flags, nkPathListFlags_Paths)) ||
                   (nk_is_file(full_item_name) && NK_CHECK_FLAGS(flags, nkPathListFlags_Files)))
                {
                    should_add_item = NK_TRUE;
                }
                if(should_add_item)
                {
                    // Make sure we have space and if not grow.
                    if(file_list->length >= file_list->size)
                    {
                        nkU64 new_size = ((file_list->size == 0) ? 32 : file_list->size * 2); // Double the size.
                        nkChar** temp_list = NK_RALLOC_TYPES(nkChar*, file_list->items, new_size);
                        if(!temp_list) break; // Failed!
                        file_list->size = new_size;
                        file_list->items = temp_list;
                    }

                    // Add the new item to the new list.
                    file_list->items[file_list->length++] = full_item_name;
                }

                // If the caller wants recursive results then explore.
                if(NK_CHECK_FLAGS(flags, nkPathListFlags_Recursive) && nk_is_path(full_item_name))
                {
                    nk_list_path_content(full_item_name, flags, file_list);
                }

                if(!should_add_item)
                {
                    NK_FREE(full_item_name);
                }
            }
        }
        while(FindNextFileA(find_file, &file_data));

        // Fixup all of the paths.
        for(nkU64 i=0; i<file_list->length; ++i)
        {
            nkChar* clean_path = nk_fixup_path(file_list->items[i]);
            NK_FREE(file_list->items[i]);
            file_list->items[i] = clean_path;
        }
    }

    FindClose(find_file);

    return NK_TRUE;
}

NKAPI void nk_free_path_content(nkFileList* file_list)
{
    NK_ASSERT(file_list);

    for(nkU64 i=0; i<file_list->length; ++i)
        NK_FREE(file_list->items[i]);
    NK_FREE(file_list->items);

    file_list->items = NULL;
    file_list->size = 0;
    file_list->length = 0;
}

NKAPI nkBool nk_create_file(const nkChar* file_name)
{
    HANDLE file = CreateFileA(file_name, GENERIC_READ, 0,
        NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    CloseHandle(file);
    return (file != INVALID_HANDLE_VALUE);
}

NKAPI nkBool nk_create_path(const nkChar* path_name)
{
    nkChar* buffer = nk_make_path_absolute(path_name);
    if(!buffer) return NK_FALSE;
    for(nkChar* c=buffer; *c; ++c) if(*c == '/') *c = '\\'; // We need backslashes for this function to work!
    nkBool success = (SHCreateDirectoryExA(NULL, buffer, NULL) == ERROR_SUCCESS);
    NK_FREE(buffer);
    return success;
}

NKAPI nkBool nk_delete_file(const nkChar* file_name)
{
    return NK_CAST(nkBool, DeleteFileA(file_name));
}

NKAPI nkBool nk_delete_path(const nkChar* path_name)
{
    SHFILEOPSTRUCTA fileop = NK_ZERO_MEM;
    fileop.hwnd = NULL;
    fileop.wFunc = FO_DELETE;
    fileop.pFrom = path_name;
    fileop.pTo = NULL;
    fileop.fFlags = FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SILENT;
    fileop.fAnyOperationsAborted = NK_FALSE;
    fileop.hNameMappings = 0;
    fileop.lpszProgressTitle = NULL;
    return (SHFileOperationA(&fileop) == 0);
}

NKAPI nkBool nk_rename_file(const nkChar* old_file, const nkChar* new_file)
{
    return NK_CAST(nkBool, MoveFileA(old_file, new_file)); // Renaming is just implemented as moving.
}

NKAPI nkBool nk_rename_path(const nkChar* old_path, const nkChar* new_path)
{
    return NK_CAST(nkBool, MoveFileA(old_path, new_path)); // Renaming is just implemented as moving.
}

NKAPI nkBool nk_copy_file(const nkChar* src_file, const nkChar* dst_file)
{
    return NK_CAST(nkBool, CopyFileA(src_file, dst_file, NK_FALSE));
}

NKAPI nkBool nk_move_file(const nkChar* src_file, const nkChar* dst_file)
{
    return NK_CAST(nkBool, MoveFileA(src_file, dst_file));
}

NKAPI nkU64 nk_file_size(const nkChar* file_name)
{
    HANDLE file = CreateFileA(file_name, GENERIC_READ, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE) return 0;
    nkU64 file_size = nk__get_file_size(file);
    CloseHandle(file);
    return file_size;
}

NKAPI nkBool nk_file_exists(const nkChar* file_name)
{
    DWORD attribs = GetFileAttributesA(file_name);
    return ((attribs != INVALID_FILE_ATTRIBUTES) &&
           !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}

NKAPI nkBool nk_path_exists(const nkChar* path_name)
{
    DWORD attribs = GetFileAttributesA(path_name);
    return ((attribs != INVALID_FILE_ATTRIBUTES) &&
            (attribs & FILE_ATTRIBUTE_DIRECTORY));
}

NKAPI nkBool nk_is_file(const nkChar* file_name)
{
    // This is the same as nk_file_exists but sometimes this makes more sense to show the intention.
    return nk_file_exists(file_name);
}

NKAPI nkBool nk_is_path(const nkChar* path_name)
{
    // This is the same as nk_path_exists but sometimes this makes more sense to show the intention.
    return nk_path_exists(path_name);
}

NKAPI nkBool nk_is_path_absolute(const nkChar* path_name)
{
    return !PathIsRelativeA(path_name);
}

NKAPI nkBool nk_is_path_relative(const nkChar* path_name)
{
    return NK_CAST(nkBool, PathIsRelativeA(path_name));
}

NKAPI nkChar* nk_strip_file_path(const nkChar* file_name)
{
    nkChar* name = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!name) return NULL;
    strcpy(name, PathFindFileNameA(file_name));
    return name;
}

NKAPI nkChar* nk_strip_file_ext(const nkChar* file_name)
{
    nkChar* name = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!name) return NULL;
    strcpy(name, file_name);
    PathFindExtensionA(name)[0] = '\0';
    return name;
}

NKAPI nkChar* nk_strip_file_name(const nkChar* file_name)
{
    nkChar* name = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!name) return NULL;
    strcpy(name, file_name);
    PathFindFileNameA(name)[0] = '\0';
    return name;
}

NKAPI nkChar* nk_strip_file_path_and_ext(const nkChar* file_name)
{
    nkChar* name = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!name) return NULL;
    strcpy(name, PathFindFileNameA(file_name));
    PathFindExtensionA(name)[0] = '\0';
    return name;
}

NKAPI nkChar* nk_strip_file_path_and_name(const nkChar* file_name)
{
    nkChar* name = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!name) return NULL;
    strcpy(name, PathFindExtensionA(file_name));
    return name;
}

NKAPI nkChar* nk_fixup_path(const nkChar* path_name)
{
    // Copy the path into a new buffer.
    nkChar* path = NK_CALLOC_TYPES(nkChar, strlen(path_name)+2); // +2 for null-terminator and potentially appending slash.
    if(!path) return NULL;
    strcpy(path, path_name);

    // Fix up the slashes inside of the path.
    for(nkChar* c=path; *c; ++c)
    {
        if(*c == '\\') *c = '/';
    }

    // Append slash if there isn't one and we're a path.
    if(!nk_is_file(path))
    {
        nkU64 path_length = strlen(path);
        if(path_length && path[path_length-1] != '/')
        {
            path[path_length  ] = '/';
            path[path_length+1] = '\0';
        }
    }

    return path;
}

NKAPI nkChar* nk_make_path_absolute(const nkChar* path_name)
{
    nkU64 path_size = 0;
    nkChar* path = nk__allocate_path_buffer(&path_size);
    if(!path) return NULL;

    GetFullPathNameA(path_name, NK_CAST(DWORD,path_size), path, NULL);

    nkChar* clean_path = nk_fixup_path(path);
    NK_FREE(path);
    if(!clean_path) return NULL;

    return clean_path;
}

#endif /* NK_OS_WIN32 ------------------------------------------------------- */

#if defined(NK_HAS_POSIX) /* ------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include <libgen.h>
#include <unistd.h>
#include <ftw.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#if defined(NK_OS_MACOS)
#include <libproc.h>
#endif // NK_OS_MACOS

NKINTERNAL nkChar* nk__allocate_path_buffer(nkU64* max_size)
{
    NK_ASSERT(max_size);
    nkChar* buffer = NK_MALLOC_TYPES(nkChar, PATH_MAX);
    (*max_size) = PATH_MAX;
    return buffer;
}

NKINTERNAL int nk__delete_path_callback(const nkChar* path_name, const struct stat* stat, int type_flag, struct FTW* ftwbuf)
{
    return remove(path_name);
}

NKAPI nkBool nk_read_file_content(nkFileContent* content, const nkChar* file_name, nkFileReadMode mode)
{
    NK_ASSERT(content);

    int file = open(file_name, O_RDONLY);
    if(file == -1) return NK_FALSE;

    nkBool success = NK_FALSE;

    nkU64 bytes_to_read = nk_file_size(file_name);

    content->size = bytes_to_read;
    content->data = NK_MALLOC_BYTES(content->size + ((mode == nkFileReadMode_Text) ? 1 : 0));
    if(content->data)
    {
        success = (read(file, content->data, bytes_to_read) != -1);
        if(success && (mode == nkFileReadMode_Text))
            NK_CAST(nkChar*,content->data)[bytes_to_read] = '\0';
    }

    close(file);

    return success;
}

NKAPI nkBool nk_write_file_content(nkFileContent* content, const nkChar* file_name)
{
    NK_ASSERT(content);

    int file = open(file_name, O_CREAT|O_WRONLY, S_IRWXU);
    if(file == -1) return NK_FALSE;

    nkBool success = (write(file, content->data, content->size) != -1);

    close(file);

    return success;
}

NKAPI void nk_free_file_content(nkFileContent* content)
{
    NK_ASSERT(content);
    NK_FREE(content->data);
    content->size = 0;
    content->data = NULL;
}

NKAPI nkChar* nk_get_exec_path(void)
{
    nkU64 path_size = 0;
    nkChar* path = nk__allocate_path_buffer(&path_size);
    if(!path) return NULL;

    memset(path, 0, path_size*sizeof(nkChar));

    // MacOS and Linux have slightly different ways of doing this, but the rest of the function is the same.
    #if defined(NK_OS_MACOS)
    proc_pidpath(getpid(), path, path_size);
    #endif // NK_OS_MACOS
    #if defined(NK_OS_LINUX)
    readlink("/proc/self/exe", path, path_size);
    #endif // NK_OS_LINUX

    nkChar* clean_path = nk_fixup_path(path);
    NK_FREE(path);
    if(!clean_path) return NULL;

    nkChar* stripped_path = nk_strip_file_name(clean_path);
    NK_FREE(clean_path);
    if(!stripped_path) return NULL;

    return stripped_path;
}

NKAPI nkChar* nk_get_data_path(void)
{
    return NULL; // @Incomplete: Do we implement this, does Linux do this?
}

NKAPI nkBool nk_list_path_content(const nkChar* path_name, nkPathListFlags flags, nkFileList* file_list)
{
    NK_ASSERT(file_list);

    nkChar* clean_path_name = nk_fixup_path(path_name);
    if(!clean_path_name) return NK_FALSE;

    DIR* dir = opendir(path_name);
    if(!dir) return NK_FALSE;

    struct dirent* entry;
    while((entry = readdir(dir)))
    {
        const nkChar* item_name = entry->d_name;
        if((strcmp(item_name, ".") == 0) || (strcmp(item_name, "..") == 0))
        {
            continue; // Prevents listing self and parent dir in content.
        }

        nkU64 item_length = strlen(clean_path_name) + strlen(item_name) + 1;
        nkChar* full_item_name = NK_MALLOC_TYPES(nkChar, item_length);
        if(!full_item_name)
        {
            break; // Failed.
        }

        strcpy(full_item_name, clean_path_name);
        strcat(full_item_name, item_name);

        // Determine if the caller wants this item added to the list.
        nkBool should_add_item = NK_FALSE;
        if((nk_is_path(full_item_name) && NK_CHECK_FLAGS(flags, nkPathListFlags_Paths)) ||
           (nk_is_file(full_item_name) && NK_CHECK_FLAGS(flags, nkPathListFlags_Files)))
        {
            should_add_item = NK_TRUE;
        }
        if(should_add_item)
        {
            // Make sure we have space and if not grow.
            if(file_list->length >= file_list->size)
            {
                nkU64 new_size = ((file_list->size == 0) ? 32 : file_list->size * 2); // Double the size.
                nkChar** temp_list = NK_RALLOC_TYPES(nkChar*, file_list->items, new_size);
                if(!temp_list) break; // Failed!
                file_list->size = new_size;
                file_list->items = temp_list;
            }

            // Add the new item to the new list.
            file_list->items[file_list->length++] = full_item_name;
        }

        // If the caller wants recursive results then explore.
        if(NK_CHECK_FLAGS(flags, nkPathListFlags_Recursive) && nk_is_path(full_item_name))
        {
            nk_list_path_content(full_item_name, flags, file_list);
        }

        if(!should_add_item)
        {
            NK_FREE(full_item_name);
        }
    }

    // Fixup all of the paths.
    for(nkU64 i=0; i<file_list->length; ++i)
    {
        nkChar* clean_path = nk_fixup_path(file_list->items[i]);
        NK_FREE(file_list->items[i]);
        file_list->items[i] = clean_path;
    }

    closedir(dir);

    return NK_TRUE;
}

NKAPI void nk_free_path_content(nkFileList* file_list)
{
    NK_ASSERT(file_list);

    for(nkU64 i=0; i<file_list->length; ++i)
        NK_FREE(file_list->items[i]);
    NK_FREE(file_list->items);

    file_list->items = NULL;
    file_list->size = 0;
    file_list->length = 0;
}

NKAPI nkBool nk_create_file(const nkChar* file_name)
{
    int file = open(file_name, O_CREAT|O_RDONLY);
    close(file);
    return (file != -1);
}

NKAPI nkBool nk_create_path(const nkChar* path_name)
{
    nkChar* clean_path = nk_fixup_path(path_name);
    if(!clean_path) return NK_FALSE;

    nkU64 temp_size = 0;
    nkChar* temp = nk__allocate_path_buffer(&temp_size);
    if(!temp) return NK_FALSE;

    nkBool success = NK_TRUE;

    const nkChar* p = clean_path;
    while((p = strchr(p, '/')) != NULL)
    {
        // Skip empty elements.
        if(p != clean_path && *(p-1) == '/')
        {
            p++;
            continue;
        }

        // Make single part of the path.
        memcpy(temp, clean_path, p-clean_path);
        temp[p-clean_path] = '\0';

        p++;

        if(mkdir(temp, 0774) != 0)
        {
            if(errno != EEXIST)
            {
                success = NK_FALSE;
                break;
            }
        }
    }

    NK_FREE(temp);

    return success;
}

NKAPI nkBool nk_delete_file(const nkChar* file_name)
{
    return (unlink(file_name) == 0);
}

NKAPI nkBool nk_delete_path(const nkChar* path_name)
{
    return (nftw(path_name, nk__delete_path_callback, 64, FTW_DEPTH | FTW_PHYS) == 0);
}

NKAPI nkBool nk_rename_file(const nkChar* old_file, const nkChar* new_file)
{
    return (rename(old_file, new_file) == 0); // Renaming is just implemented as moving.
}

NKAPI nkBool nk_rename_path(const nkChar* old_path, const nkChar* new_path)
{
    return (rename(old_path, new_path) == 0); // Renaming is just implemented as moving.
}

NKAPI nkBool nk_copy_file(const nkChar* src_file, const nkChar* dst_file)
{
    // No copy file function on Linux so we just manually do it.
    nkFileContent fc = NK_ZERO_MEM;

    nkBool success = NK_TRUE;

    success &= nk_read_file_content(&fc, src_file, nkFileReadMode_Binary);
    success &= nk_write_file_content(&fc, dst_file);

    nk_free_file_content(&fc);

    return success;
}

NKAPI nkBool nk_move_file(const nkChar* src_file, const nkChar* dst_file)
{
    return (rename(src_file, dst_file) == 0); // Renaming is just implemented as moving.
}

NKAPI nkU64 nk_file_size(const nkChar* file_name)
{
    struct stat s;
    if(stat(file_name, &s) != 0)
        return NK_FALSE;
    return s.st_size;
}

NKAPI nkBool nk_file_exists(const nkChar* file_name)
{
    struct stat s;
    if(stat(file_name, &s) != 0)
        return NK_FALSE;
    return ((s.st_mode & S_IFREG) != 0);
}

NKAPI nkBool nk_path_exists(const nkChar* path_name)
{
    struct stat s;
    if(stat(path_name, &s) != 0)
        return NK_FALSE;
    return ((s.st_mode & S_IFDIR) != 0);
}

NKAPI nkBool nk_is_file(const nkChar* file_name)
{
    // This is the same as nk_file_exists but sometimes this makes more sense to show the intention.
    return nk_file_exists(file_name);
}

NKAPI nkBool nk_is_path(const nkChar* path_name)
{
    // This is the same as nk_path_exists but sometimes this makes more sense to show the intention.
    return nk_path_exists(path_name);
}

NKAPI nkBool nk_is_path_absolute(const nkChar* path_name)
{
    return (path_name && path_name[0] == '/');
}

NKAPI nkBool nk_is_path_relative(const nkChar* path_name)
{
    return (path_name && path_name[0] != '/');
}

NKAPI nkChar* nk_strip_file_path(const nkChar* file_name)
{
    nkChar* input = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!input) return NULL;
    strcpy(input, file_name);

    nkChar* temp = basename(input);
    if(!temp) return NULL;

    nkChar* output = NK_CALLOC_TYPES(nkChar, strlen(temp)+1);
    if(!output) return NULL;
    strcpy(output, temp);

    NK_FREE(input);

    return output;
}

NKAPI nkChar* nk_strip_file_ext(const nkChar* file_name)
{
    nkChar* name = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!name) return NULL;
    strcpy(name, file_name);

    nkChar* dot = strrchr(name, '.');
    if(dot) *dot = '\0';

    return name;
}

NKAPI nkChar* nk_strip_file_name(const nkChar* file_name)
{
    nkChar* input = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!input) return NULL;
    strcpy(input, file_name);

    nkChar* temp = dirname(input);
    if(!temp) return NULL;

    nkChar* output = NK_CALLOC_TYPES(nkChar, strlen(temp)+1);
    if(!output) return NULL;
    strcpy(output, temp);

    nkChar* cleaned_path = nk_fixup_path(output);
    if(!cleaned_path) return NULL;

    NK_FREE(input);
    NK_FREE(output);

    return cleaned_path;
}

NKAPI nkChar* nk_strip_file_path_and_ext(const nkChar* file_name)
{
    nkChar* stripped_path = nk_strip_file_path(file_name);
    nkChar* stripped_both = nk_strip_file_ext(stripped_path);

    NK_FREE(stripped_path);

    return stripped_both;
}

NKAPI nkChar* nk_strip_file_path_and_name(const nkChar* file_name)
{
    nkChar* name = NK_CALLOC_TYPES(nkChar, strlen(file_name)+1);
    if(!name) return NULL;

    const nkChar* dot = strrchr(file_name, '.');
    if(dot) strcpy(name, dot);

    return name;
}

NKAPI nkChar* nk_fixup_path(const nkChar* path_name)
{
    // Copy the path into a new buffer.
    nkChar* path = NK_CALLOC_TYPES(nkChar, strlen(path_name)+2); // +2 for null-terminator and potentially appending slash.
    if(!path) return NULL;
    strcpy(path, path_name);

    // Fix up the slashes inside of the path.
    for(nkChar* c=path; *c; ++c)
    {
        if(*c == '\\') *c = '/';
    }

    // Append slash if there isn't one and we're a path.
    if(!nk_is_file(path))
    {
        nkU64 path_length = strlen(path);
        if(path_length && path[path_length-1] != '/')
        {
            path[path_length  ] = '/';
            path[path_length+1] = '\0';
        }
    }

    return path;
}

NKAPI nkChar* nk_make_path_absolute(const nkChar* path_name)
{
    nkU64 path_size = 0;
    nkChar* path = nk__allocate_path_buffer(&path_size);
    if(!path) return NULL;

    realpath(path_name, path);

    nkChar* clean_path = nk_fixup_path(path);
    NK_FREE(path);
    if(!clean_path) return NULL;

    return clean_path;
}

#endif /* NK_HAS_POSIX ------------------------------------------------------ */

#if defined(NK_OS_WEB) /* --------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NKINTERNAL nkU64 nk__get_file_size(FILE* file)
{
    fseek(file, 0L, SEEK_END);
    nkU64 file_size = ftell(file);
    rewind(file);
    return file_size;
}

NKAPI nkBool nk_read_file_content(nkFileContent* content, const nkChar* file_name, nkFileReadMode mode)
{
    NK_ASSERT(content);

    FILE* file = fopen(file_name, "rb");
    if(!file) return NK_FALSE;

    nkBool success = NK_FALSE;

    nkU64 bytes_to_read = nk__get_file_size(file);

    content->size = bytes_to_read;
    content->data = NK_CALLOC_BYTES(content->size + ((mode == nkFileReadMode_Text) ? 1 : 0));
    if(content->data)
    {
        success = (fread(content->data, bytes_to_read, 1, file) == 1);
        if(success && (mode == nkFileReadMode_Text))
            NK_CAST(nkChar*,content->data)[bytes_to_read] = '\0';
    }

    fclose(file);

    return success;
}

NKAPI nkBool nk_write_file_content(nkFileContent* content, const nkChar* file_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI void nk_free_file_content(nkFileContent* content)
{
    NK_ASSERT(content);
    NK_FREE(content->data);
    content->size = 0;
    content->data = NULL;
}

NKAPI nkChar* nk_get_exec_path(void)
{
    // @Todo: ...
    return NULL;
}

NKAPI nkChar* nk_get_data_path(void)
{
    // @Todo: ...
    return NULL;
}

NKAPI nkBool nk_list_path_content(const nkChar* path_name, nkPathListFlags flags, nkFileList* file_list)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI void nk_free_path_content(nkFileList* file_list)
{
    // @Todo: ...
}

NKAPI nkBool nk_create_file(const nkChar* file_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_create_path(const nkChar* path_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_delete_file(const nkChar* file_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_delete_path(const nkChar* path_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_rename_file(const nkChar* old_file, const nkChar* new_file)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_rename_path(const nkChar* old_path, const nkChar* new_path)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_copy_file(const nkChar* src_file, const nkChar* dst_file)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_move_file(const nkChar* src_file, const nkChar* dst_file)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkU64 nk_file_size(const nkChar* file_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_file_exists(const nkChar* file_name)
{
    // @Todo: This is a poor way of checking a file exists...
    FILE* f = fopen(file_name, "r");
    nkBool exists = (f != NULL);
    if(f) fclose(f);
    return exists;
}

NKAPI nkBool nk_path_exists(const nkChar* path_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_is_file(const nkChar* file_name)
{
    // This is the same as nk_file_exists but sometimes this makes more sense to show the intention.
    return nk_file_exists(file_name);
}

NKAPI nkBool nk_is_path(const nkChar* path_name)
{
    // This is the same as nk_path_exists but sometimes this makes more sense to show the intention.
    return nk_path_exists(path_name);
}

NKAPI nkBool nk_is_path_absolute(const nkChar* path_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkBool nk_is_path_relative(const nkChar* path_name)
{
    // @Todo: ...
    return NK_FALSE;
}

NKAPI nkChar* nk_strip_file_path(const nkChar* file_name)
{
    // @Todo: ...
    return NULL;
}

NKAPI nkChar* nk_strip_file_ext(const nkChar* file_name)
{
    // @Todo: ...
    return NULL;
}

NKAPI nkChar* nk_strip_file_name(const nkChar* file_name)
{
    // @Todo: ...
    return NULL;
}

NKAPI nkChar* nk_strip_file_path_and_ext(const nkChar* file_name)
{
    // @Todo: ...
    return NULL;
}

NKAPI nkChar* nk_strip_file_path_and_name(const nkChar* file_name)
{
    // @Todo: ...
    return NULL;
}

NKAPI nkChar* nk_fixup_path(const nkChar* path_name)
{
    // Copy the path into a new buffer.
    nkChar* path = NK_CALLOC_TYPES(nkChar, strlen(path_name)+2); // +2 for null-terminator and potentially appending slash.
    if(!path) return NULL;
    strcpy(path, path_name);

    // Fix up the slashes inside of the path.
    for(nkChar* c=path; *c; ++c)
    {
        if(*c == '\\') *c = '/';
    }

    // Append slash if there isn't one and we're a path.
    if(!nk_is_file(path))
    {
        nkU64 path_length = strlen(path);
        if(path_length && path[path_length-1] != '/')
        {
            path[path_length  ] = '/';
            path[path_length+1] = '\0';
        }
    }

    return path;
}

NKAPI nkChar* nk_make_path_absolute(const nkChar* path_name)
{
    // @Todo: ...
    return NULL;
}

#endif /* NK_OS_WEB --------------------------------------------------------- */

#endif /* NK_FILESYS_IMPLEMENTATION //////////////////////////////////////////*/

#endif /* NK_FILESYS_H__ /////////////////////////////////////////////////////*/

/*******************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
