import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

pdf = pd.read_csv("parallel-data.csv", header=0)
pff = pd.read_csv("parallel-filter.csv", header=0)

pdval = dict()
pfval = dict()



for index, row in pdf.iterrows():
  if not row['threads'] in pdval:
    pdval[row['threads']] = [float(row['sec']) + float(row['us'])/1000000]
  else:
    pdval[row['threads']].append(float(row['sec']) + float(row['us'])/1000000)

for index, row in pff.iterrows():
  if not row['threads'] in pfval:
    pfval[row['threads']] = [float(row['sec']) + float(row['us'])/1000000]
  else:
    pfval[row['threads']].append(float(row['sec']) + float(row['us'])/1000000)

x = []
y = []

for t in [1,2,4,8,16]:
  x.append(t)
  y.append(np.median(pfval[t]) - np.median(pdval[t]))


plt.stem(x,y)

plt.title('Parallel Comparison (log)')
plt.xlabel('Threads')
plt.ylabel('Filter First time - Data First Time (sec)')

plt.xscale('log')

plt.show()
