# What's this?

An Arduino sketch that automatically presses the "resume" button on Toyotas when it detects that cruise control is ready to go.

# How to use this?
Connect the base of a NPN transistor (s8050?) using a current limiting resistor (1-10k) to the assigned GPIO pin on the arduino (modify the sketch as needed, try not to use the digitalWrite() function). Connect the collector of the transistor to a 230 ohms resistor and connect it to the output pin of the cruise control lever, connect the emitter to ground.

YOU NEED TO USE THE INTERRUPT PIN ON THE MCP2515.

You can connect the arduino to either the ADAS CAN (Driving Support ECU/Camera) or the Airbag/Steering Sensor CAN (Airbag ECU, Steering Sensor ECU).

No support will be provided and you will use this at your own risk.