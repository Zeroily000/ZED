# Stereolabs ZED - Using multiple ZED

Use multiple ZED cameras in a single application. Currently available on Linux only. USB 3.0 is recommended. USB 2.0 might cause unstable.

## Prerequisites

- Ubuntu 16.04
- [CUDA](https://developer.nvidia.com/cuda-downloads)
- [OpenCV 3.1](http://opencv.org/releases.html) with CUDA on. If use CUDA 8.0 or higher, modify 'opencv-3.1.0/modules/cudalegacy/src/graphcuts.cpp'. Comment line 45 and add

    #if !defined (HAVE_CUDA) || defined (CUDA_DISABLER) || (CUDART_VERSION >= 8000)

- [ZED SDK](https://www.stereolabs.com/developers/release/2.1/#sdkdownloads_anchor)




## Build the program

In the directory, open a terminal and execute the following command:

    mkdir build
    cd build
    cmake ..
    make

## Run the program

Open a terminal in the 'build' directory and execute the following command:

    ./MultiCameras [Resolution] [FPS] [Folder]

The default value for Resolution is 720, FPS 15, and Folder 'data'.

## How it works

- Video capture for each camera is done in a separate thread for optimal performance. You can specify the number of ZED used by changing the `NUM_CAMERAS` parameter.
- OpenCV is used to display the images in 1/4 size of the captured images.
- Press 'r' to record.
- Press 'p' to pause and 'r' to resume.
- Press 'q' to quit and save each frames.
- Use timestamps as the filename of every frame.
- Frames are saved as

	data
		cam0
			timestamp0.txt
			left
			right
		cam1
			timestamp1.txt
			left
			right
		...

