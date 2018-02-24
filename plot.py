import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

df = pd.read_csv("serial-data.csv", header=0)

val = dict()

for index, row in df.iterrows():
  if not row['filter length'] in val:
    val[row['filter length']] = [float(row['sec']) + float(row['us'])/1000000]
  else:
    val[row['filter length']].append(float(row['sec']) + float(row['us'])/1000000)

x = []
y = []

for key,value in val.iteritems():
  x.append(int(key))
  y.append(512*512*256 * int(key) / np.median(value))

plt.stem(x,y)

plt.title('Normalized Serial Data First')
plt.xlabel('Filter Length')
plt.ylabel('Filter Length * Data Length / Time (op/sec)')

#plt.xscale('log')

plt.show()
