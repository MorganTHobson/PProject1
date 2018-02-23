import matplotlib.pyplot as plt
import numpy as np

x=range(1,41)
values=np.random.uniform(size=40)

plt.stem(x, values)
plt.ylim(0, 1.2)

plt.title('Title')
plt.xlabel('X Values')
plt.ylabel('Y Values')

plt.show()
