An Arduino library for connecting to Pixels Dice over Bluetooth Low Energy

This is an unofficial library for connecting to the Pixels Dice. It is only tested with the ESP32. See <https://github.com/GameWithPixels> for official Pixels Dice libraries.

The code for this was originally adapted from <https://gist.github.com/JpEncausse/cb1dbcca156784ac1e0804243da8e481>.

The interface is targeting the protocol specified in <https://github.com/GameWithPixels/.github/blob/main/doc/CommunicationsProtocol.md>

This interface is abstracts away the asynchronous events to allow the dice to be handled with a simple event loop. The functions handle interacting with the BLE services and allow the data to be handled by polling.

This has the following trade offs:
1. Events are queued and don't trigger immediate callbacks.
2. This requires more data copying and allocations then might be needed otherwise.
3. This wastes a lot of power/CPU compared to an interface that exposes more of the BLE search details.

The goal here is simplicity, hiding as much of the implementation details as possible.

I'm  not particularly knowledgeable on the details of the BLE stack, so some things are probably being done wrong/inefficiently.

# Finding / Connecting to Dice
This interface doesn't do anything until `ScanForDice` is started. This functions starts a FreeRTOS task in the background that manages searching for new dice and connecting to them. My understanding is that the connect, functionality blocks while a search is running, so the dice that are found are only connected to when the search period is complete. These scans can be stopped to save power or if you don't want to find new dice.

By default each dice found is connected to, but by setting `auto_connect` to false, connecting to the die can be handled manually with the `ConnectDie` function.

Once a die is found, it remains forever in the dice list and will be accessed by it's `PixelsDieID`. It's description can be accessed through `GetDieDescription` even if it's disconnected.

# Handling Events
Events are read by polling the `GetDieRollUpdates` and `GetDieBatteryUpdates` functions.

These return all the events sent out by the connected die since the last call.

These queues continue to grow if not read, so they should be polled regularly. There is a cutoff of 100 values currently.

# TODO
1. Implement any useful missing features of the interface.
2. Improve battery reporting. It seems to jump around, and probably doesn't need to be event driven.
3. Add additional configuration to allow changing defaults (like BLE scan parameters).
4. Add CI/testing/documentation 
