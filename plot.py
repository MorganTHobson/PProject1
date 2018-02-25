import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

sdf = pd.read_csv("serial-data.csv", header=0)
pdf = pd.read_csv("parallel-data-unrolled.csv", header=0)

sval = dict()
pval = dict()

for index, row in sdf.iterrows():
  if not row['filter length'] in sval:
    sval[row['filter length']] = [float(row['sec']) + float(row['us'])/1000000]
  else:
    sval[row['filter length']].append(float(row['sec']) + float(row['us'])/1000000)

serial_time = np.median(sval[512])

for index, row in pdf.iterrows():
  if not row['unroll'] in pval:
    pval[row['unroll']] = [float(row['sec']) + float(row['us'])/1000000]
  else:
    pval[row['unroll']].append(float(row['sec']) + float(row['us'])/1000000)

x = []
y = []

for key,value in pval.iteritems():
  speedup = serial_time/np.median(value)
  x.append(int(key))
  y.append(speedup)
  p = (8/speedup - 8)/(-7)
  print("P({}) = {}".format(key,p))

plt.stem(x,y)

plt.title('Parallel Data First Unrolled Speedup (log)')
plt.xlabel('Unroll Level')
plt.ylabel('Serial Time / Parallel Time (Unrolled)')

plt.xscale('log')

plt.show()
