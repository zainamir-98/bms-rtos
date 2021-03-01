/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

// Function prototypes
void write_log(char*);
void vBaseTaskFunction(void*);
void vMiscTaskFunction(void*);
void vBrakeTaskFunction(void*);

// Constants
#define TEMP_STABLE 0
#define TEMP_OVER 1
#define TEMP_UNDER 2
#define PRESS_STABLE 3
#define PRESS_OVER 4
#define PRESS_UNDER 5

// Semaphore mutex to protect shared write function
SemaphoreHandle_t xLogMutex = NULL;

// Queue for inter-task communication
QueueHandle_t xSensorQueue = NULL;

// A shared write function protected by mutual exclusion
void write_log(char* text) {
	static log_count = 0;
	printf("[LOG ID: %d] %s", log_count++, text);
}

// Check brake temperature periodically (T = 2.5 seconds)
void vCheckBrakeTempFunction(void* pvParameters) {
	BaseType_t xQueueStatus;
	int piState = TEMP_STABLE;
	const TickType_t xDelay = pdMS_TO_TICKS(2500);

	for (;;) {
		// Checking temperature from sensor
		xQueueStatus = xQueueSend(xSensorQueue, &piState, 0);
		if (xQueueStatus == pdPASS)
		{
			xSemaphoreTake(xLogMutex, portMAX_DELAY);
			write_log("TEMP_STABLE sent to queue\n");
			fflush(stdout);
			xSemaphoreGive(xLogMutex);
		}
		else printf("[ERROR] Could not send to queue!");
		
		vTaskDelay(xDelay);
	}
}

// Check brake fluid pressure periodically (T = 2.5 seconds)
void vCheckBrakeFluidPressureFunction(void* pvParameters) {
	BaseType_t xQueueStatus;
	int piState = PRESS_STABLE;
	const TickType_t xDelay = pdMS_TO_TICKS(2500);

	for (;;) {
		xQueueStatus = xQueueSend(xSensorQueue, &piState, 0);
		if (xQueueStatus == pdPASS)
		{
			xSemaphoreTake(xLogMutex, portMAX_DELAY);
			write_log("PRESS_STABLE sent to queue\n");
			fflush(stdout);
			xSemaphoreGive(xLogMutex);
		}
		else printf("[ERROR] Could not send to queue!");
		vTaskDelay(xDelay);
	}
}

// Checks system conditions by reading sensor queue periodically (T = 0.5 seconds)
void vBaseTaskFunction(void* pvParameters) {
	BaseType_t xQueueStatus;
	int xSensorState;
	const TickType_t xDelay = pdMS_TO_TICKS(500);
	char* condition = "STABLE";
	char text[50];

	for (;;) {
		xQueueStatus = xQueueReceive(xSensorQueue, &xSensorState, portMAX_DELAY);
		if (xQueueStatus == pdPASS)
		{
			if (xSensorState == TEMP_STABLE) {
				sprintf(text, "Condition: TEMP_STABLE\n");
			}
			else if (xSensorState == PRESS_STABLE) {
				sprintf(text, "Condition: PRESS_STABLE\n");
			}
			xSemaphoreTake(xLogMutex, portMAX_DELAY);
			write_log(text);
			fflush(stdout);
			xSemaphoreGive(xLogMutex);
			vTaskDelay(xDelay);
		}
		else printf("Could not receive from sensor queue!\n");
	}
}

// Highest-priority task that performs braking (is unblocked periodically after every 6 seconds)
void vBrakeTaskFunction(void* pvParameters) {
	const TickType_t xDelay = pdMS_TO_TICKS(6000);

	for (;;) {
		printf("\nObstacle ahead! Initiating brakes...\n\n");

		for (long long int i = 0; i < 500000000; i++) {
			// Do nothing
		}

		xSemaphoreTake(xLogMutex, portMAX_DELAY);
		write_log("Applied brakes successfully.\n");
		fflush(stdout);
		xSemaphoreGive(xLogMutex);

		vTaskDelay(xDelay);
	}
}

void main(void)
{
	printf("Braking Management System (FreeRTOS Simulation)\n\n");
	xLogMutex = xSemaphoreCreateMutex();
	xSensorQueue = xQueueCreate(2, sizeof(int));

	if (xLogMutex != NULL && xSensorQueue != NULL) {
		xTaskCreate(vBaseTaskFunction, "Base Task", 1000, NULL, 1, NULL);
		xTaskCreate(vCheckBrakeTempFunction, "Temperatre Task", 1000, NULL, 2, NULL);
		xTaskCreate(vCheckBrakeFluidPressureFunction, "Pressure Task", 1000, NULL, 2, NULL);
		xTaskCreate(vBrakeTaskFunction, "Brake Task", 1000, NULL, 3, NULL);
		vTaskStartScheduler();
	}

	for (;; );
}

