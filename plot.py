import matplotlib.pyplot as plt
import numpy as np
import serial
import time

data_x = []
data_y = []
data_z = []
data_tilt = []
t = np.linspace(0,9.9,99)

serdev = '/dev/ttyACM0'

s = serial.Serial(serdev,115200)

for i in range(0, 99):
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    data_x.append(float(line))
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    data_y.append(float(line))
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    data_z.append(float(line))
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    data_tilt.append(float(line))
    print(i)
    print('X:%f Y:%f Z:%f tilt:%d' % (data_x[i],data_y[i],data_z[i],data_tilt[i]))

plt.subplot(2,1,1)
plt.plot(t, data_x, 'r',label = 'X') # plotting t, a separately 
plt.plot(t, data_y, 'b',label = 'Y') # plotting t, b separately 
plt.plot(t, data_z, 'g',label = 'Z') # plotting t, c separately
plt.legend(loc = 'upper right')
plt.subplot(2,1,2)
plt.stem(t, data_tilt, 'b') # plotting t, c separately
plt.show()
"""


"""