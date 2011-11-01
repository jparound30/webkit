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

// local OpenCL info
//
var platform_ids;                           // array of OpenCL platform ids
var platform_id;                            // OpenCL platform id
var device_ids;                             // array of OpenCL device ids
var device_id;                              // OpenCL device id
var context;                                // OpenCL context
var queue;                                  // OpenCL command queue
var program;                                // OpenCL program
var kernel;                                 // OpenCL kernel

var curPosBuffer;                           // OpenCL buffer created from GL VBO
var curVelBuffer;                           // OpenCL buffer created from GL VBO
var nxtPosBuffer;                           // OpenCL buffer
var nxtVelBuffer;                           // OpenCL buffer

var bufferSize = null;
var cl = null;

var globalWorkSize = new Int32Array(1);
var localWorkSize = new Int32Array(1);
var workGroupSize = 0;

function getKernel(id) {
	var kernelScript = document.getElementById(id);
	if(kernelScript === null || kernelScript.type !== "x-kernel")
		return null;
	return kernelScript.firstChild.textContent;
}


function InitCL() {

	// Create CL buffers from GL VBOs
	// (Initial load of positions is via gl.bufferData)
	//
	curPosBuffer = cl.createFromGLBuffer(context, cl.MEM_READ_WRITE, userData.curPosVBO);
	if(curPosBuffer === null) {
		console.error("Failed to allocate device memory");
		return null;
	}

	curVelBuffer = cl.createFromGLBuffer(context, cl.MEM_READ_WRITE, userData.curVelVBO);
	if(curVelBuffer === null) {
		console.error("Failed to allocate device memory");
		return null;
	}

	bufferSize = NBODY * POS_ATTRIB_SIZE * Float32Array.BYTES_PER_ELEMENT;

	// Create CL working buffers (will be copied to current buffers after computation)
	//
	nxtPosBuffer = cl.createBuffer(context, cl.MEM_READ_WRITE, bufferSize, null);
	if(nxtPosBuffer === null) {
		console.error("Failed to allocate device memory");
		return null;
	}

	nxtVelBuffer = cl.createBuffer(context, cl.MEM_READ_WRITE, bufferSize, null);
	if(nxtVelBuffer === null) {
		console.error("Failed to allocate device memory");
		return null;
	}

	globalWorkSize[0] = NBODY;   
	localWorkSize[0] = Math.min(workGroupSize, NBODY);

	// Initial load of velocity data
	//
	cl.enqueueAcquireGLObjects(queue, curVelBuffer, null);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed enqueueAcquireGLObjects");
		return;
	}

	cl.enqueueWriteBuffer(queue, curVelBuffer, true, 0, bufferSize, userData.curVel, null);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to write buffer");
		return;
	}

	cl.enqueueReleaseGLObjects(queue, curVelBuffer, null);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed enqueueReleaseGLObjects");
		return;
	}

	cl.finish(queue, GetNullResults, 0);
	return cl;
}

function GetNullResults(userData)
{
}

function SimulateCL(cl) {
	if(cl === null)
		return;
	if(userData.isGLCLshared) {
		cl.enqueueAcquireGLObjects(queue, curPosBuffer, null);
		cl.enqueueAcquireGLObjects(queue, curVelBuffer, null);
	}
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed enqueueAcquireGLObjects");
		return;
	}
	var localMemSize = localWorkSize[0] * POS_ATTRIB_SIZE * Float32Array.BYTES_PER_ELEMENT;
	cl.setKernelArgGlobal(kernel, 0, curPosBuffer);
	cl.setKernelArgGlobal(kernel, 1, curVelBuffer);
	cl.setKernelArg(kernel, 2, NBODY, cl.KERNEL_ARG_INT);
	cl.setKernelArg(kernel, 3, DT, cl.KERNEL_ARG_FLOAT);
	cl.setKernelArg(kernel, 4, EPSSQR, cl.KERNEL_ARG_INT);
	cl.setKernelArgLocal(kernel, 5, localMemSize);  // __local: val (ignored) and size
	cl.setKernelArgGlobal(kernel, 6, nxtPosBuffer);
	cl.setKernelArgGlobal(kernel, 7, nxtVelBuffer);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to set kernel arguments");
		return;
	}
	cl.enqueueNDRangeKernel(queue, kernel, 1, 0, globalWorkSize, localWorkSize, null);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to execute kernel");
		return;
	}

	//cl.finish(queue, null, null);
	cl.finish(queue, GetResults, cl);
}

function GetResults(cl)
{  

	cl.enqueueCopyBuffer(queue, nxtPosBuffer, curPosBuffer, bufferSize);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to copy buffer");
		return;
	}

	cl.enqueueCopyBuffer(queue, nxtVelBuffer, curVelBuffer, bufferSize);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to copy buffer");
		return;
	}

	if(userData.isGLCLshared) {
		cl.enqueueReleaseGLObjects(queue, curPosBuffer, null);
		cl.enqueueReleaseGLObjects(queue, curVelBuffer, null);
	}

	if(!userData.isGLCLshared || userData.drawMode === JS_DRAW_MODE) {
		cl.enqueueReadBuffer(queue, curPosBuffer, true, 0, bufferSize, userData.curPos, null);
		cl.enqueueReadBuffer(queue, curVelBuffer, true, 0, bufferSize, userData.curVel, null);
	}
}

function GetWorkGroupSize() {

	if(typeof(WebCLComputeContext) === "undefined") {
		console.error("WebCLComputeContext is yet to be defined");
		return null;
	}

	cl = new WebCLComputeContext();

	if(cl === null) {
		console.error("Failed to create WebCL context");
		return;
	}

	// Select a compute device
	//
	platform_ids = cl.getPlatformIDs();
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to get platform IDs");
		return;
	}

	if(platform_ids.length === 0) {
		console.error("No platforms available");
		return;
	}
	platform_id = platform_ids[0];

	// Select a compute device
	//
	device_ids = cl.getDeviceIDs(platform_id, cl.DEVICE_TYPE_GPU);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to get device IDs");
		return;
	}
	if(device_ids.length === 0) {
		console.error("No devices available");
		//return;
	}
	device_id = device_ids[0];

	// Create a compute context
	//
	//context = cl.createContext(null, device_id, null, null);
	context = cl.createSharedContext(cl.DEVICE_TYPE_GPU, null, null);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to create a compute context");
		return;
	}

	// Create a command queue
	//
	queue = cl.createCommandQueue(context, device_id, null);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to create a command queue");
		return;
	}

	// Create the compute program from the source buffer
	//
	var kernelSource = getKernel("nbody_kernel");
	if (kernelSource === null) {
		console.error("No kernel named: " + "nbody_kernel");
		return;
	}

	program = cl.createProgramWithSource(context, kernelSource);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to create compute program");
		return;
	}

	// Build the program executable
	//
	cl.buildProgram(program, null, null, null);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to build program executable");
		var info = cl.getProgramBuildInfo(program, device_id, cl.PROGRAM_BUILD_LOG);
		console.log(info);
		return;
	}

	// Create the compute kernel in the program we wish to run
	//
	kernel = cl.createKernel(program, "nbody_kernel");
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to create compute kernel");
		return;
	}

	// Get the maximum work group size for executing the kernel on the device
	//
	workGroupSize = cl.getKernelWorkGroupInfo(kernel, device_id, cl.KERNEL_WORK_GROUP_SIZE);
	if (cl.getError() !== cl.SUCCESS) {
		console.error("Failed to retrieve kernel work group info");
		return null;
	}
	return workGroupSize;  
}
