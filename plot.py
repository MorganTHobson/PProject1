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
  y.append(np.median(value))

plt.stem(x,y)

plt.title('Title')
plt.xlabel('X Values')
plt.ylabel('Y Values')

plt.xscale('log')

plt.show()
