/*////////////////////////////////////////////////////////////////////////////*/

#define NK_FILESYS_IMPLEMENTATION

#define NK_STATIC

#include <nk_define.h>
#include <nk_filesys.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

NKINTERNAL const nkChar* VALID_EXTENSIONS[] =
{
    ".h", ".hh", ".hpp", ".hxx", ".h++", ".c", ".cc", ".cpp", ".cxx", ".c++", ".cp", ".inl", ".ipp", ".hint"
};

NKINTERNAL const nkChar* HELP_TEXT =
"A utility for printing the number of code lines for C and C++ source files.\n"
"\n"
"linecount [options] <files/paths>\n"
"\n"
"options:\n"
"\n"
"  -d             :: print detailed line count info for each file supplied\n"
"  -r             :: directories will be searched recursively for files\n"
"  -e:<file/path> :: excludes the specified file/directory from the line count\n";

NKINTERNAL nkS32 g_total_code_count;
NKINTERNAL nkS32 g_total_comment_count;
NKINTERNAL nkS32 g_total_blank_count;
NKINTERNAL nkS32 g_total_line_count;

NKINTERNAL nkFileList g_excludes;

NKINTERNAL nkU64 find_first_in_string(const nkChar* str, nkChar c)
{
    for(nkS32 i=0,n=strlen(str); i<n; ++i)
        if(str[i] == c)
            return i;
    return NK_U64_MAX;
}

NKINTERNAL nkChar* strip_leading_whitespace(nkChar* str)
{
    while(isspace(*str)) ++str;
    return str;
}

NKINTERNAL nkBool is_item_excluded(const nkChar* item_name)
{
    // Check if the item is in the exclude list or not.
    for(nkU64 i=0; i<g_excludes.length; ++i)
    {
        if(strcmp(item_name, g_excludes.items[i]) == 0)
            return NK_TRUE;
    }

    // If the item is a file make sure it has a valid C/C++ extension.
    // This prevents us from attempting to load random files as code.
    if(nk_is_file(item_name))
    {
        nkChar* ext = nk_strip_file_path_and_name(item_name);
        for(nkS32 i=0,n=NK_ARRAY_SIZE(VALID_EXTENSIONS); i<n; ++i)
            if(strcmp(ext, VALID_EXTENSIONS[i]) == 0) // @Todo: Make this a case-insensitive compare?
                return NK_FALSE;
        NK_FREE(ext);
    }

    return NK_TRUE;
}

NKINTERNAL void exclude_item(const nkChar* item_name)
{
    const nkChar* item_to_exclude = item_name + (find_first_in_string(item_name, ':') + 1);

    // Grow the list if we don't have space for another item.
    if(g_excludes.length >= g_excludes.size)
    {
        nkU64 new_size = ((g_excludes.size == 0) ? 32 : g_excludes.size * 2); // Double the size.
        nkChar** temp_list = NK_RALLOC_TYPES(nkChar*, g_excludes.items, new_size);
        if(!temp_list)
        {
            printf("[nkloc]: ERROR: failed to grow exclude list for: %s\n", item_to_exclude);
            return;
        }

        g_excludes.size = new_size;
        g_excludes.items = temp_list;
    }

    // Add the item to the exclude list.
    nkChar* file_name = NK_MALLOC_TYPES(nkChar, strlen(item_to_exclude)+1);
    if(!file_name)
    {
        printf("[nkloc]: ERROR: failed to allocate buffer for exclude: %s\n", item_to_exclude);
        return;
    }
    strcpy(file_name, item_to_exclude);
    g_excludes.items[g_excludes.length++] = file_name;
}

NKINTERNAL nkBool is_line_code(nkChar* line, nkBool* in_block_comment)
{
    nkBool line_is_code = NK_FALSE;

    nkS32 line_len = strlen(line);
    for(nkS32 i=0; i<line_len; ++i)
    {
        // Determine block comment state.
        nkBool state_change = NK_FALSE;
        if(line[i] == '/' && line[i+1] == '*')
        {
            if(!(*in_block_comment))
            {
                *in_block_comment = NK_TRUE;
                state_change = NK_TRUE;
                i += 2;
            }
        }
        else if(line[i] == '*' && line[i+1] == '/')
        {
            if(*in_block_comment)
            {
                *in_block_comment = NK_FALSE;
                state_change = NK_TRUE;
                i += 2;
            }
        }

        // If outside of the line bounds then leave.
        if(i >= line_len) break;

        // If there is a non white-space character outside of a block comment
        // at any point in the line, then there is code present and the line
        // is deemed a code line instead of a comment / block-comment line.
        if(!(*in_block_comment) && !isspace(line[i]))
        {
            line_is_code = NK_TRUE;
        }

        // If state was changed move back one afterwards to avoid stupid bugs.
        if(state_change) --i;
    }

    return line_is_code;
}

NKINTERNAL void count_loc_for_file(const nkChar* file_name, nkS32 file_name_padding, nkBool print_detailed)
{
    nkS32 code_count    = 0;
    nkS32 comment_count = 0;
    nkS32 blank_count   = 0;
    nkS32 line_count    = 0;

    nkBool in_block_comment = NK_FALSE;

    FILE* file = fopen(file_name, "r");
    if(!file)
    {
        printf("[nkloc]: ERROR: failed to open: %s!\n", file_name);
        return;
    }

    nkChar raw_line[4096] = NK_ZERO_MEM; // @Todo: Make this work for any length line...
    while(fgets(raw_line, sizeof(raw_line), file))
    {
        // Strip leading space from the line for easier parsing.
        nkChar* line = strip_leading_whitespace(raw_line);
        nkS32 line_len = strlen(line);

        if(in_block_comment)
        {
            // If in a block comment scan the line for closing statement.
            nkS32 block_comment_exit_pos = 0;
            for(nkS32 j=0; j<line_len-1; ++j)
            {
                if(line[j] == '*' && line[j+1] == '/')
                {
                    in_block_comment = NK_FALSE;
                    block_comment_exit_pos = j+2;
                    break;
                }
            }
            // If still in the block comment then add a line.
            if(in_block_comment)
            {
                ++comment_count;
            }
            else
            {
                // Otherwise parse the line normally.
                if(block_comment_exit_pos < line_len)
                {
                    nkChar* sub_line = line+block_comment_exit_pos;
                    if(is_line_code(sub_line, &in_block_comment))
                    {
                        ++code_count;
                        continue;
                    }
                }
                ++comment_count;
            }
        }
        else
        {
            // Check what type of line the current line is
            if(line[0] == '\0')
            {
                ++blank_count;
            }
            else if(line[0] == '/' && line[1] == '/')
            {
                ++comment_count;
            }
            else
            {
                if(is_line_code(line, &in_block_comment))
                {
                    ++code_count;
                }
                else
                {
                    ++comment_count;
                }
            }
        }
    }

    line_count = code_count + comment_count + blank_count;

    // Add these numbers to the total for all files.
    g_total_code_count    += code_count;
    g_total_comment_count += comment_count;
    g_total_blank_count   += blank_count;
    g_total_line_count    += line_count;

    // print out the information for the file if detailed is switched on.
    if(print_detailed)
    {
        printf("| %-*s | %10d | %10d | %10d | %10d |\n", file_name_padding,
            file_name, code_count, comment_count, blank_count, line_count);
    }

    fclose(file);
}

int main(int argc, char** argv)
{
    nkBool print_detailed   = NK_FALSE;
    nkBool recursive_search = NK_FALSE;

    //
    // Check what command line options were passed in. We expect al of our user
    // options to be passed in before any actual files/folders to be searched.
    //

    nkS32 start_arg;
    for(start_arg=1; start_arg<argc; ++start_arg)
    {
        if(argv[start_arg][0] == '-')
        {
            switch(tolower(argv[start_arg][1])) // Case-insensitive comparison.
            {
                case '?': printf("%s", HELP_TEXT); return 0;
                case 'h': printf("%s", HELP_TEXT); return 0;
                case 'd': print_detailed = NK_TRUE; break;
                case 'r': recursive_search = NK_TRUE; break;
                case 'e': exclude_item(argv[start_arg]); break;
            }
        }
        else
        {
            break; // No more options to parse.
        }
    }

    if((argc-start_arg) < 1)
    {
        printf("[nkloc]: ERROR: requires at least one file/dir\n");
        return 1;
    }

    //
    // Create a list of all the files to have their lines of code counted. This
    // list can include excluded items as they will be filtered out later on.
    //

    nkFileList file_list = NK_ZERO_MEM;
    for(nkS32 i=start_arg; i<argc; ++i)
    {
        if(nk_is_path(argv[i]))
        {
            nkPathListFlags flags = nkPathListFlags_Files | (nkPathListFlags_Recursive * recursive_search);
            if(!nk_list_path_content(argv[i], flags, &file_list))
            {
                printf("[nkloc]: ERROR: failed to read path: %s\n", argv[i]);
                return 1;
            }
        }
        else
        {
            // Grow the list if we don't have space for another item.
            if(file_list.length >= file_list.size)
            {
                nkU64 new_size = ((file_list.size == 0) ? 32 : file_list.size * 2); // Double the size.
                nkChar** temp_list = NK_RALLOC_TYPES(nkChar*, file_list.items, new_size);
                if(!temp_list)
                {
                    printf("[nkloc]: ERROR: failed to grow file list for: %s\n", argv[i]);
                    return 0;
                }

                file_list.size = new_size;
                file_list.items = temp_list;
            }

            // Add the file to the new list.
            nkChar* file_name = NK_MALLOC_TYPES(nkChar, strlen(argv[i])+1);
            if(!file_name)
            {
                printf("[nkloc]: ERROR: failed to allocate buffer for: %s\n", argv[i]);
                return 1;
            }
            strcpy(file_name, argv[i]);
            file_list.items[file_list.length++] = file_name;
        }
    }

    //
    // Determine the longest file name in order to caulcate the padding. Only
    // bother with this if doing detailed printing as that includes filenames.
    //

    nkS32 file_name_padding = 5; // Minimum amount of padding...
    if(print_detailed)
    {
        for(nkU64 i=0; i<file_list.length; ++i)
        {
            if(is_item_excluded(file_list.items[i])) continue;
            nkS32 file_name_length = strlen(file_list.items[i]);
            if(file_name_length > file_name_padding)
            {
                file_name_padding = file_name_length;
            }
        }
    }

    //
    // Count the lines of code for each file and print the final results.
    //

    printf("| %-*s | %10s | %10s | %10s | %10s |\n", file_name_padding, "FILE", "CODE", "COMMENTS", "BLANK", "TOTAL");

    for(nkU64 i=0; i<file_list.length; ++i)
    {
        if(is_item_excluded(file_list.items[i])) continue;
        count_loc_for_file(file_list.items[i], file_name_padding, print_detailed);
    }

    printf("| %-*s | %10d | %10d | %10d | %10d |\n", file_name_padding, "TOTAL",
        g_total_code_count, g_total_comment_count, g_total_blank_count, g_total_line_count);

    nk_free_path_content(&file_list);

    return 0;
}

/*////////////////////////////////////////////////////////////////////////////*/

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
