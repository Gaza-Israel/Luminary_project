#ifndef MEDIAN_FILTER_h
#define MEDIAN_FILTER_h
#include <Arduino.h>

int median(int arr[], int start, int n);
int partition(int arr[], int start, int end);
void quickSort(int arr[], int start, int end);
void quickSort_uint8(uint8_t arr[], int start, int end);
uint8_t partition_uint8(uint8_t arr[], int start, int end);
#endif