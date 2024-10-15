import matplotlib.pyplot as plt 
import numpy as np
data = np.loadtxt('output.txt')
plt.plot(data)
plt.show()