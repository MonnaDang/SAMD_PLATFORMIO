import numpy as np
import matplotlib.pyplot as plt
import pandas as pd  # Use pandas for reading .xlsx files
import Filter as ft

# File path for the Excel file
csv_filename = 'PY_LOG/TEC1_Data.xlsx'

# Load data from the Excel file using pandas
data = pd.read_excel(csv_filename)

# Extract the columns you need
ElapseTime = data['Elapsed Time (s)'].values  # Replace with actual column name
ColdData   = 30.6 - data['Cold(C)'].values    # Replace with actual column name
DeltaT     = data['Cold(C)'].values - data['Cold(C)'].values    # Replace with actual column name

SimulTime = ElapseTime[21:2400] - 10.5
ColdTemp= ColdData[21:2400]

# System plant
Ts = 0.5  # Sampling time

# Define transfer function coefficients
p_num_s = np.array([1])
p_den_s = np.array([1, 1])

Gs = ft.GeneralOrderFilter(p_num_s,p_den_s,Ts)

# E[k]*E[k]
# p_num_hat = np.array([0.78242539])
# p_den_hat = np.array([1, 0.02081427])

# abs(sum(E[max(k-9, 0):k+1]))  
p_num_hat = np.array([0.68160843])
p_den_hat = np.array([1, 0.01800801])

# sum(E[max(k-9, 0):k+1]) * sum(E[max(k-9, 0):k+1])
# p_num_hat = np.array([0.78275379])
# p_den_hat = np.array([1, 0.02082447])


Gs_hat = ft.GeneralOrderFilter(p_num_hat,p_den_hat,Ts)

Y_filter = np.zeros(len(SimulTime))
Y_hat = np.zeros(len(SimulTime))
R = np.ones(len(SimulTime))

for k, t in enumerate(SimulTime):
    Y_filter[k] = Gs.compute(ColdTemp[k])
    Y_hat[k] = Gs_hat.compute(R[k])

# Plot the data
plt.figure(figsize=(12, 6))
plt.plot(SimulTime, ColdTemp, label="Cold Temp", linestyle="--", color="Red")
plt.plot(SimulTime, Y_hat, label="Y hat", linestyle="--", color="Green")
plt.plot(SimulTime, Y_filter, label="Filter Temp", linestyle="--", color="Blue")


plt.title("Cold Temp response")
plt.xlabel("Time (s)")
plt.ylabel("Temp")
plt.legend()
plt.grid()
plt.show()
