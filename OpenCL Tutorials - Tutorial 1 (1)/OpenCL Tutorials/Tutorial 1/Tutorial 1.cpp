#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <vector>

#include <CL\cl.hpp>
#include "Utils.h"

void print_help() {
	std::cerr << "Application usage:" << std::endl;

	std::cerr << "  -p : select platform " << std::endl;
	std::cerr << "  -d : select device" << std::endl;
	std::cerr << "  -l : list all platforms and devices" << std::endl;
	std::cerr << "  -h : print this message" << std::endl;
}

int main(int argc, char **argv) {
	//Part 1 - handle command line options such as device selection, verbosity, etc.
	int platform_id = 0;
	int device_id = 0;

	for (int i = 1; i < argc; i++)	{
		if ((strcmp(argv[i], "-p") == 0) && (i < (argc - 1))) { platform_id = atoi(argv[++i]); }
		else if ((strcmp(argv[i], "-d") == 0) && (i < (argc - 1))) { device_id = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-l") == 0) { std::cout << ListPlatformsDevices() << std::endl; }
		else if (strcmp(argv[i], "-h") == 0) { print_help(); }
	}

	//detect any potential exceptions
	try {
		//Part 2 - host operations
		//2.1 Select computing devices
		cl::Context context = GetContext(platform_id, device_id);

		//display the selected device
		std::cout << "Runinng on " << GetPlatformName(platform_id) << ", " << GetDeviceName(platform_id, device_id) << std::endl;

		//create a queue to which we will push commands for the device
		cl::CommandQueue queue(context, CL_QUEUE_PROFILING_ENABLE);

		//2.2 Load & build the device code
		cl::Program::Sources sources;

		AddSources(sources, "my_kernels.cl");

		cl::Program program(context, sources);

		program.build();

		//Part 4 - memory allocation
		//host - input
		std::vector<float> A = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; //C++11 allows this type of initialisation
		std::vector<float> B = { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 };
		
		size_t vector_elements = A.size();//number of elements
		size_t vector_size = A.size()*sizeof(float);//size in bytes

		//host - output
		std::vector<float> C(vector_elements);

		//device - buffers
		cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, vector_size);
		cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, vector_size);
		cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, vector_size);

		//Part 5 - device operations

		//5.1 Copy arrays A and B to device memory
		cl::Event A_Event;
		cl::Event B_Event;
		queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, vector_size, &A[0], NULL, &A_Event);
		queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, vector_size, &B[0], NULL, &B_Event);

		//5.2 Setup and execute the kernel (i.e. device code)
		//cl::Kernel kernel_multi = cl::Kernel(program, "multi");
		//kernel_multi.setArg(0, buffer_A);
		//kernel_multi.setArg(1, buffer_B);
		//kernel_multi.setArg(2, buffer_C);

		//cl::Kernel kernel_add = cl::Kernel(program, "add");
		//kernel_add.setArg(0, buffer_C);
		//kernel_add.setArg(1, buffer_B);
		//kernel_add.setArg(2, buffer_C);

		cl::Kernel kernel_multiAdd = cl::Kernel(program, "multiAdd");
		kernel_multiAdd.setArg(0, buffer_A);
		kernel_multiAdd.setArg(1, buffer_B);
		kernel_multiAdd.setArg(2, buffer_C);


		cl::Event kernel_event;
		//queue.enqueueNDRangeKernel(kernel_multi, cl::NullRange, cl::NDRange(vector_elements), cl::NullRange);
		queue.enqueueNDRangeKernel(kernel_multiAdd, cl::NullRange, cl::NDRange(vector_elements), cl::NullRange);
		//queue.enqueueNDRangeKernel(kernel_add, cl::NullRange, cl::NDRange(vector_elements), cl::NullRange);


		//5.3 Copy the result from device to host
		queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, vector_size, &C[0]);

		std::cout << "\nA = " << A << std::endl;
		std::cout << "B = " << B << std::endl;
		std::cout << "C = " << C << std::endl;
	}
	catch (cl::Error err) {
		std::cerr << "ERROR: " << err.what() << ", " << getErrorString(err.err()) << std::endl;
	}

	system("pause");
	return 0;
	
}