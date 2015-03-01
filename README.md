Micro Bluetooth Low Energy (mBLE)
=================================

This project was started for the hackthehouse.io hackathon in Berlin, Germany 
(February, 27 - March 1).

This Repo consits of the Linux bluetooth code, used to bridge events from
relayr.io mqtt event bus over bluetooth to the relayr wunderbar.

Code
----

The folders in this repo are structured to reflect the different steps to do,
to discover, connect and bridge the events.

### folder discovery

Code to find BLE devices like the relayr.io wunderbar.

### folder mble

mble is a pure linux/c implementation of gatt.
It handles a mqtt connection to the relayr mqtt broker which has a connection to
the cloud services of Bosch Siemens Home appliances (BSH).


