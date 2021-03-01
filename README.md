# Braking Management System using FreeRTOS

This is a simulation of a braking system implemented using FreeRTOS. The tasks are as shown:
* Base Task: Lowest-priority. Checks the brake temperature and fluid pressure by reading from a 2-item queue after every 500 ms.
* Temperature Task: Medium-priority. Reads the brake temperature and sends the reading to the queue after every 2500 ms.
* Pressure Task: Medium-priority. Reads the brake fluid pressure and sends the reading to the queue after every 2500 ms.
* Brake Task: Highest-priority. Runs after every 6000 ms to simulate braking. All other tasks are pre-empted.
