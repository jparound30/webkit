/*
 * Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided the following conditions
 * are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS CORPORATION AND ITS
 * CONTRIBUTORS "AS IS", AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG
 * ELECTRONICS CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS, OR BUSINESS INTERRUPTION), HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
 * NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebCLDeviceIDList_h
#define WebCLDeviceIDList_h

#include "WebCLDeviceID.h"
#include <Opencl/opencl.h>

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

	class WebCLComputeContext;

	class WebCLDeviceIDList : public RefCounted<WebCLDeviceIDList> {
		public:
			// TODO (siba samal) Remove if not needed 
			enum WebCL_state {
				FAILURE = -1,
				SUCCESS = 0,
				/* Error Codes */
				DEVICE_NOT_FOUND                         = -1,
				DEVICE_NOT_AVAILABLE                     = -2,
				COMPILER_NOT_AVAILABLE                   = -3,
				MEM_OBJECT_ALLOCATION_FAILURE            = -4,
				OUT_OF_RESOURCES                         = -5,
				OUT_OF_HOST_MEMORY                       = -6,
				PROFILING_INFO_NOT_AVAILABLE             = -7,
				MEM_COPY_OVERLAP                         = -8,
				IMAGE_FORMAT_MISMATCH                    = -9,
				IMAGE_FORMAT_NOT_SUPPORTED               = -10,
				BUILD_PROGRAM_FAILURE                    = -11,
				MAP_FAILURE                              = -12,
				MISALIGNED_SUB_BUFFER_OFFSET             = -13,
				EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST = -14,

				INVALID_VALUE                            = -30,
				INVALID_DEVICE_TYPE                      = -31,
				INVALID_PLATFORM                         = -32,
				INVALID_DEVICE                           = -33,
				INVALID_CONTEXT                          = -34,
				INVALID_QUEUE_PROPERTIES                 = -35,
				INVALID_COMMAND_QUEUE                    = -36,
				INVALID_HOST_PTR                         = -37,
				INVALID_MEM_OBJECT                       = -38,
				INVALID_IMAGE_FORMAT_DESCRIPTOR          = -39,
				INVALID_IMAGE_SIZE                       = -40,
				INVALID_SAMPLER                          = -41,
				INVALID_BINARY                           = -42,
				INVALID_BUILD_OPTIONS                    = -43,
				INVALID_PROGRAM                          = -44,
				INVALID_PROGRAM_EXECUTABLE               = -45,
				INVALID_KERNEL_NAME                      = -46,
				INVALID_KERNEL_DEFINITION                = -47,
				INVALID_KERNEL                           = -48,
				INVALID_ARG_INDEX                        = -49,
				INVALID_ARG_VALUE                        = -50,
				INVALID_ARG_SIZE                         = -51,
				INVALID_KERNEL_ARGS                      = -52,
				INVALID_WORK_DIMENSION                   = -53,
				INVALID_WORK_GROUP_SIZE                  = -54,
				INVALID_WORK_ITEM_SIZE                   = -55,
				INVALID_GLOBAL_OFFSET                    = -56,
				INVALID_EVENT_WAIT_LIST                  = -57,
				INVALID_EVENT                            = -58,
				INVALID_OPERATION                        = -59,
				INVALID_GL_OBJECT                        = -60,
				INVALID_BUFFER_SIZE                      = -61,
				INVALID_MIP_LEVEL                        = -62,
				INVALID_GLOBAL_WORK_SIZE                 = -63,
				INVALID_PROPERTY                         = -64,

				/* OpenCL Version */
				VERSION_1_0                              = 1,
				VERSION_1_1                              = 1,  // sg: ??

				/* cl_bool */
				// FALSE                                    = 0,
				// TRUE                                     = 1,

				/* cl_platform_info */
				PLATFORM_PROFILE                         = 0x0900,
				PLATFORM_VERSION                         = 0x0901,
				PLATFORM_NAME                            = 0x0902,
				PLATFORM_VENDOR                          = 0x0903,
				PLATFORM_EXTENSIONS                      = 0x0904,

				/* cl_device_type - bitfield */
				DEVICE_TYPE_DEFAULT                      = 0x1, // (1 << 0),
				DEVICE_TYPE_CPU                          = 0x2, // (1 << 1),
				DEVICE_TYPE_GPU                          = 0x4, // (1 << 2),
				DEVICE_TYPE_ACCELERATOR                  = 0x8, // (1 << 3),
				DEVICE_TYPE_ALL                          = 0xFFFFFFFF,

				/* cl_device_info */
				DEVICE_TYPE                              = 0x1000,
				DEVICE_VENDOR_ID                         = 0x1001,
				DEVICE_MAX_COMPUTE_UNITS                 = 0x1002,
				DEVICE_MAX_WORK_ITEM_DIMENSIONS          = 0x1003,
				DEVICE_MAX_WORK_GROUP_SIZE               = 0x1004,
				DEVICE_MAX_WORK_ITEM_SIZES               = 0x1005,
				DEVICE_PREFERRED_VECTOR_WIDTH_CHAR       = 0x1006,
				DEVICE_PREFERRED_VECTOR_WIDTH_SHORT      = 0x1007,
				DEVICE_PREFERRED_VECTOR_WIDTH_INT        = 0x1008,
				DEVICE_PREFERRED_VECTOR_WIDTH_LONG       = 0x1009,
				DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT      = 0x100A,
				DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE     = 0x100B,
				DEVICE_MAX_CLOCK_FREQUENCY               = 0x100C,
				DEVICE_ADDRESS_BITS                      = 0x100D,
				DEVICE_MAX_READ_IMAGE_ARGS               = 0x100E,
				DEVICE_MAX_WRITE_IMAGE_ARGS              = 0x100F,
				DEVICE_MAX_MEM_ALLOC_SIZE                = 0x1010,
				DEVICE_IMAGE2D_MAX_WIDTH                 = 0x1011,
				DEVICE_IMAGE2D_MAX_HEIGHT                = 0x1012,
				DEVICE_IMAGE3D_MAX_WIDTH                 = 0x1013,
				DEVICE_IMAGE3D_MAX_HEIGHT                = 0x1014,
				DEVICE_IMAGE3D_MAX_DEPTH                 = 0x1015,
				DEVICE_IMAGE_SUPPORT                     = 0x1016,
				DEVICE_MAX_PARAMETER_SIZE                = 0x1017,
				DEVICE_MAX_SAMPLERS                      = 0x1018,
				DEVICE_MEM_BASE_ADDR_ALIGN               = 0x1019,
				DEVICE_MIN_DATA_TYPE_ALIGN_SIZE          = 0x101A,
				DEVICE_SINGLE_FP_CONFIG                  = 0x101B,
				DEVICE_GLOBAL_MEM_CACHE_TYPE             = 0x101C,
				DEVICE_GLOBAL_MEM_CACHELINE_SIZE         = 0x101D,
				DEVICE_GLOBAL_MEM_CACHE_SIZE             = 0x101E,
				DEVICE_GLOBAL_MEM_SIZE                   = 0x101F,
				DEVICE_MAX_CONSTANT_BUFFER_SIZE          = 0x1020,
				DEVICE_MAX_CONSTANT_ARGS                 = 0x1021,
				DEVICE_LOCAL_MEM_TYPE                    = 0x1022,
				DEVICE_LOCAL_MEM_SIZE                    = 0x1023,
				DEVICE_ERROR_CORRECTION_SUPPORT          = 0x1024,
				DEVICE_PROFILING_TIMER_RESOLUTION        = 0x1025,
				DEVICE_ENDIAN_LITTLE                     = 0x1026,
				DEVICE_AVAILABLE                         = 0x1027,
				DEVICE_COMPILER_AVAILABLE                = 0x1028,
				DEVICE_EXECUTION_CAPABILITIES            = 0x1029,
				DEVICE_QUEUE_PROPERTIES                  = 0x102A,
				DEVICE_NAME                              = 0x102B,
				DEVICE_VENDOR                            = 0x102C,
				DRIVER_VERSION                           = 0x102D,
				DEVICE_PROFILE                           = 0x102E,
				DEVICE_VERSION                           = 0x102F,
				DEVICE_EXTENSIONS                        = 0x1030,
				DEVICE_PLATFORM                          = 0x1031,
				/* = 0x1032 reserved for CL_DEVICE_DOUBLE_FP_CONFIG */
				/* = 0x1033 reserved for CL_DEVICE_HALF_FP_CONFIG */
				DEVICE_PREFERRED_VECTOR_WIDTH_HALF       = 0x1034,
				DEVICE_HOST_UNIFIED_MEMORY               = 0x1035,
				DEVICE_NATIVE_VECTOR_WIDTH_CHAR          = 0x1036,
				DEVICE_NATIVE_VECTOR_WIDTH_SHORT         = 0x1037,
				DEVICE_NATIVE_VECTOR_WIDTH_INT           = 0x1038,
				DEVICE_NATIVE_VECTOR_WIDTH_LONG          = 0x1039,
				DEVICE_NATIVE_VECTOR_WIDTH_FLOAT         = 0x103A,
				DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE        = 0x103B,
				DEVICE_NATIVE_VECTOR_WIDTH_HALF          = 0x103C,
				DEVICE_OPENCL_C_VERSION                  = 0x103D,

				/* cl_device_fp_config - bitfield */
				FP_DENORM                                = 0x01, // (1 << 0),
				FP_INF_NAN                               = 0x02, // (1 << 1),
				FP_ROUND_TO_NEAREST                      = 0x04, // (1 << 2),
				FP_ROUND_TO_ZERO                         = 0x08, // (1 << 3),
				FP_ROUND_TO_INF                          = 0x10, // (1 << 4),
				FP_FMA                                   = 0x20, // (1 << 5),
				FP_SOFT_FLOAT                            = 0x40, // (1 << 6),

				/* cl_device_mem_cache_type */
				NONE                                     = 0x0,
				READ_ONLY_CACHE                          = 0x1,
				READ_WRITE_CACHE                         = 0x2,

				/* cl_device_local_mem_type */
				LOCAL                                    = 0x1,
				GLOBAL                                   = 0x2,

				/* cl_device_exec_capabilities - bitfield */
				EXEC_KERNEL                              = 0x1, // (1 << 0),
				EXEC_NATIVE_KERNEL                       = 0x2, // (1 << 1),

				/* cl_command_queue_properties - bitfield */
				QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE      = 0x1, // (1 << 0),
				QUEUE_PROFILING_ENABLE                   = 0x2, // (1 << 1),

				/* cl_context_info  */
				CONTEXT_REFERENCE_COUNT                  = 0x1080,
				CONTEXT_DEVICES                          = 0x1081,
				CONTEXT_PROPERTIES                       = 0x1082,
				CONTEXT_NUM_DEVICES                      = 0x1083,

				/* cl_context_info + cl_context_properties */
				CONTEXT_PLATFORM                         = 0x1084,

				/* cl_command_queue_info */
				QUEUE_CONTEXT                            = 0x1090,
				QUEUE_DEVICE                             = 0x1091,
				QUEUE_REFERENCE_COUNT                    = 0x1092,
				QUEUE_PROPERTIES                         = 0x1093,

				/* cl_mem_flags - bitfield */
				MEM_READ_WRITE                           = 0x01, // (1 << 0),
				MEM_WRITE_ONLY                           = 0x02, // (1 << 1),
				MEM_READ_ONLY                            = 0x04, // (1 << 2),
				MEM_USE_HOST_PTR                         = 0x08, // (1 << 3),
				MEM_ALLOC_HOST_PTR                       = 0x10, // (1 << 4),
				MEM_COPY_HOST_PTR                        = 0x20, // (1 << 5),

				/* cl_channel_order */
				R                                        = 0x10B0,
				A                                        = 0x10B1,
				RG                                       = 0x10B2,
				RA                                       = 0x10B3,
				RGB                                      = 0x10B4,
				RGBA                                     = 0x10B5,
				BGRA                                     = 0x10B6,
				ARGB                                     = 0x10B7,
				INTENSITY                                = 0x10B8,
				LUMINANCE                                = 0x10B9,
				Rx                                       = 0x10BA,
				RGx                                      = 0x10BB,
				RGBx                                     = 0x10BC,

				/* cl_channel_type */
				SNORM_INT8                               = 0x10D0,
				SNORM_INT16                              = 0x10D1,
				UNORM_INT8                               = 0x10D2,
				UNORM_INT16                              = 0x10D3,
				UNORM_SHORT_565                          = 0x10D4,
				UNORM_SHORT_555                          = 0x10D5,
				UNORM_INT_101010                         = 0x10D6,
				SIGNED_INT8                              = 0x10D7,
				SIGNED_INT16                             = 0x10D8,
				SIGNED_INT32                             = 0x10D9,
				UNSIGNED_INT8                            = 0x10DA,
				UNSIGNED_INT16                           = 0x10DB,
				UNSIGNED_INT32                           = 0x10DC,
				HALF_FLOAT                               = 0x10DD,
				FLOAT                                    = 0x10DE,

				/* cl_mem_object_type */
				MEM_OBJECT_BUFFER                        = 0x10F0,
				MEM_OBJECT_IMAGE2D                       = 0x10F1,
				MEM_OBJECT_IMAGE3D                       = 0x10F2,

				/* cl_mem_info */
				MEM_TYPE                                 = 0x1100,
				MEM_FLAGS                                = 0x1101,
				MEM_SIZE                                 = 0x1102,
				MEM_HOST_PTR                             = 0x1103,
				MEM_MAP_COUNT                            = 0x1104,
				MEM_REFERENCE_COUNT                      = 0x1105,
				MEM_CONTEXT                              = 0x1106,
				MEM_ASSOCIATED_MEMOBJECT                 = 0x1107,
				MEM_OFFSET                               = 0x1108,

				/* cl_image_info */
				IMAGE_FORMAT                             = 0x1110,
				IMAGE_ELEMENT_SIZE                       = 0x1111,
				IMAGE_ROW_PITCH                          = 0x1112,
				IMAGE_SLICE_PITCH                        = 0x1113,
				IMAGE_WIDTH                              = 0x1114,
				IMAGE_HEIGHT                             = 0x1115,
				IMAGE_DEPTH                              = 0x1116,

				/* cl_addressing_mode */
				ADDRESS_NONE                             = 0x1130,
				ADDRESS_CLAMP_TO_EDGE                    = 0x1131,
				ADDRESS_CLAMP                            = 0x1132,
				ADDRESS_REPEAT                           = 0x1133,
				ADDRESS_MIRRORED_REPEAT                  = 0x1134,

				/* cl_filter_mode */
				FILTER_NEAREST                           = 0x1140,
				FILTER_LINEAR                            = 0x1141,

				/* cl_sampler_info */
				SAMPLER_REFERENCE_COUNT                  = 0x1150,
				SAMPLER_CONTEXT                          = 0x1151,
				SAMPLER_NORMALIZED_COORDS                = 0x1152,
				SAMPLER_ADDRESSING_MODE                  = 0x1153,
				SAMPLER_FILTER_MODE                      = 0x1154,

				/* cl_map_flags - bitfield */
				MAP_READ                                 = 0x1, // (1 << 0),
				MAP_WRITE                                = 0x2, // (1 << 1),

				/* cl_program_info */
				PROGRAM_REFERENCE_COUNT                  = 0x1160,
				PROGRAM_CONTEXT                          = 0x1161,
				PROGRAM_NUM_DEVICES                      = 0x1162,
				PROGRAM_DEVICES                          = 0x1163,
				PROGRAM_SOURCE                           = 0x1164,
				PROGRAM_BINARY_SIZES                     = 0x1165,
				PROGRAM_BINARIES                         = 0x1166,

				/* cl_program_build_info */
				PROGRAM_BUILD_STATUS                     = 0x1181,
				PROGRAM_BUILD_OPTIONS                    = 0x1182,
				PROGRAM_BUILD_LOG                        = 0x1183,

				/* cl_build_status */
				BUILD_SUCCESS                            = 0,
				BUILD_NONE                               = -1,
				BUILD_ERROR                              = -2,
				BUILD_IN_PROGRESS                        = -3,

				/* cl_kernel_info */
				KERNEL_FUNCTION_NAME                     = 0x1190,
				KERNEL_NUM_ARGS                          = 0x1191,
				KERNEL_REFERENCE_COUNT                   = 0x1192,
				KERNEL_CONTEXT                           = 0x1193,
				KERNEL_PROGRAM                           = 0x1194,

				/* cl_kernel_work_group_info */
				KERNEL_WORK_GROUP_SIZE                   = 0x11B0,
				KERNEL_COMPILE_WORK_GROUP_SIZE           = 0x11B1,
				KERNEL_LOCAL_MEM_SIZE                    = 0x11B2,
				KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE = 0x11B3,
				KERNEL_PRIVATE_MEM_SIZE                  = 0x11B4,

				/* cl_event_info  */
				EVENT_COMMAND_QUEUE                      = 0x11D0,
				EVENT_COMMAND_TYPE                       = 0x11D1,
				EVENT_REFERENCE_COUNT                    = 0x11D2,
				EVENT_COMMAND_EXECUTION_STATUS           = 0x11D3,
				EVENT_CONTEXT                            = 0x11D4,

				/* cl_command_type */
				COMMAND_NDRANGE_KERNEL                   = 0x11F0,
				COMMAND_TASK                             = 0x11F1,
				COMMAND_NATIVE_KERNEL                    = 0x11F2,
				COMMAND_READ_BUFFER                      = 0x11F3,
				COMMAND_WRITE_BUFFER                     = 0x11F4,
				COMMAND_COPY_BUFFER                      = 0x11F5,
				COMMAND_READ_IMAGE                       = 0x11F6,
				COMMAND_WRITE_IMAGE                      = 0x11F7,
				COMMAND_COPY_IMAGE                       = 0x11F8,
				COMMAND_COPY_IMAGE_TO_BUFFER             = 0x11F9,
				COMMAND_COPY_BUFFER_TO_IMAGE             = 0x11FA,
				COMMAND_MAP_BUFFER                       = 0x11FB,
				COMMAND_MAP_IMAGE                        = 0x11FC,
				COMMAND_UNMAP_MEM_OBJECT                 = 0x11FD,
				COMMAND_MARKER                           = 0x11FE,
				COMMAND_ACQUIRE_GL_OBJECTS               = 0x11FF,
				COMMAND_RELEASE_GL_OBJECTS               = 0x1200,
				COMMAND_READ_BUFFER_RECT                 = 0x1201,
				COMMAND_WRITE_BUFFER_RECT                = 0x1202,
				COMMAND_COPY_BUFFER_RECT                 = 0x1203,
				COMMAND_USER                             = 0x1204,

				/* command execution status */
				COMPLETE                                 = 0x0,
				RUNNING                                  = 0x1,
				SUBMITTED                                = 0x2,
				QUEUED                                   = 0x3,

				/* cl_buffer_create_type  */
				BUFFER_CREATE_TYPE_REGION                = 0x1220,
				/* cl_profiling_info  */
				PROFILING_COMMAND_QUEUED                 = 0x1280,
				PROFILING_COMMAND_SUBMIT                 = 0x1281,
				PROFILING_COMMAND_START                  = 0x1282,
				PROFILING_COMMAND_END                    = 0x1283
			};

			virtual ~WebCLDeviceIDList();
			static PassRefPtr<WebCLDeviceIDList> create(WebCLComputeContext*, cl_platform_id, int);
			cl_device_id getCLDeviceIDs();

			unsigned length() const;
			WebCLDeviceID* item(unsigned index);

		private:	
			WebCLDeviceIDList(WebCLComputeContext*, cl_platform_id, int);

			WebCLComputeContext* m_context;
			Vector<RefPtr<WebCLDeviceID> > m_device_id_list;
			cl_device_id* m_cl_devices;
			cl_uint m_num_devices;
	};

} // namespace WebCore

#endif // WebCLDeviceIDList_h
