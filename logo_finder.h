#ifndef LOGO_FINDER_H
#define LOGO_FINDER_H

#include <list>
#include <ctime>
#include <libvideoprocessor/video_filter.h>
#include <libvideoprocessor/png_processor.h>
#include <libvideoprocessor/utils.h>
#include <libvideoprocessor/media.h>
#include <libvideoprocessor/sample.h>
#include <libvideoprocessor/normalizer.h>
#include <libvideoprocessor/timer.h>

using namespace std;

/**
 *
 * @author Eduardo
 */
class LogoFinder : public VideoFilter {
    private:
        PngProcessor* pngProcessor;
        list<int> whiteIndices;
        list<int> blackIndices;
        list<int> grayIndices;
        int count;
    
    public:
        LogoFinder(
            Media* inputMedia,
            int roiX0, int roiY0, int roiWidth, int roiHeight,
            int sampleWidth, int sampleHeight, int samplesPerSecond, int windowPeriod,
            string logoFilePath);
        ~LogoFinder();
        
        void processSample(Sample* videoSample) throw(runtime_error);
        void checkLogo() throw(runtime_error);
        void checkLogo2() throw(runtime_error);
        void checkOpaqueLogo() throw(runtime_error);
        void extractLogo(Sample* videoSample) throw(runtime_error);
};

#endif