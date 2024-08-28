# The Celestial Compass
The Celestial Compass is a large scale project I hope to develop throughout the following years. It is based off of an idea I had of a device that would point swiveling arms towards various stars and planets in the sky. By tracking the location and orientation of the device, it is possible to use astronomical data to pin point the apparent position of almost anything in the sky. While the original idea for this project started as a table sized device, I am working on smaller versions of the device in order to prototype the mechanics before moving onto the full scale version. The current iteration is expected to be about 7-8 inches in diameter and is meant to be a sort of desk decoration device. This repository will host the code that will be used for all versions of the device as well as detail some of the design choices made along the way. At this point in time I do not plan to include the CAD files for any of the devices.

## The Mechanics
The central design of the celestial compass is achieved by rotating a pair of concentric rings that are able to provide two degrees of freedom to move a single point across a hemispherical area. (TODO: add image) These two degrees of freedom would be called φ and θ when using a spherical coordinate system, but in the context of astronomy these are called the azimuth and altitude respectively. ![spherical coordinate system](https://upload.wikimedia.org/wikipedia/commons/4/4f/3D_Spherical.svg)

By moving the bevel gear on the inside of the two arms, the gear attached to the outside arm will pivot, changing the altitude of the point. By moving both arms together, the azimuth of the point will be changed. The key here is that it is only the relative motion of the two arms that generate changes in the altitude. 

## The Code
This program is intended to be used on Arduino based microcontrollers, and the core structure is split into a heirarchy of classes. The curernt ones implemented are the **Motor**, **Set**, and **Observer** classes.

### Motor Class
The **Motor** class deals with the programming and control of an given motor. Control is given over the angular velocity of the motor as well as the desired position. By calling the **update()** function (ideally every loop), the motor will automatically know when to step in order to achieve the desired angle and velocity. Since the TC78H670FTG driver that this is based around is able to microstep the motor, there is an additional function that allows setting the step resolution during the runtime of the program.

### Set Class
The **Set** class (a.k.a. motor set) controls the relative motion of two gears, resulting in the movement of one object. Given a desired azimuth and altitude, the Set will govern its motors to move to the desired positions, taking into account the various gear ratios that are inbetween the stepper motor and its output. The Set class is also responsible for gathering the required azimuth and altitude of a specific object. When a motor set is given an object ID, it will periodically get that information based on the update frequency of the Set.

### Observer Class
The **Observer** class is meant to provide information to the various Sets, mainly azimuth and altitude calculations. Given a specific location and time, the observer is able to transform right ascension and declination coordinates of various stellar object in the celestial sphere.

## TO-DO
There is a lot left to do on this project, primarily on the hardware side but also on the code side.
- **Solar objects:** the observer needs to be able to keep track of objects within our solar system, which is a little more complicated than stars
- **Time and Location data:** right now, the data for the current time and location is just hardwired into the code, but a RTC and GPS module will be needed to update this in real time.
- **Magnetometer and magnetic declination:** because the orientation of the object can change, a magnetometer will be used to keep track of the compass' orientation with respect to north. Because we are interested in the location of true north rather than magnetic north, we need to adjust for this based on the magnetic declination of a given point. Due to the complexities involved with this, this might get turned into a gyroscope and a calibrate button.
- **Object lookup:** find out some way to efficiently store right ascension and declination of a bunch of stellar objects.
