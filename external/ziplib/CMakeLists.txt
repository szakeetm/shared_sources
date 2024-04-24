cmake_minimum_required(VERSION 3.5)
project(ziplib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CYGWIN)
set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(SOURCE_DIR "Source/ZipLib")
add_subdirectory(${SOURCE_DIR}/extlibs/bzip2)
add_subdirectory(${SOURCE_DIR}/extlibs/lzma)
add_subdirectory(${SOURCE_DIR}/extlibs/zlib)

set(headers_compression
        ${SOURCE_DIR}/compression/compression_interface.h
        )

set(headers_compression_bzip2
        ${SOURCE_DIR}/compression/bzip2/bzip2_decoder.h
        ${SOURCE_DIR}/compression/bzip2/bzip2_decoder_properties.h
        ${SOURCE_DIR}/compression/bzip2/bzip2_encoder.h
        ${SOURCE_DIR}/compression/bzip2/bzip2_encoder_properties.h
        )

set(headers_compression_deflate
        ${SOURCE_DIR}/compression/deflate/deflate_decoder.h
        ${SOURCE_DIR}/compression/deflate/deflate_decoder_properties.h
        ${SOURCE_DIR}/compression/deflate/deflate_encoder.h
        ${SOURCE_DIR}/compression/deflate/deflate_encoder_properties.h
        )

set(headers_compression_lzma
        ${SOURCE_DIR}/compression/lzma/lzma_decoder.h
        ${SOURCE_DIR}/compression/lzma/lzma_decoder_properties.h
        ${SOURCE_DIR}/compression/lzma/lzma_encoder.h
        ${SOURCE_DIR}/compression/lzma/lzma_encoder_properties.h
        )

set(headers_compression_lzma_detail
        ${SOURCE_DIR}/compression/lzma/detail/lzma_alloc.h
        ${SOURCE_DIR}/compression/lzma/detail/lzma_handle.h
        ${SOURCE_DIR}/compression/lzma/detail/lzma_header.h
        ${SOURCE_DIR}/compression/lzma/detail/lzma_in_stream.h
        ${SOURCE_DIR}/compression/lzma/detail/lzma_out_stream.h
        )

set(headers_compression_store
        ${SOURCE_DIR}/compression/store/store_decoder.h
        ${SOURCE_DIR}/compression/store/store_decoder_properties.h
        ${SOURCE_DIR}/compression/store/store_encoder.h
        ${SOURCE_DIR}/compression/store/store_encoder_properties.h
        )

set(headers_detail
        ${SOURCE_DIR}/detail/EndOfCentralDirectoryBlock.h
        ${SOURCE_DIR}/detail/ZipCentralDirectoryFileHeader.h
        ${SOURCE_DIR}/detail/ZipGenericExtraField.h
        ${SOURCE_DIR}/detail/ZipLocalFileHeader.h
        )

set(headers_methods
        ${SOURCE_DIR}/methods/Bzip2Method.h
        ${SOURCE_DIR}/methods/DeflateMethod.h
        ${SOURCE_DIR}/methods/ICompressionMethod.h
        ${SOURCE_DIR}/methods/LzmaMethod.h
        ${SOURCE_DIR}/methods/StoreMethod.h
        ${SOURCE_DIR}/methods/ZipMethodResolver.h
        )

set(headers_streams
        ${SOURCE_DIR}/streams/compression_decoder_stream.h
        ${SOURCE_DIR}/streams/compression_encoder_stream.h
        ${SOURCE_DIR}/streams/crc32stream.h
        ${SOURCE_DIR}/streams/memstream.h
        ${SOURCE_DIR}/streams/nullstream.h
        ${SOURCE_DIR}/streams/serialization.h
        ${SOURCE_DIR}/streams/substream.h
        ${SOURCE_DIR}/streams/teestream.h
        ${SOURCE_DIR}/streams/zip_cryptostream.h
        )

set(headers_streams_streambuffs
        ${SOURCE_DIR}/streams/streambuffs/compression_decoder_streambuf.h
        ${SOURCE_DIR}/streams/streambuffs/compression_encoder_streambuf.h
        ${SOURCE_DIR}/streams/streambuffs/crc32_streambuf.h
        ${SOURCE_DIR}/streams/streambuffs/mem_streambuf.h
        ${SOURCE_DIR}/streams/streambuffs/null_streambuf.h
        ${SOURCE_DIR}/streams/streambuffs/sub_streambuf.h
        ${SOURCE_DIR}/streams/streambuffs/tee_streambuff.h
        ${SOURCE_DIR}/streams/streambuffs/zip_crypto_streambuf.h
        )

set(headers_utils
        ${SOURCE_DIR}/utils/enum_utils.h
        ${SOURCE_DIR}/utils/stream_utils.h
        ${SOURCE_DIR}/utils/time_utils.h
        )

set(headers_root
        ${SOURCE_DIR}/ZipArchive.h
        ${SOURCE_DIR}/ZipArchiveEntry.h
        ${SOURCE_DIR}/ZipFile.h
        )


set(src_detail
        ${SOURCE_DIR}/detail/EndOfCentralDirectoryBlock.cpp
        ${SOURCE_DIR}/detail/ZipCentralDirectoryFileHeader.cpp
        ${SOURCE_DIR}/detail/ZipGenericExtraField.cpp
        ${SOURCE_DIR}/detail/ZipLocalFileHeader.cpp
        )

set(src_root
        ${SOURCE_DIR}/ZipArchive.cpp
        ${SOURCE_DIR}/ZipArchiveEntry.cpp
        ${SOURCE_DIR}/ZipFile.cpp
        )

set(headers_all
        ${headers_compression_bzip2}
        ${headers_compression_deflate}
        ${headers_compression_lzma}
        ${headers_compression_lzma_detail}
        ${headers_compression_store}
        ${headers_detail}
        ${headers_methods}
        ${headers_streams}
        ${headers_streams_streambuffs}
        ${headers_utils}
        ${headers_root}
        )

set(src_all
        ${src_detail}
        ${src_root}
        )

set(sources
        ${headers_all}
        ${src_all}
        )

source_group(headers\\compression					FILES ${headers_compression})
source_group(headers\\compression\\bzip2			FILES ${headers_compression_bzip2})
source_group(headers\\compression\\deflate			FILES ${headers_compression_deflate})
source_group(headers\\compression\\lzma				FILES ${headers_compression_lzma})
source_group(headers\\compression\\lzma_detail		FILES ${headers_compression_lzma_detail})
source_group(headers\\compression\\store			FILES ${headers_compression_store})
source_group(headers\\detail						FILES ${headers_detail})
source_group(headers\\methods						FILES ${headers_methods})
source_group(headers\\streams						FILES ${headers_streams})
source_group(headers\\streams\\streambuffs			FILES ${headers_streams_streambuffs})
source_group(headers\\utils							FILES ${headers_utils})
source_group(headers								FILES ${headers_root})
source_group(sources\\detail						FILES ${src_detail})
source_group(sources								FILES ${src_root})

add_library(${PROJECT_NAME}_inter INTERFACE)
target_sources(${PROJECT_NAME}_inter INTERFACE ${headers})
target_include_directories(${PROJECT_NAME}_inter SYSTEM INTERFACE Source)
add_library(${PROJECT_NAME} STATIC ${sources})
target_link_libraries(${PROJECT_NAME} PRIVATE
        bzip2
        lzma
        zlib
        ${PROJECT_NAME}_inter)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ziplib")
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS " /wd4996")
endif()