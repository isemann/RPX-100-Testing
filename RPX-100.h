/******************************************************************************
 * C++ source of RPX-100S
 *
 * File:   RPX-100.h
 * Author: Bernhard Isemann
 *
 * Created on 19 Sep 2021, 12:37
 * Updated on 19 Sep 2021, 17:00
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
#include "alsa/asoundlib.h"
#include "liquid/liquid.h"
#include "ServerSocket.h"
#include "SocketException.h"
#include <iterator>
#pragma once

#define NUM_THREADS 5 // max number of main threads
pthread_mutex_t SDRmutex;

// SDR facility

int SDRinit(double frequency, double sampleRate, int modeSelector, double normalizedGain);
int SDRfrequency(lms_device_t *device, double frequency);
void *startSocketServer(void *threadID);
void *startSDRStream(void *threadID);
void *startSocketConnect(void *threadID);
void *startWebSocket(void *threadID);
int error();

// Log facility
void print_gpio(uint8_t gpio_val);
std::stringstream msg;
std::stringstream HEXmsg;

double frequency;
double sampleRate;
int modeSelector;
double normalizedGain;
string mode = "RX";
int modeSel = 0;