#ifndef LOGO_FINDER_FACTORY_H
#define LOGO_FINDER_FACTORY_H

#include <list>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/common.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
}

#include "libvideoprocessor/utils.h"
#include "libvideoprocessor/video_processor.h"
#include "logo_finder.h"

using namespace std;

/**
 *
 * @author Eduardo
 */
class LogoFinderFactory {
    public:
        static LogoFinder* createLogoFinder(char** argv, int argc) throw(runtime_error);
        static void printLogoHelp();
};

#endif