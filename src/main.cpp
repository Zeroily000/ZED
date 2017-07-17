///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


/******************************************************************************************************************
 ** This sample demonstrates how to use two ZEDs with the ZED SDK, each grab are in a separated thread            **
 ** This sample has been tested with 3 ZEDs in HD720@30fps resolution                                             **
 ** This only works for Linux                                                                                     **
 *******************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // system
#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <sl/Camera.hpp>

// using namespace std;

// Define state
#define DISPLAY 0
#define RECORD 1
#define PAUSE 2

const int NUM_CAMERAS = 2;
const int FPS = 15;
const sl::RESOLUTION ZED_RES = sl::RESOLUTION_HD720;
// const sl::RESOLUTION ZED_RES = sl::RESOLUTION_HD720, RESOLUTION_VGA;


sl::Camera* zed[NUM_CAMERAS];
cv::Mat View[NUM_CAMERAS];
cv::Mat Display[NUM_CAMERAS];

// cv::Mat LeftView[NUM_CAMERAS];
// cv::Mat RightView[NUM_CAMERAS];


int width, height;
std::vector<unsigned long long> Timestamp[NUM_CAMERAS];

bool stop_signal;

int state = DISPLAY;

std::vector<sl::Mat> LeftVedio[NUM_CAMERAS];
std::vector<sl::Mat> RightVedio[NUM_CAMERAS];

std::string LeftVedioFolder[NUM_CAMERAS];
std::string RightVedioFolder[NUM_CAMERAS];
std::string TimestampFolder[NUM_CAMERAS];

std::string foldername = "data";

void grab_run(int x) {
    sl::Mat tmp_left, tmp_right;
    sl::RuntimeParameters rt_params;
    while (!stop_signal) {
        // sl::ERROR_CODE res = zed[x]->grab(rt_params);

        if (zed[x]->grab() == sl::SUCCESS) {
        // if (res != sl::SUCCESS) {
            // Timestamp[x].push_back(zed[x]->getCameraTimestamp());

            // zed[x]->retrieveImage(aux, sl::VIEW_LEFT, sl::MEM_CPU);
            // cv::Mat(aux.getHeight(), aux.getWidth(), CV_8UC4, aux.getPtr<sl::uchar1>(sl::MEM_CPU)).copyTo(SbSResult[x](cv::Rect(0, 0, width, height)));
            // // zed[x]->retrieveImage(aux, sl::VIEW_DEPTH, sl::MEM_CPU);
            // zed[x]->retrieveImage(aux, sl::VIEW_RIGHT, sl::MEM_CPU);
            // cv::Mat(aux.getHeight(), aux.getWidth(), CV_8UC4, aux.getPtr<sl::uchar1>(sl::MEM_CPU)).copyTo(SbSResult[x](cv::Rect(width, 0, width, height)));

            // test
            zed[x]->retrieveImage(tmp_left, sl::VIEW_LEFT, sl::MEM_CPU);
            cv::Mat(tmp_left.getHeight(), tmp_left.getWidth(), CV_8UC4, tmp_left.getPtr<sl::uchar1>(sl::MEM_CPU)).copyTo(View[x](cv::Rect(0, 0, width, height)));

            zed[x]->retrieveImage(tmp_right, sl::VIEW_RIGHT, sl::MEM_CPU);
            cv::Mat(tmp_right.getHeight(), tmp_right.getWidth(), CV_8UC4, tmp_right.getPtr<sl::uchar1>(sl::MEM_CPU)).copyTo(View[x](cv::Rect(width, 0, width, height)));
            // end test
            // std::cout << " Timestamp " << x << ": " << Timestamp[x].back() << std::endl;
            // PRE_Timestamp[x] = ZED_Timestamp[x];
            // time_t epch = ZED_Timestamp[x];
            // printf("%i -> %s", epch, asctime(gmtime(&epch)));
            if(state == RECORD){
                LeftVedio[x].push_back(sl::Mat(tmp_left));
                RightVedio[x].push_back(sl::Mat(tmp_right));
                Timestamp[x].push_back(zed[x]->getCameraTimestamp());
            }
        }
        // sl::sleep_ms(1);
    }
    delete zed[x];
}


int main(int argc, char **argv) {

#ifdef WIN32
    std::cout << "Multiple ZEDs are not available under Windows" << std::endl;
    return -1;
#endif

    // Default
    sl::InitParameters params;
    params.depth_mode = sl::DEPTH_MODE_NONE;
    // params.depth_mode = sl::DEPTH_MODE_PERFORMANCE;
    params.camera_resolution = ZED_RES;
    params.camera_fps = FPS;


    std::string res;
    // Resolution
    if(argc > 1){
        res = argv[1];
        if(res == "2K"){
            params.camera_resolution = sl::RESOLUTION_HD2K;
            params.camera_fps = 15;
        }
        else if(res == "1080")
            params.camera_resolution = sl::RESOLUTION_HD1080;
        else if(res == "720")
            params.camera_resolution = sl::RESOLUTION_HD720;
        else if(res == "VGA")
            params.camera_resolution = sl::RESOLUTION_VGA;
        else
            std::cout << "Warrning: wrong resolution input" << std::endl;
    }

    // Resolution & frame
    if(argc > 2){
        params.camera_fps = std::atoi(argv[2]);
    }

    // Folder name
    if(argc >3)
        foldername = argv[3];
    
    // Get current dir
    char cwd[1024];
    std::string dir;
    if (getcwd(cwd, sizeof(cwd)) != NULL){
        dir = cwd;
        // std::size_t found = dir.find_last_of("/\\");
        std::size_t found = dir.find_last_of("/\\");
        dir = dir.substr(0,found+1);
        // std::cout << dir << std::endl;
    }
    else
        perror("getcwd() error");
    

    // Create every ZED and init them
    for (int i = 0; i < NUM_CAMERAS; i++) {
        zed[i] = new sl::Camera();
        params.camera_linux_id = i;

        sl::ERROR_CODE err = zed[i]->open(params);

        std::cout << "ZED no. " << i << " -> Result : " << sl::errorCode2str(err) << std::endl;
        if (err != sl::SUCCESS) {
            delete zed[i];
            return 1;
        }

        width = zed[i]->getResolution().width;
        height = zed[i]->getResolution().height;
        View[i] = cv::Mat(height, width * 2, CV_8UC4, 1);

        LeftVedioFolder[i] = foldername + "/cam" + std::to_string(i) + "/left/";
        RightVedioFolder[i] = foldername + "/cam" + std::to_string(i) + "/right/";
        TimestampFolder[i] = foldername + "/cam" + std::to_string(i) + "/";
        // LeftVedioFolder[i] = "/data/cam" + std::to_string(i) + "calibrate" + "/left/";
        // RightVedioFolder[i] = "/data/cam" + std::to_string(i) + "calibrate" + "/right/";
        // TimestampFolder[i] = "/data/cam" + std::to_string(i) + "calibrate/";
    }
    
    
    char key = ' ';
    // Create each grabbing thread with the camera number as parameters
    std::vector<std::thread*> thread_vec;
    for (int i = 0; i < NUM_CAMERAS; i++)
        thread_vec.push_back(new std::thread(grab_run, i));


    // Create windows for viewing results with OpenCV
    cv::Size DisplaySize(width, height/2);

    for (int i = 0; i < NUM_CAMERAS; i++)
        Display[i] = cv::Mat(DisplaySize, CV_8UC4);


    // Loop until 'q' is pressed
    std::string Win[NUM_CAMERAS];
    for(int i = 0; i < NUM_CAMERAS; i++){
        Win[i] = "Camera No. " + std::to_string(i);
    }
    while (key != 'q') {
        // Resize and imshow
        for (int i = 0; i < NUM_CAMERAS; i++) {
            cv::resize(View[i], Display[i], DisplaySize);
            cv::imshow(Win[i], Display[i]);
            // cv::imshow(Win[i], View[i]);
        }
        key = cv::waitKey(20);

        // // Compare Timestamp between both camera (uncomment following line)
        // for (int i = 0; i < NUM_CAMERAS; i++) std::cout << " Timestamp " << i << ": " << ZED_Timestamp[i] << std::endl;
        // std::cout << " Timestamp " << 0 << ": " << ZED_Timestamp[0] << std::endl;
        

        if(key == 'r'){
            state = RECORD;
            std::cout << "Recording..." << std::endl;
        }

        if(key == 'p'){
            state = PAUSE;
            std::cout << "Pause" << std::endl;
            std::cout << "Vedio has " << LeftVedio[0].size() << " frames" << std::endl;
        }

    }

    // Get FPS
    // for(int i = 0; i < NUM_CAMERAS; i ++)
    //     std::cout << "FPS of camera " << i << " is " << zed[i]->getCurrentFPS() << std::endl;

    // Send the signal to stop every threads to finish
    stop_signal = true;
    // Wait for every thread to be stopped
    for (auto it : thread_vec){
        it->join();
        delete it;
    }

    // Close window
    for(int i = 0; i < NUM_CAMERAS; i++)
        cv::destroyWindow(Win[i]);
    std::cout << "Done" << std::endl;

    // Total frames
    for(int i = 0; i < NUM_CAMERAS; i++){
        if(LeftVedio[i].empty()) return 0;
        std::cout << "Frames of camera" << std::to_string(i) << ": " << LeftVedio[i].size() << std::endl;
    }


    // Delete existing folders
    struct stat sb;
    std::string path = dir + foldername;
    std::cout << path << std::endl;
    std::string cmd;
    int sys;
    if(stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)){
        cmd = "rm -r " + path;
        sys = system(cmd.c_str());
    }

    // Create folders
    for(int i = 0; i < NUM_CAMERAS; i++){
        cmd = "mkdir -p " + dir + TimestampFolder[i];
        sys = system(cmd.c_str());

        cmd =  "mkdir -p " + dir + LeftVedioFolder[i];
        sys = system(cmd.c_str());

        cmd = "mkdir -p " + dir + RightVedioFolder[i];
        sys = system(cmd.c_str());
        
    }

    // Save vedio
    std::cout << "Saving..." << std::endl;
    std::ofstream outs[NUM_CAMERAS];
    for(int i = 0; i < NUM_CAMERAS; i++){
        // Left view
        for(int f = 0; f < LeftVedio[i].size(); f++){
            std::string filename = dir + LeftVedioFolder[i] + std::to_string(Timestamp[i][f]) + ".png";
            LeftVedio[i][f].write(filename.c_str());
        }

        // Right view
        for(int f = 0; f < RightVedio[i].size(); f++){
            std::string filename = dir + RightVedioFolder[i] + std::to_string(Timestamp[i][f]) + ".png";
            RightVedio[i][f].write(filename.c_str());
        }

        // Timestamp
        std::string filename = dir + TimestampFolder[i] + "/timestamp" + std::to_string(i) + ".txt";
        outs[i].open(filename, std::ofstream::out);
        for(int f = 0; f < Timestamp[i].size()-1; f++){
            outs[i] << Timestamp[i][f] << '\n';
        }
        outs[i] << Timestamp[i].back();
        outs[i].close();
    }
    std::cout << "Done" << std::endl;

    return 0;
}