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

#include "config.h"

#if ENABLE(WEBCL)

#include "WebCLComputeContext.h"
#include "ScriptExecutionContext.h"
#include "Document.h"
#include "DOMWindow.h"
#include "Image.h"
#include "SharedBuffer.h"
#include "CanvasRenderingContext2D.h"
#include "ImageBuffer.h"
#include "CachedImage.h"
#include "ArrayBuffer.h"
#include "CanvasPixelArray.h"
#include "HTMLCanvasElement.h"


#include <stdio.h>
#include <wtf/ByteArray.h>
#include <CanvasRenderingContext.h>

class CanvasRenderingContext;

using namespace JSC;

namespace WebCore {   

WebCLComputeContext::WebCLComputeContext(ScriptExecutionContext* context) : ActiveDOMObject(context, this)
																				, m_videoCache(4)
{
	m_error_state = SUCCESS;
	m_num_mems = 0;
	m_num_programs = 0;
	m_num_kernels = 0;
	m_num_events = 0;
	m_num_samplers = 0;
}

PassRefPtr<WebCLComputeContext> WebCLComputeContext::create(ScriptExecutionContext* context)
{
	return adoptRef(new WebCLComputeContext(context));
}

WebCLComputeContext::~WebCLComputeContext()
{
}

void WebCLComputeContext::hello()
{
}

PassRefPtr<WebCLPlatformIDList> WebCLComputeContext::getPlatformIDs()
{
	RefPtr<WebCLPlatformIDList> o = WebCLPlatformIDList::create(this);
	if (o != NULL) {
		m_error_state = SUCCESS;
		m_platform_id = o;
		return o;
	} else {
		m_error_state = FAILURE;
		return NULL;
	}
}

PassRefPtr<WebCLDeviceIDList> WebCLComputeContext::getDeviceIDs(WebCLPlatformID* webcl_platform_id, 
		int device_type)
{
	cl_platform_id platform_id = NULL;

	if (webcl_platform_id != NULL) {
		platform_id = webcl_platform_id->getCLPlatformID();
		if (platform_id == NULL) {
			printf("Error: platform_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	} else {
		printf("Error: webcl_platform_id null\n");
		m_error_state = FAILURE;
		return NULL;
	}

	RefPtr<WebCLDeviceIDList> o = WebCLDeviceIDList::create(this, platform_id, 
			device_type);
	if (o != NULL) {
		m_error_state = SUCCESS;
		m_device_id = o;
		return o;
	} else {
		m_error_state = FAILURE;
		return NULL;
	}
}

WebCLGetInfo WebCLComputeContext::getPlatformInfo (WebCLPlatformID* webcl_platform_id,
		int platform_info)
{
	cl_platform_id platform_id = NULL;
	cl_int err = 0;

	if (webcl_platform_id != NULL) {
		platform_id = webcl_platform_id->getCLPlatformID();
		if (platform_id == NULL) {
			printf("Error: platform_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}
	else {
		printf("Error: webcl_platform_id null\n");
		m_error_state = FAILURE;
		return WebCLGetInfo();
	}

	m_error_state = SUCCESS;
	char platform_string[1024];
	switch(platform_info)
	{
		case PLATFORM_PROFILE:
			err = clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, sizeof(platform_string), &platform_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(platform_string));
			break;
		case PLATFORM_VERSION:
			err = clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, sizeof(platform_string), &platform_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(platform_string));
			break;
		case PLATFORM_NAME:
			err = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, sizeof(platform_string), &platform_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(platform_string));
			break;
		case PLATFORM_VENDOR:
			err = clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, sizeof(platform_string), &platform_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(platform_string));
			break;
		case PLATFORM_EXTENSIONS:
			err = clGetPlatformInfo(platform_id, CL_PLATFORM_EXTENSIONS, sizeof(platform_string), &platform_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(platform_string));
			break;
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Platform Info type = %d ",platform_info);
			return WebCLGetInfo();
	}

	if(err != CL_SUCCESS)
	{
		switch (err) {
			case CL_INVALID_PLATFORM :
				printf("Error: CL_INVALID_PLATFORM  \n");
				m_error_state = INVALID_PLATFORM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	}
	return WebCLGetInfo();
}

WebCLGetInfo  WebCLComputeContext::getDeviceInfo(WebCLDeviceID*   webcl_device_id,  
		int device_type)
{
	cl_device_id cl_device = NULL;
	cl_int err = 0;
	char device_string[1024];
	cl_uint uint_units = 0;
	size_t sizet_units = 0;
	size_t sizet_array[1024] = {0};
	cl_ulong  ulong_units = 0;
	cl_bool bool_units = false;
	cl_device_type type = 0;
	cl_device_mem_cache_type global_type = 0;
	cl_command_queue_properties queue_properties = 0;
	cl_device_exec_capabilities exec = NULL;
	cl_device_local_mem_type local_type = 0;

	cl_device = webcl_device_id->getCLDeviceID();
	if (cl_device == NULL) {
		m_error_state = FAILURE;
		printf("Error: devices null\n");
		return WebCLGetInfo();
	}

	m_error_state = SUCCESS;

	switch(device_type)
	{

		case DEVICE_EXTENSIONS:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_EXTENSIONS, sizeof(device_string), &device_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(device_string));	
			break;
		case DEVICE_NAME:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(device_string));	
			break;
		case DEVICE_PROFILE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PROFILE, sizeof(device_string), &device_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(device_string));	
			break;
		case DEVICE_VENDOR:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_VENDOR, sizeof(device_string), &device_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(device_string));
			break;
		case DEVICE_VERSION:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_VERSION, sizeof(device_string), &device_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(device_string));
			break;
		case DRIVER_VERSION:
			err=clGetDeviceInfo(cl_device, CL_DRIVER_VERSION, sizeof(device_string), &device_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(device_string));	
			break;
		case DEVICE_ADDRESS_BITS:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_ADDRESS_BITS , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_MAX_CLOCK_FREQUENCY:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case DEVICE_MAX_CONSTANT_ARGS:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_MAX_READ_IMAGE_ARGS:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case DEVICE_MAX_SAMPLERS:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_SAMPLERS, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_MAX_WRITE_IMAGE_ARGS:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_MEM_BASE_ADDR_ALIGN:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_VENDOR_ID:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_PREFERRED_VECTOR_WIDTH_INT:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &uint_units,NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_MAX_COMPUTE_UNITS:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_COMPUTE_UNITS , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case DEVICE_IMAGE2D_MAX_HEIGHT:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_IMAGE2D_MAX_WIDTH:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_IMAGE3D_MAX_DEPTH:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_IMAGE3D_MAX_HEIGHT:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_IMAGE3D_MAX_WIDTH:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_MAX_PARAMETER_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_MAX_WORK_GROUP_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_MAX_WORK_ITEM_SIZES:
			err=clGetDeviceInfo(cl_device, DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &sizet_units, NULL);
			if(err == CL_SUCCESS) {
				err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_WORK_ITEM_SIZES, 1024, &sizet_array, NULL);
				// TODO (siba samal) Check support for SizeTArray/Int16Array in WebCLGetInfo
				if (err == CL_SUCCESS) {
					int values[1024] = {0};
					for(int i=0; i<((int)sizet_units); i++)
						values[i] = (int)sizet_array[i];
					return WebCLGetInfo(Int32Array::create(values, (int)sizet_units));
				}
			}
			break;
		case DEVICE_PROFILING_TIMER_RESOLUTION:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_MAX_WORK_ITEM_DIMENSIONS:
			err=clGetDeviceInfo(cl_device, DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case DEVICE_LOCAL_MEM_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
			break;
		case DEVICE_MAX_CONSTANT_BUFFER_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
			break;
		case DEVICE_MAX_MEM_ALLOC_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
			break;
		case DEVICE_GLOBAL_MEM_CACHE_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(ulong_units));
			break;
		case DEVICE_GLOBAL_MEM_SIZE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(ulong_units));
			break;
		case DEVICE_AVAILABLE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_AVAILABLE , sizeof(cl_bool), &bool_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<bool>(bool_units));
			break;
		case DEVICE_COMPILER_AVAILABLE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_COMPILER_AVAILABLE , sizeof(cl_bool), &bool_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<bool>(bool_units));
			break;
		case DEVICE_ENDIAN_LITTLE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool), &bool_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<bool>(bool_units));
			break;
		case DEVICE_ERROR_CORRECTION_SUPPORT:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(cl_bool), &bool_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<bool>(bool_units));
			break;
		case DEVICE_IMAGE_SUPPORT:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &bool_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<bool>(bool_units));
			break;
		case DEVICE_TYPE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(type));
			break;
		case DEVICE_QUEUE_PROPERTIES:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_QUEUE_PROPERTIES, 
					sizeof(cl_command_queue_properties), &queue_properties, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(queue_properties));
			break;

			//cl_device_fp_config fp_config = 0;
			// Part of cl_ext.h (which isn't available in Khronos)
			//case DEVICE_DOUBLE_FP_CONFIG:
			//clGetDeviceInfo(cl_device, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);
			//case DEVICE_HALF_FP_CONFIG:
			//clGetDeviceInfo(cl_device, CL_DEVICE_HALF_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);
			//case DEVICE_SINGLE_FP_CONFIG:
			//clGetDeviceInfo(cl_device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);
			//return WebCLGetInfo(static_cast<unsigned int>(fp_config));


			//Platform ID is not Supported
			//case DEVICE_PLATFORM:
			//cl_platform_id platform_id = NULL;
			//clGetDeviceInfo(cl_device, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platform_id, NULL);
			//return WebCLGetInfo(static_cast<unsigned int>(platform_id));

		case DEVICE_EXECUTION_CAPABILITIES:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(cl_device_exec_capabilities), &exec, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(exec));
			break;
		case DEVICE_GLOBAL_MEM_CACHE_TYPE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cl_device_mem_cache_type), &global_type, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(global_type));
			break;
		case DEVICE_LOCAL_MEM_TYPE:
			err=clGetDeviceInfo(cl_device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &local_type, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(local_type));
			break;
		default:
			m_error_state = FAILURE;
			printf("Error:UNSUPPORTED DEVICE TYPE = %d ",device_type);
			return WebCLGetInfo();
	}

	switch (err) {
		case CL_INVALID_DEVICE:
			printf("Error: CL_INVALID_DEVICE \n");
			m_error_state = INVALID_DEVICE;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE\n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();
}

WebCLGetInfo WebCLComputeContext::getKernelInfo (WebCLKernel* kernel, int kernel_info)
{
	cl_kernel cl_kernel_id = NULL;
	cl_int err = 0;
	char function_name[1024];
	cl_uint uint_units = 0;
	cl_program cl_program_id = NULL;
	cl_context cl_context_id = NULL;
	RefPtr<WebCLProgram> programObj = NULL;
	RefPtr<WebCLContext> contextObj = NULL;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}

	switch(kernel_info)
	{
		case KERNEL_FUNCTION_NAME:
			err=clGetKernelInfo(cl_kernel_id, CL_KERNEL_FUNCTION_NAME, sizeof(function_name), &function_name, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(function_name));
			break;
		case KERNEL_NUM_ARGS:
			err=clGetKernelInfo(cl_kernel_id, CL_KERNEL_NUM_ARGS , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case KERNEL_REFERENCE_COUNT:
			err=clGetKernelInfo(cl_kernel_id, CL_KERNEL_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case KERNEL_PROGRAM:
			err=clGetKernelInfo(cl_kernel_id, CL_KERNEL_PROGRAM, sizeof(cl_program_id), &cl_program_id, NULL);
			programObj = WebCLProgram::create(this, cl_program_id);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(PassRefPtr<WebCLProgram>(programObj));
			break;
		case KERNEL_CONTEXT:
			err=clGetKernelInfo(cl_kernel_id, CL_KERNEL_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
			contextObj = WebCLContext::create(this, cl_context_id);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
			break;
		default:
			m_error_state = FAILURE;
			return WebCLGetInfo();
	}
	switch (err) {
		case CL_INVALID_KERNEL :
			printf("Error: CL_INVALID_KERNEL  \n");
			m_error_state = INVALID_KERNEL;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE\n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();
}

WebCLGetInfo WebCLComputeContext::getProgramInfo (WebCLProgram* program, int param_name)
{
	cl_program cl_program_id = NULL;
	cl_int err = 0;
	cl_uint uint_units = 0;
	size_t sizet_units = 0;
	char program_string[4096];
	cl_context cl_context_id = NULL;
	RefPtr<WebCLContext> contextObj  = NULL;

	if (program != NULL) {
		cl_program_id = program->getCLProgram();
		if (cl_program_id == NULL) {
			printf("Error: cl_program_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}
	switch(param_name)
	{   
		case PROGRAM_REFERENCE_COUNT:
			err=clGetProgramInfo(cl_program_id, CL_PROGRAM_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case PROGRAM_NUM_DEVICES:
			err=clGetProgramInfo(cl_program_id, CL_PROGRAM_NUM_DEVICES , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case PROGRAM_BINARY_SIZES:
			err=clGetProgramInfo(cl_program_id, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case PROGRAM_SOURCE:
			err=clGetProgramInfo(cl_program_id, CL_PROGRAM_SOURCE, sizeof(program_string), &program_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(program_string));
			break;
		case PROGRAM_BINARIES:
			err=clGetProgramInfo(cl_program_id, CL_PROGRAM_BINARIES, sizeof(program_string), &program_string, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(program_string));
			break;
		case PROGRAM_CONTEXT:
			err=clGetProgramInfo(cl_program_id, CL_PROGRAM_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
			contextObj = WebCLContext::create(this, cl_context_id);
			if(contextObj == NULL)
			{
				m_error_state = FAILURE;
				printf("Error : CL program context not NULL\n");
				return WebCLGetInfo();
			}
			if (err == CL_SUCCESS)
				return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
			break;
			// TODO (siba samal)- Handle Array of cl_device_id 
			//case PROGRAM_DEVICES:
			//  size_t numDevices;
			//  clGetProgramInfo( cl_program_id, CL_PROGRAM_DEVICES, 0, 0, &numDevices );
			//  cl_device_id *devices = new cl_device_id[numDevices];
			//  clGetProgramInfo( cl_program_id, CL_PROGRAM_DEVICES, numDevices, devices, &numDevices );
			//  return WebCLGetInfo(PassRefPtr<WebCLContext>(obj));
		default:
			m_error_state = FAILURE;
			printf("Error: UNSUPPORTED program Info type = %d ",param_name);
			return WebCLGetInfo();
	}
	switch (err) {
		case CL_INVALID_PROGRAM :
			printf("Error: CL_INVALID_PROGRAM \n");
			m_error_state = INVALID_PROGRAM;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE\n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}
	return WebCLGetInfo();
}

WebCLGetInfo WebCLComputeContext::getProgramBuildInfo(WebCLProgram* program, WebCLDeviceID* device, 
		int param_name)
{
	cl_program program_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint err = 0;
	char buffer[8192];
	size_t len = 0;

	if (program != NULL) {
		program_id = program->getCLProgram();
		if (program_id == NULL) {
			printf("Error: program_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}
	if (device != NULL) {
		device_id = device->getCLDeviceID();
		if (device_id == NULL) {
			printf("Error: device_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}

	switch (param_name) {

		case PROGRAM_BUILD_LOG:
			err = clGetProgramBuildInfo(program_id, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(buffer));
			break;
		case PROGRAM_BUILD_OPTIONS:
			err = clGetProgramBuildInfo(program_id, device_id, CL_PROGRAM_BUILD_OPTIONS, sizeof(buffer), &buffer, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(buffer));
			break;
		case PROGRAM_BUILD_STATUS:
			cl_build_status build_status;
			err = clGetProgramBuildInfo(program_id, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(build_status));
			break;
		default:
			m_error_state = FAILURE;
			printf("Error: UNSUPPORTED Program Build Info   Type = %d ",param_name);
			return WebCLGetInfo();			
	}
	switch (err) {
		case CL_INVALID_DEVICE :
			printf("Error: CL_INVALID_DEVICE   \n");
			m_error_state = INVALID_DEVICE;
			break;
		case CL_INVALID_VALUE :
			printf("Error: CL_INVALID_VALUE \n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_INVALID_PROGRAM  :
			printf("Error: CL_INVALID_PROGRAM  \n");
			m_error_state = INVALID_PROGRAM ;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();
}

WebCLGetInfo WebCLComputeContext:: getCommandQueueInfo(WebCLCommandQueue* command_queue, int param_name)
{
	cl_command_queue cl_command_queue_id = NULL;
	cl_int err = 0;
	cl_uint uint_units = 0;
	cl_context cl_context_id = NULL;
	RefPtr<WebCLContext> contextObj = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}			
	switch(param_name)
	{

		case QUEUE_REFERENCE_COUNT:
			err=clGetCommandQueueInfo(cl_command_queue_id, CL_QUEUE_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case QUEUE_CONTEXT:
			clGetCommandQueueInfo(cl_command_queue_id, CL_QUEUE_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
			contextObj = WebCLContext::create(this, cl_context_id);
			if(contextObj != NULL)
			{
				m_error_state = FAILURE;
			}
			if (err == CL_SUCCESS)
				return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
			break;
			// TODO (siba samal) Support for QUEUE_DEVICE & QUEUE_PROPERTIES
			// {
			// case QUEUE_DEVICE:
			// cl_device_id device_id = NULL;
			// clGetCommandQueueInfo(cl_command_queue_id, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device_id, NULL);
			// printf("SUCCESS: Cl Device ID  = %u\n",(unsigned int)device_id );
			// return WebCLGetInfo(static_cast<unsigned int>(device_id));							
			// }
			// {
			// case QUEUE_PROPERTIES:
			// cl_command_queue_properties queue_properties = NULL;
			// clGetCommandQueueInfo(cl_command_queue_id, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &queue_properties, NULL);
			//return WebCLGetInfo(static_cast<unsigned int>(queue_properties));

			//}

		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Commans Queue Info type\n");
			return WebCLGetInfo();
	}
	switch (err) {
		case CL_INVALID_COMMAND_QUEUE:
			printf("Error: CL_INVALID_COMMAND_QUEUE  \n");
			m_error_state = INVALID_COMMAND_QUEUE;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE\n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();
}

WebCLGetInfo WebCLComputeContext::getContextInfo(WebCLContext* context_id, int param_name)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;
	cl_uint uint_units = 0;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}

	switch(param_name)
	{	
		case CONTEXT_REFERENCE_COUNT:
			err=clGetContextInfo(cl_context_id, CL_CONTEXT_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;

			// TODO (siba samal) Handle CONTEXT_DEVICES & CONTEXT_PROPERTIES
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Context Info type\n");
			return WebCLGetInfo();
	}
	switch (err) {
		case CL_INVALID_CONTEXT:
			printf("Error: CL_INVALID_CONTEXT \n");
			m_error_state = INVALID_CONTEXT;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE\n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();
}
WebCLGetInfo WebCLComputeContext::getKernelWorkGroupInfo(WebCLKernel* kernel, WebCLDeviceID* device, int param_name)
{
	cl_int err = 0;
	cl_kernel cl_kernel_id = NULL;
	cl_device_id cl_device = NULL;
	size_t sizet_units = 0;
	cl_ulong  ulong_units = 0;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}
	if (device != NULL) {
		cl_device = device->getCLDeviceID();
		if (cl_device == NULL) {
			printf("Error: cl_device null\n");
			m_error_state = FAILURE;
			return  WebCLGetInfo();
		}
	}
	switch (param_name) {

		case KERNEL_WORK_GROUP_SIZE:
			err = clGetKernelWorkGroupInfo(cl_kernel_id, cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case KERNEL_COMPILE_WORK_GROUP_SIZE:
			err =clGetKernelWorkGroupInfo(cl_kernel_id, cl_device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
			return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case KERNEL_LOCAL_MEM_SIZE:
			err =clGetKernelWorkGroupInfo(cl_kernel_id, cl_device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
			break;
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Kernrl Info type\n");
			return WebCLGetInfo();
	}

	printf("Error: clGetKerelWorkGroupInfo\n");
	switch (err) {
		case CL_INVALID_DEVICE:
			printf("Error: CL_INVALID_DEVICE\n");
			m_error_state = INVALID_DEVICE;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE\n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_INVALID_KERNEL:
			printf("Error: CL_INVALID_KERNEL\n");
			m_error_state = INVALID_KERNEL;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();

}

WebCLGetInfo WebCLComputeContext::getMemObjectInfo(WebCLMem* memobj,int param_name)
{
	cl_mem cl_mem_id = NULL;
	cl_int err =0;
	cl_uint uint_units = 0;
	size_t sizet_units = 0;
	RefPtr<WebCLContext> contextObj = NULL;
	cl_context cl_context_id = NULL;
	cl_mem_object_type mem_type = 0;
	void* mem_ptr = NULL; 

	if (memobj != NULL) {
		cl_mem_id = memobj->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}
	switch(param_name)
	{   	
		case MEM_MAP_COUNT:
			err=clGetMemObjectInfo(cl_mem_id, CL_MEM_MAP_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case MEM_REFERENCE_COUNT:
			err=clGetMemObjectInfo(cl_mem_id, CL_MEM_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case MEM_SIZE:
			err=clGetMemObjectInfo(cl_mem_id, CL_MEM_SIZE, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case MEM_TYPE:			
			err=clGetMemObjectInfo(cl_mem_id, CL_MEM_TYPE, sizeof(cl_mem_object_type), &mem_type, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(mem_type));
			break;
		case MEM_CONTEXT:			
			err=clGetMemObjectInfo(cl_mem_id, CL_MEM_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
			contextObj = WebCLContext::create(this, cl_context_id);
			if(contextObj == NULL)
			{
				m_error_state = FAILURE;
				printf("Error: CL Mem context not NULL\n");
				return WebCLGetInfo();
			}
			if (err == CL_SUCCESS)
				return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
			break;
		case MEM_HOST_PTR:			
			err=clGetMemObjectInfo(cl_mem_id, CL_MEM_HOST_PTR, sizeof(mem_ptr), &mem_ptr, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(mem_ptr);
			break;
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Mem Info type\n");
			return WebCLGetInfo();
	}
	switch (err) {
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE   \n");
			m_error_state = INVALID_VALUE;
			break;
			// TODO (siba samal) Handle CL_INVALID_D3D10_RESOURCE_KHR Case
			//case CL_INVALID_D3D10_RESOURCE_KHR :
			//	printf("CL_INVALID_D3D10_RESOURCE_KHR    \n");
			//	m_error_state = INVALID_D3D10_RESOURCE_KHR;
			//	break; 
		case CL_INVALID_MEM_OBJECT :
			printf("Error: CL_INVALID_MEM_OBJECT    \n");
			m_error_state = INVALID_MEM_OBJECT;
			break;
		case CL_OUT_OF_RESOURCES :
			printf("Error: CL_OUT_OF_RESOURCES   \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();
}

WebCLGetInfo WebCLComputeContext::getEventInfo(WebCLEvent* event, int param_name)
{   
	cl_event cl_Event_id = NULL;
	cl_int err = 0;
	cl_uint uint_units = 0;
	cl_command_type command_type = 0;
	cl_command_queue command_queue = 0;
	RefPtr<WebCLCommandQueue> cqObj = NULL;

	if (event != NULL) {
		cl_Event_id = event->getCLEvent();
		if (cl_Event_id == NULL) {
			printf("Error: cl_Event_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}
	switch(param_name)
	{   
		case EVENT_COMMAND_EXECUTION_STATUS :
			err=clGetEventInfo(cl_Event_id, CL_EVENT_COMMAND_EXECUTION_STATUS  , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
			break;
		case EVENT_REFERENCE_COUNT:
			err=clGetEventInfo(cl_Event_id, CL_EVENT_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case EVENT_COMMAND_TYPE:			
			err=clGetEventInfo(cl_Event_id, CL_EVENT_COMMAND_TYPE , sizeof(cl_command_type), &command_type, NULL);;
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(command_type));
			break;
		case EVENT_COMMAND_QUEUE:			
			err = clGetEventInfo(cl_Event_id, CL_EVENT_COMMAND_QUEUE , sizeof(cl_command_queue), &command_queue, NULL);;
			cqObj = WebCLCommandQueue::create(this, command_queue);
			if(cqObj == NULL)
			{
				m_error_state = FAILURE;
				printf("SUCCESS: Cl Event Command Queue\n");
				return WebCLGetInfo();
			}
			if (err == CL_SUCCESS)
				return WebCLGetInfo(PassRefPtr<WebCLCommandQueue>(cqObj));
			break;
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Event Info type\n");
			return WebCLGetInfo();
	}
	switch (err) {
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE   \n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_INVALID_EVENT:
			printf("Error: CL_INVALID_EVENT   \n");
			m_error_state = INVALID_EVENT;
			break; 
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES   \n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();

}

WebCLGetInfo WebCLComputeContext::getEventProfilingInfo(WebCLEvent* event, int param_name)
{

	cl_event cl_event_id = NULL;
	cl_int err=0;
	cl_ulong  ulong_units = 0;

	if (event != NULL) {
		cl_event_id = event->getCLEvent();
		if (cl_event_id == NULL) {
			printf("Error: cl_event_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}
	switch(param_name)
	{   
		case PROFILING_COMMAND_QUEUED:
			err=clGetEventProfilingInfo(cl_event_id, CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
			break;
		case PROFILING_COMMAND_SUBMIT:
			err=clGetEventProfilingInfo(cl_event_id, CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
			break;
		case PROFILING_COMMAND_START:
			err=clGetEventProfilingInfo(cl_event_id, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
			break;
		case PROFILING_COMMAND_END:
			err=clGetEventProfilingInfo(cl_event_id, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ulong_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(ulong_units));
			break;
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Profiling Info type\n");
			return WebCLGetInfo();

	}

	switch (err) {

		case CL_PROFILING_INFO_NOT_AVAILABLE :
			printf("Error: CL_PROFILING_INFO_NOT_AVAILABLE\n");
			m_error_state = PROFILING_INFO_NOT_AVAILABLE;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE\n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_INVALID_EVENT:
			printf("Error: CL_INVALID_EVENT \n");
			m_error_state = INVALID_EVENT;
			break; 
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES\n");
			m_error_state = OUT_OF_RESOURCES;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY\n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();

}

WebCLGetInfo WebCLComputeContext:: getSamplerInfo(WebCLSampler* sampler, cl_sampler_info param_name)
{
	cl_sampler cl_sampler_id= NULL;
	cl_int err = 0;
	cl_uint uint_units = 0;
	cl_bool bool_units = false;
	cl_context cl_context_id = NULL;
	RefPtr<WebCLContext> contextObj =  NULL;

	if (sampler != NULL) {
		cl_sampler_id = sampler->getCLSampler();
		if (cl_sampler_id == NULL) {
			printf("Error: cl_sampler_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();

		}
	}
	switch(param_name)
	{   
		case SAMPLER_REFERENCE_COUNT:
			err=clGetSamplerInfo (cl_sampler_id, CL_SAMPLER_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case SAMPLER_NORMALIZED_COORDS:
			err=clGetSamplerInfo(cl_sampler_id, CL_SAMPLER_NORMALIZED_COORDS , sizeof(cl_bool), &bool_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<bool>(bool_units));
			break;
		case SAMPLER_CONTEXT:
			err=clGetSamplerInfo(cl_sampler_id, CL_SAMPLER_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
			contextObj = WebCLContext::create(this, cl_context_id);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
			break;
		case SAMPLER_ADDRESSING_MODE:
			err=clGetSamplerInfo (cl_sampler_id, CL_SAMPLER_ADDRESSING_MODE , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		case SAMPLER_FILTER_MODE:
			err=clGetSamplerInfo (cl_sampler_id, CL_SAMPLER_FILTER_MODE , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(uint_units));
			break;
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Sampler Info type\n");
			return WebCLGetInfo();	
	}
	switch (err) {

		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE \n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_INVALID_SAMPLER:
			printf("Error: CL_INVALID_SAMPLER\n");
			m_error_state = INVALID_SAMPLER;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES\n");
			m_error_state = OUT_OF_RESOURCES;
			break; 
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY\n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();

}
WebCLGetInfo WebCLComputeContext::getImageInfo(WebCLImage* image, cl_image_info param_name) 	
{
	cl_mem cl_Image_id = NULL;
	cl_int err = 0;
	size_t sizet_units = 0;
	if (image != NULL) {
		cl_Image_id = image->getCLImage();
		if (cl_Image_id == NULL) {
			printf("Error: cl_Image_id null\n");
			m_error_state = FAILURE;
			return WebCLGetInfo();
		}
	}

	switch(param_name)
	{   
		case IMAGE_ELEMENT_SIZE:
			err=clGetImageInfo(cl_Image_id, CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case IMAGE_ROW_PITCH:
			err=clGetImageInfo(cl_Image_id, CL_IMAGE_ROW_PITCH, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case IMAGE_SLICE_PITCH:
			err=clGetImageInfo(cl_Image_id, CL_IMAGE_SLICE_PITCH, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case IMAGE_WIDTH:
			err=clGetImageInfo(cl_Image_id, CL_IMAGE_WIDTH, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case IMAGE_HEIGHT:
			err=clGetImageInfo(cl_Image_id, CL_IMAGE_HEIGHT, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
		case IMAGE_DEPTH:
			err=clGetImageInfo(cl_Image_id, CL_IMAGE_DEPTH, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
			break;
			// TODO (siba samal) Handle Image Format & CL_IMAGE_D3D10_SUBRESOURCE_KHR types
		default:
			m_error_state = FAILURE;
			printf("Error: Unsupported Image Info type\n");
			return WebCLGetInfo();
	}
	switch (err) {
		case CL_INVALID_MEM_OBJECT:
			printf("Error: CL_INVALID_MEM_OBJECT \n");
			m_error_state = INVALID_MEM_OBJECT;
			break;
		case CL_INVALID_VALUE:
			printf("Error: CL_INVALID_VALUE \n");
			m_error_state = INVALID_VALUE;
			break;
		case CL_OUT_OF_RESOURCES:
			printf("Error: CL_OUT_OF_RESOURCES\n");
			m_error_state = OUT_OF_RESOURCES;
			break; 
		case CL_OUT_OF_HOST_MEMORY:
			printf("Error: CL_OUT_OF_HOST_MEMORY\n");
			m_error_state = OUT_OF_HOST_MEMORY;
			break;
		default:
			printf("Error: Invaild Error Type\n");
			m_error_state = FAILURE;
			break;
	}				
	return WebCLGetInfo();

}
long WebCLComputeContext::getError()
{
	return m_error_state;
}

PassRefPtr<WebCLEvent> WebCLComputeContext::createUserEvent(WebCLContext* context_id)
{
	cl_int err = -1;
	cl_context cl_context_id = NULL;
	cl_event event = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			m_error_state = FAILURE;
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}

	//(TODO) To be uncommented for OpenCL1.1
	//event =  clCreateUserEvent(cl_context_id, &err);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT :
				printf("Error: CL_INVALID_CONTEXT \n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_OUT_OF_RESOURCES :
				printf("Error: CCL_OUT_OF_RESOURCES \n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY :
				printf("Error: CCL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, event);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

void WebCLComputeContext::setUserEventStatus (WebCLEvent* event, int exec_status)
{
	cl_event cl_event_id = NULL;
	cl_int err = -1;
	if (event != NULL) {
		cl_event_id = event->getCLEvent();
		if (cl_event_id == NULL) {
			printf("Error: cl_event_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}

	// TODO (siba samal) To be uncommented for  OpenCL 1.1	
	// http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clSetUserEventStatus.html
	if(exec_status == COMPLETE)	{   
		//err = clSetUserEventStatus(cl_event_id , CL_COMPLETE)
	}
	else if (exec_status < 0) {
		//err = clSetUserEventStatus(cl_event_id , exec_status)
	}
	else
	{
		printf("Error: Invaild Error Type\n");
		return;
	}
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_EVENT :
				printf("Error: CL_INVALID_EVENT \n");
				m_error_state = INVALID_EVENT;
				break;
			case CL_INVALID_VALUE :
				printf("Error: CL_INVALID_VALUE  \n");
				m_error_state = INVALID_VALUE ;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES  \n");
				m_error_state = OUT_OF_RESOURCES ;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION  \n");
				m_error_state = INVALID_OPERATION ;
				break;
			case CL_OUT_OF_HOST_MEMORY :
				printf("Error: CCL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} 
	return;
}

void WebCLComputeContext::waitForEvents(WebCLEventList* events)
{
	cl_int err = 0;
	cl_event* cl_event_id = NULL;

	if (events != NULL) {
		cl_event_id = events->getCLEvents();
		if (cl_event_id == NULL) {
			printf("Error: cl_event null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clWaitForEvents(events->length(), cl_event_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT \n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE \n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT :
				printf("Error: CL_INVALID_EVENT  \n");
				m_error_state = INVALID_EVENT ;
				break;
				//OpenCL 1.1
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST \n");
				//	m_error_state = EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
				//	break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;

			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;

		}
	} 
	return;
}


PassRefPtr<WebCLContext> WebCLComputeContext::createContext(int contextProperties, 
		WebCLDeviceIDList* devices, int pfn_notify, int user_data)
{

	cl_int err = 0;
	cl_context cl_context_id = 0;
	cl_device_id cl_device = NULL;

	if (devices != NULL) {
		cl_device = devices->getCLDeviceIDs();
		if (cl_device == NULL) {
			printf("Error: devices null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	} else {
		printf("Error: webcl_devices null\n");
		printf("Unused in createContext prop=%d pfn_notify=%d user_data=%d\n", 
				contextProperties, pfn_notify, user_data);

		return NULL;
	}

	// TODO(won.jeon) - prop, pfn_notify, and user_data need to be addressed later
	cl_context_id = clCreateContext(NULL, 1, &cl_device, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PLATFORM:
				printf("Error: CL_INVALID_PLATFORM\n");
				m_error_state = INVALID_PLATFORM;
				break;
				//case CL_INVALID_PROPERTY:
				//	printf("CL_INVALID_PROPERTY  \n");
				//	m_error_state = INVALID_PROPERTY;
				//	break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
			case CL_DEVICE_NOT_AVAILABLE:
				printf("Error: CL_DEVICE_NOT_AVAILABLE\n");
				m_error_state = DEVICE_NOT_AVAILABLE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
				// TODO (siba samal) Error handling following Error Types
				//	case CL_INVALID_D3D10_DEVICE_KHR:
				//		printf("Error: CL_INVALID_D3D10_DEVICE_KHR \n");
				//		m_error_state = INVALID_D3D10_DEVICE_KHR;
				//		break;
				//		case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR :
				//			printf("Error: CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR\n");
				//			m_error_state = INVALID_GL_SHAREGROUP_REFERENCE_KHR;
				//			break;

			default:
				printf("Error: Invalid ERROR Type\n");
				m_error_state = FAILURE;
				break;

		}
	} else {
		RefPtr<WebCLContext> o = WebCLContext::create(this, cl_context_id);
		if (o != NULL) {
			m_error_state = SUCCESS;
			m_context = o;
			return o;
		} else {
			m_error_state = FAILURE;
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLContext> WebCLComputeContext::createContextFromType(int contextProperties, 
		int device_type, int pfn_notify, int user_data)
{

	printf("CreateContext prop=%d pfn_notify=%d user_data=%d\n", 
			contextProperties, pfn_notify, user_data);
	cl_int err = 0;
	cl_context cl_context_id = 0;

	//TODO (siba samal) Need to handle context properties	
	if((CONTEXT_PLATFORM != contextProperties) &&  (0 != contextProperties))
	{
		printf("Error: INVALID CONTEXT PROPERTIES\n");
		return NULL;
	}

	switch((unsigned int)device_type) {
		case DEVICE_TYPE_GPU:
			cl_context_id = clCreateContextFromType(NULL, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
			break;
		case DEVICE_TYPE_CPU:
			cl_context_id = clCreateContextFromType(NULL, CL_DEVICE_TYPE_CPU, NULL, NULL, &err);
			break;  
		case DEVICE_TYPE_ACCELERATOR:
			cl_context_id = clCreateContextFromType(NULL, CL_DEVICE_TYPE_ACCELERATOR, NULL, NULL, &err);
			break;  
		case DEVICE_TYPE_DEFAULT:
			cl_context_id = clCreateContextFromType(NULL, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &err);
			break;
		case DEVICE_TYPE_ALL:
			cl_context_id = clCreateContextFromType(NULL, CL_DEVICE_TYPE_ALL, NULL, NULL, &err);
			break;
		default:
			printf("Error:Invalid Device Type \n");
			m_error_state = FAILURE;
			break;
	}

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PLATFORM:
				printf("Error: CL_INVALID_PLATFORM\n");
				m_error_state = INVALID_PLATFORM;
				break;
				//case CL_INVALID_PROPERTY:
				//	printf("CL_INVALID_PROPERTY  \n");
				//	m_error_state = INVALID_PROPERTY;
				//	break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
			case CL_DEVICE_NOT_AVAILABLE:
				printf("Error: CL_DEVICE_NOT_AVAILABLE\n");
				m_error_state = DEVICE_NOT_AVAILABLE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
				// TODO (siba samal) Error handling following Error Types
				//	case CL_INVALID_D3D10_DEVICE_KHR:
				//		printf("Error: CL_INVALID_D3D10_DEVICE_KHR \n");
				//		m_error_state = INVALID_D3D10_DEVICE_KHR;
				//		break;
				//		case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR :
				//			printf("Error: CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR\n");
				//			m_error_state = INVALID_GL_SHAREGROUP_REFERENCE_KHR;
				//			break;

			default:
				printf("Error: Invalid ERROR Type\n");
				m_error_state = FAILURE;
				break;

		}
	} else {
		RefPtr<WebCLContext> o = WebCLContext::create(this, cl_context_id);
		if (o != NULL) {
			m_error_state = SUCCESS;
			m_context = o;
			return o;
		} else {
			m_error_state = FAILURE;
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLContext> WebCLComputeContext::createSharedContext(int device_type, 
		int pfn_notify, int user_data)
{

	cl_int err = 0;
	cl_context cl_context_id = NULL;

	CGLContextObj kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);

	cl_context_properties properties[] = {
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
		(cl_context_properties)kCGLShareGroup, 0
	};
	// TODO (siba samal) Handle NULL parameters
	cl_context_id = clCreateContext(properties, 0, 0, 0, 0, &err);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PLATFORM:
				printf("Error: CL_INVALID_PLATFORM\n");
				m_error_state = INVALID_PLATFORM;
				break;
				//case CL_INVALID_PROPERTY:
				//	printf("Error: CL_INVALID_PROPERTY\n");
				//	m_error_state = INVALID_PROPERTY;
				//	break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE \n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
			case CL_DEVICE_NOT_AVAILABLE:
				printf("Error: CL_DEVICE_NOT_AVAILABLE\n");
				m_error_state = DEVICE_NOT_AVAILABLE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
				//case CL_INVALID_D3D10_DEVICE_KHR:
				//	printf("Error: CL_INVALID_D3D10_DEVICE_KHR\n");
				//	m_error_state = INVALID_D3D10_DEVICE_KHR;
				//	break;
				//case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR :
				//	printf("Error: CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR\n");
				//	m_error_state = INVALID_GL_SHAREGROUP_REFERENCE_KHR;
				//	break;

			default:
				printf("Error: Invalid ERROR type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLContext> o = WebCLContext::create(this, cl_context_id);
		if (o != NULL) {
			m_error_state = SUCCESS;
			m_context = o;
			return o;
		} else {
			m_error_state = FAILURE;
			printf("WebCLComputeContext::createSharedContext device_type=%d pfn_notify=%d user_data=%d\n",
					device_type, pfn_notify, user_data);
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLContext> WebCLComputeContext::createContext(int contextProperties, 
		WebCLDeviceID* device, int pfn_notify, int user_data)
{

	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_device_id cl_device = NULL;

	if (device != NULL) {
		cl_device = device->getCLDeviceID();
		if (cl_device == NULL) {
			printf("Error: devices null\n");
			return NULL;
		}
		m_device_id_ = device = NULL;
	} else {
		printf("Error: webcl_devices null\n");
		return NULL;
	}

	// TODO(won.jeon) - prop, pfn_notify, and user_data need to be addressed later
	cl_context_id = clCreateContext(NULL, 1, &cl_device, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PLATFORM:
				printf("Error: CL_INVALID_PLATFORM\n");
				m_error_state = INVALID_PLATFORM;
				break;
				//case CL_INVALID_PROPERTY:
				//	printf("Error: CL_INVALID_PROPERTY\n");
				//	m_error_state = INVALID_PROPERTY;
				//	break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
			case CL_DEVICE_NOT_AVAILABLE:
				printf("Error: CL_DEVICE_NOT_AVAILABLE\n");
				m_error_state = DEVICE_NOT_AVAILABLE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
				//case CL_INVALID_D3D10_DEVICE_KHR:
				//	printf("Error: CL_INVALID_D3D10_DEVICE_KHR\n");
				//	m_error_state = INVALID_D3D10_DEVICE_KHR;
				//	break;
				//case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR :
				//	printf("Error: CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR\n");
				//	m_error_state = INVALID_GL_SHAREGROUP_REFERENCE_KHR;
				//	break;

			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLContext> o = WebCLContext::create(this, cl_context_id);
		if (o != NULL) {
			m_error_state = SUCCESS;
			m_context = o;
			return o;
		} else {
			m_error_state = FAILURE;
			printf("WebCLComputeContext::createContext prop=%d pfn_notify=%d user_data=%d\n", 
					contextProperties, pfn_notify, user_data);
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLCommandQueue> WebCLComputeContext::createCommandQueue(WebCLContext* context_id, 
		WebCLDeviceIDList* devices, int command_queue_prop)
{
	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_device_id cl_device = NULL;
	cl_command_queue cl_command_queue_id = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}
	if (devices != NULL) {
		cl_device = devices->getCLDeviceIDs();
		if (cl_device == NULL) {
			printf("Error: cl_device null\n");
			return NULL;
		}
	}
	switch (command_queue_prop)
	{
		case QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
			cl_command_queue_id = clCreateCommandQueue(cl_context_id, cl_device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
			break;
		case QUEUE_PROFILING_ENABLE:
			cl_command_queue_id = clCreateCommandQueue(cl_context_id, cl_device, QUEUE_PROFILING_ENABLE, &err);
			break;
		default:
			printf("WebCLComputeContext::createCommandQueue prop (NULL/INVALID PROP)=%d\n", command_queue_prop);
			cl_command_queue_id = clCreateCommandQueue(cl_context_id, cl_device, NULL, &err);
			break;
	}
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT \n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE \n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE \n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_QUEUE_PROPERTIES:
				printf("Error: CL_INVALID_QUEUE_PROPERTIES \n");
				m_error_state = INVALID_QUEUE_PROPERTIES;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLCommandQueue> o = WebCLCommandQueue::create(this, cl_command_queue_id);
		if (o != NULL) {
			m_error_state = SUCCESS;
			m_command_queue = o;
			return o;
		} else {
			m_error_state = FAILURE;
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLCommandQueue> WebCLComputeContext::createCommandQueue(WebCLContext* context_id, 
		WebCLDeviceID* device, int command_queue_prop)
{
	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_device_id cl_device = NULL;
	cl_command_queue cl_command_queue_id = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}
	if (device != NULL) {
		cl_device = device->getCLDeviceID();
		if (cl_device == NULL) {
			printf("Error: cl_device null\n");
			return NULL;
		}
	}
	switch (command_queue_prop)
	{
		case QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
			cl_command_queue_id = clCreateCommandQueue(cl_context_id, cl_device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
			break;
		case QUEUE_PROFILING_ENABLE:
			cl_command_queue_id = clCreateCommandQueue(cl_context_id, cl_device, QUEUE_PROFILING_ENABLE, &err);
			break;
		default:
			printf("WebCLComputeContext::createCommandQueue prop (NULL/INVALID PROP)=%d\n", command_queue_prop);
			cl_command_queue_id = clCreateCommandQueue(cl_context_id, cl_device, NULL, &err);
			break;
	}

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT \n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE \n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE  \n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_QUEUE_PROPERTIES:
				printf("Error: CL_INVALID_QUEUE_PROPERTIES  \n");
				m_error_state = INVALID_QUEUE_PROPERTIES;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;

			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLCommandQueue> o = WebCLCommandQueue::create(this, cl_command_queue_id);
		if (o != NULL) {
			m_error_state = SUCCESS;
			m_command_queue = o;
			return o;
		} else {
			m_error_state = FAILURE;
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLProgram> WebCLComputeContext::createProgramWithSource(WebCLContext* context_id, 
		const String& kernelSource)
{

	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_program cl_program_id = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}

	const char* source = strdup(kernelSource.utf8().data());

	// TODO(won.jeon) - the second and fourth arguments need to be addressed later
	cl_program_id = clCreateProgramWithSource(cl_context_id, 1, (const char**)&source, 
			NULL, &err);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT \n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE \n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLProgram> o = WebCLProgram::create(this, cl_program_id);
		if (o != NULL) {
			m_program_list.append(o);
			m_num_programs++;
			m_error_state = SUCCESS;
			return o;
		} else {
			m_error_state = FAILURE;
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLProgram> WebCLComputeContext::createProgramWithBinary(WebCLContext* context_id,
		WebCLDeviceIDList* cl_devices,const String& kernelBinary)
{
	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_program cl_program_id = NULL;
	cl_device_id cl_device = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}
	if (cl_devices != NULL) {
		cl_device = cl_devices->getCLDeviceIDs();
		if (cl_device == NULL) {
			printf("Error: devices null\n");
			return NULL;
		}
	} else {
		printf("Error: webcl_devices null\n");
		return NULL;
	}

	const char* binary = strdup(kernelBinary.utf8().data());
	// TODO(siba samal) - length & binary_status arguments need to be addressed later
	cl_program_id = clCreateProgramWithBinary(cl_context_id, cl_devices->length(),(const cl_device_id*)&cl_device,
			NULL, (const unsigned char**)&binary, NULL, &err);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT \n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE \n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE  \n");
				m_error_state = INVALID_DEVICE ;
				break;
			case CL_INVALID_BINARY:	
				printf("Error: CL_INVALID_BINARY   \n");
				m_error_state = INVALID_BINARY;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;

			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLProgram> o = WebCLProgram::create(this, cl_program_id);
		if (o != NULL) {
			m_program_list.append(o);
			m_num_programs++;
			m_error_state = SUCCESS;
			return o;
		} else {
			m_error_state = FAILURE;
			return NULL;
		}
	}
	return NULL;
}

void WebCLComputeContext::buildProgram(WebCLProgram* program, int options, 
		int pfn_notify, int user_data)
{
	cl_int err = 0;
	cl_program program_id = NULL;

	if (program != NULL) {
		program_id = program->getCLProgram();
		if (program_id == NULL) {
			printf("Error: program_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}

	// TODO(won.jeon) - needs to be addressed later
	err = clBuildProgram(program_id, 0, NULL, NULL, NULL, NULL);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				m_error_state = INVALID_PROGRAM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_BINARY:
				printf("Error: CL_INVALID_BINARY\n");
				m_error_state = INVALID_BINARY;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
			case CL_INVALID_BUILD_OPTIONS :
				printf("Error: CL_INVALID_BUILD_OPTIONS\n");
				m_error_state = INVALID_BUILD_OPTIONS;
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				printf("Error: CL_COMPILER_NOT_AVAILABLE\n");
				m_error_state = COMPILER_NOT_AVAILABLE;
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				printf("Error: CL_BUILD_PROGRAM_FAILURE\n");
				m_error_state = BUILD_PROGRAM_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::buildProgram normal options=%d pfn_notify=%d user_data=%d\n", 
						options, pfn_notify, user_data);
				m_error_state = FAILURE;
				break;

		}

	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::buildProgram(WebCLProgram* program, WebCLDeviceID*   device_id,
		int options, int pfn_notify, int user_data)
{
	cl_int err = 0;
	cl_program program_id = NULL;
	cl_device_id cl_device = NULL;

	if (program != NULL) {
		program_id = program->getCLProgram();
		if (program_id == NULL) {
			printf("Error: program_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	cl_device = device_id->getCLDeviceID();
	if (cl_device == NULL) {
		printf("Error: devices null\n");
		m_error_state = FAILURE;
		return;
	}

	// TODO(siba samal) - NULL parameters needs to be addressed later
	err = clBuildProgram(program_id, 1, (const cl_device_id*)&cl_device, NULL, NULL, NULL);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				m_error_state = INVALID_PROGRAM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_BINARY:
				printf("Error: CL_INVALID_BINARY\n");
				m_error_state = INVALID_BINARY;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
			case CL_INVALID_BUILD_OPTIONS :
				printf("Error: CL_INVALID_BUILD_OPTIONS\n");
				m_error_state = INVALID_BUILD_OPTIONS;
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				printf("Error: CL_COMPILER_NOT_AVAILABLE\n");
				m_error_state = COMPILER_NOT_AVAILABLE;
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				printf("Error: CL_BUILD_PROGRAM_FAILURE\n");
				m_error_state = BUILD_PROGRAM_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::buildProgram WebCLDeviceID options=%d pfn_notify=%d user_data=%d\n", 
						options, pfn_notify, user_data);
				m_error_state = FAILURE;
				break;

		}

	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::buildProgram(WebCLProgram* program, WebCLDeviceIDList* cl_devices,
		int options, int pfn_notify, int user_data)
{
	cl_int err = 0;
	cl_program program_id = NULL;
	cl_device_id cl_device = NULL;

	if (program != NULL) {
		program_id = program->getCLProgram();
		if (program_id == NULL) {
			printf("Error: program_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if (cl_devices != NULL) {
		cl_device = cl_devices->getCLDeviceIDs();
		if (cl_device == NULL) {
			printf("Error: devices null\n");
			return;
		}
	} else {
		printf("Error: webcl_devices null\n");
		return;
	}

	// TODO(siba samal) - NULL parameters needs to be addressed later
	err = clBuildProgram(program_id, cl_devices->length(), (const cl_device_id*)&cl_device, NULL, NULL, NULL);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				m_error_state = INVALID_PROGRAM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				m_error_state = INVALID_DEVICE;
				break;
			case CL_INVALID_BINARY:
				printf("Error: CL_INVALID_BINARY\n");
				m_error_state = INVALID_BINARY;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
			case CL_INVALID_BUILD_OPTIONS :
				printf("Error: CL_INVALID_BUILD_OPTIONS\n");
				m_error_state = INVALID_BUILD_OPTIONS;
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				printf("Error: CL_COMPILER_NOT_AVAILABLE\n");
				m_error_state = COMPILER_NOT_AVAILABLE;
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				printf("Error: CL_BUILD_PROGRAM_FAILURE\n");
				m_error_state = BUILD_PROGRAM_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::buildProgram WebCLDeviceIDList  options=%d pfn_notify=%d user_data=%d\n", 
						options, pfn_notify, user_data);
				m_error_state = FAILURE;
				break;

		}

	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

PassRefPtr<WebCLKernel> WebCLComputeContext::createKernel(WebCLProgram* program, 
		const String& kernel_name)
{
	cl_int err = 0;
	cl_program cl_program_id = NULL;
	cl_kernel cl_kernel_id = NULL;

	if (program != NULL) {
		cl_program_id = program->getCLProgram();
		if (cl_program_id == NULL) {
			printf("Error: cl_program_id null\n");
			return NULL;
		}
	}

	// TODO(won.jeon) - more detailed error code need to be addressed later
	const char* kernel_name_str = strdup(kernel_name.utf8().data());
	cl_kernel_id = clCreateKernel(cl_program_id, kernel_name_str, &err);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				m_error_state = INVALID_PROGRAM;
				break;
			case CL_INVALID_PROGRAM_EXECUTABLE:
				printf("Error: CL_INVALID_PROGRAM_EXECUTABLE\n");
				m_error_state = INVALID_PROGRAM_EXECUTABLE;
				break;
			case CL_INVALID_KERNEL_NAME:
				printf("Error: CL_INVALID_KERNEL_NAME\n");
				m_error_state = INVALID_KERNEL_NAME;
				break;
			case CL_INVALID_KERNEL_DEFINITION:
				printf("Error: CL_INVALID_KERNEL_DEFINITION\n");
				m_error_state = INVALID_KERNEL_DEFINITION;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLKernel> o = WebCLKernel::create(this, cl_kernel_id);
		m_kernel_list.append(o);
		m_num_kernels++;
		m_error_state = SUCCESS;

		return o;
	}
	return NULL;
}

PassRefPtr<WebCLKernelList> WebCLComputeContext::createKernelsInProgram(WebCLProgram* program)
{
	cl_int err = 0;
	cl_program cl_program_id = NULL;
	cl_kernel* kernelBuf = NULL;
	cl_uint num = 0;

	if (program != NULL) {
		cl_program_id = program->getCLProgram();
		if (cl_program_id == NULL) {
			printf("Error: cl_program_id null\n");
			return NULL;
		}
	}

	err = clCreateKernelsInProgram (cl_program_id, NULL, NULL, &num);
	if (err != CL_SUCCESS) {
		printf("Error: clCreateKernelsInProgram \n");
		m_error_state = FAILURE;
		return NULL;
	}
	if(num == 0) {
		printf("Warning: createKernelsInProgram - Number of Kernels is 0 \n");
		m_error_state = FAILURE;
		return NULL;
	}

	kernelBuf = (cl_kernel*)malloc (sizeof(cl_kernel) * num);
	if (!kernelBuf) {
		m_error_state = OUT_OF_HOST_MEMORY;	
		return NULL;
	}

	err = clCreateKernelsInProgram (cl_program_id, num, kernelBuf, NULL);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				m_error_state = INVALID_PROGRAM;
				break;
			case CL_INVALID_PROGRAM_EXECUTABLE:
				printf("Error: CL_INVALID_PROGRAM_EXECUTABLE\n");
				m_error_state = INVALID_PROGRAM_EXECUTABLE;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLKernelList> o = WebCLKernelList::create(this, kernelBuf, num);
		printf("WebCLKernelList Size = %d \n\n\n\n", num);
		//m_kernel_list = o;
		m_num_kernels = num;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createBuffer(WebCLContext* context_id, 
		int flags, int size, int host_ptr)
{
	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_mem cl_mem_id = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}
	// TODO(won.jeon) - NULL parameter needs to be addressed later
	switch (flags)
	{
		case MEM_READ_ONLY:
			cl_mem_id = clCreateBuffer(cl_context_id, CL_MEM_READ_ONLY, size, NULL, &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_id = clCreateBuffer(cl_context_id, CL_MEM_WRITE_ONLY, size, NULL, &err);
			break;
		case MEM_READ_WRITE:
			cl_mem_id = clCreateBuffer(cl_context_id, CL_MEM_READ_WRITE, size, NULL, &err);
			break;
		default:
			printf("Error: Unsupported Mem Flsg\n");
			printf("WebCLComputeContext::createBuffer host_ptr = %d\n", host_ptr);
			m_error_state = INVALID_CONTEXT;
			break;
	}
	if (err != CL_SUCCESS) {
		printf("Error: clCreateBuffer\n");
		switch(err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_BUFFER_SIZE:
				printf("Error: CL_INVALID_BUFFER_SIZE\n");
				m_error_state = INVALID_BUFFER_SIZE;
				break;
			case CL_INVALID_HOST_PTR:
				printf("Error: CL_INVALID_HOST_PTR\n");
				m_error_state = INVALID_HOST_PTR;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_id, false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueWriteBuffer(WebCLCommandQueue* command_queue, 
		WebCLMem* mem, bool blocking_write, int offset, int buffer_size, 
		Float32Array* ptr, int event_wait_list)
{

	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			return NULL;
		}
	}
	if (mem != NULL) {
		cl_mem_id = mem->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			return NULL;
		}
	}
	// TODO(won.jeon) - NULL parameters need to be addressed later
	if (blocking_write) {
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE, offset, 
					buffer_size, ptr->data(), event_wait_list, NULL, &cl_event_id);
    } else {
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_FALSE, offset, 
					buffer_size, ptr->data(), event_wait_list, NULL, &cl_event_id);
	}
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueWriteBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
				//case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				//	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
				//	m_error_state = MISALIGNED_SUB_BUFFER_OFFSET;
				//	break;
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
				//	m_error_state = EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
				//	break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::enqueueWriteBuffer offset=%d buffer_size=%d event_wait_list=%d\n", 
						offset, buffer_size, event_wait_list);
				m_error_state = FAILURE;
				break;

		}

	} else {
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

#if 0
PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueWriteBuffer(WebCLCommandQueue* command_queue,
		WebCLMem* mem, bool blocking_write, int offset, int buffer_size,
		ImageData* ptr, int event_wait_list)
{
	printf("WebCLComputeContext::enqueueWriteBuffer(ImageData) offset=%d buffer_size=%d event_wait_list=%d\n",
			offset, buffer_size, event_wait_list);

	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			return NULL;
		}
	}
	if (mem != NULL) {
		cl_mem_id = mem->getCLMem();
		//cl_mem cl_mem_id = (m_mem_list[0].get())->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			return NULL;
		}
	}

	// TODO(won.jeon) - NULL parameters need to be addressed later
	switch(blocking_write) {
		case true:
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE, 
					offset, buffer_size, ptr->data()->data()->data(), event_wait_list, NULL, &cl_event_id);
			break;
		case false:
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_FALSE, 
					offset, buffer_size, ptr->data()->data()->data(), event_wait_list, NULL, &cl_event_id);
			break;
	}
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueWriteBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				break;
			default:
				m_error_state = FAILURE;
				printf("Error: Invaild Error Type\n");
				break;
		}

	} else {
		printf("Success: clEnqueueWriteBuffer\n");
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		printf("m_num_events=%ld\n", m_num_events);
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}
#endif
PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueWriteBuffer(WebCLCommandQueue* command_queue, 
		WebCLMem* mem, bool blocking_write, int offset, int buffer_size, 
		Int32Array* ptr, int event_wait_list)
{

	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			return NULL;
		}
	}
	if (mem != NULL) {
		cl_mem_id = mem->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			return NULL;
		}


	}

	// TODO(won.jeon) - NULL parameters need to be addressed later
	if(blocking_write) {
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE, 
					offset, buffer_size, ptr->data(), event_wait_list, NULL, &cl_event_id);
    } else {
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_FALSE, 
					offset, buffer_size, ptr->data(), event_wait_list ,NULL, &cl_event_id);
	}
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueWriteBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
				//case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				//	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
				//	m_error_state = MISALIGNED_SUB_BUFFER_OFFSET;
				//	break;
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
				//	m_error_state = EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
				//	break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::enqueueWriteBuffer offset=%d buffer_size=%d event_wait_list=%d\n", 
						offset, buffer_size, event_wait_list);
				m_error_state = FAILURE;
				break;
		}

	} else {
		printf("Success: clEnqueueWriteBuffer\n");
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueWriteBuffer(WebCLCommandQueue* command_queue, 
		WebCLMem* mem, bool blocking_write, int offset, int buffer_size, 
		Uint8Array* ptr, int event_wait_list)
{

	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			return NULL;
		}
	}
	if (mem != NULL) {
		cl_mem_id = mem->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			return NULL;
		}


	}

	// TODO(won.jeon) - NULL parameters need to be addressed later
	if (blocking_write) {
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE, 
					offset, buffer_size, ptr->data(), event_wait_list, NULL, &cl_event_id);
    } else {
			err = clEnqueueWriteBuffer(cl_command_queue_id, cl_mem_id, CL_FALSE, 
					offset, buffer_size, ptr->data(),event_wait_list, NULL, &cl_event_id);
	}
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueWriteBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
				//case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				//	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
				//	m_error_state = MISALIGNED_SUB_BUFFER_OFFSET;
				//	break;
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
				//	m_error_state = EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
				//	break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::enqueueWriteBuffer offset=%d buffer_size=%d event_wait_list=%d\n", 
						offset, buffer_size, event_wait_list);
				m_error_state = FAILURE;
				break;

		}
	} else {
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLEvent>  WebCLComputeContext::enqueueReadBuffer(WebCLCommandQueue* command_queue, 
		WebCLMem* mem, bool blocking_read, int offset, int buffer_size, 
		Float32Array* ptr, int event_wait_list)
{

	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			return NULL;
		}
	}
	if (mem != NULL) {
		cl_mem_id = mem->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			return NULL;
		}


	}
	// TODO(won.jeon) - NULL parameters need to be addressed later
	if(blocking_read) {
			err = clEnqueueReadBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE, 
					offset, buffer_size, ptr->data(),event_wait_list, NULL,&cl_event_id);
    } else {
			err = clEnqueueReadBuffer(cl_command_queue_id, cl_mem_id, CL_FALSE, 
					offset, buffer_size, ptr->data(),event_wait_list, NULL, &cl_event_id);
	}
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueReadBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
				//case CL_MISALIGNED_SUB_BUFFER_OFFSET :
				//	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
				//	m_error_state = MISALIGNED_SUB_BUFFER_OFFSET;
				//	break;
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  :
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  \n");
				//	m_error_state = EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
				//	break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::enqueueReadBuffer(Float32Array) offset=%d buffer_size=%d event_wait_list=%d\n", offset, buffer_size, event_wait_list);
				m_error_state = FAILURE;
				break;

		}

	} else {
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

#if 0
PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueReadBuffer(WebCLCommandQueue* command_queue, 
		WebCLMem* mem, bool blocking_read, int offset, int buffer_size, 
		HTMLCanvasElement* canvas_element, int event_wait_list)
{
	// TODO(won.jeon) - return type temporarily set to void (instead of WebCLEvent)
	printf("WebCLComputeContext::enqueueReadBuffer(HTMLCanvasElement) offset=%d buffer_size=%d event_wait_list=%d\n", 
			offset, buffer_size, event_wait_list);

	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;
	CanvasRenderingContext2D* rendering_context = NULL;
	RefPtr<ImageData> image_data = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			return NULL;
		}
	}
	if (mem != NULL) {
		cl_mem_id = mem->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			return NULL;
		}
	}
	if (canvas_element != NULL) {
		//graphics_context = canvas_element->drawingContext();
		rendering_context = static_cast<CanvasRenderingContext2D*>(canvas_element->renderingContext());

	} else {
		printf("Error: canvas_element == NULL\n");
		return NULL;
	}

	unsigned char* tmp = (unsigned char*)malloc(buffer_size*sizeof(unsigned char));
	// TODO(won.jeon) - NULL parameters need to be addressed later
	switch(blocking_read) {
		case true:
			//err = clEnqueueReadBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE, 
			//		offset, buffer_size, image_data->data()->data()->data(), 0, NULL, NULL);
			err = clEnqueueReadBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE,
					offset, buffer_size, tmp,event_wait_list, NULL, &cl_event_id);
			break;
		case false:
			err = clEnqueueReadBuffer(cl_command_queue_id, cl_mem_id, CL_FALSE, 
					offset, buffer_size, image_data->data()->data()->data(),event_wait_list, NULL, &cl_event_id);
			break;
	}
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				break;
			default:
				printf("Error: Invaild Error Type\n");
				break;
		}
		m_error_state = FAILURE;

	} else {
		printf("Success: clEnqueueReadBuffer\n");

		for (int i = 0; i < buffer_size; i++) {
			//printf("%u ", image_data->data()->data()->get(i));
			printf("%u ", tmp[i]);
			if ((i+1) % 4 == 0) {
				printf("\n");
			}
		}
		printf("\n");
		//ExceptionCode ec = 0;
		RefPtr<ByteArray> array = ByteArray::create(buffer_size);
		for (int i = 0; i < buffer_size; i++) {
			//array->set(i, (unsigned char)tmp[i]);
			array->set(i, (unsigned char)128);
			if ((i+1) % 4 == 0)
				array->set(i, (unsigned char)127);
		}
		image_data = ImageData::create(IntSize(canvas_element->width(), canvas_element->height()), array);	
		//rendering_context->drawImage(image_data.get(), 0.0, 0.0, ec);
		//rendering_context->paintRenderingResultsToCanvas();
		//GraphicsContext* graphics_context = canvas_element->drawingContext();
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}
#endif
PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueReadBuffer(WebCLCommandQueue* command_queue, 
		WebCLMem* mem, bool blocking_read, int offset, int buffer_size, 
		Int32Array* ptr, int event_wait_list)
{

	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			return NULL;
		}
	}
	if (mem != NULL) {
		cl_mem_id = mem->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			return NULL;
		}
	}
	// TODO(won.jeon) - NULL parameters need to be addressed later
	if(blocking_read) {
			err = clEnqueueReadBuffer(cl_command_queue_id, cl_mem_id, CL_TRUE, 
					offset, buffer_size, ptr->data(), event_wait_list, NULL, &cl_event_id);
    } else {
			err = clEnqueueReadBuffer(cl_command_queue_id, cl_mem_id, CL_FALSE, 
					offset, buffer_size, ptr->data(), event_wait_list, NULL,&cl_event_id);
	}
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueReadBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: C_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
				//case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				//	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
				//	m_error_state = MISALIGNED_SUB_BUFFER_OFFSET;
				//	break;
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST \n");
				//	m_error_state = EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
				//	break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::enqueueReadBuffer offset=%d buffer_size=%d event_wait_list=%d\n", 
						offset, buffer_size, event_wait_list);
				m_error_state = FAILURE;
				break;
		}
	} else {
		printf("ptr->data() [%d][%d][%d][%d][%d]\n", (ptr->data())[0], (ptr->data())[1], 
				(ptr->data())[2], (ptr->data())[3], (ptr->data())[4]);
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}
void WebCLComputeContext::setKernelArg(WebCLKernel* kernel, unsigned int argIndex, 
		PassRefPtr<WebCLKernelTypeValue> kernelObject, int argType)
{
	cl_int err = 0;
	cl_kernel cl_kernel_id = NULL;
	RefPtr<WebCLKernelTypeVector> array = NULL;
	char nodeIdCh = NULL;
	unsigned char nodeIduCh = NULL;
	short nodeIdSh = 0;
	unsigned short nodeIduSh = 0;
	int nodeIdInt = 0;
	unsigned int nodeIduInt = 0;
	long nodeIdLong = 0;
	long nodeIduLong = 0;
	float nodeIdFloat = 0;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return ;
		}
	}
	// TODO (siba samal) argType & argIndex Validation
	if(kernelObject == NULL)
	{
		printf("Error: kernelObject null\n");
		m_error_state = FAILURE;
		return ;
	}
	switch (argType) {
		case KERNEL_ARG_CHAR:
			printf("CL_KERNEL_ARG_CHAR\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIdCh,
					argIndex, sizeof(cl_char));		
			break;
		case KERNEL_ARG_UCHAR:
			printf("CL_KERNEL_ARG_UCHAR\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIduCh,
					argIndex, sizeof(cl_uchar));								
			break;
		case KERNEL_ARG_SHORT:
			printf("CL_KERNEL_ARG_SHORT\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIdSh,
					argIndex, sizeof(cl_short));				
			break;
		case KERNEL_ARG_USHORT:
			printf("CL_KERNEL_ARG_USHORT\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIduSh,
					argIndex, sizeof(cl_ushort));				
			break;
		case KERNEL_ARG_INT:
			printf("CL_KERNEL_ARG_INT\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIdInt,
					argIndex, sizeof(cl_int));							
			break;
		case KERNEL_ARG_UINT:
			printf("CL_KERNEL_ARG_UINT\n");

			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIduInt,
					argIndex, sizeof(cl_uint));				
			break;
		case KERNEL_ARG_LONG:
			printf("CL_KERNEL_ARG_LONG\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIdLong,
					argIndex, sizeof(cl_long));
			break;
		case KERNEL_ARG_ULONG:
			printf("CL_KERNEL_ARG_ULONG\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIduLong,
					argIndex, sizeof(cl_ulong));
			break;
		case KERNEL_ARG_FLOAT:
			printf("CL_KERNEL_ARG_FLOAT\n");
			err = clSetKernelArgPrimitiveType(cl_kernel_id, kernelObject, nodeIdFloat,
					argIndex, sizeof(cl_float));									
			break;
		case KERNEL_ARG_CHAR2:				
			printf("CL_KERNEL_ARG_CHAR2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_char2), 2);									
			break;
		case KERNEL_ARG_UCHAR2:				
			printf("CL_KERNEL_ARG_UCHAR2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uchar2), 2);									
			break;
		case KERNEL_ARG_SHORT2:
			printf("CL_KERNEL_ARG_SHORT2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_short2), 2);		
			break;
		case KERNEL_ARG_USHORT2:
			printf("CL_KERNEL_ARG_USHORT2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ushort2), 2);							
			break;
		case KERNEL_ARG_INT2:
			printf("CL_KERNEL_ARG_INT2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_int2), 2);	
			break;
		case KERNEL_ARG_UINT2:
			printf("CL_KERNEL_ARG_INT2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uint2), 2);					
			break;
		case KERNEL_ARG_LONG2:
			printf("CL_KERNEL_ARG_LONG2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_long2), 2);					
			break;
		case KERNEL_ARG_ULONG2:
			printf("CL_KERNEL_ARG_ULONG2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ulong2), 2);	
			break;
		case KERNEL_ARG_FLOAT2:
			printf("CL_KERNEL_ARG_FLOAT2\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_float2), 2);
			break;
		case KERNEL_ARG_CHAR3:
			printf("CL_KERNEL_ARG_CHAR3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_char4), 3);
			break;
		case KERNEL_ARG_UCHAR3:
			printf("CL_KERNEL_ARG_UCHAR3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uchar4), 3);					
			break;
		case KERNEL_ARG_SHORT3:
			printf("CL_KERNEL_ARG_SHORT3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_short4), 3);		
			break;
		case KERNEL_ARG_USHORT3:
			printf("CL_KERNEL_ARG_USHORT3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ushort4), 3);							
			break;
		case KERNEL_ARG_INT3:
			printf("CL_KERNEL_ARG_INT3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_int4), 3);	
			break;
		case KERNEL_ARG_UINT3:
			printf("CL_KERNEL_ARG_INT3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uint4), 3);									
			break;
		case KERNEL_ARG_LONG3:
			printf("CL_KERNEL_ARG_LONG3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_long4), 3);					
			break;
		case KERNEL_ARG_ULONG3:
			printf("CL_KERNEL_ARG_ULONG3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ulong4), 3);					
			break;
		case KERNEL_ARG_FLOAT3:
			printf("CL_KERNEL_ARG_FLOAT3\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_float4), 3);
			break;	
		case KERNEL_ARG_CHAR4:
			printf("CL_KERNEL_ARG_CHAR4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_char4), 4);
			break;
		case KERNEL_ARG_UCHAR4:
			printf("CL_KERNEL_ARG_UCHAR4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uchar4), 4);					
			break;
		case KERNEL_ARG_SHORT4:
			printf("CL_KERNEL_ARG_SHORT4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_short4), 4);		
			break;
		case KERNEL_ARG_USHORT4:
			printf("CL_KERNEL_ARG_USHORT4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ushort4), 4);							
			break;
		case KERNEL_ARG_INT4:
			printf("CL_KERNEL_ARG_INT4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_int4), 4);	
			break;
		case KERNEL_ARG_UINT4:
			printf("CL_KERNEL_ARG_INT4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uint4), 4);									
			break;
		case KERNEL_ARG_LONG4:
			printf("CL_KERNEL_ARG_LONG4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_long4), 4);					
			break;
		case KERNEL_ARG_ULONG4:
			printf("CL_KERNEL_ARG_ULONG4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ulong4), 4);					
			break;
		case KERNEL_ARG_FLOAT4:
			printf("CL_KERNEL_ARG_FLOAT4\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_float4), 4);
			break;		
		case KERNEL_ARG_CHAR8:
			printf("CL_KERNEL_ARG_CHAR8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_char8), 8);
			break;
		case KERNEL_ARG_UCHAR8:
			printf("CL_KERNEL_ARG_UCHAR8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uchar8), 8);					
			break;
		case KERNEL_ARG_SHORT8:
			printf("CL_KERNEL_ARG_SHORT8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_short8), 8);		
			break;
		case KERNEL_ARG_USHORT8:
			printf("CL_KERNEL_ARG_USHORT8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ushort8), 8);							
			break;
		case KERNEL_ARG_INT8:
			printf("CL_KERNEL_ARG_INT8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_int8), 8);	
			break;
		case KERNEL_ARG_UINT8:
			printf("CL_KERNEL_ARG_INT8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uint8), 8);									
			break;
		case KERNEL_ARG_LONG8:
			printf("CL_KERNEL_ARG_LONG8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_long8), 8);					
			break;
		case KERNEL_ARG_ULONG8:
			printf("CL_KERNEL_ARG_ULONG8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ulong8), 8);					
			break;
		case KERNEL_ARG_FLOAT8:
			printf("CL_KERNEL_ARG_FLOAT8\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_float8), 8);
			break;	
		case KERNEL_ARG_CHAR16:
			printf("CL_KERNEL_ARG_CHAR16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_char16), 16);
			break;
		case KERNEL_ARG_UCHAR16:
			printf("CL_KERNEL_ARG_UCHAR16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uchar16), 16);					
			break;
		case KERNEL_ARG_SHORT16:
			printf("CL_KERNEL_ARG_SHORT16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_short16), 16);		
			break;
		case KERNEL_ARG_USHORT16:
			printf("CL_KERNEL_ARG_USHORT16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ushort16), 16);							
			break;
		case KERNEL_ARG_INT16:
			printf("CL_KERNEL_ARG_INT16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_int16), 16);	
			break;
		case KERNEL_ARG_UINT16:
			printf("CL_KERNEL_ARG_INT16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_uint16), 16);									
			break;
		case KERNEL_ARG_LONG16:
			printf("CL_KERNEL_ARG_LONG16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_long16), 16);					
			break;
		case KERNEL_ARG_ULONG16:
			printf("CL_KERNEL_ARG_ULONG16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_ulong16), 16);					
			break;
		case KERNEL_ARG_FLOAT16:
			printf("CL_KERNEL_ARG_FLOAT16\n");
			err = clSetKernelArgVectorType(cl_kernel_id, kernelObject, array,
					argIndex, sizeof(cl_float16), 16);
			break;	
		case KERNEL_ARG_SAMPLER:
			printf("CL_KERNEL_ARG_SAMPLER - Not Handled\n");
			m_error_state = FAILURE;
			break;			
		default:
			printf("Error: Invaild Kernel Argument Type\n");
			m_error_state = FAILURE;
			return;
	}
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_KERNEL:
				printf("Error: CL_INVALID_KERNEL \n");
				m_error_state = INVALID_KERNEL;
				break;
			case CL_INVALID_ARG_INDEX:
				printf("Error: CL_INVALID_ARG_INDEX \n");
				m_error_state = INVALID_ARG_INDEX;
				break;
			case CL_INVALID_ARG_VALUE:
				printf("Error: CL_INVALID_ARG_VALUE \n");
				m_error_state = INVALID_ARG_VALUE;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT  \n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_SAMPLER:
				printf("Error: CL_INVALID_SAMPLER  \n");
				m_error_state = INVALID_SAMPLER;
				break;
			case CL_INVALID_ARG_SIZE:
				printf("Error: CL_INVALID_ARG_SIZE  \n");
				m_error_state = INVALID_ARG_SIZE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	}
	else
	{
		m_error_state = SUCCESS;			
	}
}

void WebCLComputeContext::setKernelArgGlobal(WebCLKernel* kernel, unsigned int arg_index, WebCLMem* arg_value)
{
	cl_int err = 0;
	cl_kernel cl_kernel_id = NULL;
	cl_mem cl_mem_id = NULL;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if (arg_value != NULL) {
		cl_mem_id = arg_value->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clSetKernelArg(cl_kernel_id, arg_index, sizeof(cl_mem), &cl_mem_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_KERNEL:
				printf("Error: CL_INVALID_KERNEL \n");
				m_error_state = INVALID_KERNEL;
				break;
			case CL_INVALID_ARG_INDEX:
				printf("Error: CL_INVALID_ARG_INDEX \n");
				m_error_state = INVALID_ARG_INDEX;
				break;
			case CL_INVALID_ARG_VALUE:
				printf("Error: CL_INVALID_ARG_VALUE \n");
				m_error_state = INVALID_ARG_VALUE;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT  \n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_SAMPLER:
				printf("Error: CL_INVALID_SAMPLER  \n");
				m_error_state = INVALID_SAMPLER;
				break;
			case CL_INVALID_ARG_SIZE:
				printf("Error: CL_INVALID_ARG_SIZE  \n");
				m_error_state = INVALID_ARG_SIZE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		printf("Success: clSetKernelArg Global -\n");
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::setKernelArgConstant(WebCLKernel* kernel, unsigned int arg_index, WebCLMem* arg_value)
{
	cl_int err = 0;
	cl_kernel cl_kernel_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_device_id cl_device = NULL;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if (arg_value != NULL) {
		cl_mem_id = arg_value->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if (m_device_id_ != NULL) {
		cl_device = m_device_id_->getCLDeviceID();
		cl_ulong max_buffer_size = 0;
		clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &max_buffer_size, NULL);
		cl_uint max_args = 0;
		clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &max_args,NULL);
		// Check for __constant qualifier restrictions
		if ( (sizeof(cl_mem) <= max_buffer_size)  && (arg_index <= max_args))
		{
			err = clSetKernelArg(cl_kernel_id, arg_index, sizeof(cl_mem), &cl_mem_id);
			if (err != CL_SUCCESS) {
				switch (err) {
					case CL_INVALID_KERNEL:
						printf("Error: CL_INVALID_KERNEL \n");
						m_error_state = INVALID_KERNEL;
						break;
					case CL_INVALID_ARG_INDEX:
						printf("Error: CL_INVALID_ARG_INDEX \n");
						m_error_state = INVALID_ARG_INDEX;
						break;
					case CL_INVALID_ARG_VALUE:
						printf("Error: CL_INVALID_ARG_VALUE \n");
						m_error_state = INVALID_ARG_VALUE;
						break;
					case CL_INVALID_MEM_OBJECT:
						printf("Error: CL_INVALID_MEM_OBJECT  \n");
						m_error_state = INVALID_MEM_OBJECT;
						break;
					case CL_INVALID_SAMPLER:
						printf("Error: CL_INVALID_SAMPLER  \n");
						m_error_state = INVALID_SAMPLER;
						break;
					case CL_INVALID_ARG_SIZE:
						printf("Error: CL_INVALID_ARG_SIZE  \n");
						m_error_state = INVALID_ARG_SIZE;
						break;
					case CL_OUT_OF_RESOURCES:
						printf("Error: CL_OUT_OF_RESOURCES\n");
						m_error_state = OUT_OF_RESOURCES;
						break;
					case CL_OUT_OF_HOST_MEMORY:
						printf("Error: CL_OUT_OF_HOST_MEMORY\n");
						m_error_state = OUT_OF_HOST_MEMORY;
						break;
					default:
						printf("Error: Invaild Error Type\n");
						m_error_state = FAILURE;
						break;
				}

				return;
			} else {
				printf("Success: clSetKernelArg - Constant\n");
				m_error_state = SUCCESS;
				return;
			}
		}
	}
	return;
}

void WebCLComputeContext::setKernelArgLocal(WebCLKernel* kernel, unsigned int arg_index, 
		unsigned int arg_size)
{
	cl_int err = 0;
	cl_kernel cl_kernel_id = NULL;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}

	err = clSetKernelArg(cl_kernel_id, arg_index, arg_size, NULL);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_KERNEL:
				printf("Error: CL_INVALID_KERNEL \n");
				m_error_state = INVALID_KERNEL;
				break;
			case CL_INVALID_ARG_INDEX:
				printf("Error: CL_INVALID_ARG_INDEX \n");
				m_error_state = INVALID_ARG_INDEX;
				break;
			case CL_INVALID_ARG_VALUE:
				printf("Error: CL_INVALID_ARG_VALUE \n");
				m_error_state = INVALID_ARG_VALUE;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT  \n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_SAMPLER:
				printf("Error: CL_INVALID_SAMPLER  \n");
				m_error_state = INVALID_SAMPLER;
				break;
			case CL_INVALID_ARG_SIZE:
				printf("Error: CL_INVALID_ARG_SIZE  \n");
				m_error_state = INVALID_ARG_SIZE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

		return;
	} else {
		printf("Success: clSetKernelArg - Local\n");
		m_error_state = SUCCESS;
		return;
	}
}

// TODO (siba samal) Is this API is needed??
#if 0
unsigned long WebCLComputeContext::getKernelWorkGroupInfo(WebCLKernel* kernel, WebCLDeviceIDList* devices, int param_name)
{
	cl_int err = 0;
	cl_kernel cl_kernel_id = NULL;
	cl_device_id cl_device = NULL;
	size_t ret = 0;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (devices != NULL) {
		cl_device = devices->getCLDeviceIDs();
		if (cl_device == NULL) {
			printf("Error: cl_device null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	switch (param_name) {
		case KERNEL_WORK_GROUP_SIZE:
			err = clGetKernelWorkGroupInfo(cl_kernel_id, cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ret), &ret, NULL);
			if (err != CL_SUCCESS) {
				switch (err) {
					case CL_INVALID_DEVICE:
						printf("Error: CL_INVALID_DEVICE\n");
						m_error_state = INVALID_DEVICE;
						break;
					case CL_INVALID_VALUE:
						printf("Error: CL_INVALID_VALUE\n");
						m_error_state = INVALID_VALUE;
						break;
					case CL_INVALID_KERNEL:
						m_error_state = INVALID_KERNEL;
						printf("Error: CL_INVALID_KERNEL\n");
						break;
					default:
						printf("Error: Invaild Error Type\n");
						m_error_state = FAILURE;
						break;
				}

			} else {
				m_error_state = SUCCESS;

				return ret;
			}
			break;
		default:
			break;
	}
	return NULL;
}
#endif

#if 0
// NOTE(won.jeon)
// it only works with integer-type of global/local workgroup size
// it does not work with vector-type of global/local workgroup size
// it will be deprecated
PassRefPtr<WebCLEvent>  WebCLComputeContext::enqueueNDRangeKernel(WebCLCommandQueue* command_queue, 
		WebCLKernel* kernel, unsigned int work_dim, unsigned int global_work_offset, 
		unsigned int global_work_size, unsigned int local_work_size, int event_wait_list)
{
	printf("WebCLComputeContext::enqueueNDRangeKernel(int) event_wait_list=%d\n", event_wait_list);

	cl_int err = 0;
	cl_command_queue cl_command_queue_id = NULL;
	cl_kernel cl_kernel_id = NULL;
	cl_event cl_event_id = NULL;

	size_t g_work_offset = global_work_offset;
	size_t g_work_size = global_work_size;
	size_t l_work_size = local_work_size;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_comamnd_queue_id null\n");
			return NULL;
		}
	}
	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			return NULL;
		}
	}

	err = clEnqueueNDRangeKernel(cl_command_queue_id, cl_kernel_id, work_dim, 
			&g_work_offset, &g_work_size, &l_work_size, event_wait_list, NULL, &cl_event_id);
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueNDRangeKernel\n");
		m_error_state = FAILURE;
		return NULL;
	} else {
		printf("Success: clEnqueueNDRangeKernel\n");
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		printf("m_num_events=%ld\n", m_num_events);
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;	

}
#endif
PassRefPtr<WebCLEvent>  WebCLComputeContext::enqueueNDRangeKernel(WebCLCommandQueue* command_queue, 
		WebCLKernel* kernel, unsigned int work_dim, unsigned int global_work_offset, 
		Int32Array* global_work_size, Int32Array* local_work_size, int event_wait_list)
{

	cl_int err = 0;
	cl_command_queue cl_command_queue_id = NULL;
	cl_kernel cl_kernel_id = NULL;
	cl_event cl_event_id = NULL;

	size_t g_work_offset = global_work_offset;
	size_t *g_work_size = NULL;
	size_t *l_work_size = NULL;
	//size_t g1_work_size;
	//size_t l1_work_size;
	m_error_state = SUCCESS;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_comamnd_queue_id null\n");
			return NULL;
		}
	}
	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			printf("WebCLComputeContext::enqueueNDRangeKernel(Int32Array) event_wait_list=%d\n", event_wait_list);
			return NULL;
		}
	}

	g_work_size = (size_t*)malloc(global_work_size->length() * sizeof(size_t));
	for (unsigned int i = 0; i < global_work_size->length(); i++) {
		g_work_size[i] = global_work_size->item(i);
	}
	l_work_size = (size_t*)malloc(local_work_size->length() * sizeof(size_t));
	for (unsigned int i = 0; i < local_work_size->length(); i++) {
		l_work_size[i] = local_work_size->item(i);
	}

	printf("Size of g_work_size & l_work_size = %d %d\n", global_work_size->length(), local_work_size->length());
	err = clEnqueueNDRangeKernel(cl_command_queue_id, cl_kernel_id, work_dim, 
			&g_work_offset, g_work_size, l_work_size, event_wait_list, NULL, &cl_event_id);

	if (err != CL_SUCCESS) {
		switch(err) {
			case CL_INVALID_PROGRAM_EXECUTABLE:
				printf("Error: CL_INVALID_PROGRAM_EXECUTABLE\n");
				m_error_state = INVALID_PROGRAM_EXECUTABLE;
				break;
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_KERNEL:
				printf("Error: CL_INVALID_KERNEL\n");
				m_error_state = INVALID_KERNEL;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_KERNEL_ARGS:
				printf("Error: CL_INVALID_KERNEL_ARGS\n");
				m_error_state = INVALID_KERNEL_ARGS;
				break;
			case CL_INVALID_WORK_DIMENSION:
				printf("Error: CL_INVALID_WORK_DIMENSION\n");
				m_error_state = INVALID_WORK_DIMENSION;
				break;
				//case CL_INVALID_GLOBAL_WORK_SIZE:
				//	printf("Error: CL_INVALID_GLOBAL_WORK_SIZE\n");
				//	m_error_state = INVALID_GLOBAL_WORK_SIZE;
				//	break;
			case CL_INVALID_GLOBAL_OFFSET:
				printf("Error: CL_INVALID_GLOBAL_OFFSET\n");
				m_error_state = INVALID_GLOBAL_OFFSET;
				break;
			case CL_INVALID_WORK_GROUP_SIZE:
				printf("Error: CL_INVALID_WORK_GROUP_SIZE\n");
				m_error_state = INVALID_WORK_GROUP_SIZE;
				break;
				//case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				//	   printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
				//	   m_error_state = MISALIGNED_SUB_BUFFER_OFFSET;
				//	   break;
			case CL_INVALID_WORK_ITEM_SIZE:
				printf("Error: CL_INVALID_WORK_ITEM_SIZE\n");
				m_error_state = INVALID_WORK_ITEM_SIZE;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state = INVALID_IMAGE_SIZE;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;

	}
	return NULL;	

}

// TODO(siba.samal) Need to change user_data to Object type
void WebCLComputeContext::finish(WebCLCommandQueue* command_queue, PassRefPtr<WebCLFinishCallback> notify, int user_data/*object userData*/)
{
	cl_command_queue cl_command_queue_id = NULL;
	cl_int err = 0;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clFinish(cl_command_queue_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMAND_QUEUE\n");
				m_error_state=INVALID_COMMAND_QUEUE;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("Unused Argument %d\n",user_data);
				m_error_state=FAILURE;
				break;
		}		
	} else {
		m_error_state = SUCCESS;
		m_finishCallback = notify;
		//m_finishCallback->handleEvent(user_data);			
		m_finishCallback->handleEvent(this);			
		return;
	}
	m_error_state = FAILURE;
	return;
}

void WebCLComputeContext::flush(WebCLCommandQueue* command_queue)
{
	cl_command_queue cl_command_queue_id = NULL;
	cl_int err = 0;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clFlush(cl_command_queue_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMAND_QUEUE\n");
				m_error_state=INVALID_COMMAND_QUEUE;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES \n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state=FAILURE;
				break;
		}
	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::releaseMemObject(WebCLMem* memobj)
{
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;

	if (memobj != NULL) {
		cl_mem_id = memobj->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clReleaseMemObject(cl_mem_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT \n");
				m_error_state=INVALID_MEM_OBJECT;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES  \n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		unsigned int i;
		for (i = 0; i < m_mem_list.size(); i++) {
			if ((m_mem_list[i].get())->getCLMem() == cl_mem_id) {				
				m_mem_list.remove(i);
				m_num_mems = m_mem_list.size();
				break;
			}
		}
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::releaseProgram(WebCLProgram* program)
{
	cl_program cl_program_id = NULL;
	cl_int err = 0;

	if (program != NULL) {
		cl_program_id = program->getCLProgram();
		if (cl_program_id == NULL) {
			printf("Error: cl_program_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clReleaseProgram(cl_program_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM  :
				printf("Error: CL_INVALID_PROGRAM  \n");
				m_error_state=INVALID_PROGRAM;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES  \n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		for (int i = 0; i < m_num_programs; i++) {
			if ((m_program_list[i].get())->getCLProgram() == cl_program_id) {
				m_program_list.remove(i);
				m_num_programs = m_program_list.size();
				break;
			}
		}
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::releaseKernel(WebCLKernel* kernel)
{
	cl_kernel cl_kernel_id = NULL;
	cl_int err = 0;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clReleaseKernel(cl_kernel_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_KERNEL :
				printf("Error: CL_INVALID_KERNEL \n");
				m_error_state=INVALID_KERNEL;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES  \n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		for (int i = 0; i < m_num_kernels; i++) {
			if ((m_kernel_list[i].get())->getCLKernel() == cl_kernel_id) {
				m_kernel_list.remove(i);
				m_num_kernels = m_kernel_list.size();
				break;
			}
		}
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::retainKernel(WebCLKernel* kernel)
{
	cl_kernel cl_kernel_id = NULL;
	cl_int err = 0;

	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clRetainKernel(cl_kernel_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_KERNEL :
				printf("Error: CL_INVALID_KERNEL\n");
				m_error_state=INVALID_KERNEL;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		printf("Success: clRetainKernel\n");
		// TODO - Check if has to be really added
		RefPtr<WebCLKernel> o = WebCLKernel::create(this, cl_kernel_id);
		m_kernel_list.append(o);
		m_num_kernels++;
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::retainProgram(WebCLProgram* program)
{
	cl_program cl_program_id = NULL;
	cl_int err = 0;

	if (program != NULL) {
		cl_program_id = program->getCLProgram();
		if (cl_program_id == NULL) {
			printf("Error: cl_program_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clRetainProgram(cl_program_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				m_error_state=INVALID_PROGRAM;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		printf("Success: clRetainProgram\n");
		// TODO - Check if has to be really added
		RefPtr<WebCLProgram> o = WebCLProgram::create(this, cl_program_id);
		m_program_list.append(o);
		m_num_programs++;
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::retainEvent(WebCLEvent* event)
{
	cl_event cl_event_id = NULL;
	cl_int err = 0;

	if (event != NULL) {
		cl_event_id = event->getCLEvent();
		if (cl_event_id == NULL) {
			printf("Error: cl_event_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clRetainEvent(cl_event_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_EVENT:
				printf("Error: CL_INVALID_EVENT\n");
				m_error_state=INVALID_EVENT;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		printf("Success: clRetainEvent\n");
		// TODO - Check if has to be really added
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::retainContext(WebCLContext* context_id)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clRetainContext(cl_context_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT :
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		printf("Success: clRetainContext\n");
		// TODO - Check if has to be really added
		RefPtr<WebCLContext> o = WebCLContext::create(this, cl_context_id);
		m_context_list.append(o);
		m_num_contexts++;
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::retainCommandQueue(WebCLCommandQueue* command_queue)
{
	cl_command_queue cl_command_queue_id = NULL;
	cl_int err = 0;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clRetainCommandQueue(cl_command_queue_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state=INVALID_COMMAND_QUEUE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		// TODO - Check if has to be really added
		RefPtr<WebCLCommandQueue> o = WebCLCommandQueue::create(this, cl_command_queue_id);
		m_commandqueue_list.append(o);
		m_num_commandqueues++;
		m_error_state = SUCCESS;
		return;
	}
	return;
}
void WebCLComputeContext::retainSampler(WebCLSampler* sampler)
{
	cl_sampler cl_sampler_id= NULL;
	cl_int err = 0;

	if (sampler != NULL) {
		cl_sampler_id = sampler->getCLSampler();
		if (cl_sampler_id == NULL) {
			printf("Error: cl_sampler_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clRetainSampler(cl_sampler_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_SAMPLER   :
				printf("Error: CL_INVALID_SAMPLER\n");
				m_error_state=INVALID_SAMPLER ;
				break;
			case CL_OUT_OF_RESOURCES  :
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY  :
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		// TODO - Check if has to be really added
		RefPtr<WebCLSampler> o = WebCLSampler::create(this, cl_sampler_id);
		m_sampler_list.append(o);
		m_num_samplers++;
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::retainMemObject(WebCLMem* memobj)
{
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;

	if (memobj != NULL) {
		cl_mem_id = memobj->getCLMem();
		if (cl_mem_id == NULL) {
			printf("Error: cl_mem_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clRetainMemObject(cl_mem_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_MEM_OBJECT  :
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state=INVALID_MEM_OBJECT ;
				break;
			case CL_OUT_OF_RESOURCES  :
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES ;
				break;
			case CL_OUT_OF_HOST_MEMORY  :
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY ;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		// TODO - Check if has to be really added
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_id,false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::releaseEvent(WebCLEvent* event)
{
	cl_event cl_event_id = NULL;
	cl_int err = 0;

	if (event != NULL) {
		cl_event_id = event->getCLEvent();
		if (cl_event_id == NULL) {
			printf("Error: cl_event_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clReleaseEvent(cl_event_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_EVENT  :
				printf("Error: CL_INVALID_EVENT\n");
				m_error_state=INVALID_EVENT;
				break;
			case CL_OUT_OF_RESOURCES  :
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY  :
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;

		}
	} else {
		for (int i = 0; i < m_num_events; i++) {
			if ((m_event_list[i].get())->getCLEvent() == cl_event_id) {
				m_event_list.remove(i);
				m_num_events = m_event_list.size();
				break;
			}
		}
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::releaseSampler(WebCLSampler* sampler)
{
	cl_sampler cl_sampler_id = NULL;
	cl_int err = 0;

	if (sampler != NULL) {
		cl_sampler_id = sampler->getCLSampler();
		if (cl_sampler_id == NULL) {
			printf("Error: cl_sampler_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clReleaseSampler(cl_sampler_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_SAMPLER :
				printf("Error: CL_INVALID_SAMPLER\n");
				m_error_state=INVALID_SAMPLER;
				break;
			case CL_OUT_OF_RESOURCES :
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY :
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;

			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		for (int i = 0; i < m_num_samplers; i++) {
			if ((m_sampler_list[i].get())->getCLSampler() == cl_sampler_id) {
				m_sampler_list.remove(i);
				m_num_samplers = m_sampler_list.size();
				break;
			}
		}
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::releaseCommandQueue(WebCLCommandQueue* command_queue)
{
	cl_command_queue cl_command_queue_id = NULL;
	cl_int err = 0;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clReleaseCommandQueue(cl_command_queue_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE  :
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state=INVALID_COMMAND_QUEUE;
				break;
			case CL_OUT_OF_RESOURCES   :
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY    :
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		m_error_state = SUCCESS;
		m_command_queue = NULL;
		return;
	}
	return; 
}

void WebCLComputeContext::releaseContext(WebCLContext* context_id)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clReleaseContext(cl_context_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT :
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_OUT_OF_RESOURCES  :
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY  :
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		m_error_state = SUCCESS;
		m_context = NULL;
		return;
	}
	return;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createImage2D(WebCLContext* context, 
		int flags, 
		HTMLCanvasElement* canvasElement)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;
	cl_mem cl_mem_image = NULL;
	cl_image_format image_format = {CL_RGBA, CL_UNSIGNED_INT32};
	cl_uint width = 0;
	cl_uint height = 0;

	ImageBuffer* imageBuffer = NULL;
	RefPtr<ByteArray> bytearray = NULL;
	if (context != NULL) {
		cl_context_id = context->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}

	if (canvasElement != NULL) {
		image_format.image_channel_data_type = CL_UNSIGNED_INT8;
		image_format.image_channel_order = CL_RGBA;
		width = (cl_uint) canvasElement->width();
		height = (cl_uint) canvasElement->height();

		imageBuffer = canvasElement->buffer();
		if ( imageBuffer == NULL)
			printf("image is null\n");
		bytearray = imageBuffer->getUnmultipliedImageData(IntRect(0,0,width,height));

		if ( bytearray == NULL)
			printf("bytearray is null\n");
	}

	if(bytearray->data() == NULL)
	{
		printf("bytearray->data() is null\n");
		m_error_state = FAILURE;
		return NULL;
	}
	void* image = (void*) bytearray->data();
	if(image == NULL)
	{
		printf("image is null\n");
		m_error_state = FAILURE;
		return NULL;
	}

	switch (flags) {
		case MEM_READ_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR ,
					&image_format, width, height,0, image, &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR),
					&image_format, width, height, 0, image, &err);
			break;
			// TODO (siba samal) Support other mem_flags & testing 
	}
	if (cl_mem_image == NULL) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_VALUE;
				break;
			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				printf("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
				m_error_state=INVALID_IMAGE_FORMAT_DESCRIPTOR;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state=INVALID_IMAGE_SIZE;
				break;
			case CL_INVALID_HOST_PTR:
				printf("Error: CL_INVALID_HOST_PTR\n");
				m_error_state=INVALID_HOST_PTR;
				break;
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				printf("Error: CL_INVALID_FORMAT_NOT_SUPPORTED\n");
				m_error_state=IMAGE_FORMAT_NOT_SUPPORTED;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state=MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state=INVALID_OPERATION;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_image,false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}	

PassRefPtr<WebCLMem> WebCLComputeContext::createImage2D(WebCLContext* context, int flags, HTMLImageElement* image)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;
	cl_mem cl_mem_image = 0;
	cl_image_format image_format;
	cl_uint width = 0;
	cl_uint height = 0;

	Image* imagebuf = NULL;
	CachedImage* cachedImage = NULL; 
	if (context != NULL) {
		cl_context_id = context->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (image != NULL) {
		image_format.image_channel_data_type = CL_UNSIGNED_INT8;
		image_format.image_channel_order = CL_RGBA;
		cachedImage = image->cachedImage();
		if (cachedImage == NULL) {
			m_error_state = FAILURE;
			printf("Error: image null\n");
			return NULL;
		} 
		else {
			width = (cl_uint) image->width();
			height = (cl_uint) image->height();
			imagebuf = cachedImage->image();
			if(imagebuf == NULL)
			{
				m_error_state = FAILURE;
				printf("Error: imagebuf null\n");
				return NULL;
			}
		}
	} 
	else {
		printf("Error: imageElement null\n");
		m_error_state = FAILURE;
		return NULL;
	}
	const char* image1  = (const char*)cachedImage->image()->data()->data() ;
	if(image1 == NULL)
	{
		m_error_state = FAILURE;
		printf("Error: image data is null\n");
		return NULL;
	}
	switch (flags) {
		case MEM_READ_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_READ_ONLY| CL_MEM_USE_HOST_PTR, 
					&image_format, width, height, 0, (void*)image1, &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_WRITE_ONLY, 
					&image_format, width, height, 0, (void*)image1, &err);
			break;
			// TODO (siba samal) Support other flags & testing
	}
	if (cl_mem_image == NULL) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_VALUE;
				break;
			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				printf("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
				m_error_state=INVALID_IMAGE_FORMAT_DESCRIPTOR;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state=INVALID_IMAGE_SIZE;
				break;
			case CL_INVALID_HOST_PTR:
				printf("Error: CL_INVALID_HOST_PTR\n");
				m_error_state=INVALID_HOST_PTR;
				break;
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				printf("Error: CL_INVALID_FORMAT_NOT_SUPPORTED\n");
				m_error_state=IMAGE_FORMAT_NOT_SUPPORTED;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state=MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state=INVALID_OPERATION;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_image,false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createImage2D(WebCLContext* context, int flags, HTMLVideoElement* video)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;
	cl_mem cl_mem_image = NULL;
	cl_image_format image_format;
	cl_uint width = 0;
	cl_uint height = 0;

	RefPtr<Image> image = NULL;
	SharedBuffer* sharedBuffer = NULL;
	const char* image_data = NULL;
	if (context != NULL) {
		cl_context_id = context->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (video != NULL) {
		image_format.image_channel_data_type = CL_UNSIGNED_INT8;
		image_format.image_channel_order = CL_RGBA;
		width = (cl_uint) video->width();
		height = (cl_uint) video->height();
		image = videoFrameToImage(video);
		sharedBuffer = image->data();
		if (sharedBuffer == NULL) {
			printf("Error: sharedBuffer null\n");
			m_error_state = FAILURE;
			return NULL;
		} else {
			image_data = sharedBuffer->data();

			if (image_data == NULL) {
				printf("Error: image_data null\n");
				m_error_state = FAILURE;
				return NULL;
			}
		}

	} else {
		printf("Error: canvasElement null\n");
		m_error_state = FAILURE;
		return NULL;
	}

	switch (flags) {
		case MEM_READ_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_READ_ONLY| CL_MEM_USE_HOST_PTR, 
					&image_format, width, height, 0, (void *)image_data, &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_WRITE_ONLY, 
					&image_format, width, height, 0, (void *)image_data, &err);
			break;
			// TODO (siba samal) Support other flags & testing
	}
	if (cl_mem_image == NULL) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_VALUE;
				break;
			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				printf("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
				m_error_state=INVALID_IMAGE_FORMAT_DESCRIPTOR;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state=INVALID_IMAGE_SIZE;
				break;
			case CL_INVALID_HOST_PTR:
				printf("Error: CL_INVALID_HOST_PTR\n");
				m_error_state=INVALID_HOST_PTR;
				break;
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				printf("Error: CL_INVALID_FORMAT_NOT_SUPPORTED\n");
				m_error_state=IMAGE_FORMAT_NOT_SUPPORTED;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state=MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state=INVALID_OPERATION;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_image,false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createImage2D(WebCLContext* context, int flags, ImageData* data)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;
	cl_mem cl_mem_image = NULL;
	cl_image_format image_format;
	cl_uint width = 0;
	cl_uint height = 0;

	CanvasPixelArray* pixelarray = NULL;
	ByteArray* bytearray = NULL;
	if (context != NULL) {
		cl_context_id = context->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (data != NULL) {
		image_format.image_channel_data_type = CL_UNSIGNED_INT8;
		image_format.image_channel_order = CL_RGBA;
		pixelarray = data->data();
		if(pixelarray != NULL)
		{
			bytearray = pixelarray->data();
			if(bytearray == NULL)
				return NULL;
		}			
		width = (cl_uint) data->width();
		height = (cl_uint) data->height();
	} else {
		printf("Error: canvasElement null\n");
		m_error_state = FAILURE;
		return NULL;
	}

	switch (flags) {
		case MEM_READ_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, 
					&image_format, width, height, 0, (void*)bytearray, &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_WRITE_ONLY, 
					&image_format, width, height, 0, (void*)bytearray, &err);
			break;
			// TODO (siba samal) Support other flags & testing
	}
	if (cl_mem_image == NULL) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_VALUE;
				break;
			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				printf("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
				m_error_state=INVALID_IMAGE_FORMAT_DESCRIPTOR;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state=INVALID_IMAGE_SIZE;
				break;
			case CL_INVALID_HOST_PTR:
				printf("Error: CL_INVALID_HOST_PTR\n");
				m_error_state=INVALID_HOST_PTR;
				break;
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				printf("Error: CL_INVALID_FORMAT_NOT_SUPPORTED\n");
				m_error_state=IMAGE_FORMAT_NOT_SUPPORTED;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state=MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state=INVALID_OPERATION;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_image,false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createImage2D(WebCLContext* context,int flags,unsigned int width, unsigned int height,ArrayBuffer* data)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;
	cl_mem cl_mem_image = NULL;
	cl_image_format image_format;
	cl_uint cl_width = 0;
	cl_uint cl_height = 0;

	if (context != NULL) {
		cl_context_id = context->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (data != NULL) {
		image_format.image_channel_data_type = CL_UNSIGNED_INT8;
		image_format.image_channel_order = CL_RGBA;
		cl_width = width;
		cl_height = height;
	} else {
		printf("Error: canvasElement null\n");
		m_error_state = FAILURE;
		return NULL;
	}

	switch (flags) {
		case MEM_READ_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, 
					&image_format, cl_width, cl_height, 0, data->data(), &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_image = clCreateImage2D(cl_context_id, CL_MEM_WRITE_ONLY, 
					&image_format, cl_width, cl_height, 0, data->data(), &err);
			break;
			// TODO (siba samal) Support other flags & testing
	}
	if (cl_mem_image == NULL) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_VALUE;
				break;
			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				printf("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
				m_error_state=INVALID_IMAGE_FORMAT_DESCRIPTOR;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state=INVALID_IMAGE_SIZE;
				break;
			case CL_INVALID_HOST_PTR:
				printf("Error: CL_INVALID_HOST_PTR\n");
				m_error_state=INVALID_HOST_PTR;
				break;
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				printf("Error: CL_INVALID_FORMAT_NOT_SUPPORTED\n");
				m_error_state=IMAGE_FORMAT_NOT_SUPPORTED;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state=MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state=INVALID_OPERATION;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_image,false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createImage3D(WebCLContext* context,
		int flags,unsigned int width, 
		unsigned int height,
		unsigned int depth,
		ArrayBuffer* data)
{
	cl_context cl_context_id = NULL;
	cl_int err = 0;
	cl_mem cl_mem_image = NULL;
	cl_image_format image_format;
	cl_uint cl_width = 0;
	cl_uint cl_height = 0;
	cl_uint cl_depth = 0;

	if (context != NULL) {
		cl_context_id = context->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (data != NULL) {
		image_format.image_channel_data_type = CL_UNSIGNED_INT8;
		image_format.image_channel_order = CL_RGBA;
		cl_width = width;
		cl_height = height;
		cl_depth = depth;
	} else {
		printf("Error: canvasElement null\n");
		m_error_state = FAILURE;
		return NULL;
	}

	switch (flags) {
		case MEM_READ_ONLY:
			cl_mem_image = clCreateImage3D(cl_context_id, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, 
					&image_format, cl_width, cl_height, cl_depth,0, 0, data->data(), &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_image = clCreateImage3D(cl_context_id, CL_MEM_WRITE_ONLY, 
					&image_format, cl_width, cl_height, cl_depth, 0, 0,data->data(), &err);
			break;
			// TODO (siba samal) Support other flags & testing
	}
	if (cl_mem_image == NULL) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_VALUE;
				break;
			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				printf("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
				m_error_state=INVALID_IMAGE_FORMAT_DESCRIPTOR;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state=INVALID_IMAGE_SIZE;
				break;
			case CL_INVALID_HOST_PTR:
				printf("Error: CL_INVALID_HOST_PTR\n");
				m_error_state=INVALID_HOST_PTR;
				break;
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				printf("Error: CL_INVALID_FORMAT_NOT_SUPPORTED\n");
				m_error_state=IMAGE_FORMAT_NOT_SUPPORTED;
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state=MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state=INVALID_OPERATION;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_image,false);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueWriteImage(WebCLCommandQueue* command_queue, 
		WebCLMem* mem_image, 
		bool blocking_write, 
		Int32Array* origin, 
		Int32Array* region, 
		HTMLCanvasElement* canvasElement, 
		int event_wait_list)
{

	cl_int err = 0;
	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_image = NULL;
	cl_event cl_event_id = NULL;


	size_t *origin_array = NULL;
	size_t *region_array = NULL;


	SharedBuffer* sharedBuffer = NULL;

	Image* image = NULL;
	const char* image_data = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}

	if (mem_image != NULL) {
		cl_mem_image = mem_image->getCLMem();
		if (cl_mem_image == NULL) {
			printf("Error: cl_mem_image null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}

	unsigned origin_array_length = origin->length();
	origin_array = (size_t*)malloc(origin_array_length*sizeof(size_t));
	for (unsigned int i = 0; i < origin_array_length; i++) {
		origin_array[i] = origin->item(i);
	}
	unsigned  region_array_length = region->length();
	region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
	for (unsigned int i = 0; i < region_array_length; i++) {
		region_array[i] = region->item(i);
	}

	if (canvasElement != NULL) {
		image = canvasElement->copiedImage();
		sharedBuffer = image->data();
		if (sharedBuffer == NULL) {
			printf("Error: sharedBuffer null\n");
			m_error_state = FAILURE;
			return NULL;
		} else {
			image_data = sharedBuffer->data();

			if (image_data == NULL) {
				printf("Error: image_data null\n");
				m_error_state = FAILURE;
				return NULL;
			}
		}
	}

	if (blocking_write) {
			err = clEnqueueWriteImage(cl_command_queue_id, cl_mem_image, CL_TRUE, 
					origin_array, region_array, 0, 0, image_data, event_wait_list, NULL, &cl_event_id);
    } else {
			err = clEnqueueWriteImage(cl_command_queue_id, cl_mem_image, CL_FALSE, 
					origin_array, region_array, 0, 0, image_data, event_wait_list, NULL, &cl_event_id);
	}
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueWriteImage\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("Error: CL_INVALID_IMAGE_SIZE\n");
				m_error_state = INVALID_IMAGE_SIZE;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state = INVALID_OPERATION;
				break;
				// CHECK(won.jeon) - from spec 1.1?
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
				//	break;
				//case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				//	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
				//	m_error_state = EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
				//	break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				printf("WebCLComputeContext::enqueueWriteImage width=%d height=%d event_wait_list=%d\n", 
						canvasElement->width(), canvasElement->height(), event_wait_list);
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createFromGLBuffer(WebCLContext* context_id, 
		int flags, WebGLBuffer* bufobj)
{
	cl_context cl_context_id = NULL;
	cl_mem cl_mem_id = NULL;
	cl_int err = 0;
	Platform3DObject buf_id = 0;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (bufobj != NULL) {
		buf_id = bufobj->object();
		if (buf_id == 0) {
			printf("Error: buf_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	switch (flags) {
		case MEM_READ_ONLY:
			cl_mem_id = clCreateFromGLBuffer(cl_context_id, CL_MEM_READ_ONLY, buf_id, &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_id = clCreateFromGLBuffer(cl_context_id, CL_MEM_WRITE_ONLY, buf_id, &err);
			break;
		case MEM_READ_WRITE:
			cl_mem_id = clCreateFromGLBuffer(cl_context_id, CL_MEM_READ_WRITE, buf_id, &err);
			break;
		case MEM_USE_HOST_PTR:
			cl_mem_id = clCreateFromGLBuffer(cl_context_id, CL_MEM_USE_HOST_PTR, buf_id, &err);
			break;
		case MEM_ALLOC_HOST_PTR:
			cl_mem_id = clCreateFromGLBuffer(cl_context_id, CL_MEM_ALLOC_HOST_PTR, buf_id, &err);
			break;
		case MEM_COPY_HOST_PTR:
			cl_mem_id = clCreateFromGLBuffer(cl_context_id, CL_MEM_COPY_HOST_PTR, buf_id, &err);
			break;

	}
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_VALUE;
				break;
			case CL_INVALID_GL_OBJECT:
				printf("Error: CL_INVALID_GL_OBJECT\n");
				m_error_state=INVALID_GL_OBJECT;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_id, true);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;

	}
	return NULL;
}

PassRefPtr<WebCLSampler> WebCLComputeContext::createSampler(WebCLContext* context_id, 
		bool norm_cords, int addr_mode, int fltr_mode)
{
	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_bool normalized_coords = CL_FALSE;
	cl_addressing_mode addressing_mode = CL_ADDRESS_NONE;
	cl_filter_mode filter_mode = CL_FILTER_NEAREST;
	cl_sampler cl_sampler_id = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}
	if(norm_cords)
		normalized_coords = CL_TRUE;
	switch(addr_mode)
	{
		case ADDRESS_NONE:
			addressing_mode = CL_ADDRESS_NONE;
			break;
		case ADDRESS_CLAMP_TO_EDGE:
			addressing_mode = CL_ADDRESS_CLAMP_TO_EDGE;
			break;
		case ADDRESS_CLAMP:
			addressing_mode = CL_ADDRESS_CLAMP;
			break;
		case ADDRESS_REPEAT: 
			addressing_mode = CL_ADDRESS_REPEAT;
			break;
		default:
			printf("Error: Invaild Addressing Mode\n");
			m_error_state = FAILURE;
			return NULL;
	}
	switch(fltr_mode)
	{
		case FILTER_LINEAR:
			filter_mode = CL_FILTER_LINEAR;
			break;
		case FILTER_NEAREST :
			filter_mode = CL_FILTER_NEAREST ;
			break;
		default:
			printf("Error: Invaild Filtering Mode\n");
			m_error_state = FAILURE;
			return NULL;
	}
	cl_sampler_id = clCreateSampler(cl_context_id, normalized_coords, addressing_mode, 
			filter_mode, &err);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT \n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE \n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_OPERATION :
				printf("Error: CL_INVALID_OPERATION   \n");
				m_error_state = INVALID_OPERATION  ;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY \n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;

			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLSampler> o = WebCLSampler::create(this, cl_sampler_id);
		if (o != NULL) {
			m_sampler_list.append(o);
			m_num_samplers++;
			m_error_state = SUCCESS;
			return o;
		} else {
			m_error_state = FAILURE;
			return NULL;
		}
	}
	return NULL;
}

PassRefPtr<WebCLMem> WebCLComputeContext::createFromGLTexture2D(WebCLContext* context_id, 
		int flags, GC3Denum texture_target, GC3Dint miplevel, GC3Duint texture)
{
	cl_int err = 0;
	cl_context cl_context_id = NULL;
	cl_mem cl_mem_id = NULL;

	if (context_id != NULL) {
		cl_context_id = context_id->getCLContext();
		if (cl_context_id == NULL) {
			printf("Error: cl_context_id null\n");
			return NULL;
		}
	}
	switch (flags)
	{
		case MEM_READ_ONLY:
			cl_mem_id = clCreateFromGLTexture2D(cl_context_id, CL_MEM_READ_ONLY, 
					texture_target, miplevel, texture, &err);
			break;
		case MEM_WRITE_ONLY:
			cl_mem_id = clCreateFromGLTexture2D(cl_context_id, CL_MEM_WRITE_ONLY,
					texture_target, miplevel, texture, &err);
			break;
		case MEM_READ_WRITE:
			cl_mem_id = clCreateFromGLTexture2D(cl_context_id, CL_MEM_READ_WRITE,
					texture_target, miplevel, texture, &err);
			break;
	}
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state=INVALID_CONTEXT;
				break;
			case CL_INVALID_MIP_LEVEL:
				printf("Error: CL_INVALID_MIP_LEVEL\n");
				m_error_state=INVALID_MIP_LEVEL;
				break;
			case CL_INVALID_GL_OBJECT  :
				printf("Error: CL_INVALID_GL_OBJECT\n");
				m_error_state=INVALID_GL_OBJECT;
				break;

			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				printf("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
				m_error_state=INVALID_IMAGE_FORMAT_DESCRIPTOR;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				m_error_state=INVALID_OPERATION;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state=OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state=OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		RefPtr<WebCLMem> o = WebCLMem::create(this, cl_mem_id, true);
		m_mem_list.append(o);
		m_num_mems++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

void WebCLComputeContext::enqueueAcquireGLObjects(WebCLCommandQueue* command_queue, 
		WebCLMem* mem_objects, int event_wait_list)
{
	cl_int err = 0;
	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_ids = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if ((mem_objects != NULL) && (mem_objects->isShared() == true)) {
		cl_mem_ids = mem_objects->getCLMem();
		if (cl_mem_ids == NULL) {
			printf("Error: cl_mem_ids null\n");
			m_error_state = FAILURE;
			return;
		}
	}

	// TODO(won.jeon) - currently event-related arguments are ignored
	err = clEnqueueAcquireGLObjects(cl_command_queue_id, 1, &cl_mem_ids, 0, 0, 0);
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueAcquireGLObjects\n");
		switch (err) {
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_GL_OBJECT:
				printf("Error: CL_INVALID_GL_OBJECT\n");
				m_error_state = INVALID_GL_OBJECT;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				//TODO (siba samal) Handle Event 
				printf("WebCLComputeContext::enqueueAcquireGLObjects event_wait_list=%d\n",
						event_wait_list);
				m_error_state = FAILURE;
				break;
		}
	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::enqueueReleaseGLObjects(WebCLCommandQueue* command_queue, 
		WebCLMem* mem_objects, int event_wait_list)
{
	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_mem_ids = NULL;
	cl_int err = 0;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if ((mem_objects != NULL) && (mem_objects->isShared())) {
		cl_mem_ids = mem_objects->getCLMem();
		if (cl_mem_ids == NULL) {
			printf("Error: cl_mem_ids null\n");
			m_error_state = FAILURE;
			return;
		}
	}

	err = clEnqueueReleaseGLObjects(cl_command_queue_id, 1, &cl_mem_ids, 
			0, 0, 0);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_GL_OBJECT:
				printf("Error: CL_INVALID_GL_OBJECT\n");
				m_error_state = INVALID_GL_OBJECT;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				// TODO (siba samal) Handle Event related args
				printf("WebCLComputeContext::enqueueReleaseGLObjects event_wait_list=%d\n",
						event_wait_list);
				m_error_state = FAILURE;
				break;
		}
	} else {
		unsigned int i;

		for (i = 0; i < m_mem_list.size(); i++) {
			if ((m_mem_list[i].get())->getCLMem() == cl_mem_ids) {
				m_mem_list.remove(i);
				m_num_mems = m_mem_list.size();
				break;
			}
		}
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::enqueueCopyBuffer(WebCLCommandQueue* command_queue, 
		WebCLMem* src_buffer, WebCLMem* dst_buffer, int cb)
{
	cl_command_queue cl_command_queue_id = NULL;
	cl_mem cl_src_buffer_id = NULL;
	cl_mem cl_dst_buffer_id = NULL;
	cl_int err = 0;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if (src_buffer != NULL) {
		cl_src_buffer_id = src_buffer->getCLMem();
		if (cl_src_buffer_id == NULL) {
			printf("Error: cl_src_buffer_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	if (dst_buffer != NULL) {
		cl_dst_buffer_id = dst_buffer->getCLMem();
		if (cl_dst_buffer_id == NULL) {
			printf("Error: cl_dst_buffer_id null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clEnqueueCopyBuffer(cl_command_queue_id, cl_src_buffer_id, cl_dst_buffer_id,
			0, 0, cb, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueCopyBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_MEM_OBJECT:
				printf("Error: CL_INVALID_MEM_OBJECT\n");
				m_error_state = INVALID_MEM_OBJECT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
			case CL_MEM_COPY_OVERLAP:
				printf("Error: CL_MEM_COPY_OVERLAP\n");
				m_error_state = MEM_COPY_OVERLAP;
				break;
				/* CHECK(won.jeon) - defined in 1.1?
				   case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				   printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
				   break;
				 */
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}
	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::enqueueBarrier(WebCLCommandQueue* command_queue)
{
	cl_int err = 0;
	cl_command_queue cl_command_queue_id = 0;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return ;
		}
	}
	err = clEnqueueBarrier(cl_command_queue_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				m_error_state = FAILURE;
				printf("Error: Invaild Error Type\n");
				break;
		}
	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}


void WebCLComputeContext::unloadCompiler()
{
	cl_int err =  clUnloadCompiler();
	if (err != CL_SUCCESS) {
		m_error_state = FAILURE;
		printf("Error: Invaild Error Type\n");
	}
	else {
		m_error_state = SUCCESS;
	}
	return;
}

void WebCLComputeContext::enqueueMarker(WebCLCommandQueue* commandqueue, WebCLEvent* event)
{
	cl_int err = 0;
	cl_command_queue cl_command_queue_id = 0;
	cl_event cl_event_id = 0;

	if (commandqueue != NULL) {
		cl_command_queue_id = commandqueue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return ;
		}
	}
	if (event != NULL) {
		cl_event_id = event->getCLEvent();
		if (cl_event_id == NULL) {
			printf("cl_event_id null\n");
			m_error_state = FAILURE;
			return ;
		}
	}

	err = clEnqueueMarker(cl_command_queue_id, &cl_event_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				m_error_state = FAILURE;
				printf("Error: Invaild Error Type\n");
				break;
		}
	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

void WebCLComputeContext::enqueueWaitForEvents(WebCLCommandQueue* commandqueue, WebCLEventList* events)
{
	cl_int err = 0;
	cl_command_queue cl_command_queue_id = 0;
	cl_event* cl_event_id = NULL;

	if (commandqueue != NULL) {
		cl_command_queue_id = commandqueue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return ;
		}
	}
	if (events != NULL) {
		cl_event_id = events->getCLEvents();
		if (cl_event_id == NULL) {
			printf("Error: cl_event null\n");
			m_error_state = FAILURE;
			return;
		}
	}
	err = clEnqueueWaitForEvents(cl_command_queue_id, events->length(), cl_event_id);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_EVENT:
				printf("Error: CL_INVALID_EVENT\n");
				m_error_state = INVALID_EVENT;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				m_error_state = INVALID_VALUE;
				break;
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES\n");
				m_error_state = OUT_OF_RESOURCES;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			default:
				m_error_state = FAILURE;
				printf("Error: Invaild Error Type\n");
				break;
		}
	} else {
		m_error_state = SUCCESS;
		return;
	}
	return;
}

PassRefPtr<WebCLEvent> WebCLComputeContext::enqueueTask(WebCLCommandQueue* command_queue,
		WebCLKernel* kernel, int event_wait_list)
{

	cl_command_queue cl_command_queue_id = NULL;
	cl_kernel cl_kernel_id = NULL;
	cl_int err = 0;
	cl_event cl_event_id = NULL;

	if (command_queue != NULL) {
		cl_command_queue_id = command_queue->getCLCommandQueue();
		if (cl_command_queue_id == NULL) {
			printf("Error: cl_command_queue_id null\n");
			m_error_state = FAILURE;
			return NULL;
		}
	}
	if (kernel != NULL) {
		cl_kernel_id = kernel->getCLKernel();
		if (cl_kernel_id == NULL) {
			printf("Error: cl_kernel_id null\n");
			//TODO (siba samal) Handle enqueueTask  API
			printf("WebCLComputeContext::enqueueTask event_wait_list=%d\n",
					event_wait_list);
			m_error_state = FAILURE;
			return NULL;
		}
	}

	err = clEnqueueTask(cl_command_queue_id, cl_kernel_id, 0, NULL,&cl_event_id); 

	if (err != CL_SUCCESS) {
		printf("Error: clEnqueueWriteBuffer\n");
		switch (err) {
			case CL_INVALID_COMMAND_QUEUE:
				printf("Error: CL_INVALID_COMMAND_QUEUE\n");
				m_error_state = INVALID_COMMAND_QUEUE;
				break;
			case CL_INVALID_CONTEXT:
				printf("Error: CL_INVALID_CONTEXT\n");
				m_error_state = INVALID_CONTEXT;
				break;
			case CL_INVALID_PROGRAM_EXECUTABLE :
				printf("Error: CL_INVALID_PROGRAM_EXECUTABLE \n");
				m_error_state = INVALID_PROGRAM_EXECUTABLE;
				break;				
			case CL_INVALID_KERNEL :
				printf("Error: CL_INVALID_KERNEL \n");
				m_error_state = INVALID_KERNEL;
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
				m_error_state = INVALID_EVENT_WAIT_LIST;
				break;
			case CL_INVALID_KERNEL_ARGS :
				printf("Error: CL_INVALID_KERNEL_ARGS \n");
				m_error_state = INVALID_KERNEL_ARGS;
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				m_error_state = OUT_OF_HOST_MEMORY;
				break;
			case CL_INVALID_WORK_GROUP_SIZE :
				printf("Error: CL_INVALID_WORK_GROUP_SIZE \n");
				m_error_state = FAILURE;
				break;		
			case CL_OUT_OF_RESOURCES:
				printf("Error: CL_OUT_OF_RESOURCES  \n");
				m_error_state = OUT_OF_RESOURCES;
				break;			
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE  \n");
				m_error_state = MEM_OBJECT_ALLOCATION_FAILURE;
				break;	
			default:
				printf("Error: Invaild Error Type\n");
				m_error_state = FAILURE;
				break;
		}

	} else {
		RefPtr<WebCLEvent> o = WebCLEvent::create(this, cl_event_id);
		m_event_list.append(o);
		m_num_events++;
		m_error_state = SUCCESS;
		return o;
	}
	return NULL;
}

	WebCLComputeContext::LRUImageBufferCache::LRUImageBufferCache(int capacity)
	: m_buffers(adoptArrayPtr(new OwnPtr<ImageBuffer>[capacity]))
	  , m_capacity(capacity)
{
}

ImageBuffer* WebCLComputeContext::LRUImageBufferCache::imageBuffer(const IntSize& size)
{
	int i;
	for (i = 0; i < m_capacity; ++i) {
		ImageBuffer* buf = m_buffers[i].get();
		if (!buf)
			break;
		if (buf->size() != size)
			continue;
		bubbleToFront(i);
		return buf;
	}

	OwnPtr<ImageBuffer> temp = ImageBuffer::create(size);
	if (!temp)
		return 0;
	i = std::min(m_capacity - 1, i);
	m_buffers[i] = temp.release();

	ImageBuffer* buf = m_buffers[i].get();
	bubbleToFront(i);
	return buf;
}

void WebCLComputeContext::LRUImageBufferCache::bubbleToFront(int idx)
{
	for (int i = idx; i > 0; --i)
		m_buffers[i].swap(m_buffers[i-1]);
}

PassRefPtr<Image> WebCLComputeContext::videoFrameToImage(HTMLVideoElement* video)
{
	if (!video || !video->videoWidth() || !video->videoHeight()) {
		return 0;
	}
	IntSize size(video->videoWidth(), video->videoHeight());
	ImageBuffer* buf = m_videoCache.imageBuffer(size);
	if (!buf) {
		return 0;
	}
	IntRect destRect(0, 0, size.width(), size.height());
	// FIXME: Turn this into a GPU-GPU texture copy instead of CPU readback.
	video->paintCurrentFrameInContext(buf->context(), destRect);
	return buf->copyImage();
}
template<class T> inline unsigned int WebCLComputeContext::
clSetKernelArgPrimitiveType(cl_kernel cl_kernel_id,
		PassRefPtr<WebCLKernelTypeValue> kernelObject,
		T nodeId, unsigned int argIndex, int size)
{			
	if(!kernelObject->asNumber(&nodeId))
	{
		printf("Error: WebCL Kernel Type Value Not Proper\n");
		m_error_state = FAILURE;
		return -1;	
	}
	cl_int err  =  clSetKernelArg(cl_kernel_id, argIndex, size, &nodeId);		
	return(err);
}

inline unsigned int WebCLComputeContext::
clSetKernelArgVectorType(cl_kernel cl_kernel_id,
		PassRefPtr<WebCLKernelTypeValue> kernelObject,
		RefPtr<WebCLKernelTypeVector> array , unsigned int argIndex,
		int size,unsigned int length)
{		
	if ((!kernelObject->asVector(&array)) || (length != array->length()))
	{
		printf("Error: Invalid WebCL Kernel Type %d\n", array->length());
		m_error_state = FAILURE;
		return -1;	
	}
	cl_int err = clSetKernelArg(cl_kernel_id, argIndex, size, &array);			
	return(err);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
