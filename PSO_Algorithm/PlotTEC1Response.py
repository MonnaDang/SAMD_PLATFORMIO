import numpy as np
import matplotlib.pyplot as plt
import pandas as pd  # Use pandas for reading .xlsx files
import Filter as ft

# File path for the Excel file
csv_filename = 'PY_LOG\TEC1_Response.xlsx'

# Load data from the Excel file using pandas
data = pd.read_excel(csv_filename)

# Extract the columns you need
ElapseTime = data['Elapsed Time (s)'].values  # Replace with actual column name
ColdData   = data['Cold(C)'].values    # Replace with actual column name
Tset   = data['Tset(C)'].values    # Replace with actual column name


plt.figure(figsize=(12, 6))
plt.plot(ElapseTime, ColdData, label="Cold Temp", color="Blue")
plt.plot(ElapseTime, Tset, label="Set Temp", color="Red")
plt.title("Cold Temp response")
plt.xlabel("Time (s)")
plt.ylabel("Temp")
plt.legend()
plt.grid()
plt.show()