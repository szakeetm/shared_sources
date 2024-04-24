cmake_minimum_required(VERSION 3.5)
project(lzma)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CYGWIN)
set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(MSVC)
set(SOURCE_DIR ".")
set(headers
        ${SOURCE_DIR}/7z.h
        ${SOURCE_DIR}/7zAlloc.h
        ${SOURCE_DIR}/7zBuf.h
        ${SOURCE_DIR}/7zCrc.h
        ${SOURCE_DIR}/7zFile.h
        ${SOURCE_DIR}/7zVersion.h
        ${SOURCE_DIR}/Alloc.h
        ${SOURCE_DIR}/Bcj2.h
        ${SOURCE_DIR}/Bra.h
        ${SOURCE_DIR}/CpuArch.h
        ${SOURCE_DIR}/Delta.h
        ${SOURCE_DIR}/LzFind.h
        ${SOURCE_DIR}/LzFindMt.h
        ${SOURCE_DIR}/LzHash.h
        ${SOURCE_DIR}/Lzma2Dec.h
        ${SOURCE_DIR}/Lzma2Enc.h
        ${SOURCE_DIR}/Lzma86.h
        ${SOURCE_DIR}/LzmaDec.h
        ${SOURCE_DIR}/LzmaEnc.h
        ${SOURCE_DIR}/LzmaLib.h
        ${SOURCE_DIR}/MtCoder.h
        ${SOURCE_DIR}/Ppmd.h
        ${SOURCE_DIR}/Ppmd7.h
        ${SOURCE_DIR}/RotateDefs.h
        ${SOURCE_DIR}/Sha256.h
        ${SOURCE_DIR}/Threads.h
        ${SOURCE_DIR}/Types.h
        ${SOURCE_DIR}/Xz.h
        ${SOURCE_DIR}/XzCrc64.h
        ${SOURCE_DIR}/XzEnc.h
        )

set(src
        ${SOURCE_DIR}/7zAlloc.c
        ${SOURCE_DIR}/7zBuf.c
        ${SOURCE_DIR}/7zBuf2.c
        ${SOURCE_DIR}/7zCrc.c
        ${SOURCE_DIR}/7zCrcOpt.c
        ${SOURCE_DIR}/7zDec.c
        ${SOURCE_DIR}/7zFile.c
        ${SOURCE_DIR}/7zIn.c
        ${SOURCE_DIR}/7zStream.c
        ${SOURCE_DIR}/Alloc.c
        ${SOURCE_DIR}/Bcj2.c
        ${SOURCE_DIR}/Bra.c
        ${SOURCE_DIR}/Bra86.c
        ${SOURCE_DIR}/BraIA64.c
        ${SOURCE_DIR}/CpuArch.c
        ${SOURCE_DIR}/Delta.c
        ${SOURCE_DIR}/LzFind.c
        ${SOURCE_DIR}/LzFindMt.c
        ${SOURCE_DIR}/Lzma2Dec.c
        ${SOURCE_DIR}/Lzma2Enc.c
        ${SOURCE_DIR}/Lzma86Dec.c
        ${SOURCE_DIR}/Lzma86Enc.c
        ${SOURCE_DIR}/LzmaDec.c
        ${SOURCE_DIR}/LzmaEnc.c
        ${SOURCE_DIR}/LzmaLib.c
        ${SOURCE_DIR}/MtCoder.c
        ${SOURCE_DIR}/Ppmd7.c
        ${SOURCE_DIR}/Ppmd7Dec.c
        ${SOURCE_DIR}/Ppmd7Enc.c
        ${SOURCE_DIR}/Sha256.c
        ${SOURCE_DIR}/Threads.c
        ${SOURCE_DIR}/Xz.c
        ${SOURCE_DIR}/XzCrc64.c
        ${SOURCE_DIR}/XzDec.c
        ${SOURCE_DIR}/XzEnc.c
        ${SOURCE_DIR}/XzIn.c
        )
else()
set(SOURCE_DIR "unix")
set(headers
        ${SOURCE_DIR}/7zBuf.h      ${SOURCE_DIR}/Bra.h        ${SOURCE_DIR}/LzFind.h     ${SOURCE_DIR}/LzmaDec.h    ${SOURCE_DIR}/Ppmd8.h      ${SOURCE_DIR}/Types.h
        ${SOURCE_DIR}/7zCrc.h      ${SOURCE_DIR}/BwtSort.h    ${SOURCE_DIR}/LzFindMt.h   ${SOURCE_DIR}/LzmaEnc.h    ${SOURCE_DIR}/RotateDefs.h ${SOURCE_DIR}/Xz.h
        ${SOURCE_DIR}/7zVersion.h  ${SOURCE_DIR}/CpuArch.h    ${SOURCE_DIR}/LzHash.h     ${SOURCE_DIR}/MtCoder.h    ${SOURCE_DIR}/Sha256.h     ${SOURCE_DIR}/XzCrc64.h
        ${SOURCE_DIR}/Aes.h        ${SOURCE_DIR}/Delta.h      ${SOURCE_DIR}/Lzma2Dec.h   ${SOURCE_DIR}/Ppmd.h       ${SOURCE_DIR}/Sort.h       ${SOURCE_DIR}/XzEnc.h
        ${SOURCE_DIR}/Alloc.h      ${SOURCE_DIR}/HuffEnc.h    ${SOURCE_DIR}/Lzma2Enc.h   ${SOURCE_DIR}/Ppmd7.h      ${SOURCE_DIR}/Threads.h
        )

set(src
        ${SOURCE_DIR}/7zBuf2.c   ${SOURCE_DIR}/Alloc.c    ${SOURCE_DIR}/CpuArch.c  ${SOURCE_DIR}/Lzma2Dec.c ${SOURCE_DIR}/Ppmd7.c    ${SOURCE_DIR}/Ppmd8Enc.c ${SOURCE_DIR}/XzCrc64.c
        ${SOURCE_DIR}/7zCrc.c    ${SOURCE_DIR}/Bra.c      ${SOURCE_DIR}/Delta.c    ${SOURCE_DIR}/Lzma2Enc.c ${SOURCE_DIR}/Ppmd7Dec.c ${SOURCE_DIR}/Sha256.c   ${SOURCE_DIR}/XzDec.c
        ${SOURCE_DIR}/7zCrcOpt.c ${SOURCE_DIR}/Bra86.c    ${SOURCE_DIR}/HuffEnc.c  ${SOURCE_DIR}/LzmaDec.c  ${SOURCE_DIR}/Ppmd7Enc.c ${SOURCE_DIR}/Sort.c     ${SOURCE_DIR}/XzEnc.c
        ${SOURCE_DIR}/7zStream.c ${SOURCE_DIR}/BraIA64.c  ${SOURCE_DIR}/LzFind.c   ${SOURCE_DIR}/LzmaEnc.c  ${SOURCE_DIR}/Ppmd8.c    ${SOURCE_DIR}/Threads.c  ${SOURCE_DIR}/XzIn.c
        ${SOURCE_DIR}/Aes.c      ${SOURCE_DIR}/BwtSort.c  ${SOURCE_DIR}/LzFindMt.c ${SOURCE_DIR}/MtCoder.c  ${SOURCE_DIR}/Ppmd8Dec.c ${SOURCE_DIR}/Xz.c

        )
endif()


set(sources
        ${headers}
        ${src}
        )

add_library(${PROJECT_NAME} STATIC ${sources})
if(MSVC)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS " /wd4996")
endif()