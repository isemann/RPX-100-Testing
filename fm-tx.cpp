/******************************************************************************
 * C++ source of RPX-100
 *
 * File:   fm-tx.cpp
 * Author: Bernhard Isemann
 *
 * Created on 30 May 2021, 11:35
 * Updated on 05 Aug 2021, 15:18
 * Version 1.00
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>
#include <syslog.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <chrono>
#include <cstring>
#include <bitset>
#include "ini.h"
#include "log.h"
#include "lime/LimeSuite.h"
#include <chrono>
#include <math.h>
#include "liquid/liquid.h"

using namespace std;
lms_device_t *device = NULL;
std::stringstream msg;
std::stringstream HEXmsg;
uint8_t *setGPIO;
uint8_t readGPIO;
uint8_t setRX = 0x00;     // all other bit = 0 --> 6m
uint8_t setTXwoBP = 0x0F; // all other bit = 0 --> direct path without BP
uint8_t setTX6m = 0x03;   // all other bit = 0 --> 6m with BP
uint8_t setTX2m = 0x07;   // all other bit = 0 --> 2m with BP
uint8_t setTX70cm = 0x0B; // all other bit = 0 --> 70cm with BP
float centerFrequency = 52.8e6;
string mode = "TX6m";
float normalizedGain = 1;
float modFactor = 0.2f;
int modeSelector = 2;
int duration = 10;
float toneFrequency = 2e3;
float sampleRate = 2e6;

int error();
void print_gpio(uint8_t gpio_val);

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        cout << "Starting RPX-100 with default settings:\n";
        cout << "Mode: " << mode << endl;
        cout << "Frequency: " << centerFrequency << endl;
        cout << "Gain: " << normalizedGain << endl;
        cout << "Tone Frequency: " << toneFrequency << endl;
        cout << "Modulation Factor: " << modFactor << endl;
        cout << "Duration: " << duration << endl;
        cout << "Sample Rate: " << sampleRate << endl;
        cout << endl;
        cout << "type \033[36m'fm-tx help'\033[0m to see all options !" << endl;
    }
    else if (argc >= 2)
    {
        for (int c = 0; c < argc; c++)
        {
            switch (c)
            {
            case 1:
                mode = (string)argv[c];
                if (mode == "RX")
                {
                    cout << "Starting RPX-100 with following setting:\n";
                    cout << "Mode: " << argv[c] << endl;
                    modeSelector = 0;
                    setGPIO = &setRX;
                }
                else if (mode == "TXwoBP")
                {
                    cout << "Starting RPX-100 with following setting:\n";
                    cout << "Mode: " << argv[c] << endl;
                    modeSelector = 1;
                    setGPIO = &setTXwoBP;
                }
                else if (mode == "TX6m")
                {
                    cout << "Starting RPX-100 with following setting:\n";
                    cout << "Mode: " << argv[c] << endl;
                    modeSelector = 2;
                    setGPIO = &setTX6m;
                }
                else if (mode == "TX2m")
                {
                    cout << "Starting RPX-100 with following setting:\n";
                    cout << "Mode: " << argv[c] << endl;
                    modeSelector = 3;
                    setGPIO = &setTX2m;
                }
                else if (mode == "TX70cm")
                {
                    cout << "Starting RPX-100 with following setting:\n";
                    cout << "Mode: " << argv[c] << endl;
                    modeSelector = 4;
                    setGPIO = &setTX70cm;
                }
                else if (mode == "help")
                {
                    cout << "Options for starting RPX-100: fm-tx \033[36mMODE CENTER-FREQUENCY GAIN TONE-FREQUENCY MODULATION-FACTOR DURATION SAMPLE-RATE\033[0m" << endl;
                    cout << endl;
                    cout << "\033[36mMODE\033[0m:" << endl;
                    cout << "     \033[32mRX\033[0m for receive mode" << endl;
                    cout << "     \033[31mTXwoBP\033[0m for transmit mode without bandpass filter" << endl;
                    cout << "     \033[31mTX6m\033[0m for transmit mode with bandpass filter for 50-54 MHz" << endl;
                    cout << "     \033[31mTX2m\033[0m for transmit mode with bandpass filter for 144-146 MHz" << endl;
                    cout << "     \033[31mTX70cm\033[0m for transmit mode with bandpass filter for 430-440 MHz" << endl;
                    cout << endl;
                    cout << "\033[36mCENTER-FREQUENCY\033[0m:" << endl;
                    cout << "     in Hz, number of type float" << endl;
                    cout << endl;
                    cout << "\033[36mGAIN\033[0m:" << endl;
                    cout << "     0 - 1.0, number of type float" << endl;
                    cout << endl;
                    cout << "\033[36mTONE-FREQUENCY\033[0m:" << endl;
                    cout << "     in Hz, number of type float" << endl;
                    cout << endl;
                    cout << "\033[36mMODULATION FACTOR\033[0m:" << endl;
                    cout << "     in Hz, number of type float" << endl;
                    cout << endl;
                    cout << "\033[36mDURATION\033[0m:" << endl;
                    cout << "     in sec, number of type int" << endl;
                    cout << endl;
                    cout << "\033[36mSAMPLE-RATE\033[0m:" << endl;
                    cout << "     in Hz, number of type int" << endl;
                    cout << endl;
                    return 0;
                }
                else
                {
                    cout << "Wrong settings, please type  \033[36m'fm-tx help'\033[0m to see all options !" << endl;
                    return 0;
                }
                break;

            case 2:
                cout << "Center Frequency: " << argv[c] << endl;
                centerFrequency = stof(argv[c]);
                break;

            case 3:
                cout << "Gain: " << argv[c] << endl;
                normalizedGain = atof(argv[c]);
                break;

            case 4:
                cout << "Tone Frequency: " << argv[c] << endl;
                toneFrequency = stof(argv[c]);
                break;

            case 5:
                cout << "Modulation Factor: " << argv[c] << endl;
                modFactor = stof(argv[c]);
                break;

            case 6:
                cout << "Duration: " << argv[c] << endl;
                duration = stoi(argv[c]);
                break;

            case 7:
                cout << "Sample Rate: " << argv[c] << endl;
                sampleRate = stof(argv[c]);
                break;
            }
        }
    }

    // pid_t pid, sid;
    // pid = fork();
    // if (pid < 0)
    // {
    //     return 1;
    // }
    // if (pid > 0)
    // {
    //     return 1;
    // }

    // umask(0);

    // sid = setsid();
    // if (sid < 0)
    // {
    //     return 1;
    // }

    // if ((chdir("/")) < 0)
    // {
    //     return 1;
    // }

    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);

    LogInit();
    Logger("RPX-100 was started succesfully with following settings:\n");
    msg.str("");
    msg << "Mode: " << mode << "\n";
    Logger(msg.str());
    msg.str("");
    msg << "Center Frequency: " << centerFrequency << "\n";
    Logger(msg.str());
    msg.str("");
    msg << "Gain: " << normalizedGain << "\n";
    Logger(msg.str());
    msg.str("");
    msg << "Tone Frequency: " << toneFrequency << "\n";
    Logger(msg.str());
    msg.str("");
    msg << "Modulation factor: " << modFactor << "\n";
    Logger(msg.str());
    msg.str("");
    msg << "Duration: " << duration << "\n";
    Logger(msg.str());
    msg.str("");
    msg << "Sample Rate: " << sampleRate << "\n";
    Logger(msg.str());

    // Find devices
    int n;
    lms_info_str_t list[8]; // should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0)
    {
        error(); // NULL can be passed to only get number of devices
    }
    msg.str("");
    msg << "Number of devices found: " << n << endl;
    Logger(msg.str()); // print number of devices
    if (n < 1)
    {
        return -1;
    }

    if (LMS_Open(&device, list[0], NULL)) // open the first device
    {
        error();
    }
    sleep(1);
    // Initialize device with default configuration
    if (LMS_Init(device) != 0)
    {
        error();
    }
    sleep(1);

    if (LMS_GPIORead(device, &readGPIO, 1) != 0)
    {
        error();
    }
    msg.str("");
    msg << "Read current GPIO state: " << (int)readGPIO << endl;
    Logger(msg.str());

    print_gpio(readGPIO);

    uint8_t gpio_dir = 0xFF;
    if (LMS_GPIODirWrite(device, &gpio_dir, 1) != 0)
    {
        error();
    }

    if (LMS_GPIODirRead(device, &readGPIO, 1) != 0)
    {
        error();
    }
    msg.str("");
    msg << "Set GPIOs direction to output." << endl;
    Logger(msg.str());
    print_gpio(readGPIO);

    msg.str("");
    msg << "setGPIO: " << (int)*setGPIO <<endl;
    Logger(msg.str());

    if (LMS_GPIOWrite(device, setGPIO, 1) != 0)
    {
        error();
    }
 
    if (LMS_GPIORead(device, &readGPIO, 1) != 0)
    {
        error();
    }
    msg.str("");
    msg << "GPIO Output: " << (int)readGPIO << endl;
    Logger(msg.str());
    print_gpio(readGPIO);

    msg.str("");
    msg << "LimeRFE set to " << mode << endl;
    Logger(msg.str());

    // Send single tone
    const int tx_time = (const int)duration;
    float f_ratio = toneFrequency / sampleRate;

    // Enable TX channel,Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, true) != 0)
        error();

    // Set sample rate
    if (LMS_SetSampleRate(device, sampleRate, 0) != 0)
        error();
    msg.str("");
    msg << "Sample rate: " << sampleRate / 1e6 << " MHz" << endl;
    Logger(msg.str());

    // Set center frequency
    if (LMS_SetLOFrequency(device, LMS_CH_TX, 0, centerFrequency) != 0)
        error();
    msg.str("");
    msg << "Center frequency: " << centerFrequency / 1e6 << " MHz" << endl;
    Logger(msg.str());

    // select Low TX path for LimeSDR mini --> TX port 2 (misslabed in MINI, correct in USB)
    if (LMS_SetAntenna(device, LMS_CH_TX, 0, LMS_PATH_TX2) != 0)
        error();

    // set TX gain
    if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, normalizedGain) != 0)
        error();

    // calibrate Tx, continue on failure
    LMS_Calibrate(device, LMS_CH_TX, 0, sampleRate, 0);

    // Streaming Setup

    lms_stream_t tx_stream;                        // stream structure
    tx_stream.channel = 0;                         // channel number
    tx_stream.fifoSize = 256 * 1024;               // fifo size in samples
    tx_stream.throughputVsLatency = 0.8;           // 0 min latency, 1 max throughput
    tx_stream.dataFmt = lms_stream_t::LMS_FMT_F32; // set dataformat for tx_stream to uint16 or floating point samples
    tx_stream.isTx = true;

    // modulator
    unsigned int num_samples = 1024;         // number of samples
    freqmod mod = freqmod_create(modFactor); // modulator

    // Initialize data buffers
    const int buffer_size = 1024;
    liquid_float_complex mod_buffer[buffer_size]; // TX buffer to hold complex values - liquid library)
    float test_tone[2 * buffer_size];

    msg.str("");
    msg << "Modulation Factor: " << modFactor << endl;
    Logger(msg.str());

    for (int t = 0; t < 2 * buffer_size; t++)
    {
        float w = 2 * M_PI * t * f_ratio;
        test_tone[t] = 5 * sin(w + 0.2);
    }
    freqmod_modulate_block(mod, test_tone, buffer_size, mod_buffer);

    const int send_cnt = int(buffer_size * f_ratio) / f_ratio;
    msg.str("");
    msg << "sample count per send call: " << send_cnt << std::endl;
    Logger(msg.str());

    // Streaming
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;
    LMS_SetupStream(device, &tx_stream);
    LMS_StartStream(&tx_stream);                                                 // Start streaming
    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(tx_time)) // run for 10 seconds
    {
        // Transmit samples
        int ret = LMS_SendStream(&tx_stream, mod_buffer, send_cnt, nullptr, 1000);
        if (ret != send_cnt)
        {
            msg.str("");
            msg << "error: samples sent: " << ret << "/" << send_cnt << endl;
            Logger(msg.str());
        }

        // Print data rate (once per second)
        if (chrono::high_resolution_clock::now() - t2 > chrono::seconds(5))
        {
            t2 = chrono::high_resolution_clock::now();
            lms_stream_status_t status;
            LMS_GetStreamStatus(&tx_stream, &status); // Get stream status
            msg.str("");
            msg << "TX data rate: " << status.linkRate / 1e6 << " MB/s\n"; // link data rate
            Logger(msg.str());
        }
    }

    sleep(1);
    // Stop streaming
    LMS_StopStream(&tx_stream);
    LMS_DestroyStream(device, &tx_stream);

    // Disable TX channel
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, false) != 0)
        error();

    setGPIO = &setRX;
    if (LMS_GPIOWrite(device, setGPIO, 1) != 0)
    {
        error();
    }
 
    if (LMS_GPIORead(device, &readGPIO, 1) != 0)
    {
        error();
    }
    msg.str("");
    msg << "GPIO Output: " << (int)readGPIO << endl;
    Logger(msg.str());
    print_gpio(readGPIO);

    // Close device
    if (LMS_Close(device) == 0)
    {
        msg.str("");
        msg << "Closed" << endl;
        Logger(msg.str());
    }

    return 0;
}

int error()
{
    msg.str("");
    msg << "ERROR: " << LMS_GetLastErrorMessage();
    Logger(msg.str());
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}

void print_gpio(uint8_t gpio_val)
{
    msg.str("");
    msg << endl;
    for (int i = 0; i < 8; i++)
    {
        bool set = gpio_val & (0x01 << i);
        msg << "GPIO" << i << ": " << (set ? "High" : "Low") << std::endl;
    }
    Logger(msg.str());
}
