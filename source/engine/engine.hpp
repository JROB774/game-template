/*////////////////////////////////////////////////////////////////////////////*/

#define STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_STATIC
#define GLEW_STATIC
#define NK_STATIC

#define NK_SINGLE_UNIT

#include <nk_define.h>
#include <nk_math.h>
#include <nk_filesys.h>
#include <nk_npak.h>
#include <nk_array.h>
#include <nk_string.h>
#include <nk_hashmap.h>
#include <nk_stack.h>
#include <nk_defer.h>

#include <stb_image.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <wchar.h>

#include "utility.hpp"
#include "noise.hpp"
#include "collision.hpp"
#include "asset_manager.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "bitmap_font.hpp"
#include "truetype_font.hpp"
#include "animation.hpp"
#include "renderer.hpp"
#include "imm.hpp"
#include "post_process.hpp"
#include "debug_ui.hpp"
#include "assets.hpp"
#include "application.hpp"
#include "platform.hpp"

#include "utility.cpp"
#include "noise.cpp"
#include "collision.cpp"
#include "platform.cpp"
#include "asset_manager.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "bitmap_font.cpp"
#include "truetype_font.cpp"
#include "animation.cpp"
#include "renderer.cpp"
#include "imm.cpp"
#include "post_process.cpp"
#include "debug_ui.cpp"

/*////////////////////////////////////////////////////////////////////////////*/
