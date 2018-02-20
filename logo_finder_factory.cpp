#include "logo_finder_factory.h"
#include <boost/algorithm/string.hpp>

//int decodeError = 0;

static void my_avlog_callback(void* avcl, int level, const char* fmt, va_list vl) {
    if(level == AV_LOG_ERROR ) {
        printf("%s", fmt);
        if(boost::algorithm::contains(fmt, "error while decoding MB")) {
        //if(boost::algorithm::contains(fmt, "decode_slice_header error")) {
            //decodeError = 1;
        }
    }

    av_log_default_callback(avcl, level, fmt, vl);
}

LogoFinder* LogoFinderFactory::createLogoFinder(char** argv, int argc) throw(runtime_error) {
    av_log_set_callback(my_avlog_callback);

    //FILE* file = fopen("./vs.config", "r");

    bool debugFlag = false;

    string videoName = "";
    bool help = false;
    bool showVersion = false;
    
    int minFreezeTime = 10;
    int x0 = 0, y0 = 0, frameWidth = 0, frameHeight = 0;
    string logoFilePath = "";
    
    list<string> optList;
    
    opterr = 0;
    int c;
    while((c = getopt(argc, argv, "Df:hi:l:t:v")) != -1) {
        printf("%c: '%s'\n", c, optarg);
        
        switch(c) {
            case 'f':
                sscanf(optarg, "%d,%d,%d,%d", &x0, &y0, &frameWidth, &frameHeight);
                break;
            case 'h':
                help = true;
                break;
            case 'i':
                videoName = optarg;
                break;
            case 'l':
                logoFilePath = optarg;
                break;
            case 't':
                minFreezeTime = atoi(optarg);
                break;
            case 'v':
                showVersion = true;
                break;
            case 'D':
                debugFlag = true;
                break;
        }
    }
    
    while(optind < argc) {
        optList.push_back(argv[optind]);
        optind++;
    }
    
    //--------------------------------
    
    if(showVersion) {
        printf("1.0.0\n");
        exit(0);
    }
    if(help) {
        printLogoHelp();
        exit(0);
    }
    
    Media* inputMedia = new Media(videoName);
    
    if(frameWidth == 0 || frameHeight == 0) {
        frameWidth = inputMedia->getWidth();
        frameHeight = inputMedia->getHeight();
    }

    //inputVideoName = "http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8?dummy=param.mjpg";
    
    //Video Freeze Detector
    LogoFinder* logoFinder = new LogoFinder(
        inputMedia,
        x0, y0, frameWidth, frameHeight, frameWidth, frameHeight,
        1, minFreezeTime, logoFilePath
    );

    return logoFinder;
}

void LogoFinderFactory::printLogoHelp() {
    printf("\
Sintaxe: ./freeze-detector -i -s SRC -L -t FREEZE_THRESHOLD -f x,y,W,H  -e EVENT_CMD -C CHANNEL_LABEL -c COMPONENT_NAME  -1 EVENT_FREEZE_ON -0 EVENT_FREEZE_OFF\n\
onde:\n\
\n\
-f x,y,W,H		Informa x,y,W,H como a area do vídeo a ser considerada na análise. x,y indicam as\n\
			coordenadas do ponto superior esquerdo (em pixels) e W e H indicam a largura horizontal\n\
			e a altura em pixels, respectivamente. Opcional, Default=todo o frame\n\
-h			Imprime esta mensagem\n\
-i			Habilita o modo \"Instantâneo\". Esse modo indica que o evento \"Freeze ON\" deve ser reportado\n\
			imediatamente ao ser detectado. Caso não esteja habilitado, o \"Freeze ON\" detectado não\n\
			será reportado até que ocorra um \"Freeze OFF\" ou o fim do vídeo. O default é modo\n\
			instantaneo desligado.\n\
-l CHANNEL_LABEL	Informa o texto do parametro 2 a ser passado para o comando do comando EVENTCMD.\n\
			Opcional, default=string recebido para \"SRC\" (opção -s)\n\
-t FREEZE_THRESHOLD	Informa FREEZETHRESHOLD como o tempo (em segundos) que o video deve congelar antes\n\
			de indicar um FREEZE. Opcional, Default=10.\n\
-D			Habilita mensagens de debug (que devem ser mostradas na saida de erro)\n");
}
