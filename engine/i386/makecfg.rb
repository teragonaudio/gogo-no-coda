#!/usr/bin/ruby

require 'tempfile'
require 'htmlsplit'

class TagArray < Array
  def get_tag_blocks(tag_name)
    result = Array.new
    cursor = 0
    while cursor < size do
      e = at(cursor)
      if e.instance_of?(StartTag) and e.name == tag_name then
        result << TagArray.new
        cursor = cursor+1
        while cursor < size do
          e = at(cursor)
          if e.instance_of?(EndTag) and e.name == tag_name then
            cursor = cursor+1
            break
          end
          result.last << e
          cursor = cursor+1
        end
      else
        cursor = cursor+1
      end
    end
    result
  end
  
  def get_first_text(tag_name)
    next_is_result = false
    each { |e|
      if next_is_result 
        return e
      end
      if e.instance_of?(StartTag) and e.name == tag_name
        next_is_result = true
      end
    }
    nil
  end
end

class TagFile
  def initialize(name)
    @tags = TagArray.new
    HTMLSplit.new(name).document.each { |e|
      @tags << e
    }
  end
  
  def compounds
    @tags.get_tag_blocks("compound")
  end
end

def fit_separator(path)
  if File::ALT_SEPARATOR
    path.gsub('/',  File::ALT_SEPARATOR)
  else
    path.clone
  end
end

current_path = fit_separator(File::expand_path('./'))

tag_file = Tempfile.new("makecfg", ".")
tag_file.close

doxygen_parameter_file = Tempfile.new("makecfg", ".")
doxygen_parameter_file.print <<DOXYGENPARAMETERS
PROJECT_NAME           = makecfg
PROJECT_NUMBER         = 
OUTPUT_DIRECTORY       = 
OUTPUT_LANGUAGE        = English
EXTRACT_ALL            = NO
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = NO
HIDE_UNDOC_MEMBERS     = NO
HIDE_UNDOC_CLASSES     = NO
BRIEF_MEMBER_DESC      = YES
REPEAT_BRIEF           = YES
ALWAYS_DETAILED_SEC    = NO
FULL_PATH_NAMES        = NO
STRIP_FROM_PATH        = 
INTERNAL_DOCS          = NO
CLASS_DIAGRAMS         = YES
SOURCE_BROWSER         = NO
INLINE_SOURCES         = NO
STRIP_CODE_COMMENTS    = YES
CASE_SENSE_NAMES       = YES
SHORT_NAMES            = NO
HIDE_SCOPE_NAMES       = NO
VERBATIM_HEADERS       = YES
SHOW_INCLUDE_FILES     = YES
JAVADOC_AUTOBRIEF      = NO
INHERIT_DOCS           = YES
INLINE_INFO            = YES
SORT_MEMBER_DOCS       = YES
DISTRIBUTE_GROUP_DOC   = NO
TAB_SIZE               = 8
ENABLED_SECTIONS       = 
GENERATE_TODOLIST      = NO
GENERATE_TESTLIST      = NO
GENERATE_BUGLIST       = NO
ALIASES                = 
MAX_INITIALIZER_LINES  = 30
OPTIMIZE_OUTPUT_FOR_C  = NO
SHOW_USED_FILES        = YES
QUIET                  = NO
WARNINGS               = YES
WARN_IF_UNDOCUMENTED   = YES
WARN_FORMAT            = 
WARN_LOGFILE           = 
INPUT                  = #{fit_separator(current_path+'/../')}
FILE_PATTERNS          = global.h thread.h l3side.h quantize_pvt.h tables.h vbrtag.h
RECURSIVE              = NO
EXCLUDE                = 
EXCLUDE_PATTERNS       = 
EXAMPLE_PATH           = 
EXAMPLE_PATTERNS       = 
IMAGE_PATH             = 
INPUT_FILTER           = 
FILTER_SOURCE_FILES    = NO
ALPHABETICAL_INDEX     = NO
COLS_IN_ALPHA_INDEX    = 5
IGNORE_PREFIX          = 
GENERATE_HTML          = NO
HTML_OUTPUT            = 
HTML_HEADER            = 
HTML_FOOTER            = 
HTML_STYLESHEET        = 
HTML_ALIGN_MEMBERS     = NO
GENERATE_HTMLHELP      = NO
GENERATE_CHI           = NO
BINARY_TOC             = NO
TOC_EXPAND             = NO
DISABLE_INDEX          = NO
ENUM_VALUES_PER_LINE   = 4
GENERATE_TREEVIEW      = NO
TREEVIEW_WIDTH         = 250
GENERATE_LATEX         = NO
LATEX_OUTPUT           = 
COMPACT_LATEX          = NO
PAPER_TYPE             = a4wide
EXTRA_PACKAGES         = 
LATEX_HEADER           = 
PDF_HYPERLINKS         = NO
USE_PDFLATEX           = NO
LATEX_BATCHMODE        = NO
GENERATE_RTF           = NO
RTF_OUTPUT             = 
COMPACT_RTF            = NO
RTF_HYPERLINKS         = NO
RTF_STYLESHEET_FILE    = 
RTF_EXTENSIONS_FILE    = 
GENERATE_MAN           = NO
MAN_OUTPUT             = 
MAN_EXTENSION          = 
MAN_LINKS              = NO
GENERATE_XML           = NO
ENABLE_PREPROCESSING   = YES
MACRO_EXPANSION        = NO
EXPAND_ONLY_PREDEF     = NO
SEARCH_INCLUDES        = YES
INCLUDE_PATH           = 
INCLUDE_FILE_PATTERNS  = 
PREDEFINED             = 
EXPAND_AS_DEFINED      = 
TAGFILES               = 
GENERATE_TAGFILE       = #{fit_separator(tag_file.path)}
ALLEXTERNALS           = NO
PERL_PATH              = 
HAVE_DOT               = NO
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
INCLUDE_GRAPH          = YES
INCLUDED_BY_GRAPH      = YES
GRAPHICAL_HIERARCHY    = YES
DOT_PATH               = 
MAX_DOT_GRAPH_WIDTH    = 1024
MAX_DOT_GRAPH_HEIGHT   = 1024
GENERATE_LEGEND        = NO
DOT_CLEANUP            = YES
SEARCHENGINE           = NO
CGI_NAME               = 
CGI_URL                = 
DOC_URL                = 
DOC_ABSPATH            = 
BIN_ABSPATH            = 
EXT_DOC_PATHS          = 
DOXYGENPARAMETERS
doxygen_parameter_file.close
raise 'cound not execute doxygen' unless system('doxygen', fit_separator(doxygen_parameter_file.path))

define_members = ''
define_offsets = ''
define_sizes   = ''
structs = TagFile.new(tag_file.open.read).compounds
structs.each { |s|
  name = s.get_first_text("name").to_s
  define_sizes += "\tputSize( #{name} );\n"
  members = s.get_tag_blocks("member")
  members.each { |m|
    member_name = m.get_first_text("name").to_s
    if name == "ro_t" 
      define_members += "\tputMem( RO, #{member_name} );\n"
    elsif name == "rw_t" 
      define_members += "\tputMem( RW, #{member_name} );\n"
    end
    define_offsets += "\tputOff( #{name}, #{member_name} );\n"
  }
}

File.new("makecfg.c", "w").puts <<MAKECFG_C_TEMPLATE
#define _GOGO_C_
#include <stddef.h>
#include "../common.h"
#include "../global.h"
#include "../thread.h"
#include "../l3side.h"
#include "../quantize_pvt.h"
#include "../tables.h"
#include "../vbrtag.h"

RW_t RW;
RO_t RO;

#if (defined(WIN32) || defined(__linux__) || defined(__os2__)) &&  !defined(__BORLANDC__) &&  !defined(__MINGW32_VERSION)
	#define USE_BSS
#endif

static void putMember( const char *base, const char *name, int offset )
{
	printf( "%%define %s.%s\\t\\t(%s+%d)\\n", base, name, base, offset );
}

static void putOffset( const char *base, const char *name, int offset )
{
	printf( "%%define %s.%s\\t\\t(%d)\\n", base, name, offset );
}

#define OFFSET( base, x ) ( (int)&base.x - (int)&base )

#ifndef offsetof
  #define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

#define putMem( base, x ) putMember( #base, #x, OFFSET( base, x ) )
#define putOff( base, x ) putOffset( #base, #x, offsetof( struct base, x ) )
#define putSize( base ) { printf( "%%define sizeof_%s\\t(%d)\\n", #base, sizeof( struct base ) ); }

int main( void )
{
	static const char nameTbl[][4] = { "RO", "RW", "" };
	static const int valSize[] = { sizeof(RO) / 4, (sizeof(RW)+127) / 4 };
	int i;

	puts( "\\t%include \\"nasm.cfg\\"" );

	puts( "%ifdef DEF_GLOBAL_VARS" );

	/* global mode */
	printf( "\\tglobaldef RO\\n" );
	printf( "\\tglobaldef RW\\n" );
#ifdef USE_BSS
	printf( "\\tsegment_bss\\n" );
	for( i = 0; i < 2; i++ ){
		printf( "\\talignb 32\\n" );
		printf( "%s\\tresd %d\\n", nameTbl[i], valSize[i] );
	}
#else	/* USE_BSS */
	printf( "\\tsegment_data\\n" );
	for( i = 0; i < 2; i++ ){
		printf( "\\talign 32\\n" );
		printf( "%s\\ttimes %d dd 0\\n", nameTbl[i], valSize[i] );
	}
#endif	/* USE_BSS */

	puts( "%else  ; DEF_GLOBAL_VARS" );

	/* extern mode */
	puts( "\\texterndef RO\\n" );
	puts( "\\texterndef RW\\n" );
	
#{define_members}

#{define_offsets}

#{define_sizes}

	puts( "%endif ; DEF_GLOBAL_VARS" );
	return 0;
}
MAKECFG_C_TEMPLATE
