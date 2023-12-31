/* AUDIO PLAY FOR INGENIC, BASED OFF SAMPLES */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "imp/imp_audio.h"
#include "imp/imp_log.h"
#include "version.h"

const char *TAG = "AO_T31";
const int AO_TEST_SAMPLE_RATE = 16000;
const int AO_TEST_SAMPLE_TIME = 1;

#ifdef __UCLIBC__
#define AO_TEST_BUF_SIZE (AO_TEST_SAMPLE_RATE * sizeof(short) * AO_TEST_SAMPLE_TIME / 1000)
#else
const int AO_TEST_BUF_SIZE = AO_TEST_SAMPLE_RATE * sizeof(short) * AO_TEST_SAMPLE_TIME / 1000;
#endif

const int DEFAULT_VOLUME = 10;
const int DEFAULT_GAIN = 28;

typedef struct {
    FILE *input;
    int chnVol;
    int aogain;
} AudioConfig;

/* Display the usage of the program */
void usage() {
    printf("AUDIOPLAY for INGENIC\n");
    printf("Info: 16khz, raw pcm_s16le, mono format supported.\n");
    printf("Usage: audioplay_t31 [-s|--stdin] <file> <vol 0-100> <gain> \n");
    printf("       -s, --stdin: Read audio data from stdin\n");
    exit(1);
}

/* Open the audio source, either from stdin or from a file */
FILE* open_audio_source(const char *source) {
    if (strcmp(source, "-s") == 0 || strcmp(source, "--stdin") == 0) {
        return stdin;
    } else {
        FILE *file = fopen(source, "rb");
        if (!file) {
            IMP_LOG_ERR(TAG, "[ERROR] Could not open file: %s\n", source);
            return NULL;
        }
        return file;
    }
}

/* Handle any audio error by logging the error message */
int handle_audio_error(const char *msg) {
    IMP_LOG_ERR(TAG, "%s", msg);
    return -1;
}

void *ao_test_play_thread(void *arg) {
    IMP_LOG_INFO(TAG, "Starting AUDIOPLAY for INGENIC\n");
    AudioConfig *config = (AudioConfig *)arg;

    unsigned char *buf = (unsigned char *)malloc(AO_TEST_BUF_SIZE);
    if (!buf) {
        handle_audio_error("[FATAL] malloc audio buf error");
        return NULL;
    }

    FILE *play_file = config->input;
    if (!play_file) {
        free(buf);
        return NULL;
    }

    int devID = 0;
    IMPAudioIOAttr attr = {
        .samplerate = AUDIO_SAMPLE_RATE_16000,
        .bitwidth = AUDIO_BIT_WIDTH_16,
        .soundmode = AUDIO_SOUND_MODE_MONO,
        .frmNum = 20,
        .numPerFrm = 640,  // Number of samples per frame
        .chnCnt = 1
    };

    if (IMP_AO_SetPubAttr(devID, &attr) || IMP_AO_GetPubAttr(devID, &attr) || IMP_AO_Enable(devID) ||
        IMP_AO_EnableChn(devID, 0) || IMP_AO_SetVol(devID, 0, config->chnVol) ||
        IMP_AO_SetGain(devID, 0, config->aogain)) {
        free(buf);
        return NULL;
    }

    int size;
    while ((size = fread(buf, 1, AO_TEST_BUF_SIZE, play_file)) == AO_TEST_BUF_SIZE) {
        IMPAudioFrame frm = {.virAddr = (uint32_t *)buf, .len = size};
        if (IMP_AO_SendFrame(devID, 0, &frm, BLOCK)) {
            handle_audio_error("IMP_AO_SendFrame data error");
            free(buf);
            return NULL;
        }
    }

    if (IMP_AO_FlushChnBuf(devID, 0) || IMP_AO_DisableChn(devID, 0) || IMP_AO_Disable(devID)) {
        free(buf);
        return NULL;
    }

    free(buf);
    pthread_exit(0);
}

int ao_basic_test(AudioConfig *config) {
    pthread_t play_thread_id;
    printf("[INFO] Playing from %s\n", (config->input == stdin) ? "stdin" : "file");
    printf("[INFO] Volume is set to %d.\n", config->chnVol);
    printf("[INFO] Gain is set to %d.\n", config->aogain);

    int ret = pthread_create(&play_thread_id, NULL, ao_test_play_thread, config);
    if (ret) return handle_audio_error("[FATAL] pthread_create failed");

    pthread_join(play_thread_id, NULL);
    return 0;
}

int main(int argc, char *argv[]) {
    printf("INGENIC AUDIOPLAY_T31 Version: %s\n", VERSION);

    if (argc < 2) usage();

    AudioConfig config = {
        .input = open_audio_source(argv[1]),
        .chnVol = (argc > 2) ? atoi(argv[2]) : DEFAULT_VOLUME,
        .aogain = (argc > 3) ? atoi(argv[3]) : DEFAULT_GAIN
    };

    if (!config.input) return 1;

    int ret = ao_basic_test(&config);
    if (ret) printf("Error in ao_basic_test\n");

    if (config.input != stdin) {
        fclose(config.input);
    }

    return ret;
}
