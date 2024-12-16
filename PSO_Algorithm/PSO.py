import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import Filter as ft

# File path for the Excel file
csv_filename = 'PY_LOG/TEC1_Data.xlsx'

# Load data from the Excel file using pandas
data = pd.read_excel(csv_filename)

# Extract the columns you need
ElapseTime = data['Elapsed Time (s)'].values  # Replace with actual column name
ColdData   = 30.6 - data['Cold(C)'].values    # Replace with actual column name
DeltaT     = data['Hot(C)'].values - data['Cold(C)'].values    # Replace with actual column name

SimulTime = ElapseTime[21:2400] - 10.5
ColdTemp= ColdData[21:2400]

# System parameters
Ts = 0.5
Runtime = len(SimulTime)

p_num_s = np.array([1])
p_den_s = np.array([1, 1])

Gs = ft.GeneralOrderFilter(p_num_s,p_den_s,Ts)

def Objective_function(a, K):
    # system plant
    p_num = np.array([K])
    p_den = np.array([1, a])

    Plant = ft.GeneralOrderFilter(p_num, p_den, Ts)

    p_num_s = np.array([6.8])
    p_den_s = np.array([1, 10.02])

    Gs = ft.GeneralOrderFilter(p_num_s, p_den_s, Ts)

    R = np.ones(len(SimulTime))
    Y = np.zeros(len(SimulTime))
    Y_hat = np.zeros(len(SimulTime))
    E = np.zeros(len(SimulTime))

    costvalue = 0
    for k, t in enumerate(SimulTime):
        # Plant computation
        Y_hat[k] = Plant.compute(R[k])
        Y[k] = ColdTemp[k]
        # Y[k] = Gs.compute(R[k])
        E[k] = Y[k] - Y_hat[k]
        # Accumulate the ISE, ensure you don't go out of bounds
        # costvalue += E[k]*E[k]  
        costvalue +=  sum(E[max(k-9, 0):k+1])*sum(E[max(k-9, 0):k+1])
    print(f"Cost: {costvalue}, error: {E[-1]}")
    return costvalue

# Define the details of the objective function
nVar = 2  # a, K
ub = np.array([1, 1])  # Upper bounds of searching landscape
lb = np.array([0, 0])     # Lower bounds of searching landscape

# PSO parameters 
noP = 40  # Number of particles
maxIteration = 100  # Maximum iterations
wMax = 0.9  # Maximum inertia weight
wMin = 0.01  # Minimum inertia weight
c1 = 2  # Cognitive component
c2 = 2  # Social component

# Initialize particles
Swarm = {
    'Particles': [],
    'GBEST': {'X': np.zeros(nVar), 'O': np.inf}  # Global best
}

for _ in range(noP):
    particle = {
        'X': (ub - lb) * np.random.rand(nVar) + lb,
        'V': np.zeros(nVar),  # Random initial velocity (or small value)
        'PBEST': {'X': np.zeros(nVar), 'O': np.inf}  # Personal best
    }
    Swarm['Particles'].append(particle)

# Main loop 
cgCurve = []  # Convergence curve

for t in range(maxIteration):
    for k in range(noP):
        currentX = Swarm['Particles'][k]['X']
        currentO = Objective_function(currentX[0], currentX[1])
        
        # Update PBEST
        if currentO < Swarm['Particles'][k]['PBEST']['O']:
            Swarm['Particles'][k]['PBEST']['X'] = currentX.copy()
            Swarm['Particles'][k]['PBEST']['O'] = currentO
        
        # Update GBEST
        if currentO < Swarm['GBEST']['O']:
            Swarm['GBEST']['X'] = currentX.copy()
            Swarm['GBEST']['O'] = currentO
    
    # Update inertia weight
    w = wMax - t * ((wMax - wMin) / maxIteration)
    
    for k in range(noP):
        # Update velocity
        r1, r2 = np.random.rand(nVar), np.random.rand(nVar)  # Random factors
        Swarm['Particles'][k]['V'] = (
            w * Swarm['Particles'][k]['V'] +
            c1 * r1 * (Swarm['Particles'][k]['PBEST']['X'] - Swarm['Particles'][k]['X']) +
            c2 * r2 * (Swarm['GBEST']['X'] - Swarm['Particles'][k]['X'])
        )
        
        # Update position
        Swarm['Particles'][k]['X'] += Swarm['Particles'][k]['V']
        
        # Clip position to bounds
        Swarm['Particles'][k]['X'] = np.clip(Swarm['Particles'][k]['X'], lb, ub)
    
    if t % 10 == 0:
        print(Swarm['GBEST']['O'], Swarm['GBEST']['X'])  # Print every 10 iterations
    cgCurve.append(Swarm['GBEST']['O'])  # Store convergence curve value

print(Swarm['GBEST'])

# Plot convergence curve
plt.semilogy(cgCurve)
plt.title('Convergence Curve')
plt.xlabel('Iteration')
plt.ylabel('Objective Function Value')
plt.grid()
plt.show()
