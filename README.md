# CarDash

carDash software based on Qt5

This project allows you to create your own car dash and connect it to your car via OBDII adapter.
User your own xml config for you car model that you can find on internet, or edit a default one to add commands.
The project is written using QT5 framework, QML is used for UI.

## Usage
Build the project normally using QtCreator or command line. Once built, you get a binary that you can run on your hardware.
Run it the first time:

    $./CarDash /path/to/your/car_config.xml /dev/ttyUSB0
where  */dev/ttyUSB0* is the port that is used by your OBDII adapter
CarDash will save the port you use, so you can skip the port next time.

##Editing xml config for your car
The default config looks like this:


    ?xml version="1.0" encoding="utf-8"?>
    <OBD2>
      <init>
        <command send="atl0"></command>
        <command send="ate0"></command>
        <command send="atal"></command>
        <command send="atsp0"></command>
      </init>

      <rotation>
        <command send="010C" replyLength="2" skipCount="0" targetId="37" conversion="V/4"></command>
        <command send="010D" replyLength="1" skipCount="0" targetId="81" units="km/h"></command>
        <command send="012F" replyLength="2" skipCount="100" targetId="170" conversion="100 * V / 255"></command>
        <command send="0105" replyLength="2" skipCount="10" targetId="14" conversion="(V - 40) / 130.0 * 100"></command>
        <command send="0146" replyLength="2" skipCount="10" targetId="173" conversion="V - 40"></command>
        <command send="01A4" replyLength="1" skipCount="10" targetId="139"></command>
      </rotation>
    </OBD2>
In the *init* section there are commands to init OBD|| adapter. You mostlikely won't need to change those.
In the *rotation* section there are command that are sent to OBD|| adapter to get information about the car speed, RPM, oil temperature, etc. These command can be different for each car model, you can find those on internet.
Command consists of several fields:
- send - OBD|| parameter id,  which is meaningful to your car, for example 010C means RPM (see [Wikipedia](https://ru.wikipedia.org/wiki/OBD-II_PIDs "Wikipedia"))
- replyLength - number of bytes in reply from adapter
- skipCount - how often to skip the command, which is basically how often you want to send it. The bigger, the more seldom command is sent
- targetId - Identification of the command that is used in CarDash project. See [*cardash_target_identifiers.txt*](config/cardash_target_identifiers.txt)
- conversion - the formulae to convert the bits received from a car to human readable value

##How to customize UI
###Logo
To customize logo replace *resources/logo.gif*  with your own.
If you want to use PNG file format, you will also need to change *../resources/logo.gif* to  *../resources/logo.png* in [*qml/main.qml*](qml/main.qml) file (search for *splashScreenLogo*) component.

    AnimatedImage {
            id: splashScreenLogo

            anchors.centerIn: parent
            height: parent.height / 2
            source: "../resources/logo.png"
            fillMode: Image.PreserveAspectFit
    }

## How to add support for more car parameters
Check [*qml/Hardware/Hardware.qml*](qml/Hardware/Hardware.qml) to see what parameters you can add. There is a list fo them, with default values. For example:

    property real speed: 0
    property real rpm: 0
    property bool rightRearDoorOpen: false
    property bool engineProblem: false
    property bool batteryProblem: false

You need to add processing for the parameters you want to add. You can see how it is done in the same *Hardware.qml* file below:

    switch (targetId) {
                  case "81": speed = value; break;
                  case "37": rpm = value; break;

*targetId* matches the one in [*cardash_target_identifiers.txt*](config/cardash_target_identifiers.txt) and [*config/obd2.xml*](config/obd2.xml). Add a new *case* string with targetId you want to add, add a command string to [*config/obd2.xml*](config/obd2.xml) with the same *targetId* and other values.
