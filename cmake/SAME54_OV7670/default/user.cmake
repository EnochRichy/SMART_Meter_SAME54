# user.cmake — adds TinyEngine ML inference sources and include paths
#
# Included by CMakeLists.txt AFTER main.cmake, so the object-library
# targets already exist. We add our sources and include paths directly.

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../../src")

# ── TinyEngine include directories (needed by both C and C++ targets) ─
set(TINYENGINE_INCLUDES
    "${SRC}/TinyEngine/include"
    "${SRC}/TinyEngine/include/arm_cmsis"
    "${SRC}/TinyEngine/codegen/Include"
    "${SRC}/packs/CMSIS/CMSIS/NN/Include"
)

# ── Additional C sources ─────────────────────────────────────────────
set(TINYENGINE_C_SOURCES
    "${SRC}/img_preprocess.c"

    # codegen: model graph + depthwise kernels
    "${SRC}/TinyEngine/codegen/Source/genModel.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride1_inplace_CHW_fpreq.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride1_inplace_CHW_fpreq_bitmask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride1_inplace_CHW_fpreq_mask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride2_inplace_CHW_fpreq.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride2_inplace_CHW_fpreq_bitmask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride2_inplace_CHW_fpreq_mask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride1_inplace_CHW_fpreq.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride1_inplace_CHW_fpreq_bitmask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride1_inplace_CHW_fpreq_mask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride2_inplace_CHW_fpreq.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride2_inplace_CHW_fpreq_bitmask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride2_inplace_CHW_fpreq_mask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride1_inplace_CHW_fpreq.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride1_inplace_CHW_fpreq_bitmask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride1_inplace_CHW_fpreq_mask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride2_inplace_CHW_fpreq.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride2_inplace_CHW_fpreq_bitmask.c"
    "${SRC}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride2_inplace_CHW_fpreq_mask.c"

    # fp-requantize kernels
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.c"
    "${SRC}/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.c"

    # int forward-pass kernels
    "${SRC}/TinyEngine/src/kernels/int_forward_op/add.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/avgpooling.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/concat_ch.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/element_mult.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/fully_connected.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/maxpooling.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/stable_softmax.c"
    "${SRC}/TinyEngine/src/kernels/int_forward_op/upsample_byte.c"
)
set_source_files_properties(${TINYENGINE_C_SOURCES} PROPERTIES LANGUAGE C)

# ── Additional C++ sources ───────────────────────────────────────────
set(TINYENGINE_CPP_SOURCES
    "${SRC}/app_ml.cpp"
)
set_source_files_properties(${TINYENGINE_CPP_SOURCES} PROPERTIES LANGUAGE CXX)

# ── Add sources to existing targets ──────────────────────────────────
# The C object library was created by main.cmake if there were C sources.
if(TARGET SAME54_OV7670_default_default_XC32_compile)
    target_sources(SAME54_OV7670_default_default_XC32_compile PRIVATE ${TINYENGINE_C_SOURCES})
    target_include_directories(SAME54_OV7670_default_default_XC32_compile PRIVATE ${TINYENGINE_INCLUDES})
endif()

# The C++ object library may not exist yet if there were no prior cpp files.
if(TARGET SAME54_OV7670_default_default_XC32_compile_cpp)
    target_sources(SAME54_OV7670_default_default_XC32_compile_cpp PRIVATE ${TINYENGINE_CPP_SOURCES})
    target_include_directories(SAME54_OV7670_default_default_XC32_compile_cpp PRIVATE ${TINYENGINE_INCLUDES})
else()
    # Create the C++ object library for the first time
    add_library(SAME54_OV7670_default_default_XC32_compile_cpp OBJECT ${TINYENGINE_CPP_SOURCES})
    SAME54_OV7670_default_default_XC32_compile_cpp_rule(SAME54_OV7670_default_default_XC32_compile_cpp)
    target_include_directories(SAME54_OV7670_default_default_XC32_compile_cpp PRIVATE ${TINYENGINE_INCLUDES})
    # Link the C++ objects into the final executable
    target_sources(SAME54_OV7670_default_image_LWWzwC1h PRIVATE "$<TARGET_OBJECTS:SAME54_OV7670_default_default_XC32_compile_cpp>")
endif()
