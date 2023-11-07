# Smart Home Project using FreeRTOS and STM32F103C8 (ARM M3)
<br>This Smart Home project is built for the STM32F103C8 microcontroller using FreeRTOS to control doors, fans, and windows. It employs various FreeRTOS elements such as Tasks, Queues, Semaphores, and Event Flags for efficient communication and synchronization among different functionalities.

<br> Project Details
<br> Functionalities
This project is structured around five tasks:

### Password Task

<br>Highest priority (osPriorityNormal)
<br>Monitors the password queue for received data with port max delay.
<br>Validates the correctness of the received password.
<br>Suspends two tasks (Keypad Task, Password Task) if the password is correct.
<br>Writes to LCD by requesting the semaphore.
### Keypad Task

<br>Priority: osPriorityLow1
<br>Checks for inputs from the keypad.
<br>Sends data to the front of the password queue with no task delay, directly sending to the Password Task if there's received data.
### Temperature Task

Priority: osPriorityLow
Initiates the ADC, interrupt-based, to convert temperature data.
Upon data conversion, sends the temperature information to the Temperature Queue via a callback function.
Adjusts fan speed based on the received temperature data.
Acquires the LCD semaphore to display the temperature on the LCD.
###Gas Task

Priority: osPriorityLow
Monitors gas levels.
If the gas level exceeds 200, triggers an event group bit, prompting the Window Task to open the window.
Window Task

###Event-based task.
Waits for the event triggered by the Gas Task.
Upon event trigger, acquires the semaphore to write a message to the LCD indicating gas leakage and the action of opening the window.
##Utilized Resources
###Semaphore
Utilized by the LCD for task coordination and to ensure one task writes to the LCD at a time.
###Queues

<br>Password Queue: Transfers password-related data between the Password Task and the Keypad Task.
<br>Temperature Queue: Receives and processes temperature data from the Temperature Task.
<br>Gas Queue: Monitors gas levels received from the Gas Task.
###Event

Utilized by the Window Task to respond to a gas level event, indicating the need to open the window.
##Conclusion
This Smart Home project effectively employs FreeRTOS, making use of various tasks, queues, semaphores, and event flags to facilitate the control of doors, fans, and windows based on different environmental conditions, enhancing the automation and security aspects of a smart home setup.




