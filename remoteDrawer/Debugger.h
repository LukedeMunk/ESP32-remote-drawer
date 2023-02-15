/*
 * File:        Debugger.h
 * Author:      Luke de Munk
 * Version:     1.0
 * 
 * Can be used to debug ESP32/Arduino projects using serial communication.
 * Don't forget to enable serial by 'Serial.begin(115200);'.
 * 
 */
#ifndef DEBUGGER_H
#define DEBUGGER_H

/* Enable or disable debug here */
#define DEBUG               1

#if DEBUG == 1
    #define debug(x) Serial.print(x)
    #define debugln(x) Serial.println(x)
#else
    #define debug(x)
    #define debugln(x)
#endif

#endif
