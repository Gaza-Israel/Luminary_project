#include "median_filter.h"

#include <Arduino.h>
#include <string.h>

int median(int arr[], int start, int n) {
  int numBytes = sizeof(int) * n;
  int a[n];
  memcpy(a, arr + start, numBytes);
  int m;

  quickSort(a, 0, n - 1);
  if (n % 2 == 0)
    m = (a[n / 2 - 1] + a[n / 2]) / 2.0;
  else
    m = a[n / 2];
  return m;
}

int partition(int arr[], int start, int end) {
  int pivot = arr[start];

  int buff = 0;
  int count = 0;
  for (int i = start + 1; i <= end; i++) {
    if (arr[i] <= pivot)
      count++;
  }

  // Giving pivot element its correct position
  int pivotIndex = start + count;
  //   swap (arr[pivotIndex], arr[start]);
  buff = arr[pivotIndex];
  arr[pivotIndex] = arr[start];
  arr[start] = buff;

  // Sorting left and right parts of the pivot element
  int i = start, j = end;

  while (i < pivotIndex && j > pivotIndex) {
    while (arr[i] <= pivot) {
      i++;
    }

    while (arr[j] > pivot) {
      j--;
    }

    if (i < pivotIndex && j > pivotIndex) {
      buff = arr[i];
      arr[i] = arr[j];
      arr[j] = buff;
      i++;
      j--;
      //    swap (arr[i++], arr[j--]);
    }
  }

  return pivotIndex;
}
void quickSort(int arr[], int start, int end) {
  // base case
  if (start >= end)
    return;

  // partitioning the array
  int p = partition(arr, start, end);

  // Sorting the left part
  quickSort(arr, start, p - 1);

  // Sorting the right part
  quickSort(arr, p + 1, end);
}
void quickSort_uint8(uint8_t arr[], int start, int end) {
  // base case
  if (start >= end)
    return;

  // partitioning the array
  uint8_t p = partition_uint8(arr, start, end);

  // Sorting the left part
  quickSort_uint8(arr, start, p - 1);

  // Sorting the right part
  quickSort_uint8(arr, p + 1, end);
}
uint8_t partition_uint8(uint8_t arr[], int start, int end) {
  uint8_t pivot = arr[start];

  int buff = 0;
  int count = 0;
  for (int i = start + 1; i <= end; i++) {
    if (arr[i] <= pivot)
      count++;
  }

  // Giving pivot element its correct position
  int pivotIndex = start + count;
  //   swap (arr[pivotIndex], arr[start]);
  buff = arr[pivotIndex];
  arr[pivotIndex] = arr[start];
  arr[start] = buff;

  // Sorting left and right parts of the pivot element
  int i = start, j = end;

  while (i < pivotIndex && j > pivotIndex) {
    while (arr[i] <= pivot) {
      i++;
    }

    while (arr[j] > pivot) {
      j--;
    }

    if (i < pivotIndex && j > pivotIndex) {
      buff = arr[i];
      arr[i] = arr[j];
      arr[j] = buff;
      i++;
      j--;
      //    swap (arr[i++], arr[j--]);
    }
  }

  return pivotIndex;
}