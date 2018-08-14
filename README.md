# PISTON HAND
A robot hand with three finger for grasping objects.
The hand includes two pressure sensors on two of the fingers, an infrared sensor on the other finger and an endstop at the top of the cylinder. In the fresh start up, the piston runs to the top and reachs the endstop for calibration pos = 0. Then, it moves to an idle position which is at a predefined offset distance from the top p_off. 

If an object presents, the piston runs so that the fingers tend to grasp the object. If the object disappear, the piston releases the fingers to the offset position p_off. When the fingers already grasped an object, the motor stops. In order to release the finger, a button needs to be pressed.
