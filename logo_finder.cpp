#include "logo_finder.h"

LogoFinder::LogoFinder(
    Media* inputMedia,
    int roiX0, int roiY0, int roiWidth, int roiHeight,
    int sampleWidth, int sampleHeight, int samplesPerSecond, int windowPeriod,
    string logoFilePath)
    : VideoFilter(inputMedia, roiX0, roiY0, roiWidth, roiHeight, sampleWidth, sampleHeight, windowPeriod, samplesPerSecond)
{
    count = 0;
    pngProcessor = new PngProcessor();
    pngProcessor->readPngFile(logoFilePath);
    
    for(int y = 0; y < pngProcessor->height; y++) {
        png_bytep row = pngProcessor->row_pointers[y];

        for(int x = 0; x < pngProcessor->width; x++) {
            png_bytep px = &(row[x * 4]);

            if(px[3] == 0) {
                continue;
            }
            
            if(px[0] == 0x00) {
                blackIndices.push_back(x + y*pngProcessor->width);
            }
            else if(px[0] == 0xFF) {
                whiteIndices.push_back(x + y*pngProcessor->width);
            }
            else {
                grayIndices.push_back(x + y*pngProcessor->width);
            }
        }
    }
    
    for(int y = 0; y < pngProcessor->height; y++) {
        png_bytep row = pngProcessor->row_pointers[y];

        for(int x = 0; x < pngProcessor->width; x++) {
            png_bytep px = &(row[x * 4]);
            
            if(px[3] == 0) {
                continue;
            }
            
        }
    }
}

LogoFinder::~LogoFinder() {
    for(list<Sample*>::iterator it = lastSamples.begin(); it != lastSamples.end(); it++) {
        Sample* sample = (*it);
        delete sample;
    }

    delete pngProcessor;
}

void LogoFinder::processSample(Sample* videoSample) throw(runtime_error) {
    
    if(!lastSamples.empty()) {
        lastSamples.back()->timeMillisEnd = videoSample->timeMillisBegin;

        if(lastSamples.back()->timeMillisEnd - lastSamples.front()->timeMillisBegin >= processingPeriodMillis) {
//            printf("SIZE: %ld / %ld,%ld\n", lastSamples.size(), lastSamples.back()->timeMillisEnd, lastSamples.front()->timeMillisBegin);
//            printf(">>> %ld >= %d\n", lastSamples.back()->timeMillisEnd - lastSamples.front()->timeMillisBegin, meanSampleTimeMillis);
//            printf("meanSampleTimeMillis: %d\n", meanSampleTimeMillis);
            
            Sample* newSample = extractMeanSample();
            bool readyToCheckLogo = updateProcessedSampleList(newSample);
            
            if(readyToCheckLogo) {
                //checkLogo2();
                checkOpaqueLogo();
            }
        }
    }
    
    //--------------------------------

    lastSamples.push_back(videoSample);
}

void LogoFinder::checkLogo() throw(runtime_error) {
    Sample* meanSample = new Sample(pngProcessor->width, pngProcessor->height);
    
    for(int y = 0; y < pngProcessor->height; y++) {
        for(int x = 0; x < pngProcessor->width; x++) {
            int index = x + y*meanSample->width;
            
            for(auto it = lastProcessedSamples.begin(); it != lastProcessedSamples.end(); it++) {
                Sample* sample = (*it);
                meanSample->values[index] += sample->values[index];
            }
            
            meanSample->values[index] /= lastProcessedSamples.size();
        }
    }

    //-----------------------------------
        
    int backgroundSum = 0;
    for(auto it = grayIndices.begin(); it != grayIndices.end(); it++) {
        int index = (*it);
        
        backgroundSum += meanSample->values[index];
    }
    int backgroundMean = backgroundSum / grayIndices.size();
    
    //-----------------------------------

    int verifiedBrighter = 0;
    for(auto it = whiteIndices.begin(); it != whiteIndices.end(); it++) {
        int index = (*it);

        if(meanSample->values[index] > backgroundMean) {
            verifiedBrighter++;
        }
    }
    
    //-----------------------------------

    int verifiedDarker = 0;
    for(auto it = blackIndices.begin(); it != blackIndices.end(); it++) {
        int index = (*it);

        if(meanSample->values[index] < backgroundMean) {
            verifiedDarker++;
        }
    }
    
    //-----------------------------------

    if((double) verifiedBrighter / (double) whiteIndices.size() > 0.9 ||
       (double) verifiedDarker / (double) blackIndices.size() > 0.9) {
        printf("LOGO FOUND: %ld\n", (lastProcessedSamples.front()->timeMillisBegin + lastProcessedSamples.front()->timeMillisEnd) / 2);
    }
    else {
        printf("------------------------ LOGO NOT FOUND!!!\n");
    }
}

void LogoFinder::checkLogo2() throw(runtime_error) {
    Sample* meanSample = new Sample(pngProcessor->width, pngProcessor->height);
    meanSample->timeMillisBegin = (lastProcessedSamples.front()->timeMillisBegin + lastProcessedSamples.front()->timeMillisEnd) / 2;
    meanSample->timeMillisEnd = meanSample->timeMillisBegin;
    
    for(int y = 0; y < pngProcessor->height; y++) {
        for(int x = 0; x < pngProcessor->width; x++) {
            int index = x + y*meanSample->width;
            
            for(auto it = lastProcessedSamples.begin(); it != lastProcessedSamples.end(); it++) {
                Sample* sample = (*it);
                meanSample->values[index] += sample->values[index];
            }
            
            meanSample->values[index] /= lastProcessedSamples.size();
        }
    }

    //-----------------------------------

    int okBlackPixels = 0;
    int totalBlackPixels = 0;
    int okWhitePixels = 0;
    int totalWhitePixels = 0;
    int WINDOW_RADIUS = 4;
    for(int y = WINDOW_RADIUS; y < pngProcessor->height-WINDOW_RADIUS; y++) {
        png_bytep row = pngProcessor->row_pointers[y];
        
        for(int x = WINDOW_RADIUS; x < pngProcessor->width-WINDOW_RADIUS; x++) {
            png_bytep px = &(row[x * 4]);
            
            if(px[0] != 0x00 && px[0] != 0xFF) {
                continue;
            }
            
            double sum = 0;
            int total = 0;
            
            for(int h = -WINDOW_RADIUS; h <= WINDOW_RADIUS; h++) {
                png_bytep row2 = pngProcessor->row_pointers[y+h];
                
                for(int w = -WINDOW_RADIUS; w <= WINDOW_RADIUS; w++) {
                    png_bytep px2 = &(row2[(x+w) * 4]);
                    int index = (x+w) + (y+h)*meanSample->width;
                    
                    if(px[0] == px2[0]) {
                        continue;
                    }
                    
                    sum += meanSample->values[index];
                    total++;
                }
            }
            
            if(total > 0) {
//                printf(">>>>>>>>>>>> TOTAL: %d\n", total);
                double localMean = (double) sum / (double) total;
                
                //Black
                if(px[0] == 0x00) {
                    if(meanSample->values[x + y*meanSample->width] < localMean) {
                        okBlackPixels++;
                        
                        //DEBUG
//                        if(meanSample->timeMillisBegin == 1304766) {
//                            printf("%lf < %lf\n", meanSample->values[x + y*meanSample->width], localMean);
//                        }
                    }
                    totalBlackPixels++;
                }
                //White
                else {
                    if(meanSample->values[x + y*meanSample->width] > localMean) {
                        okWhitePixels++;
                        
                        //DEBUG
//                        if(meanSample->timeMillisBegin == 1304766) {
//                            printf("%lf > %lf\n", meanSample->values[x + y*meanSample->width], localMean);
//                        }
                    }
                    totalWhitePixels++;
                }
            }
        }
    }
    
    //-----------------------------------
    
    /*if(
        meanSample->timeMillisBegin == 1139766 ||
        meanSample->timeMillisBegin == 1140766 ||
        meanSample->timeMillisBegin == 1141766 ||

        meanSample->timeMillisBegin == 1304766 ||
        meanSample->timeMillisBegin == 1305766 ||
        meanSample->timeMillisBegin == 1306766 ||
        meanSample->timeMillisBegin == 1307766 ||
        meanSample->timeMillisBegin == 1308766 ||
        meanSample->timeMillisBegin == 1309766 ||
        meanSample->timeMillisBegin == 1310766 ||
        meanSample->timeMillisBegin == 1311766 ||
        meanSample->timeMillisBegin == 1312766
    )*/ {
        PngProcessor* processor = new PngProcessor();
        processor->readPngFile("/home/eduardo/axn.png");

        for(int y = 0; y < processor->height; y++) {
            png_bytep row = processor->row_pointers[y];

            for(int x = 0; x < processor->width; x++) {
                png_bytep px = &(row[x * 4]);

                px[0] = meanSample->values[x + y*meanSample->width];
                px[1] = meanSample->values[x + y*meanSample->width];
                px[2] = meanSample->values[x + y*meanSample->width];
                px[3] = 255;
            }
        }

        char fileName[256];
        sprintf(fileName, "/home/eduardo/band_logos/logo%ld.png", meanSample->timeMillisBegin);
        processor->writePngFile(fileName);
    }

    if((double) okBlackPixels / (double) totalBlackPixels > 0.95 ||
       (double) okWhitePixels / (double) totalWhitePixels > 0.95) {
        printf("LOGO FOUND: %ld\n", meanSample->timeMillisBegin);
    }
    else {
        printf("------------------------ LOGO NOT FOUND!!!\n");
    }
    
    delete meanSample;
}

void LogoFinder::checkOpaqueLogo() throw(runtime_error) {
    Sample* meanSample = new Sample(pngProcessor->width, pngProcessor->height);
    meanSample->timeMillisBegin = (lastProcessedSamples.front()->timeMillisBegin + lastProcessedSamples.front()->timeMillisEnd) / 2;
    meanSample->timeMillisEnd = meanSample->timeMillisBegin;
    
    //Geração do frame médio
    for(int y = 0; y < pngProcessor->height; y++) {
        for(int x = 0; x < pngProcessor->width; x++) {
            int index = x + y*meanSample->width;
            
            for(auto it = lastProcessedSamples.begin(); it != lastProcessedSamples.end(); it++) {
                Sample* sample = (*it);
                meanSample->values[index] += sample->values[index];
            }
            
            meanSample->values[index] /= lastProcessedSamples.size();
        }
    }

    //-----------------------------------

    //Cálculo do erro médio entre o template e o frame médio
    double errorSum = 0;
    int total = 0;
    for(int y = 0; y < pngProcessor->height; y++) {
        png_bytep row = pngProcessor->row_pointers[y];
        
        for(int x = 0; x < pngProcessor->width; x++) {
            png_bytep px = &(row[x * 4]);
            
            if(px[3] == 0x00) {
                continue;
            }
            
            int index = x + y*meanSample->width;
            double diff = meanSample->values[index] - (px[0]+px[1]+px[2])/3;
            errorSum += diff < 0 ? -diff : diff;
            total++;
        }
    }
    
    double meanError = errorSum / total;
    printf("ERROR: %lf\n", meanError);
    
    
    //-----------------------------------
    
    /*if(
        meanSample->timeMillisBegin == 1139766 ||
        meanSample->timeMillisBegin == 1140766 ||
        meanSample->timeMillisBegin == 1141766 ||

        meanSample->timeMillisBegin == 1304766 ||
        meanSample->timeMillisBegin == 1305766 ||
        meanSample->timeMillisBegin == 1306766 ||
        meanSample->timeMillisBegin == 1307766 ||
        meanSample->timeMillisBegin == 1308766 ||
        meanSample->timeMillisBegin == 1309766 ||
        meanSample->timeMillisBegin == 1310766 ||
        meanSample->timeMillisBegin == 1311766 ||
        meanSample->timeMillisBegin == 1312766
    )*/
//    {
//        PngProcessor* processor = new PngProcessor();
//        processor->readPngFile("/home/eduardo/espn_1086,57,135,33.png");
//
//        for(int y = 0; y < processor->height; y++) {
//            png_bytep row = processor->row_pointers[y];
//
//            for(int x = 0; x < processor->width; x++) {
//                png_bytep px = &(row[x * 4]);
//
//                px[0] = meanSample->values[x + y*meanSample->width];
//                px[1] = meanSample->values[x + y*meanSample->width];
//                px[2] = meanSample->values[x + y*meanSample->width];
//                px[3] = 255;
//            }
//        }
//
//        char fileName[256];
//        sprintf(fileName, "/home/eduardo/band_logos/logo%ld.png", meanSample->timeMillisBegin);
//        processor->writePngFile(fileName);
//    }
    
    printf("PERCENT: %lf\n", (255-meanError) / 255.0);

    if((255-meanError) / 255.0 > 0.95) {
        printf("LOGO FOUND: %ld\n", meanSample->timeMillisBegin);
    }
    else {
        printf("------------------------ LOGO NOT FOUND!!!\n");
    }
    
    delete meanSample;
}

void LogoFinder::extractLogo(Sample* videoSample) throw(runtime_error) {
    
    if(!lastSamples.empty()) {
        lastSamples.back()->timeMillisEnd = videoSample->timeMillisBegin;

        if(lastSamples.back()->timeMillisEnd - lastSamples.front()->timeMillisBegin >= processingPeriodMillis) {
//            printf("SIZE: %ld / %ld,%ld\n", lastSamples.size(), lastSamples.back()->timeMillisEnd, lastSamples.front()->timeMillisBegin);
//            printf(">>> %ld >= %d\n", lastSamples.back()->timeMillisEnd - lastSamples.front()->timeMillisBegin, meanSampleTimeMillis);
//            printf("meanSampleTimeMillis: %d\n", meanSampleTimeMillis);
            
            Sample* newSample = extractMeanSample();
            bool readyToCheckLogo = updateProcessedSampleList(newSample);

            if(readyToCheckLogo) {
                for(int y = 0; y < pngProcessor->height; y++) {
                    png_bytep row = pngProcessor->row_pointers[y];

                    for(int x = 0; x < pngProcessor->width; x++) {
                        png_bytep px = &(row[x * 4]);
                        // Do something awesome for each pixel here...
                        //printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);

                        int sumR = 0;
                        int sumG = 0;
                        int sumB = 0;
                        
                        for(auto it = lastProcessedSamples.begin(); it != lastProcessedSamples.end(); it++) {
                            Sample* sample = (*it);
                            
                            sumR += sample->values[x + y*sample->width];
                            sumG += sample->values[x + y*sample->width];
                            sumB += sample->values[x + y*sample->width];
                        }
                        
                        px[0] = sumR / lastProcessedSamples.size();
                        px[1] = sumG / lastProcessedSamples.size();
                        px[2] = sumB / lastProcessedSamples.size();
                        px[3] = 255;
                        
                        //printf("%d = %d / %d\n", px[0], sumR, lastMeanSamples.size());
                    }
                }

                char fileName[256];
                sprintf(fileName, "/home/eduardo/logo%d.png", count++);
                pngProcessor->writePngFile(fileName);
            }
        }
    }
    
    //--------------------------------

    lastSamples.push_back(videoSample);
}