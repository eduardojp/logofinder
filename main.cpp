#include "logo_finder_factory.h"
//#include "freeze_errors.h"
#include <string>
#include <thread>

#include <libvideoprocessor/video_processor.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <getopt.h>
#include <libvideoprocessor/safe_queue.h>

using namespace std;

void consumeSamples(LogoFinder* logoFinder, SafeQueue<Sample*>* sampleBuffer) {
    while(true) {
        Sample* sample = sampleBuffer->pop();
        
        if(!sample) {
            break;
        }
        
        logoFinder->processSample(sample);
    }
}

int main(int argc, char** argv) {
    try {
        avformat_network_init();
        av_register_all();
        
        //----------------------------------
        
        //Logo
        LogoFinder* logoFinder = LogoFinderFactory::createLogoFinder(argv, argc);
        
        list<Normalizer*> audioNormalizerList;
        list<Normalizer*> videoNormalizerList;
        list<thread*> threadList;
        
        Normalizer* normalizer = logoFinder->getNormalizer();
        videoNormalizerList.push_back(normalizer);
        threadList.push_back(new thread(consumeSamples, logoFinder, normalizer->getSampleBuffer()));

        Media* inputMedia = logoFinder->getInputMedia();
        Media* outputMedia = 0;
        
        //----------------------------------
        
        VideoProcessor videoProcessor;
        videoProcessor.processStream(inputMedia, outputMedia, &videoNormalizerList, &audioNormalizerList);
        
        for(auto it = threadList.begin(); it != threadList.end(); it++) {
            (*it)->join();
        }
        
        //----------------------------------
        
//        if(videoFreezeDetector) delete videoFreezeDetector;
//        if(audioFreezeDetector) delete audioFreezeDetector;
        
        while(threadList.size() > 0) {
            thread* consumerThread = threadList.front();
            threadList.pop_front();
            delete consumerThread;
        }
        
        delete inputMedia;
        if(outputMedia) delete outputMedia;
    }
    catch(runtime_error& e) {
        printf("ERROR %s\n", e.what());
        fflush(stdout);
        exit(-1);
    }

    return 0;
}
