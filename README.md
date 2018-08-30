# PISTON HAND
The detail of the project can be found at <http://protopaja.aalto.fi/protopaja-2018/futurice-2018/>. 

A robot hand with three finger for grasping objects.
The hand includes two pressure sensors on two of the fingers, 
an infrared sensor on the other finger and an endstop at the top 
of the cylinder. In the fresh start up, the piston runs to the top 
and reaches the endstop for calibration pos = 0. Then, it moves to 
an idle position which is at a predefined offset distance from the top p_off. 

If an object presents, the piston runs so that the fingers tend to 
grasp the object. If the object disappear, the piston releases the 
fingers to the offset position p_off. When the fingers already 
grasped an object, the motor stops. In order to release the finger, 
a button needs to be pressed. If the button is pressed more than one second,
the hand will try to apply as much pressure as it can onto the object.

As a result, the hand could handle an eggshell without breaking it.

## Technical details
* The bipolar stepper motors [SM-42BYG011-25](https://www.sparkfun.com/datasheets/Robotics/SM-42BYG011-25.pdf "Mercury motor") 
and [NEMA17-13-04 SD-amt112s](https://www.cui.com/product/resource/nema17-amt112s.pdf)
 can provide a torque of approximatly 0.3 Nm.
* The pressure sensor 
[FSR402](https://www.trossenrobotics.com/productdocs/2010-10-26-DataSheet-FSR402-Layout2.pdf "Pressure sensor datasheet")
 can detect a pressure between 0.5 and 10 N.
* Infrared proximity sensor Polulu [GP2Y0D805Z0F](https://www.pololu.com/product/1132 "Infrared proximity sensor specifications")
 can sense an object within 0.5 and 5 cm range.
